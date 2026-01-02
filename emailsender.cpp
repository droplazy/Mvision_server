#include "EmailSender.h"
#include <QDebug>
#include <QTimer>
#include <QEventLoop>
#include <QElapsedTimer>
#include <QThread>
#include <QDateTime>

EmailSender::EmailSender(QObject *parent)
    : QObject(parent)
    , m_smtpServer("smtp.yeah.net")
    , m_smtpPort(465)
    , m_timeoutMs(15000)
    , m_isLoggedIn(false)
    , m_socket(nullptr)
{
}

bool EmailSender::login(const QString &email, const QString &authCode)
{
    m_email = email;
    m_authCode = authCode;
    m_isLoggedIn = false;
    m_errorString.clear();

    qDebug() << "开始登录邮箱:" << m_email;

    // 验证邮箱格式
    if (!m_email.contains("@") || !m_email.contains(".")) {
        m_errorString = "邮箱格式不正确";
        qDebug() << "✗" << m_errorString;
        return false;
    }

    if (m_authCode.isEmpty()) {
        m_errorString = "授权码不能为空";
        qDebug() << "✗" << m_errorString;
        return false;
    }

    // 测试连接但不真正登录（实际发送邮件时再认证）
    // 这里只验证参数，实际登录在sendEmail中进行
    qDebug() << "✓ 登录参数验证通过";
    m_isLoggedIn = true;

    return true;
}

// 原来的sendEmail方法（保持兼容）
bool EmailSender::sendEmail(const QString &toEmail, const QString &message)
{
    EmailInfo info(toEmail, message);
    return sendEmailInternal(info);
}

void EmailSender::setSmtpServer(const QString &server, int port)
{
    m_smtpServer = server;
    m_smtpPort = port;
}

bool EmailSender::connectToSmtp()
{
    if (m_socket) {
        m_socket->deleteLater();
    }

    m_socket = new QSslSocket(this);

    qDebug() << "连接到" << m_smtpServer << ":" << m_smtpPort;

    // 忽略SSL错误（测试用，生产环境应验证证书）
    m_socket->ignoreSslErrors();

    m_socket->connectToHostEncrypted(m_smtpServer, m_smtpPort);

    if (!m_socket->waitForConnected(m_timeoutMs)) {
        m_errorString = "连接失败: " + m_socket->errorString();
        return false;
    }

    if (!m_socket->waitForEncrypted(m_timeoutMs)) {
        m_errorString = "SSL加密失败: " + m_socket->errorString();
        return false;
    }

    return true;
}

bool EmailSender::sendCommand(const QString &command, const QString &expectedResponse)
{
    if (!m_socket || !m_socket->isWritable()) {
        m_errorString = "Socket不可写";
        return false;
    }

    qDebug() << "发送:" << command.trimmed();

    m_socket->write(command.toUtf8() + "\r\n");
    m_socket->flush();

    QString response = waitForResponse(m_timeoutMs);

    if (response.isEmpty()) {
        m_errorString = "等待响应超时";
        return false;
    }

    qDebug() << "接收:" << response.trimmed();

    if (response.length() >= 3) {
        QString responseCode = response.left(3);

        if (responseCode == expectedResponse) {
            return true;
        } else {
            m_errorString = QString("期望%1，收到%2").arg(expectedResponse).arg(responseCode);
            return false;
        }
    }

    m_errorString = "无效响应: " + response;
    return false;
}

bool EmailSender::sendAuth()
{
    // AUTH LOGIN
    if (!sendCommand("AUTH LOGIN", "334")) {
        m_errorString = "AUTH失败: " + m_errorString;
        return false;
    }

    // 发送用户名
    QByteArray usernameBase64 = m_email.toUtf8().toBase64();
    if (!sendCommand(usernameBase64, "334")) {
        m_errorString = "用户名认证失败: " + m_errorString;
        return false;
    }

    // 发送授权码
    QByteArray passwordBase64 = m_authCode.toUtf8().toBase64();
    if (!sendCommand(passwordBase64, "235")) {
        m_errorString = "密码认证失败: " + m_errorString;
        return false;
    }

    return true;
}
// 发送邮件数据（使用结构体）
bool EmailSender::sendMailData(const EmailInfo &emailInfo)
{
    // MAIL FROM
    if (!sendCommand(QString("MAIL FROM:<%1>").arg(m_email))) {
        m_errorString = "发件人设置失败";
        return false;
    }

    // RCPT TO
    if (!sendCommand(QString("RCPT TO:<%1>").arg(emailInfo.toEmail))) {
        m_errorString = "收件人设置失败";
        return false;
    }

    // DATA
    if (!sendCommand("DATA", "354")) {
        m_errorString = "准备发送数据失败";
        return false;
    }

    // 邮件内容（使用结构体中的主题）
    QString emailData;
    emailData += QString("From: %1\r\n").arg(m_email);
    emailData += QString("To: %1\r\n").arg(emailInfo.toEmail);
    emailData += QString("Subject: %1\r\n").arg(emailInfo.subject);
    emailData += "Content-Type: text/plain; charset=utf-8\r\n";
    emailData += "\r\n";
    emailData += emailInfo.message + "\r\n";
    emailData += ".\r\n";

    if (!sendCommand(emailData, "250")) {
        m_errorString = "发送邮件内容失败";
        return false;
    }

    sendCommand("QUIT", "221");
    return true;
}

QString EmailSender::waitForResponse(int timeout)
{
    if (!m_socket) {
        m_errorString = "Socket未初始化";
        return "";
    }

    m_responseBuffer.clear();
    QElapsedTimer timer;
    timer.start();

    while (timer.elapsed() < timeout) {
        if (m_socket->waitForReadyRead(100)) {
            while (m_socket->canReadLine()) {
                QString line = QString::fromUtf8(m_socket->readLine());
                m_responseBuffer += line;

                // SMTP响应结束标记：第4个字符是空格
                if (line.length() >= 4 && line[3] == ' ') {
                    return m_responseBuffer;
                }
            }
        }
        QThread::msleep(10);
    }

    m_errorString = "等待响应超时";
    return "";
}
// 内部发送邮件方法
bool EmailSender::sendEmailInternal(const EmailInfo &emailInfo)
{
    m_errorString.clear();

    if (!m_isLoggedIn) {
        m_errorString = "请先登录邮箱";
        return false;
    }

    if (!emailInfo.toEmail.contains("@") || !emailInfo.toEmail.contains(".")) {
        m_errorString = "收件人邮箱格式不正确";
        return false;
    }

    qDebug() << "开始发送邮件到:" << emailInfo.toEmail;

    // 连接到SMTP服务器
    if (!connectToSmtp()) {
        return false;
    }

    // 读取欢迎消息
    QString welcome = waitForResponse(5000);
    if (welcome.isEmpty() || !welcome.startsWith("220")) {
        m_errorString = "服务器欢迎消息异常: " + welcome;
        disconnectSmtp();
        return false;
    }

    // 发送EHLO
    QString domain = m_email.mid(m_email.indexOf('@') + 1);
    if (!sendCommand("EHLO " + domain, "250")) {
        disconnectSmtp();
        return false;
    }

    // 认证
    if (!sendAuth()) {
        disconnectSmtp();
        return false;
    }

    // 发送邮件数据
    if (!sendMailData(emailInfo)) {
        disconnectSmtp();
        return false;
    }

    disconnectSmtp();
    return true;
}
void EmailSender::disconnectSmtp()
{
    if (m_socket) {
        if (m_socket->state() == QAbstractSocket::ConnectedState) {
            m_socket->write("QUIT\r\n");
            m_socket->waitForBytesWritten(1000);
        }

        m_socket->disconnectFromHost();
        if (m_socket->state() != QAbstractSocket::UnconnectedState) {
            m_socket->waitForDisconnected(1000);
        }

        m_socket->deleteLater();
        m_socket = nullptr;
    }
}
// 槽函数：接收邮件发送请求
void EmailSender::onSendEmailRequested(const EmailInfo &emailInfo)
{
    bool success = sendEmailInternal(emailInfo);
    QString resultMsg = success ? "邮件发送成功" : m_errorString;
    emit emailSent(success, resultMsg);
}
