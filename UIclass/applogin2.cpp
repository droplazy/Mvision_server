#include "applogin2.h"
#include "ui_applogin2.h"
#include <QDebug>
#include <QMessageBox>
#include <QTimer>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QDir>

applogin2::applogin2(const QString& deviceSerial,
                     const QString& platformName,
                     const QString& currentStatus,
                     const QString& commandId,
                      DatabaseManager *p_db,
                     QWidget *parent)
    : QDialog(parent)
    , m_commandId(commandId)
    , ui(new Ui::applogin2)
    , m_deviceSerial(deviceSerial)
    , m_platformName(platformName)
    , m_currentStatus(currentStatus)
    , m_account("")
    , m_dbManager(p_db)
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
    setFixedSize(580, 275);

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
    // connect(ui->pushButton_loggin, &QPushButton::clicked,
    //         this, &applogin2::on_pushButton_loggin_clicked);
    // connect(ui->pushButton_sendcode, &QPushButton::clicked,
    //         this, &applogin2::on_pushButton_sendcode_clicked);
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
    insertCommandFromJson(jsonString);

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
// 从JSON中解析命令信息并插入数据库
void applogin2::insertCommandFromJson(const QString& jsonString)
{
    if (!m_dbManager) {
        qDebug() << "数据库指针为空，无法插入命令历史";
        return;
    }

    QJsonParseError parseError;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(jsonString.toUtf8(), &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析错误:" << parseError.errorString();
        return;
    }

    if (!jsonDoc.isObject()) {
        qDebug() << "JSON不是对象";
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 解析command_id，可以从根对象或data对象中获取
    QString commandId = "";
    if (jsonObj.contains("command_id")) {
        commandId = jsonObj["command_id"].toString();
    } else if (jsonObj.contains("data")) {
        QJsonObject dataObj = jsonObj["data"].toObject();
        if (dataObj.contains("command_id")) {
            commandId = dataObj["command_id"].toString();
        }
    }

    if (commandId.isEmpty()) {
        qDebug() << "未找到command_id，跳过数据库插入";
        return;
    }

    // 解析其他字段
    QString action = "";
    QString subAction = "";
    QString startTime = "00:00:00";
    QString endTime = "23:59:59";
    QString remark = "";
    QString timestamp = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    if (jsonObj.contains("data")) {
        QJsonObject dataObj = jsonObj["data"].toObject();

        if (dataObj.contains("action")) {
            action = dataObj["action"].toString();
        }

        if (dataObj.contains("sub_action")) {
            subAction = dataObj["sub_action"].toString();
        }

        if (dataObj.contains("start_time")) {
            startTime = dataObj["start_time"].toString();
        }

        if (dataObj.contains("end_time")) {
            endTime = dataObj["end_time"].toString();
        }

        if (dataObj.contains("remark")) {
            remark = dataObj["remark"].toString();
        }
    }

    // 如果从data中没找到，尝试从根对象中查找
    if (action.isEmpty() && jsonObj.contains("action")) {
        action = jsonObj["action"].toString();
    }

    if (subAction.isEmpty() && jsonObj.contains("sub_action")) {
        subAction = jsonObj["sub_action"].toString();
    }

    // 获取时间戳
    if (jsonObj.contains("timestamp")) {
        QString isoTime = jsonObj["timestamp"].toString();
        // 尝试将ISO时间转换为标准格式
        QDateTime dt = QDateTime::fromString(isoTime, Qt::ISODate);
        if (dt.isValid()) {
            timestamp = dt.toString("yyyy-MM-dd hh:mm:ss");
        }
    }

    qDebug() << "=== 解析JSON并准备插入数据库 ===";
    qDebug() << "命令ID:" << commandId;
    qDebug() << "动作:" << action;
    qDebug() << "子动作:" << subAction;
    qDebug() << "开始时间:" << startTime;
    qDebug() << "结束时间:" << endTime;
    qDebug() << "备注:" << remark;
    qDebug() << "时间戳:" << timestamp;
    qDebug() << "原始JSON:" << jsonString;

    // 创建SQL_CommandHistory对象
    SQL_CommandHistory commandHistory;
    commandHistory.commandId = commandId;
    commandHistory.status = "执行中";  // 固定为"执行中"
    commandHistory.action = action;
    commandHistory.sub_action = subAction;
    commandHistory.start_time = startTime;
    commandHistory.end_time = endTime;
    commandHistory.remark = remark;
    commandHistory.completeness = "0%";  // 初始完成度
    commandHistory.completed_url = "";
    commandHistory.total_tasks = 1;
    commandHistory.completed_tasks = 0;
    commandHistory.failed_tasks = 0;

    // 插入数据库
    bool success = m_dbManager->insertCommandHistory(commandHistory);

    if (success) {
        qDebug() << "命令历史记录插入成功";
    } else {
        qDebug() << "命令历史记录插入失败";
    }
}
QString applogin2::generateLoginCRcode()
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
    dataObject["sub_action"] = "二维码登录";
    dataObject["start_time"] = "00:00:00";
    dataObject["end_time"] = "23:59:59";
    dataObject["remark"] = QString("MARK:CRCODE_LOGGIN:MARK");

    jsonObject["data"] = dataObject;
    jsonObject["messageType"] = "command";
    jsonObject["password"] = "securePassword123";
    jsonObject["timestamp"] = getCurrentTimestamp();
    jsonObject["username"] = "user123";

    // 转换为JSON字符串
    QJsonDocument jsonDoc(jsonObject);
    return jsonDoc.toJson(QJsonDocument::Compact);
}
void applogin2::on_pushButton_CRcode_clicked()
{
    // 生成JSON
    QString jsonString = generateLoginCRcode();

    // 将命令信息插入数据库
    insertCommandFromJson(jsonString);

    // 发射信号发送JSON到MQTT
    emit sendJsonToMQTT(m_deviceSerial, jsonString);

    // 更新状态显示
    QString statusText = QString("正在获取二维码...\n"
                                 "设备: %1\n"
                                 "平台: %2\n"
                                 "请等待设备响应\n"
                                 "命令ID: %3")
                             .arg(m_deviceSerial)
                             .arg(m_platformName)
                             .arg(m_commandId);
    ui->label_sta->setText(statusText);

    // 禁用所有按钮，防止重复点击
    ui->pushButton_loggin->setEnabled(false);
    ui->pushButton_sendcode->setEnabled(false);
    ui->pushButton_CRcode->setEnabled(false);

    // 可选：也禁用输入框
    ui->lineEdit_ac->setEnabled(false);
    ui->lineEdit_pwd->setEnabled(false);
    ui->lineEdit_code->setEnabled(false);

    // 可选：设置等待提示
    ui->pushButton_CRcode->setText("获取中...");

    // // 可选：启动定时器，在一段时间后重新启用按钮（如果没收到响应）
    // QTimer::singleShot(30000, this, [this]() {
    //     if (ui->pushButton_CRcode->text() == "获取中...") {
    //         ui->pushButton_loggin->setEnabled(true);
    //         ui->pushButton_sendcode->setEnabled(true);
    //         ui->pushButton_CRcode->setEnabled(true);
    //         ui->pushButton_CRcode->setText("二维码登录");

    //         ui->lineEdit_ac->setEnabled(true);
    //         ui->lineEdit_pwd->setEnabled(true);
    //         ui->lineEdit_code->setEnabled(true);

    //         ui->label_sta->setText(QString("二维码获取超时\n"
    //                                        "设备: %1\n"
    //                                        "平台: %2\n"
    //                                        "请重试或使用其他登录方式")
    //                                    .arg(m_deviceSerial)
    //                                    .arg(m_platformName));
    //     }
    // });
}

void applogin2::onCRcodeImgReceived(const QString& commandId)
{
    // 检查是否是本窗口的命令ID
    if (commandId != m_commandId) {
        qDebug() << "命令ID不匹配，期望:" << m_commandId << "收到:" << commandId;
        return;
    }

    qDebug() << "收到二维码图片请求，命令ID:" << commandId;


    QDir currentDir = QDir::current();
    QString uploadDirPath = currentDir.filePath("Upload");

    qDebug() << "查找目录:" << uploadDirPath;

    // 检查Upload文件夹是否存在
    QDir uploadDir(uploadDirPath);
    if (!uploadDir.exists()) {
        qDebug() << "Upload文件夹不存在:" << uploadDirPath;
        QMessageBox::warning(this, "错误", "Upload文件夹不存在！");
        return;
    }

    // 查找名为commandId的文件夹
    QString targetFolderPath = uploadDirPath + "/" + commandId;
    QDir targetFolder(targetFolderPath);

    if (!targetFolder.exists()) {
        qDebug() << "目标文件夹不存在:" << targetFolderPath;
        QMessageBox::warning(this, "错误",
                             QString("找不到命令ID对应的文件夹: %1").arg(commandId));
        return;
    }

    qDebug() << "找到目标文件夹:" << targetFolderPath;

    // 获取文件夹中的所有图片文件
    QStringList imageFilters;
    imageFilters << "*.png" << "*.jpg" << "*.jpeg" << "*.bmp" << "*.gif" << "*.tiff" << "*.webp";

    QStringList imageFiles = targetFolder.entryList(imageFilters, QDir::Files);

    if (imageFiles.isEmpty()) {
        qDebug() << "文件夹中没有找到图片文件";
        QMessageBox::warning(this, "错误",
                             QString("在文件夹 %1 中没有找到图片文件").arg(commandId));
        return;
    }

    // 如果有多个图片文件，只取第一个
    QString imageFileName = imageFiles.first();
    QString imagePath = targetFolderPath + "/" + imageFileName;

    qDebug() << "找到图片文件:" << imagePath;

    // 加载图片
    QPixmap pixmap;
    if (!pixmap.load(imagePath)) {
        qDebug() << "无法加载图片:" << imagePath;
        QMessageBox::warning(this, "错误", "无法加载二维码图片！");
        return;
    }

    // 检查图片尺寸，如果太大可以调整
    qDebug() << "原始图片尺寸:" << pixmap.size();

    // 将图片缩放到label的尺寸（250x250），保持纵横比
    QPixmap scaledPixmap = pixmap.scaled(250, 250, Qt::KeepAspectRatio, Qt::SmoothTransformation);

    // 设置图片到label上
    ui->label_CRcode->setPixmap(scaledPixmap);
    ui->label_CRcode->setAlignment(Qt::AlignCenter);

    // 可选：如果图片尺寸不合适，可以添加背景色
    ui->label_CRcode->setStyleSheet("QLabel { background-color: white; }");

    qDebug() << "二维码图片已显示在label_CRcode上，尺寸:" << scaledPixmap.size();

    // 可选：显示图片路径信息
    ui->label_CRcode->setToolTip(QString("二维码图片: %1\n尺寸: %2x%3")
                                     .arg(imageFileName)
                                     .arg(pixmap.width())
                                     .arg(pixmap.height()));

    // 如果有多张图片，给出提示
    if (imageFiles.size() > 1) {
        qDebug() << "警告：文件夹中有" << imageFiles.size() << "张图片，只显示了第一张";
    }
}

