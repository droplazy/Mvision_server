#ifndef MEDIAMTX_MANAGER_H
#define MEDIAMTX_MANAGER_H

#include <QObject>
#include <QProcess>

class MediaMTX_Manager : public QObject
{
    Q_OBJECT

public:
    explicit MediaMTX_Manager(QObject *parent = nullptr);
    ~MediaMTX_Manager();

    // 启动mediamtx服务器
    bool startServer(const QString &mediamtxPath);

    // 停止服务器
    void stopServer();

    // 生成RTSP地址
    static QString generateRTSPUrl(const QString &ip = "127.0.0.1",
                                   int port = 8554,
                                   const QString &stream = "audio");

    // 生成ffmpeg推流命令
    static QString generateFFmpegCommand(const QString &ffmpegPath,
                                         const QString &audioFile,
                                         const QString &rtspUrl);

    // 服务器是否运行
    bool isRunning() const;

    // 获取当前RTSP地址
    QString currentUrl() const;

signals:
    void serverStarted(const QString &rtspUrl);
    void serverStopped();
    void error(const QString &msg);

private slots:
    void onProcessError(QProcess::ProcessError error);
    void onProcessFinished(int exitCode, QProcess::ExitStatus exitStatus);

private:
    QProcess *m_process;
    QString m_mediamtxPath;
    QString m_ip;
    int m_port;
};

#endif // MEDIAMTX_MANAGER_H
