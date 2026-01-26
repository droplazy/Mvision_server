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
}

RealtimeSpeechRecognizer::~RealtimeSpeechRecognizer()
{
    stopRecognition();
}

void RealtimeSpeechRecognizer::setConfig(const Config &config)
{
    m_config = config;
}

bool RealtimeSpeechRecognizer::startRecognition(const QString &rtspUrl)
{
    m_lastRtspUrl = rtspUrl;  // 保存URL
    m_reconnectCount = 0;     // 重置重连计数
    if (m_isRecognizing) {
        emit errorOccurred("已经在识别中");
        return false;
    }

    if (m_config.apiKey.isEmpty() || m_config.apiSecret.isEmpty()) {
        emit errorOccurred("API凭证未设置");
        return false;
    }

    if (!QFile::exists(m_config.ffmpegPath)) {
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

    m_ffmpegProcess->start(m_config.ffmpegPath, args);

    if (!m_ffmpegProcess->waitForStarted(3000)) {
        emit errorOccurred("FFmpeg启动失败");
        m_isRecognizing = false;
        return false;
    }

    // 2. 连接WebSocket
    if (!initWebSocket()) {
        m_ffmpegProcess->terminate();
        m_isRecognizing = false;
        return false;
    }

    emit statusMessage("开始识别...");
    return true;
}

void RealtimeSpeechRecognizer::stopRecognition()
{
    if (!m_isRecognizing) return;

    m_isRecognizing = false;
    m_sendTimer->stop();

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

    qDebug() << "📥 收到第" << responseCount << "个服务器响应，长度:"
             << message.length() << "字符";

    QJsonDocument doc = QJsonDocument::fromJson(message.toUtf8());
    if (doc.isNull()) {
        qDebug() << "❌ 响应不是有效的JSON";
        return;
    }

    QJsonObject response = doc.object();
    int code = response["code"].toInt();

    // 打印完整的响应（前200字符）
    QString shortResponse = message;
    if (shortResponse.length() > 200) {
        shortResponse = shortResponse.left(200) + "...";
    }
    qDebug() << "服务器响应内容:" << shortResponse;

    // 处理10165错误 - 自动重连
    if (code == 10165) {
        QString errorMsg = response["message"].toString();
        qDebug() << "🔄 收到10165错误，准备重连:" << errorMsg;

        // 保存当前状态
        bool wasRecognizing = m_isRecognizing;

        // 停止当前连接
        stopRecognition();

        // 延迟1秒后重连
        if (wasRecognizing) {
            QTimer::singleShot(1000, this, [this]() {
                qDebug() << "🔄 开始重连...";
                // 这里需要保存RTSP URL以便重连
                // 你可以添加一个成员变量 m_lastRtspUrl 来保存
            });
        }

        emit errorOccurred(QString("会话超时，正在重连: %1").arg(errorMsg));
        return;
    }

    if (code == 0 && response.contains("data")) {
        QJsonObject data = response["data"].toObject();

        // 检查是否有sid
        if (response.contains("sid")) {
            QString sid = response["sid"].toString();
            qDebug() << "🆔 会话ID:" << sid;
        }

        // 检查服务器是否返回了最终结果（status=2）
        int serverStatus = data["status"].toInt();
        if (serverStatus == 2) {
            qDebug() << "⚠️  服务器返回最终状态(status=2)，准备重连";

            // 延迟500ms后重连
            QTimer::singleShot(500, this, [this]() {
                if (m_isRecognizing) {
                    qDebug() << "🔄 服务器结束会话，重新连接...";
                    // 这里需要触发重连
                }
            });
        }

        if (data.contains("result")) {
            QJsonObject result = data["result"].toObject();
            if (result.contains("ws")) {
                QString text;
                QJsonArray wsArray = result["ws"].toArray();
                for (const auto &wsVal : wsArray) {
                    QJsonObject wsObj = wsVal.toObject();
                    if (wsObj.contains("cw")) {
                        QJsonArray cwArray = wsObj["cw"].toArray();
                        if (!cwArray.isEmpty()) {
                            QJsonObject cwObj = cwArray.first().toObject();
                            QString word = cwObj["w"].toString();
                            if (!word.isEmpty()) {
                                text += word;
                            }
                        }
                    }
                }
                if (!text.isEmpty()) {
                    qDebug() << "🎤 识别到文本:" << text;
                    emit textReceived(text);
                } else {
                    qDebug() << "🔇 识别结果为空（可能是音乐/噪声）";
                }
            }
        }
    } else if (code != 0) {
        QString errorMsg = response["message"].toString();
        qDebug() << "❌ 服务器错误" << code << ":" << errorMsg;
        emit errorOccurred(QString("错误 %1: %2").arg(code).arg(errorMsg));
    }
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
        qDebug() << "⚠️  数据不足，跳过此帧，当前缓冲区:"
                 << m_audioBuffer.size() << "字节";

        // 如果没有数据，发送静音帧保持连接
        static int emptyCount = 0;
        emptyCount++;

        if (emptyCount > 3) {  // 连续3次没有数据
            qDebug() << "🔇 连续" << emptyCount << "次无数据，发送静音帧保持连接";
            QByteArray silence(m_frameSize, 0);
            sendAudioFrame(silence);
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
    business["vad_eos"] = 10000;
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
