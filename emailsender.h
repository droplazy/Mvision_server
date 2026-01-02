#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QObject>
#include <QSslSocket>

class EmailSender : public QObject
{
    Q_OBJECT

public:
    explicit EmailSender(QObject *parent = nullptr);

    // 接口1: 登录邮箱
    bool login(const QString &email, const QString &authCode);

    // 接口2: 发送邮件
    bool sendEmail(const QString &toEmail, const QString &message);

    // 获取错误信息
    QString errorString() const { return m_errorString; }

    // 是否已登录
    bool isLoggedIn() const { return m_isLoggedIn; }

    // 设置SMTP服务器
    void setSmtpServer(const QString &server, int port = 465);

    // 设置连接超时时间
    void setTimeout(int ms) { m_timeoutMs = ms; }

private:
    // 私有成员1: 登录的邮箱地址
    QString m_email;

    // 私有成员2: 登录邮箱的授权码
    QString m_authCode;

    // SMTP服务器配置
    QString m_smtpServer;
    int m_smtpPort;
    int m_timeoutMs;

    // 状态和错误
    bool m_isLoggedIn;
    QString m_errorString;

    // 网络相关
    QSslSocket *m_socket;

    // 响应缓冲区
    QString m_responseBuffer;

    // 内部方法
    bool connectToSmtp();
    bool sendCommand(const QString &command, const QString &expectedResponse = "250");
    bool sendAuth();
    bool sendMailData(const QString &toEmail, const QString &message);
    QString waitForResponse(int timeout = 10000);
    void disconnectSmtp();
};

#endif // EMAILSENDER_H
