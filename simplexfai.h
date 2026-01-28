#ifndef SIMPLEXFAI_H
#define SIMPLEXFAI_H

#include <QObject>

class SimpleXFAI : public QObject
{
    Q_OBJECT
public:
    explicit SimpleXFAI(QObject *parent = nullptr);

    // 设置认证信息（从你的截图）
    void setAuthInfo(const QString &apiKey, const QString &apiSecret);

    // 发送问题到讯飞AI
    void askQuestion(const QString &question);

signals:
    // 收到回复
    void responseReceived(const QString &response);

    // 错误
    void errorOccurred(const QString &error);

private slots:
    void onReplyFinished();

private:
    class QNetworkAccessManager *m_manager;
    class QNetworkReply *m_reply;
    QString m_pendingQuestion;      // 等待处理的问题
    bool m_hasPendingQuestion;      // 是否有等待的问题
    bool m_isRequesting;            // 是否正在请求中
    QString m_apiKey;
    QString m_apiSecret;
};

#endif // SIMPLEXFAI_H
