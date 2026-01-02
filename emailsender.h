#ifndef EMAILSENDER_H
#define EMAILSENDER_H

#include <QObject>
#include <QSslSocket>
#include "publicheader.h"

class EmailSender : public QObject
{
    Q_OBJECT

public:
    explicit EmailSender(QObject *parent = nullptr);

    // 接口1: 登录邮箱
    bool login(const QString &email, const QString &authCode);

    // 接口2: 直接发送邮件
    bool sendEmail(const QString &toEmail, const QString &message);

    // 获取错误信息
    QString errorString() const { return m_errorString; }

    // 是否已登录
    bool isLoggedIn() const { return m_isLoggedIn; }

    // 设置SMTP服务器
    void setSmtpServer(const QString &server, int port = 465);

    // 设置连接超时时间
    void setTimeout(int ms) { m_timeoutMs = ms; }

public slots:
    // 槽函数：接收邮件信息并发送
    void onSendEmailRequested(const EmailInfo &emailInfo);

signals:
    // 信号：邮件发送结果
    void emailSent(bool success, const QString &message);

private:
    // 私有成员
    QString m_email;
    QString m_authCode;
    QString m_smtpServer;
    int m_smtpPort;
    int m_timeoutMs;
    bool m_isLoggedIn;
    QString m_errorString;
    QSslSocket *m_socket;
    QString m_responseBuffer;

    // 内部方法
    bool connectToSmtp();
    bool sendCommand(const QString &command, const QString &expectedResponse = "250");
    bool sendAuth();
    bool sendMailData(const EmailInfo &emailInfo);
    QString waitForResponse(int timeout = 10000);
    void disconnectSmtp();

    // 实际发送邮件（内部使用）
    bool sendEmailInternal(const EmailInfo &emailInfo);
};

#endif // EMAILSENDER_H
