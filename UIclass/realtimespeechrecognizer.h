#ifndef REALTIMESPEECHRECOGNIZER_H
#define REALTIMESPEECHRECOGNIZER_H

#include <QObject>
#include <QProcess>
#include <QWebSocket>
#include <QTimer>
#include <QJsonObject>

class RealtimeSpeechRecognizer : public QObject
{
    Q_OBJECT

public:
    struct Config {
        QString appId;
        QString apiKey;
        QString apiSecret;
        QString ffmpegPath;
        int sampleRate = 16000;
    };

    explicit RealtimeSpeechRecognizer(QObject *parent = nullptr);
    ~RealtimeSpeechRecognizer();

    void setConfig(const Config &config);
    bool startRecognition(const QString &rtspUrl);
    void stopRecognition();
    bool isRecognizing() const;

signals:
    void textReceived(const QString &text);
    void errorOccurred(const QString &error);
    void statusMessage(const QString &message);

private slots:
    void onWebSocketConnected();
    void onWebSocketDisconnected();
    void onWebSocketTextMessageReceived(const QString &message);
    void onFFmpegReadyRead();
    void onSendTimerTimeout();
    void reconnect();  // 重连函数
private:
    bool initWebSocket();
    bool startFFmpegStream(const QString &rtspUrl);
    QString generateAuthUrl();
    void sendStartFrame();
    void sendAudioFrame(const QByteArray &audioData);
    void sendEndFrame();

    Config m_config;
    QWebSocket *m_webSocket;
    QProcess *m_ffmpegProcess;
    QTimer *m_sendTimer;
    bool m_isRecognizing;
    bool m_hasSentStartFrame;
    QByteArray m_audioBuffer;
    int m_frameSize;
    QString m_lastRtspUrl;  // 保存最后一次的RTSP URL
    int m_reconnectCount = 0;  // 重连次数
    const int MAX_RECONNECT = 3;  // 最大重连次数
};

#endif // REALTIMESPEECHRECOGNIZER_H
