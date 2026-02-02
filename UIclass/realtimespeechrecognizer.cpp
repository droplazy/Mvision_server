#include "realtimespeechrecognizer.h"
#include <QCoreApplication>
#include <QFile>
#include <QJsonDocument>
#include <QMessageAuthenticationCode>
#include <QCryptographicHash>
#include <QUrl>
#include <QUrlQuery>
#include <QDebug>
#include <QDateTime>
#include <QJsonArray>

RealtimeSpeechRecognizer::RealtimeSpeechRecognizer(QObject *parent)
    : QObject(parent)
    , m_webSocket(new QWebSocket())
    , m_ffmpegProcess(new QProcess(this))
    , m_sendTimer(new QTimer(this))
     , m_timeoutTimer(new QTimer(this))  // 新增
    , m_isRecognizing(false)
    , m_hasSentStartFrame(false)
    , m_frameSize(1280)  // 16000Hz * 40ms * 1channel * 2bytes / 1000
{
    connect(m_webSocket, &QWebSocket::connected,
            this, &RealtimeSpeechRecognizer::onWebSocketConnected);
    connect(m_webSocket, &QWebSocket::disconnected,
            this, &RealtimeSpeechRecognizer::onWebSocketDisconnected);
    connect(m_webSocket, &QWebSocket::textMessageReceived,
            this, &RealtimeSpeechRecognizer::onWebSocketTextMessageReceived);

    connect(m_ffmpegProcess, &QProcess::readyRead,
            this, &RealtimeSpeechRecognizer::onFFmpegReadyRead);

    m_sendTimer->setInterval(40);  // 40ms发送一帧
    connect(m_sendTimer, &QTimer::timeout,
            this, &RealtimeSpeechRecognizer::onSendTimerTimeout);

    // 配置超时计时器（30秒）
    m_timeoutTimer->setInterval(60000);  // 30秒
    m_timeoutTimer->setSingleShot(true);  // 单次触发
    connect(m_timeoutTimer, &QTimer::timeout,
            this, &RealtimeSpeechRecognizer::onTimeout);
}

RealtimeSpeechRecognizer::~RealtimeSpeechRecognizer()
{
    // 停止所有计时器
    if (m_timeoutTimer) {
        m_timeoutTimer->stop();
        delete m_timeoutTimer;
    }

    stopRecognition();
}
void RealtimeSpeechRecognizer::onTimeout()
{
    if (!m_isRecognizing) return;

    qDebug() << "⏰ 识别超时（30秒），准备结束会话";
    emit statusMessage("识别超时，正在结束...");

    // 1. 先停止发送音频数据
    stopSendingAudio();

    // 2. 等待100ms，确保最后几帧音频发送完成
    QTimer::singleShot(100, this, [this]() {
        if (!m_isRecognizing) return;

        qDebug() << "📤 发送超时结束帧";

        // 3. 发送带音频数据的结束帧（最后一个音频包）
        if (m_audioBuffer.size() > 0) {
            // 如果有剩余音频数据，发送带音频的结束帧
            QByteArray lastFrame = m_audioBuffer.left(qMin(m_frameSize, m_audioBuffer.size()));
            sendAudioFrameForend(lastFrame);
            m_audioBuffer.clear();
        } else {
            // 如果没有音频数据，发送空的结束帧
            sendEndFrame();
        }

        // 4. 等待服务器响应，5秒后强制关闭
        QTimer::singleShot(5000, this, [this]() {
            if (m_isRecognizing) {
                qDebug() << "⏰ 超时后未收到服务器响应，强制结束";
                emit sessionCompleted();
                stopRecognition();
            }
        });
    });
}
void RealtimeSpeechRecognizer::setConfig(const Config &config)
{
    m_config = config;
}
// 新增：停止发送音频函数
void RealtimeSpeechRecognizer::stopSendingAudio()
{
    if (m_sendTimer->isActive()) {
        m_sendTimer->stop();
        qDebug() << "⏹️ 停止发送音频定时器";
    }

    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
        qDebug() << "⏹️ 停止超时计时器";
    }
}
bool RealtimeSpeechRecognizer::startRecognition(const QString &rtspUrl)
{
    // qDebug() << "=== startRecognition 开始 ===";
    // qDebug() << "RTSP URL:" << rtspUrl;
    // qDebug() << "FFmpeg路径:" << m_config.ffmpegPath;
    // qDebug() << "API Key长度:" << m_config.apiKey.length();
    // qDebug() << "API Secret长度:" << m_config.apiSecret.length();

    m_lastRtspUrl = rtspUrl;  // 保存URL
    m_reconnectCount = 0;     // 重置重连计数

    if (m_isRecognizing) {
        qDebug() << "错误: 已经在识别中";
        emit errorOccurred("已经在识别中");
        return false;
    }

    if (m_config.apiKey.isEmpty() || m_config.apiSecret.isEmpty()) {
        qDebug() << "错误: API凭证未设置";
        qDebug() << "API Key为空:" << m_config.apiKey.isEmpty();
        qDebug() << "API Secret为空:" << m_config.apiSecret.isEmpty();
        emit errorOccurred("API凭证未设置");
        return false;
    }

    // 检查ffmpeg文件是否存在
  //  qDebug() << "检查FFmpeg文件:" << m_config.ffmpegPath;
    bool ffmpegExists = QFile::exists(m_config.ffmpegPath);
 //   qDebug() << "FFmpeg文件存在:" << ffmpegExists;

    if (!ffmpegExists) {
        qDebug() << "错误: FFmpeg不存在";
        emit errorOccurred("FFmpeg不存在: " + m_config.ffmpegPath);
        return false;
    }

    m_isRecognizing = true;
    m_hasSentStartFrame = false;
    m_audioBuffer.clear();

    // 1. 启动FFmpeg拉流
    QStringList args;
    args << "-y"
         << "-rtsp_transport" << "tcp"
         << "-i" << rtspUrl
         << "-vn"
         << "-acodec" << "pcm_s16le"
         << "-ar" << QString::number(m_config.sampleRate)
         << "-ac" << "1"
         << "-f" << "s16le"
         << "pipe:1";

    // qDebug() << "FFmpeg命令:";
    // qDebug() << "  程序:" << m_config.ffmpegPath;
    // qDebug() << "  参数:" << args;

 //   qDebug() << "启动FFmpeg进程...";
    m_ffmpegProcess->start(m_config.ffmpegPath, args);

  //  qDebug() << "等待FFmpeg启动...";
    bool started = m_ffmpegProcess->waitForStarted(3000);
 //   qDebug() << "FFmpeg启动结果:" << started;

    if (!started) {
        qDebug() << "错误: FFmpeg启动失败";
        qDebug() << "FFmpeg错误输出:" << m_ffmpegProcess->readAllStandardError();
        qDebug() << "FFmpeg标准输出:" << m_ffmpegProcess->readAllStandardOutput();
        qDebug() << "进程状态:" << m_ffmpegProcess->state();
        qDebug() << "退出码:" << m_ffmpegProcess->exitCode();

        emit errorOccurred("FFmpeg启动失败");
        m_isRecognizing = false;
        return false;
    }

 //   qDebug() << "FFmpeg启动成功，PID:" << m_ffmpegProcess->processId();

    // 2. 连接WebSocket
 //   qDebug() << "开始连接WebSocket...";
    if (!initWebSocket()) {
        qDebug() << "错误: 无法连接WebSocket";
        qDebug() << "终止FFmpeg进程...";
        m_ffmpegProcess->terminate();
        m_ffmpegProcess->waitForFinished(2000);
        m_isRecognizing = false;
        return false;
    }
    m_isRecognizing = true;
    m_hasSentStartFrame = false;
    m_audioBuffer.clear();

    // 重置重连计数
    m_reconnectCount = 0;
    // 启动30秒超时计时器
    m_timeoutTimer->start();
    // qDebug() << "⏱️ 启动30秒超时计时器";
    // qDebug() << "=== startRecognition 成功 ===";
    emit statusMessage("开始识别...");
    return true;
}

void RealtimeSpeechRecognizer::stopRecognition()
{
    if (!m_isRecognizing) return;

    qDebug() << "🛑 手动停止识别";

    // 停止所有计时器
    stopSendingAudio();

    if (m_timeoutTimer->isActive()) {
        m_timeoutTimer->stop();
    }

    m_isRecognizing = false;

    if (m_webSocket && m_webSocket->state() == QAbstractSocket::ConnectedState) {
        sendEndFrame();
        m_webSocket->close();
    }

    if (m_ffmpegProcess && m_ffmpegProcess->state() == QProcess::Running) {
        m_ffmpegProcess->terminate();
    }

    emit statusMessage("识别停止");
}

bool RealtimeSpeechRecognizer::isRecognizing() const
{
    return m_isRecognizing;
}

bool RealtimeSpeechRecognizer::initWebSocket()
{
    // 生成认证URL
    QDateTime utcTime = QDateTime::currentDateTimeUtc();
    QString date = utcTime.toString("ddd, dd MMM yyyy hh:mm:ss 'GMT'");

    QString host = "iat-api.xfyun.cn";
    QString path = "/v2/iat";

    // 生成签名
    QString signatureStr = QString("host: %1\ndate: %2\nGET %3 HTTP/1.1")
                               .arg(host).arg(date).arg(path);

    QByteArray signature = QMessageAuthenticationCode::hash(
                               signatureStr.toUtf8(),
                               m_config.apiSecret.toUtf8(),
                               QCryptographicHash::Sha256
                               ).toBase64();

    QString authOrigin = QString("api_key=\"%1\", algorithm=\"hmac-sha256\", headers=\"host date request-line\", signature=\"%2\"")
                             .arg(m_config.apiKey).arg(QString(signature));

    QString authorization = QString::fromUtf8(authOrigin.toUtf8().toBase64());

    // 构造URL
    QUrl url;
    url.setScheme("wss");
    url.setHost(host);
    url.setPath(path);

    QUrlQuery query;
    query.addQueryItem("host", host);
    query.addQueryItem("date", date);
    query.addQueryItem("authorization", authorization);
    url.setQuery(query);

    m_webSocket->open(url);
    return true;
}

bool RealtimeSpeechRecognizer::startFFmpegStream(const QString &rtspUrl)
{
    // 已经在startRecognition中实现了
    return true;
}
void RealtimeSpeechRecognizer::onWebSocketConnected()
{
    qDebug() << "✅ WebSocket连接成功";
    emit statusMessage("WebSocket连接成功");

    // 立即发送开始帧
    sendStartFrame();

    // 等待500ms让服务器处理开始帧，然后开始发送音频
    QTimer::singleShot(500, [this]() {
        if (m_isRecognizing && m_hasSentStartFrame) {
            m_sendTimer->start();
            qDebug() << "🚀 开始发送音频数据，定时器启动";
            emit statusMessage("开始发送音频数据");
        } else {
            qDebug() << "❌ 开始帧未发送成功或识别已停止";
            emit errorOccurred("开始帧未发送成功");
        }
    });
}
void RealtimeSpeechRecognizer::onWebSocketDisconnected()
{
    qDebug() << "🔌 WebSocket断开连接";
    emit statusMessage("WebSocket断开");
}
void RealtimeSpeechRecognizer::reconnect()
{
    if (m_reconnectCount >= MAX_RECONNECT) {
        qDebug() << "❌ 达到最大重连次数，停止重连";
        emit errorOccurred("达到最大重连次数，请检查音频源");
        return;
    }

    m_reconnectCount++;
    qDebug() << "🔄 第" << m_reconnectCount << "次重连尝试";

    if (!m_lastRtspUrl.isEmpty()) {
        // 清理状态但保持识别标志
        m_hasSentStartFrame = false;
     //   m_isWebSocketConnected = false;
        m_audioBuffer.clear();

        // 重新连接WebSocket
        if (initWebSocket()) {
            emit statusMessage(QString("正在重连(%1/%2)...").arg(m_reconnectCount).arg(MAX_RECONNECT));
        }
    }
}

void RealtimeSpeechRecognizer::onWebSocketTextMessageReceived(const QString &message)
{
    static int responseCount = 0;
    responseCount++;

    // qDebug() << "📥 收到第" << responseCount << "个服务器响应，长度:"
    //          << message.length() << "字符";

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) {
        qDebug() << "❌ 响应不是有效的JSON";
        return;
    }

    QJsonObject response = doc.object();
    int code = response["code"].toInt();

    // 处理服务器响应
    if (code == 0 && response.contains("data")) {
        QJsonObject data = response["data"].toObject();

        // 检查会话状态
        int serverStatus = data["status"].toInt();
       // qDebug() << "🔍 服务器状态码:" << serverStatus;

        // 状态码=2表示会话结束
        if (serverStatus == 2) {
            qDebug() << "✅ 服务器返回最终状态(status=2)，本次会话结束";

            // 检查是否有最终的识别结果
            if (data.contains("result")) {
                QJsonObject result = data["result"].toObject();
                QString finalText = extractTextFromResult(result);

                // 打印sn（句子序号）
                int sn = result["sn"].toInt(-1);
                if (sn != -1) {
              //      qDebug() << "📝 句子序号(sn):" << sn;
                }

                if (!finalText.isEmpty()) {
              //      qDebug() << "🎤 最终识别结果:" << finalText;
                    emit textReceived(finalText);
                } else {
              //      qDebug() << "🔇 最终识别结果为空";
                }
            }

            // 触发会话结束信号
            emit sessionCompleted();

            // 停止发送定时器
            if (m_sendTimer->isActive()) {
                m_sendTimer->stop();
                qDebug() << "⏹️ 停止发送音频定时器";
            }

            return; // 不再处理后续内容
        }

        // 处理正常识别结果（status!=2）
        if (data.contains("result")) {
            QJsonObject result = data["result"].toObject();

            // 打印sn（句子序号）
            int sn = result["sn"].toInt(-1);
            if (sn != -1) {
            //    qDebug() << "📝 句子序号(sn):" << sn;
            }

            if (result.contains("ws")) {
                QString text = extractTextFromResult(result);

                if (!text.isEmpty()) {
             //       qDebug() << "🎤 识别到文本:" << text;
                    emit textReceived(text);
                } else {
              //      qDebug() << "🔇 识别结果为空（可能是音乐/噪声）";
                }
            }
        }
    }
    else if (code == 10165) {
        // 处理10165错误 - 自动重连
        QString errorMsg = response["message"].toString();
        qDebug() << "🔄 收到10165错误，准备重连:" << errorMsg;
        emit errorOccurred(QString("会话超时: %1").arg(errorMsg));

        // 这里可以添加重连逻辑
    }
    else if (code != 0) {
        QString errorMsg = response["message"].toString();
        qDebug() << "❌ 服务器错误" << code << ":" << errorMsg;
        emit errorOccurred(QString("错误 %1: %2").arg(code).arg(errorMsg));
    }
}


QString RealtimeSpeechRecognizer::extractTextFromResult(const QJsonObject &result)
{
    QString text;

    if (result.contains("ws")) {
        QJsonArray wsArray = result["ws"].toArray();
        for (const auto &wsVal : wsArray) {
            QJsonObject wsObj = wsVal.toObject();
            if (wsObj.contains("cw")) {
                QJsonArray cwArray = wsObj["cw"].toArray();
                if (!cwArray.isEmpty()) {
                    QJsonObject cwObj = cwArray.first().toObject();
                    QString word = cwObj["w"].toString();
                    if (!word.isEmpty() && word != "？") {
                        text += word;
                    }
                }
            }
        }
    }

    return text;
}

void RealtimeSpeechRecognizer::onFFmpegReadyRead()
{
    QByteArray data = m_ffmpegProcess->readAllStandardOutput();

    if (data.isEmpty()) {
        qDebug() << "FFmpeg输出为空";
        return;
    }

    static qint64 totalBytes = 0;
    totalBytes += data.size();

    // qDebug() << "🎵 收到FFmpeg数据:" << data.size()
    //          << "字节, 累计:" << totalBytes << "字节";

    m_audioBuffer.append(data);
}


void RealtimeSpeechRecognizer::onSendTimerTimeout()
{
    if (!m_isRecognizing || !m_hasSentStartFrame) {
        qDebug() << "⏰ 定时器跳过: 识别中?" << m_isRecognizing
                 << "开始帧已发?" << m_hasSentStartFrame;
        return;
    }

    // qDebug() << "📊 音频缓冲区大小:" << m_audioBuffer.size()
    //          << "字节, 需要:" << m_frameSize << "字节";

    if (m_audioBuffer.size() >= m_frameSize) {
        QByteArray frame = m_audioBuffer.left(m_frameSize);
        m_audioBuffer.remove(0, m_frameSize);
        sendAudioFrame(frame);
    } else {
        // qDebug() << "⚠️  数据不足，跳过此帧，当前缓冲区:"
        //          << m_audioBuffer.size() << "字节";

        // 如果没有数据，发送静音帧保持连接
        static int emptyCount = 0;
        emptyCount++;

        if (emptyCount > 500) {  // 连续3次没有数据
            qDebug() << "🔇 连续" << emptyCount << "次无数据 手动停止识别";
            QByteArray silence(m_frameSize, 0);
          //  sendAudioFrame(silence);
         //   sendEndFrame();
         //   qDebug() << "⏰ 超时后未收到服务器响应，强制结束";
            emit sessionCompleted();
            stopRecognition();
        }
    }
}


void RealtimeSpeechRecognizer::sendStartFrame()
{
    QJsonObject common;
    common["app_id"] = m_config.appId;

    QJsonObject business;
    business["language"] = "zh_cn";
    business["domain"] = "iat";
    business["accent"] = "mandarin";
    business["vad_eos"] = 10000;//停顿多久结束
    business["ptt"] = 1;
    business["dwa"] = "wpgs";  // 动态修正

    QJsonObject data;
    data["status"] = 0;
    data["format"] = QString("audio/L16;rate=%1").arg(m_config.sampleRate);
    data["encoding"] = "raw";
    data["audio"] = "";

    QJsonObject root;
    root["common"] = common;
    root["business"] = business;
    root["data"] = data;

    QJsonDocument doc(root);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    qDebug() << "📤 发送第一帧(status=0), JSON长度:" << jsonStr.length();
    qDebug() << "第一帧内容:" << jsonStr;

    m_webSocket->sendTextMessage(jsonStr);
    m_hasSentStartFrame = true;
}



void RealtimeSpeechRecognizer::sendAudioFrame(const QByteArray &audioData)
{
    static int frameCount = 0;
    frameCount++;

    QJsonObject data;
    data["status"] = 1;
    data["format"] = QString("audio/L16;rate=%1").arg(m_config.sampleRate);
    data["encoding"] = "raw";
    data["audio"] = QString::fromLatin1(audioData.toBase64());

    QJsonObject root;
    root["data"] = data;

    QJsonDocument doc(root);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    // 每10帧打印一次，避免日志太多
    // if (frameCount % 10 == 0) {
    //     qDebug() << "📤 发送第" << frameCount << "帧音频(status=1), 音频大小:"
    //              << audioData.size() << "字节, Base64后:"
    //              << data["audio"].toString().length() << "字符";
    // }

    m_webSocket->sendTextMessage(jsonStr);
}
void RealtimeSpeechRecognizer::sendAudioFrameForend(const QByteArray &audioData)
{
    QJsonObject data;
    data["status"] = 2;  // 结束帧
    data["format"] = QString("audio/L16;rate=%1").arg(m_config.sampleRate);
    data["encoding"] = "raw";
    data["audio"] = QString::fromLatin1(audioData.toBase64());

    QJsonObject root;
    root["data"] = data;

    QJsonDocument doc(root);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));

    qDebug() << "📤 发送带音频的结束帧(status=2)，音频大小:"
             << audioData.size() << "字节";

    m_webSocket->sendTextMessage(jsonStr);
}
void RealtimeSpeechRecognizer::sendEndFrame()
{
    QJsonObject data;
    data["status"] = 2;

    QJsonObject root;
    root["data"] = data;

    QJsonDocument doc(root);
    QString jsonStr = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
    m_webSocket->sendTextMessage(jsonStr);
}
