#ifndef XUNFEIAI_H
#define XUNFEIAI_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QJsonObject>

class XunfeiAI : public QObject
{
    Q_OBJECT

public:
    // 响应结构体
    struct Response {
        bool success;
        QString content;          // AI回复内容
        QString reasoning;        // 推理过程（X1.5特有）
        int tokens;              // 消耗的token数
        QString error;           // 错误信息
    };

    // 构造函数
    explicit XunfeiAI(QObject *parent = nullptr);

    // 核心接口
    void setup(const QString &apiKey, const QString &apiSecret);
    void ask(const QString &question);
    void askWithSystem(const QString &systemPrompt, const QString &question);

    // 参数设置（可选）
    void setMaxTokens(int maxTokens);
    void setTemperature(float temperature);
    void enableThinking(bool enable = true);
    void enableWebSearch(bool enable = true);

signals:
    void responseReceived(const Response &response);
    void errorOccurred(const QString &error);

private:
    QNetworkAccessManager *m_netManager;
    QString m_apiKey;
    QString m_apiSecret;

    // 参数
    int m_maxTokens = 2048;
    float m_temperature = 0.7;
    bool m_thinkingEnabled = true;
    bool m_webSearchEnabled = false;

    // 私有方法
    QByteArray buildRequest(const QString &role, const QString &content);
    Response parseResponse(const QByteArray &data);
    void sendRequest(const QByteArray &data);

private slots:
    void onRequestFinished();
};

#endif // XUNFEIAI_H
