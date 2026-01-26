#include "mediamtx_manager.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QThread>


MediaMTX_Manager::MediaMTX_Manager(QObject *parent)
    : QObject(parent)
    , m_process(nullptr)
    , m_ip("127.0.0.1")
    , m_port(8554)
{
}

MediaMTX_Manager::~MediaMTX_Manager()
{
    stopServer();
}

bool MediaMTX_Manager::startServer(const QString &mediamtxPath)
{
    if (isRunning()) {
        emit error("服务器已在运行");
        return false;
    }

    m_mediamtxPath = mediamtxPath;

    // 检查文件是否存在
    if (!QFile::exists(m_mediamtxPath)) {
        emit error("mediamtx文件不存在: " + m_mediamtxPath);
        return false;
    }

    // 创建进程
    m_process = new QProcess(this);
    connect(m_process, &QProcess::errorOccurred, this, &MediaMTX_Manager::onProcessError);
    connect(m_process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &MediaMTX_Manager::onProcessFinished);

    // 设置工作目录到mediamtx所在目录
    QFileInfo fileInfo(m_mediamtxPath);
    m_process->setWorkingDirectory(fileInfo.absolutePath());

    // 启动mediamtx
    qDebug() << "启动MediaMTX服务器:" << m_mediamtxPath;
    m_process->start(m_mediamtxPath, QStringList());

    if (m_process->waitForStarted(3000)) {
        // 等待mediamtx完全启动
        QThread::sleep(2);

        QString url = currentUrl();
        qDebug() << "MediaMTX服务器启动成功";
        qDebug() << "RTSP地址:" << url;
        qDebug() << "支持协议: RTSP(" << m_port << "), RTMP(1935), HLS(8888)";

        emit serverStarted(url);
        return true;
    }

    emit error("无法启动mediamtx服务器");
    return false;
}

void MediaMTX_Manager::stopServer()
{
    if (m_process && m_process->state() == QProcess::Running) {
        qDebug() << "停止MediaMTX服务器...";
        m_process->terminate();
        if (!m_process->waitForFinished(3000)) {
            m_process->kill();
        }
    }
    if (m_process) {
        m_process->deleteLater();
        m_process = nullptr;
    }
    emit serverStopped();
}

QString MediaMTX_Manager::generateRTSPUrl(const QString &ip, int port, const QString &stream)
{
    return QString("rtsp://%1:%2/%3").arg(ip).arg(port).arg(stream);
}

QString MediaMTX_Manager::generateFFmpegCommand(const QString &ffmpegPath,
                                                const QString &audioFile,
                                                const QString &rtspUrl)
{
    return QString("\"%1\" -re -i \"%2\" -vn -c:a aac -b:a 128k -ar 44100 -ac 2 -f rtsp \"%3\"")
    .arg(ffmpegPath)
        .arg(audioFile)
        .arg(rtspUrl);
}

bool MediaMTX_Manager::isRunning() const
{
    return m_process && m_process->state() == QProcess::Running;
}

QString MediaMTX_Manager::currentUrl() const
{
    return generateRTSPUrl(m_ip, m_port, "live");
}

void MediaMTX_Manager::onProcessError(QProcess::ProcessError error)
{
    QString errorMsg;
    switch (error) {
    case QProcess::FailedToStart:
        errorMsg = "无法启动mediamtx，请检查路径和权限";
        break;
    case QProcess::Crashed:
        errorMsg = "mediamtx崩溃";
        break;
    case QProcess::Timedout:
        errorMsg = "启动超时";
        break;
    default:
        errorMsg = "未知错误";
    }
    emit this->error(errorMsg);
}

void MediaMTX_Manager::onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        emit error(QString("mediamtx异常退出，代码: %1").arg(exitCode));
    }
    stopServer();
}
