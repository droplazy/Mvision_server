#include "applogin2.h"
#include "ui_applogin2.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>


applogin2::applogin2(const QString& deviceSerial,
                     const QString& platformName,
                     const QString& currentStatus,
                     const QString& commandId,
                     QWidget *parent)
    : QDialog(parent)
    , m_commandId(commandId)
    , ui(new Ui::applogin2)
    , m_deviceSerial(deviceSerial)
    , m_platformName(platformName)
    , m_currentStatus(currentStatus)
    , m_account("")
{
    ui->setupUi(this);

    // 初始化UI
    initUI();

    // 初始状态下禁用登录按钮
    ui->pushButton_loggin->setEnabled(false);

    qDebug() << "生成的指令ID:" << m_commandId;
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
    setFixedSize(290, 275);

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
    ui->lineEdit_code->setPlaceholderText("请输入验证码");

    // 连接按钮信号
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

// 生成当前时间戳（ISO 8601格式）
QString applogin2::getCurrentTimestamp() const
{
    QDateTime currentTime = QDateTime::currentDateTimeUtc();
    return currentTime.toString(Qt::ISODate);
}

// 生成发送验证码的JSON
QString applogin2::generateSendCodeJson(const QString& account)
{
    QString platform = "";
    if(m_platformName == "Bilibili")
        platform = "BILIBILI";
    else if(m_platformName == "TikTok")
        platform = "抖音";

    // 生成JSON
    QJsonObject jsonObject;

    // data 对象
    QJsonObject dataObject;
    dataObject["command_id"] = m_commandId;
    dataObject["action"] = platform;
    dataObject["sub_action"] = "登录";
    dataObject["start_time"] = "00:00:00";
    dataObject["end_time"] = "23:59:59";
    dataObject["remark"] = QString("ID:%1:ID  MARK:LOGGIN_APP:MARK").arg(account);

    jsonObject["data"] = dataObject;
    jsonObject["messageType"] = "command";
    jsonObject["password"] = "securePassword123";
    jsonObject["timestamp"] = getCurrentTimestamp();
    jsonObject["username"] = "user123";

    // 转换为JSON字符串
    QJsonDocument jsonDoc(jsonObject);
    return jsonDoc.toJson(QJsonDocument::Compact);
}

// 生成登录的JSON
QString applogin2::generateLoginJson(const QString& account, const QString& verifyCode)
{
    QString platform = "";
    if(m_platformName == "Bilibili")
        platform = "BILIBILI";
    else if(m_platformName == "TikTok")
        platform = "抖音";

    // 生成JSON
    QJsonObject jsonObject;

    // data 对象
    QJsonObject dataObject;
    dataObject["command_id"] = m_commandId;
    dataObject["action"] = platform;
    dataObject["sub_action"] = "验证码";
    dataObject["start_time"] = "00:00:00";
    dataObject["end_time"] = "23:59:59";
    dataObject["remark"] = QString("ID:%1:ID MSG:%2:MSG MARK:LOGGIN_APP:MARK").arg(account).arg(verifyCode);

    jsonObject["data"] = dataObject;
    jsonObject["messageType"] = "command";
    jsonObject["password"] = "securePassword123";
    jsonObject["timestamp"] = getCurrentTimestamp();
    jsonObject["username"] = "user123";

    // 转换为JSON字符串
    QJsonDocument jsonDoc(jsonObject);
    return jsonDoc.toJson(QJsonDocument::Compact);
}

// 接收登录结果的槽函数
// 接收登录结果的槽函数
void applogin2::onLoginResultReceived(const QString& commandId, const QString& status, const QString& message)
{
    // 检查是否是本窗口的命令ID
    if (commandId != m_commandId) {
        return;
    }

    qDebug() << "收到登录结果 - 命令ID:" << commandId
             << "状态:" << status
             << "消息:" << message;

    // 更新UI显示
    QString resultText;

    if (status == "success" || status == "成功") {
        resultText = QString("✅ 登录成功！\n"
                             "设备: %1\n"
                             "平台: %2\n"
                             "账号: %3\n"
                             "时间: %4")
                         .arg(m_deviceSerial)
                         .arg(m_platformName)
                         .arg(m_account)
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));

        // 显示成功提示
        QMessageBox::information(this, "成功", "登录成功！");

        // 成功后延迟关闭窗口，给父窗口刷新数据的时间
        QTimer::singleShot(1500, this, [this]() {
            accept();
        });

    } else {
        resultText = QString("❌ 登录失败！\n"
                             "设备: %1\n"
                             "平台: %2\n"
                             "账号: %3\n"
                             "错误: %4\n"
                             "时间: %5")
                         .arg(m_deviceSerial)
                         .arg(m_platformName)
                         .arg(m_account)
                         .arg(message)
                         .arg(QDateTime::currentDateTime().toString("hh:mm:ss"));

        // 显示失败提示
        QMessageBox::warning(this, "失败", QString("登录失败: %1").arg(message));

        // 失败后重新启用按钮和输入框
        ui->pushButton_loggin->setEnabled(true);
        ui->pushButton_sendcode->setEnabled(true);
        ui->lineEdit_ac->setEnabled(true);
        ui->lineEdit_code->setEnabled(true);
        ui->lineEdit_code->clear(); // 清空验证码输入框
        ui->lineEdit_code->setFocus(); // 焦点回到验证码输入框
    }

    ui->label_sta->setText(resultText);
}
void applogin2::on_pushButton_sendcode_clicked()
{
    // 获取账号
    QString account = getAccount();

    // 检查账号是否为空
    if (account.isEmpty()) {
        ui->label_sta->setText("错误: 账号不能为空");
        QMessageBox::warning(this, "警告", "请输入账号！");
        ui->lineEdit_ac->setFocus();
        return;
    }

    // 保存账号
    m_account = account;

    // 生成JSON
    QString jsonString = generateSendCodeJson(account);

    // 发射信号发送JSON到MQTT
    emit sendJsonToMQTT(m_deviceSerial, jsonString);

    // 更新状态显示
    QString statusText = QString("验证码请求已发送\n"
                                 "账号: %1\n"
                                 "设备: %2\n"
                                 "平台: %3\n"
                                 "请查看手机获取验证码")
                             .arg(account)
                             .arg(m_deviceSerial)
                             .arg(m_platformName);
    ui->label_sta->setText(statusText);

    // 启用验证码输入框和登录按钮
    ui->lineEdit_code->setEnabled(true);
    ui->lineEdit_code->setFocus();
    ui->pushButton_loggin->setEnabled(true);
}

void applogin2::on_pushButton_loggin_clicked()
{
    // 获取账号和验证码
    QString account = getAccount();
    QString verifyCode = getVerifyCode();

    // 检查输入是否完整
    if (account.isEmpty()) {
        ui->label_sta->setText("错误: 账号不能为空");
        QMessageBox::warning(this, "警告", "请输入账号！");
        ui->lineEdit_ac->setFocus();
        return;
    }

    if (verifyCode.isEmpty()) {
        ui->label_sta->setText("错误: 验证码不能为空");
        QMessageBox::warning(this, "警告", "请输入验证码！");
        ui->lineEdit_code->setFocus();
        return;
    }

    // 保存账号
    m_account = account;

    // 生成JSON
    QString jsonString = generateLoginJson(account, verifyCode);

    // 发射信号发送JSON到MQTT
    emit sendJsonToMQTT(m_deviceSerial, jsonString);

    // 更新状态显示 - 显示等待提示
    QString statusText = QString("正在登录，请等待结果...\n"
                                 "设备: %1\n"
                                 "平台: %2\n"
                                 "账号: %3\n"
                                 "命令ID: %4")
                             .arg(m_deviceSerial)
                             .arg(m_platformName)
                             .arg(account)
                             .arg(m_commandId);
    ui->label_sta->setText(statusText);

    // 禁用按钮，防止重复点击
    ui->pushButton_loggin->setEnabled(false);
    ui->pushButton_sendcode->setEnabled(false);
    ui->lineEdit_ac->setEnabled(false);
    ui->lineEdit_code->setEnabled(false);
}
