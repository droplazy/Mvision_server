#include "mainwindow.h"
#include <QMqttClient>
#include <QApplication>
#include <QLocale>
#include <QTranslator>

#include "emailsender.h"
#include <QTcpSocket>
#include <QDateTime>
void testSmtpPorts()
{
    QString host = "smtp.126.com";
    QVector<int> ports = {465, 587, 25};

    for (int port : ports) {
        QTcpSocket socket;
        socket.connectToHost(host, port);

        if (socket.waitForConnected(3000)) {
            qDebug() << QString("端口 %1: ✓ 可用").arg(port);
            socket.disconnectFromHost();
        } else {
            qDebug() << QString("端口 %1: ✗ 不可用 - %2")
                            .arg(port).arg(socket.errorString());
        }
    }
}

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MuiltiControlSer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
   // w.show();

    return a.exec();
}
#if 0
    EmailSender sender;

    // 设置SMTP服务器（yeah.net邮箱）
    sender.setSmtpServer("smtp.yeah.net", 465);

    // 1. 登录邮箱
    qDebug() << "\n1. 正在登录邮箱...";
    QString email = "zwdz668@yeah.net";
    QString authCode = "XFaYuyxRWqQXJp7w"; // 您的授权码

    if (sender.login(email, authCode)) {
        qDebug() << "✓ 登录成功!";
    } else {
        qDebug() << "✗ 登录失败:" << sender.errorString();
        return -1;
    }

    // 2. 发送邮件
    qDebug() << "\n2. 正在发送邮件...";
    QString toEmail = "601321904@qq.com";
    QString message = "您的验证码是：666888\n\n这是一封测试邮件。";

    if (sender.sendEmail(toEmail, message)) {
        qDebug() << "✓ 邮件发送成功!";
        qDebug() << "  收件人:" << toEmail;
        qDebug() << "  内容:" << message;
    } else {
        qDebug() << "✗ 发送失败:" << sender.errorString();
        return -1;
    }

    // 3. 再发送一封测试邮件
    qDebug() << "\n3. 发送第二封测试邮件...";
    QString testMessage =
        "这是第二封测试邮件\n"
        "发送时间: " + QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss") + "\n"
                                                                         "测试验证码: 123456";

    if (sender.sendEmail(toEmail, testMessage)) {
        qDebug() << "✓ 第二封邮件发送成功!";
    } else {
        qDebug() << "✗ 发送失败:" << sender.errorString();
    }

    qDebug() << "\n======================================";
    qDebug() << "邮件发送DEMO完成";
    qDebug() << "======================================";
#endif
