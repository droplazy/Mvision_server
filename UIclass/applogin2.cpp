#include "applogin2.h"
#include "ui_applogin2.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>


applogin2::applogin2(const QString& deviceSerial,
                     const QString& platformName,
                     const QString& currentStatus,
                     QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::applogin2)
    , m_deviceSerial(deviceSerial)
    , m_platformName(platformName)
    , m_currentStatus(currentStatus)
{
    ui->setupUi(this);

    // 初始化UI
    initUI();
}

applogin2::~applogin2()
{
    delete ui;
}

// 初始化UI
void applogin2::initUI()
{
    // 设置窗口标题
    setWindowTitle(QString("账号登录 - %1 (%2)").arg(m_deviceSerial).arg(m_platformName));

    // 设置固定大小
    setFixedSize(400, 300);

    // 在 label_sta 中显示设备信息
    QString infoText = QString("设备号: %1\n"
                               "平台: %2\n"
                               "当前状态: %3\n"
                               "请输入登录信息：")
                           .arg(m_deviceSerial)
                           .arg(m_platformName)
                           .arg(m_currentStatus);
    ui->label_sta->setText(infoText);

    // 设置输入框占位符
    ui->lineEdit_ac->setPlaceholderText("请输入账号");
    ui->lineEdit_pwd->setPlaceholderText("请输入密码");
 //   ui->lineEdit_pwd->setEchoMode(QLineEdit::Password);  // 密码模式
    ui->lineEdit_code->setPlaceholderText("请输入验证码");

    // 连接按钮信号（如果还没有在UI设计器中连接）
    connect(ui->pushButton_loggin, &QPushButton::clicked,
            this, &applogin2::on_pushButton_loggin_clicked);
    connect(ui->pushButton_sendcode, &QPushButton::clicked,
            this, &applogin2::on_pushButton_sendcode_clicked);
}

// 获取账号
QString applogin2::getAccount() const
{
    return ui->lineEdit_ac->text().trimmed();
}

// 获取密码
QString applogin2::getPassword() const
{
    return ui->lineEdit_pwd->text().trimmed();
}

// 获取验证码
QString applogin2::getVerifyCode() const
{
    return ui->lineEdit_code->text().trimmed();
}

void applogin2::on_pushButton_sendcode_clicked()
{
    // 获取账号和密码
    QString account = getAccount();
    QString password = getPassword();

    // 打印到控制台
    qDebug() << "========================================";
    qDebug() << "【获取验证码按钮点击】";
    qDebug() << "设备号:" << m_deviceSerial;
    qDebug() << "平台名称:" << m_platformName;
    qDebug() << "账号:" << account;
    qDebug() << "密码:" << password;
    qDebug() << "========================================";

    // 检查账号密码是否为空
    if (account.isEmpty()) {
        ui->label_sta->setText("错误: 账号不能为空");
        QMessageBox::warning(this, "警告", "请输入账号！");
        ui->lineEdit_ac->setFocus();
        return;
    }

    if (password.isEmpty()) {
        ui->label_sta->setText("错误: 密码不能为空");
        QMessageBox::warning(this, "警告", "请输入密码！");
        ui->lineEdit_pwd->setFocus();
        return;
    }

    // 发射信号
    emit sendCodeRequested(m_deviceSerial, m_platformName, account, password);

    // 更新状态显示
    QString statusText = QString("验证码已发送到账号: %1\n"
                                 "设备: %2, 平台: %3")
                             .arg(account)
                             .arg(m_deviceSerial)
                             .arg(m_platformName);
    ui->label_sta->setText(statusText);

    // 启用验证码输入框
    ui->lineEdit_code->setEnabled(true);
    ui->lineEdit_code->setFocus();

    // 这里可以添加实际的验证码发送逻辑
    // 例如: 调用API发送验证码、开始倒计时等
}

void applogin2::on_pushButton_loggin_clicked()
{
    // 获取账号、密码和验证码
    QString account = getAccount();
    QString password = getPassword();
    QString verifyCode = getVerifyCode();

    // 打印到控制台
    qDebug() << "========================================";
    qDebug() << "【登录按钮点击】";
    qDebug() << "设备号:" << m_deviceSerial;
    qDebug() << "平台名称:" << m_platformName;
    qDebug() << "账号:" << account;
    qDebug() << "密码:" << password;
    qDebug() << "验证码:" << verifyCode;
    qDebug() << "========================================";

    // 检查输入是否完整
    if (account.isEmpty()) {
        ui->label_sta->setText("错误: 账号不能为空");
        QMessageBox::warning(this, "警告", "请输入账号！");
        ui->lineEdit_ac->setFocus();
        return;
    }

    if (password.isEmpty()) {
        ui->label_sta->setText("错误: 密码不能为空");
        QMessageBox::warning(this, "警告", "请输入密码！");
        ui->lineEdit_pwd->setFocus();
        return;
    }

    if (verifyCode.isEmpty()) {
        ui->label_sta->setText("错误: 验证码不能为空");
        QMessageBox::warning(this, "警告", "请输入验证码！");
        ui->lineEdit_code->setFocus();
        return;
    }

    // 发射信号
    emit loginRequested(m_deviceSerial, m_platformName, account, password, verifyCode);

    // 更新状态显示
    QString statusText = QString("正在登录...\n"
                                 "设备: %1\n"
                                 "平台: %2\n"
                                 "账号: %3")
                             .arg(m_deviceSerial)
                             .arg(m_platformName)
                             .arg(account);
    ui->label_sta->setText(statusText);

    // 禁用按钮，防止重复点击
    ui->pushButton_loggin->setEnabled(false);
    ui->pushButton_sendcode->setEnabled(false);

    // 这里可以添加实际的登录逻辑
    // 例如: 调用API验证账号密码和验证码

    // 模拟登录成功（2秒后）
    QTimer::singleShot(2000, this, [this, account]() {
        QString successText = QString("登录成功！\n"
                                      "设备: %1\n"
                                      "平台: %2\n"
                                      "账号: %3")
                                  .arg(m_deviceSerial)
                                  .arg(m_platformName)
                                  .arg(account);
        ui->label_sta->setText(successText);

        QMessageBox::information(this, "成功", "登录成功！");

        // 关闭窗口
        accept();
    });
}
