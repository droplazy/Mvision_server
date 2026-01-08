#ifndef APPLOGIN2_H
#define APPLOGIN2_H

#include <QDialog>
#include <QString>

namespace Ui {
class applogin2;
}

class applogin2 : public QDialog
{
    Q_OBJECT

public:
    // 构造函数，接收设备号、平台名称和当前状态
    explicit applogin2(const QString& deviceSerial,
                       const QString& platformName,
                       const QString& currentStatus,
                       QWidget *parent = nullptr);
    ~applogin2();

    // 获取用户输入的信息
    QString getAccount() const;
    QString getPassword() const;
    QString getVerifyCode() const;

signals:
    // 信号：当登录按钮被点击时发射
    void loginRequested(const QString& deviceSerial,
                        const QString& platformName,
                        const QString& account,
                        const QString& password,
                        const QString& verifyCode);

    // 信号：当发送验证码按钮被点击时发射
    void sendCodeRequested(const QString& deviceSerial,
                           const QString& platformName,
                           const QString& account,
                           const QString& password);

private slots:
    void on_pushButton_loggin_clicked();
    void on_pushButton_sendcode_clicked();

private:
    Ui::applogin2 *ui;
    QString m_deviceSerial;    // 设备号
    QString m_platformName;    // 平台名称
    QString m_currentStatus;   // 当前状态

    // 初始化UI
    void initUI();
};

#endif // APPLOGIN2_H
