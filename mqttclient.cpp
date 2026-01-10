#include "mqttclient.h"
#include "publicheader.h"
#include <QTimer>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicName>

mqttclient::mqttclient(DatabaseManager *db,
                       const QString &hostname,
                       int port,
                       const QString &clientId)
    : mqttClient(new QMqttClient(this))
    , dbManager(db)
    , m_hostname(hostname)
    , m_port(port)
    , m_clientId(clientId)
{
    connect(mqttClient, &QMqttClient::messageReceived, this, &mqttclient::onMessageReceived);
    connect(mqttClient, &QMqttClient::stateChanged, this, &mqttclient::onStateChanged);

    connectToBroker();

    QList<SQL_Device> devices = dbManager->getAllDevices();
    for (const SQL_Device &device : std::as_const(devices)) {
        ADDsubscribeTopic(device.serial_number);
    }
    // // 初始化定时器
    // initTimer();

    // // 连接信号槽
    // connect(mqttClient, &QMqttClient::messageReceived,
    //         this, &mqttclient::onMessageReceived);
    // connect(mqttClient, &QMqttClient::stateChanged,
    //         this, &mqttclient::onStateChanged);
}

void mqttclient::setConnectionParams(const QString &hostname, int port, const QString &clientId)
{
    m_hostname = hostname;
    m_port = port;
    m_clientId = clientId;

    // 如果已连接，断开并重新连接
    if (mqttClient->state() == QMqttClient::Connected) {
        mqttClient->disconnectFromHost();
        QTimer::singleShot(1000, this, &mqttclient::connectToBroker);
    }
}


void mqttclient::connectToBroker()
{
    qDebug() << __FUNCTION__;
    mqttClient->setHostname(m_hostname);
    mqttClient->setPort(m_port);
    mqttClient->setClientId(m_clientId);

    mqttClient->connectToHost();
}

void mqttclient::reconnectBroker()
{
    if (mqttClient->state() == QMqttClient::Connected) {
        mqttClient->disconnectFromHost();
    }
    connectToBroker();
}

void mqttclient::subscribeToTopic(QString topic)
{
    if (mqttClient->state() == QMqttClient::Connected) {
        QMqttTopicFilter topicFilter(topic);
        mqttClient->subscribe(topicFilter);
     //   qDebug() << "topic :" <<topic << "scribed!";
    }
}

void mqttclient::subscribeALLTopic()
{
    for (const QString &topic : std::as_const(topicList)) {
        subscribeToTopic(topic);
    }
}

void mqttclient::publishMessage(QMqttTopicName topicName, QByteArray message)
{
    if (mqttClient->state() == QMqttClient::Connected) {
        mqttClient->publish(topicName, message, 0, false);
    } else {
        qDebug() << "Failed to publish message, MQTT client is not connected.";
    }
}

void mqttclient::ADDsubscribeTopic(QString device)
{
    QString topic = "Device/Report/" + device;
    topicList.append(topic);
    subscribeToTopic(topic);
}
void mqttclient::SingleTopicPub(QString deviceSerial ,QString msg)
{
    QString topic = "Device/Dispatch/" + deviceSerial;
    QByteArray message = msg.toUtf8();
    QMqttTopicName topicName(topic);

    if (mqttClient->state() == QMqttClient::Connected) {
        qDebug()<<topicName << "::  " << message;
        mqttClient->publish(topicName, message, 0, false);

    } else {
        qDebug() << "Failed to publish message, MQTT client is not connected.";
    }
}
void mqttclient::CommandMuiltSend(QJsonObject json)
{
    qDebug() << "=== 开始转发指令到多个设备 ===";

    // 检查必需的字段
    if (!json.contains("data")) {
        qDebug() << "错误：JSON缺少data字段";
        return;
    }

    QJsonObject dataObj = json["data"].toObject();

    // 检查必需的数据字段
    if (!dataObj.contains("command_id") ||
        !dataObj.contains("action") ||
        !dataObj.contains("serial_numbers")) {
        qDebug() << "错误：data字段缺少必需的command_id、action或serial_numbers";
        qDebug() << "收到的JSON：" << json;
        return;
    }

    QString commandId = dataObj["command_id"].toString();
    QJsonArray serial_numbers = dataObj["serial_numbers"].toArray();

    qDebug() << "指令ID:" << commandId;
    qDebug() << "目标设备数量:" << serial_numbers.size();

    // 验证指令是否存在（可选）
    if (dbManager) {
        SQL_CommandHistory cmd = dbManager->getCommandById(commandId);
        if (cmd.commandId.isEmpty()) {
            qDebug() << "警告：指令ID" << commandId << "在数据库中不存在";
            // 根据需求决定是否继续发送
        }
    }

    // 准备发送的消息（使用原JSON，只确保格式正确）
    // 确保JSON格式符合设备要求
    QJsonObject newJson;

    // 复制原始数据（保持指令ID等信息不变）
    QJsonObject sendData;
    sendData["command_id"] = commandId;
    sendData["action"] = dataObj["action"].toString();
    sendData["sub_action"] = dataObj.value("sub_action").toString();  // 使用value避免不存在时报错
    sendData["start_time"] = dataObj.value("start_time").toString();
    sendData["end_time"] = dataObj.value("end_time").toString();
    sendData["remark"] = dataObj.value("remark").toString();

    // 移除serial_numbers字段，设备不需要这个
    // sendData.remove("serial_numbers");

    newJson["data"] = sendData;
    newJson["messageType"] = "command";
    newJson["password"] = "securePassword123";
    newJson["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    newJson["username"] = "user123";

    // 转换为JSON字符串
    QByteArray message = QJsonDocument(newJson).toJson();

    qDebug() << "指令JSON格式验证通过";
    qDebug() << "发送的消息格式:" << newJson;

    // 发送到每个设备
    int sentCount = 0;
    for (const QJsonValue &serialNumber : std::as_const(serial_numbers)) {
        QString deviceSerial = serialNumber.toString().trimmed();
        if (deviceSerial.isEmpty()) {
            qDebug() << "跳过空的设备序列号";
            continue;
        }

        QString topic = "Device/Dispatch/" + deviceSerial;
        QMqttTopicName topicName(topic);

        qDebug() << "发送到设备" << deviceSerial << "主题:" << topic;

        // 检查MQTT连接状态
        if (mqttClient->state() != QMqttClient::Connected) {
            qDebug() << "错误：MQTT客户端未连接，无法发送指令";
            break;
        }

        publishMessage(topicName, message);
        sentCount++;

        // 短暂延迟，避免发送过快
        QThread::msleep(10);
    }

    qDebug() << "指令转发完成，成功发送到" << sentCount << "个设备";
    // 可选：更新指令状态为发送中
    if (dbManager && sentCount > 0) {
        // 将指令状态更新为sending或executing
        SQL_CommandHistory cmd;
        cmd.commandId = commandId;
        cmd.status = "execing";  // 或 "sending"
        // 从数据库获取原有的完整数据
        SQL_CommandHistory originalCmd = dbManager->getCommandById(commandId);
        cmd.action = originalCmd.action;
        cmd.sub_action = originalCmd.sub_action;
        cmd.start_time = originalCmd.start_time;
        cmd.end_time = originalCmd.end_time;
        cmd.remark = originalCmd.remark;
        cmd.completeness = originalCmd.completeness;
        cmd.completed_url = originalCmd.completed_url;
        cmd.total_tasks = originalCmd.total_tasks;
        cmd.completed_tasks = originalCmd.completed_tasks;
        if (!dbManager->updateCommandHistory(cmd)) {
            qDebug() << "警告：更新指令状态失败";
        } else {
            qDebug() << "指令状态已更新为executing";
        }
    }
}

void mqttclient::ProcessDevtSend(QJsonObject json)
{
    QString serial_number = json["serial_number"].toString();
    QByteArray message = QJsonDocument(json).toJson();
    QString topic = "Device/Dispatch/" + serial_number;
    QMqttTopicName topicName(topic);
    publishMessage(topicName, message);
}
void mqttclient::devUpgrade(QStringList upgradeList)
{
    qDebug() << "=== 开始处理设备升级请求 ===";

    if (upgradeList.isEmpty()) {
        qDebug() << "升级列表为空";
        return;
    }

    // 第一个元素是固件文件名
    QString firmwareFile = upgradeList.first();
    qDebug() << "固件文件:" << firmwareFile;

    // 从第二个元素开始是设备列表
    QStringList deviceList = upgradeList.mid(1);
    qDebug() << "需要升级的设备数量:" << deviceList.size();
    qDebug() << "设备列表:" << deviceList;

    if (deviceList.isEmpty()) {
        qDebug() << "没有需要升级的设备";
        return;
    }

    // 1. 计算固件文件的MD5
    QString firmwarePath = QDir::current().filePath("Download/" + firmwareFile);
    QFile file(firmwarePath);

    if (!file.exists()) {
        qDebug() << "固件文件不存在:" << firmwarePath;
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "无法打开固件文件:" << firmwarePath;
        return;
    }

    QCryptographicHash hash(QCryptographicHash::Md5);
    if (hash.addData(&file)) {
        QByteArray md5Hash = hash.result().toHex().toLower();  // 转换为小写
        QString md5String = QString::fromLatin1(md5Hash);
        qDebug() << "固件文件MD5:" << md5String;
        file.close();

        // 2. 构建升级JSON
        QJsonObject upgradeJson;
        QJsonObject dataObj;

        // 构建URL，假设服务器地址为192.168.10.103:8080
        QString url = QString("http://192.168.10.103:8080/download?filename=%1").arg(firmwareFile);
        dataObj["url"] = url;
        dataObj["md5"] = md5String;

        upgradeJson["data"] = dataObj;
        upgradeJson["messageType"] = "upgrade";
        upgradeJson["password"] = "securePassword123";
        upgradeJson["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        upgradeJson["username"] = "user123";

        // 转换为JSON字符串
        QJsonDocument doc(upgradeJson);
        QString jsonString = doc.toJson(QJsonDocument::Indented);

        qDebug() << "\n=== 升级指令JSON ===";
        qDebug() << jsonString;

        // 3. 打印需要发送的设备列表
        qDebug() << "\n=== 需要升级的设备列表 ===";
        for (int i = 0; i < deviceList.size(); ++i) {
            qDebug() << QString("%1. %2").arg(i+1).arg(deviceList[i]);
        }

        // 4. 打印发送信息（实际项目中可以取消注释下面的代码来真正发送）
        qDebug() << "\n=== 模拟发送升级指令 ===";
        qDebug() << "固件文件:" << firmwareFile;
        qDebug() << "下载URL:" << url;
        qDebug() << "MD5校验码:" << md5String;
        qDebug() << "目标设备数量:" << deviceList.size();


        // 实际发送代码（注释状态）
        for (const QString &deviceSerial : deviceList) {
        QString topic = "Device/Dispatch/" + deviceSerial;
        QMqttTopicName topicName(topic);
        qDebug() << topicName << " : " << jsonString.toUtf8() ;
        publishMessage(topicName, jsonString.toUtf8());
        }


        qDebug() << "=== 升级指令处理完成 ===";

    } else {
        qDebug() << "计算MD5失败";
        file.close();
    }
}
void mqttclient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject jsonObj = doc.object();
   // qDebug() << "onMessageReceived called : " << topic<< "msg:" <<message;
    if (!jsonObj.isEmpty()) {
        QString messageType = getMessageType(jsonObj);

        if (messageType == "") {
            qDebug() << "Error: messageType is empty!";
            return;
        } else if (messageType == "heart") {
            DeviceStatus deviceStatus = parseJsonHeartBeat(jsonObj);
            emit updateDeviceInfo(deviceStatus);
        } else if (messageType == "appliaction_status") {
            // 处理应用状态消息
            handleApplicationStatus(jsonObj);
        }
        else {
            qDebug() << "Error: messageType is unknown!";
                     qDebug() << topic << "\n" <<message;
        }
    } else {
        qDebug() << "error: not a json ...";
         qDebug() << topic << "\n" <<message;
    }
}

void mqttclient::onStateChanged(QMqttClient::ClientState state)
{
    if (state == QMqttClient::Connected) {
        qDebug() << "MQTT client connected successfully. Host:" << m_hostname << "Port:" << m_port;
        subscribeALLTopic();
        emit mqttclientconnted(true);
    } else if (state == QMqttClient::Disconnected) {
        qDebug() << "MQTT client disconnected. Attempting to reconnect...";
        emit mqttclientconnted(false);
        // 3秒后尝试重连
        QTimer::singleShot(3000, this, &mqttclient::reconnectBroker);
    } else {
        qDebug() << "MQTT client state changed:" << state;
    }
}

DeviceStatus mqttclient::parseJsonHeartBeat(const QJsonObject& jsonObj)
{
    QString serialNumber = jsonObj["serial_number"].toString();
    QString status = jsonObj["messageType"].toString();
    QString location = "";
    QString currentAction = jsonObj["data"]["current_action"]["name"].toString();
    QString current_start = jsonObj["data"]["current_action"]["start_time"].toString();
    QString current_end = jsonObj["data"]["current_action"]["end_time"].toString();
    QString next_action = jsonObj["data"]["next_action"]["name"].toString();
    QString next_action_start = jsonObj["data"]["next_action"]["start_time"].toString();
    QString next_action_end = jsonObj["data"]["next_action"]["end_time"].toString();
    QString ip = jsonObj["data"]["ip"].toString();
    QString trafficStatistics = jsonObj["data"]["traffic_statistics"].toString();
    QString lastHeartbeat = jsonObj["timestamp"].toString();
    QString usedProcess = jsonObj["data"]["usedProcess"].toString();
    QString ProcessID = jsonObj["data"]["ProcessID"].toString();
    QString HardVersion = jsonObj["data"]["firmware_version"].toString();
    QString CheckSum = jsonObj["verification_code"].toString();

    float temp = jsonObj["data"]["temperature"].toDouble(0.0);

    DeviceStatus deviceStatus(serialNumber, status, location, currentAction,
                              trafficStatistics, lastHeartbeat, ip,
                              current_start, current_end, next_action,
                              next_action_start, next_action_end, usedProcess, ProcessID, temp);
    deviceStatus.hardversion  =HardVersion;
    deviceStatus.checksum  =CheckSum;

    return deviceStatus;
}

QString mqttclient::getMessageType(const QJsonObject& jsonObj)
{
    if (jsonObj.contains("messageType") && !jsonObj["messageType"].toString().isEmpty()) {
        return jsonObj["messageType"].toString();
    } else {
        return "";
    }
}

void mqttclient::run()
{
    while (true) {
        sleep(1);
    }
}
QString mqttclient::extractAccountUsingSplit(const QString& remark)
{
    // 按空格分割字符串
    QStringList parts = remark.split(" ", Qt::SkipEmptyParts);

    for (const QString& part : parts) {
        // 查找以"ID:"开头且包含":ID"的部分
        if (part.startsWith("ID:") && part.contains(":ID")) {
            // 去掉开头的"ID:"和结尾的":ID"
            QString account = part.mid(3);  // 去掉"ID:"
            account = account.left(account.length() - 3);  // 去掉":ID"
            return account;
        }
    }

    return "";
}

void mqttclient::handleApplicationStatus(const QJsonObject &jsonObj)
{
    qDebug() << "=== 处理应用状态消息 ===";

    // 解析消息
    QString serialNumber = jsonObj["serial_number"].toString();
    QString verificationCode = jsonObj["verification_code"].toString();
    QString timestamp = jsonObj["timestamp"].toString();

    QJsonObject dataObj = jsonObj["data"].toObject();
    QString appName = dataObj["app_name"].toString();
    QString commandId = dataObj["command_id"].toString();
    QString status = dataObj["status"].toString();

    qDebug() << "设备序列号:" << serialNumber;
    qDebug() << "验证码:" << verificationCode;
    qDebug() << "时间戳:" << timestamp;
    qDebug() << "应用名称:" << appName;
    qDebug() << "指令ID:" << commandId;
    qDebug() << "状态:" << status;

    // 验证设备校验码
    if (!verifyDeviceChecksum(serialNumber, verificationCode)) {
        qDebug() << "设备校验码验证失败";
        return;
    }

    if (!dbManager) {
        qDebug() << "数据库管理器不可用";
        return;
    }

    // 处理指令任务统计
    if (!commandId.isEmpty()) {
        if (status == "finish") {
           SQL_CommandHistory cmd =  dbManager->getCommandById(commandId);
            qDebug() << "查看一下remark:" << cmd.remark;

   //        if()

           if(cmd.remark.contains("MARK:LOGGIN_APP:MARK") )
            {
                QString account = extractAccountUsingSplit(cmd.remark);
                   if (appName == "抖音" || appName.toLower() == "tiktok") {
                    dbManager->updateDeviceAppStatus(serialNumber, "抖音", account);
                } else if (appName == "BILIBILI" || appName.toLower() == "bilibili") {
                    dbManager->updateDeviceAppStatus(serialNumber, "BILIBILI", account);
                } else if (appName == "小红书" || appName.toLower() == "xhs") {
                    dbManager->updateDeviceAppStatus(serialNumber, "小红书", account);
                } else if (appName == "微博" || appName.toLower() == "weibo") {
                    dbManager->updateDeviceAppStatus(serialNumber, "微博", account);
                } else if (appName == "快手" || appName.toLower() == "kuaishou") {
                    dbManager->updateDeviceAppStatus(serialNumber, "快手", account);
                }
                emit applogginstatus(commandId,true);
           }
           else if(cmd.remark.contains("MARK:CRCODE_LOGGIN:MARK") )
           {
               QString account = "CRCODE";
               if (appName == "抖音" || appName.toLower() == "tiktok") {
                   dbManager->updateDeviceAppStatus(serialNumber, "抖音", account);
               } else if (appName == "BILIBILI" || appName.toLower() == "bilibili") {
                   dbManager->updateDeviceAppStatus(serialNumber, "BILIBILI", account);
               } else if (appName == "小红书" || appName.toLower() == "xhs") {
                   dbManager->updateDeviceAppStatus(serialNumber, "小红书", account);
               } else if (appName == "微博" || appName.toLower() == "weibo") {
                   dbManager->updateDeviceAppStatus(serialNumber, "微博", account);
               } else if (appName == "快手" || appName.toLower() == "kuaishou") {
                   dbManager->updateDeviceAppStatus(serialNumber, "快手", account);
               }
               emit applogginstatus(commandId,true);
           }
            // 成功任务增加
            bool taskIncremented = dbManager->incrementCommandCompletedTasks(commandId);
            if (taskIncremented) {
                qDebug() << "指令成功任务数增加成功";


            } else {
                qDebug() << "指令成功任务数增加失败";
            }
        } else {
            // 失败任务增加
            bool failedIncremented = dbManager->incrementCommandFailedTasks(commandId);
            if (failedIncremented) {
                qDebug() << "指令失败任务数增加成功";
                SQL_CommandHistory cmd =  dbManager->getCommandById(commandId);
                if(cmd.remark.contains("MARK:LOGGIN_APP:MARK"))
                {
                    emit applogginstatus(commandId,false);

                }


                if(status == "unlogin")
                {
                    if (appName == "抖音" || appName.toLower() == "tiktok") {
                        dbManager->updateDeviceAppStatus(serialNumber, "抖音", "未登录");
                    } else if (appName == "BILIBILI" || appName.toLower() == "bilibili") {
                        dbManager->updateDeviceAppStatus(serialNumber, "BILIBILI", "未登录");
                    } else if (appName == "小红书" || appName.toLower() == "xhs") {
                        dbManager->updateDeviceAppStatus(serialNumber, "小红书", "未登录");
                    } else if (appName == "微博" || appName.toLower() == "weibo") {
                        dbManager->updateDeviceAppStatus(serialNumber, "微博", "未登录");
                    } else if (appName == "快手" || appName.toLower() == "kuaishou") {
                        dbManager->updateDeviceAppStatus(serialNumber, "快手", "未登录");
                    }
                }
            } else {
                qDebug() << "指令失败任务数增加失败";
            }
        }
    } else {
        qDebug() << "指令ID为空，跳过任务统计";
    }

    // 可选：发送信号通知界面更新
    // if (!commandId.isEmpty()) {
    //     emit commandTaskUpdated(commandId);
    // }

    qDebug() << "=== 应用状态消息处理完成 ===";
}

// 新增：验证设备校验码
bool mqttclient::verifyDeviceChecksum(const QString &serialNumber, const QString &verificationCode)
{
    if (!dbManager) {
        qDebug() << "数据库管理器不可用，跳过校验";
        return true; // 如果不校验，返回true
    }

    // 从数据库获取设备信息
    SQL_Device device = dbManager->getDeviceBySerialNumber(serialNumber);

    if (device.serial_number.isEmpty()) {
        qDebug() << "设备不存在:" << serialNumber;
        return false;
    }

    // 比较校验码
    bool isValid = (device.checksum == verificationCode);

    if (!isValid) {
        qDebug() << "校验码不匹配";
        qDebug() << "设备校验码:" << device.checksum;
        qDebug() << "接收校验码:" << verificationCode;
    } else {
        qDebug() << "校验码验证通过";
    }

    return isValid;
}
// 初始化定时器
void mqttclient::initTimer()
{
    m_tenSecondTimer = new QTimer(this);
    m_tenSecondTimer->setInterval(10000);  // 10秒 = 10000毫秒

    // 连接定时器信号到槽函数
    connect(m_tenSecondTimer, &QTimer::timeout,
            this, &mqttclient::tenSecondTimerFunction);

    // 启动定时器
    m_tenSecondTimer->start();
}

// 十秒定时函数
void mqttclient::tenSecondTimerFunction()
{

}
