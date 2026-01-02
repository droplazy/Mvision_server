#include "mqttclient.h"
#include "publicheader.h"
#include <QTimer>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicName>

mqttclient::mqttclient(DatabaseManager *db):mqttClient(new QMqttClient(this)),  dbManager(db)
{
            connect(mqttClient, &QMqttClient::messageReceived, this, &mqttclient::onMessageReceived);
            connect(mqttClient, &QMqttClient::stateChanged, this, &mqttclient::onStateChanged);  // 连接状态变化信号

            connectToBroker();   // 连接到 MQTT 代理

            QList<SQL_Device> devices = dbManager->getAllDevices();
            // 假设 SQL_Device 结构体包含 id 和 name
            for (const SQL_Device &device : std::as_const(devices))
            {
                ADDsubscribeTopic(device.serial_number);
            }
}

// mqttclient::mqttclient(DatabaseManager *db, QObject *parent ) :  dbManager(db)
// {
//         connect(mqttClient, &QMqttClient::messageReceived, this, &mqttclient::onMessageReceived);
//         connect(mqttClient, &QMqttClient::stateChanged, this, &mqttclient::onStateChanged);  // 连接状态变化信号

//         connectToBroker();   // 连接到 MQTT 代理

//         QList<SQL_Device> devices = dbManager->getAllDevices();
// }

void mqttclient::run()
{
  //  connectToBroker();   // 连接到 MQTT 代理
    while (true)
    {
    //    publishMessage();
        sleep(1);
    }
    // // 每3秒发布一次消息
    // QTimer *timer = new QTimer(this);
    // connect(timer, &QTimer::timeout, this, &mqttclient::publishMessage);
    // timer->start(200);  // 设置定时器每3秒触发一次
}
void mqttclient::connectToBroker()
{

    qDebug() << __FUNCTION__;
    mqttClient->setHostname("192.168.10.103");  // 设置MQTT代理地址
    mqttClient->setPort(1883);  // 设置MQTT代理端口

    // 设置客户端ID为 "123"
    mqttClient->setClientId("pcpcpc222333");  // 设置 clientId（client name）

    // 连接到代理之前，连接stateChanged信号到槽函数
    connect(mqttClient, &QMqttClient::stateChanged, this, &mqttclient::onStateChanged);

    mqttClient->connectToHost();  // 连接到代理
}

void mqttclient::subscribeToTopic(QString topic)
{
   // QString topic=
    if (mqttClient->state() == QMqttClient::Connected) {
        QMqttTopicFilter topicFilter(topic);  // 使用QMqttTopicFilter类型的主题过滤器
        mqttClient->subscribe(topicFilter);  // 订阅主题
        qDebug() << "topic:" << topic << "scbscribed!   >>>> ...... ";
    }
}
//     else {
//         qDebug() << "Failed to subscribe, MQTT client is not connected.";
//     }
// }

void mqttclient::subscribeALLTopic()
{
    // 遍历所有的主题并调用 subscribeToTopic 订阅
    for (const QString &topic : std::as_const(topicList)) {
        subscribeToTopic(topic);
    }
}
void mqttclient::publishMessage(QMqttTopicName topicName, QByteArray message)
{
    if (mqttClient->state() == QMqttClient::Connected) {

        // QByteArray message = "来自PC端";  // 消息内容
        // QMqttTopicName topicName("device/YDAT250701000007");
        mqttClient->publish(topicName, message, 0, false);  // 发布消息到 test/topic123 主题
    }
    else {
        qDebug() << "Failed to publish message, MQTT client is not connected.";
    }
}

void mqttclient::ADDsubscribeTopic(QString device)
{
    QString topic  = "Device/Report/" +device;
    topicList.append(topic);
    subscribeToTopic(topic);
}
void mqttclient::CommandMuiltSend(QJsonObject json)
{

  //  qDebug() << "recive :" << json;
    // 解析传入的 JSON 对象
    QJsonObject dataObj = json["data"].toObject();
    QString action = dataObj["action"].toString();
    QString sub_action = dataObj["sub_action"].toString();
    QString start_time = dataObj["start_time"].toString();
    QString end_time = dataObj["end_time"].toString();
    QString remark = dataObj["remark"].toString();

    QJsonArray serial_numbers = dataObj["serial_numbers"].toArray();

    // 构建新的 message 内容
    QJsonObject newJson;
    newJson["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    newJson["username"] = "user123";
    newJson["password"] = "securePassword123";
    newJson["messageType"] = "command";

    QJsonObject data;
    data["action"] = action;
    data["sub_action"] = sub_action;
    data["start_time"] = start_time;
    data["end_time"] = end_time;
    data["remark"] = remark;

    newJson["data"] = data;
/**********************************指令数据库操作*/
 //插入新指令
SQL_CommandHistory cmd1;
cmd1.commandId = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
cmd1.status = "todo";
cmd1.action = action;
cmd1.sub_action = sub_action;
cmd1.start_time =start_time;
cmd1.end_time =  end_time;
cmd1.remark = remark;
cmd1.Completeness = "0%";
cmd1.completed_url = "";
dbManager->insertCommandHistory(cmd1);
QByteArray message = QJsonDocument(newJson).toJson();

    // 发布消息，主题是 /device/serial_number，序列号逐个变化
    for (const QJsonValue &serialNumber : std::as_const(serial_numbers)) {
        QString topic = "Device/Dispatch/" + serialNumber.toString();
        QMqttTopicName topicName(topic);
        publishMessage(topicName, message);
    //    qDebug() << "Publishing message to topic:" << topicName.name() << "\n" << message;
    }
}

void mqttclient::ProcessDevtSend(QJsonObject json)
{

    //  qDebug() << "recive :" << json;
    // 解析传入的 JSON 对象serial_number

      QString serial_number = json["serial_number"].toString();
    QByteArray message = QJsonDocument(json).toJson();
        QString topic = "Device/Dispatch/" + serial_number;
        QMqttTopicName topicName(topic);
        publishMessage(topicName, message);
}

void mqttclient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{

    // 处理收到的消息
//     QString string = QString::fromUtf8(message);
// //    qDebug() << "Received message on topic" << topic.name() << ":" << string;

    QJsonDocument doc = QJsonDocument::fromJson(message);
    QJsonObject jsonObj = doc.object();
    if (!jsonObj.isEmpty()) {
        QString messageType = getMessageType(jsonObj);

        if (messageType == "") {
            qDebug() << "Error: messageType is empty!";
            return;
        }
        else if (messageType == "heart")
        {
            //  parseJsonHeartBeat(jsonObj);
            DeviceStatus deviceStatus = parseJsonHeartBeat(jsonObj);
            emit updateDeviceInfo(deviceStatus);
            // 打印结果
         //   deviceStatus.printInfo();
        } else {
            qDebug() << "Error: messageType is unknow!";
        }
    }
    else
    {
        qDebug() << "error :not a json ..... ";
    }
}

// 处理连接状态变化
void mqttclient::onStateChanged(QMqttClient::ClientState state)
{
    if (state == QMqttClient::Connected) {
        qDebug() << "MQTT client connected successfully.";
        subscribeALLTopic();

    } else {
        qDebug() << "MQTT client disconnected. State: " << state;
    }
}
// 解析 JSON 数据并创建 DeviceStatus 结构体实例
DeviceStatus mqttclient::parseJsonHeartBeat(const QJsonObject& jsonObj)
{
    // 获取设备数据
    QString serialNumber = jsonObj["serial_number"].toString();
    QString status = jsonObj["messageType"].toString();  // 假设消息类型为设备状态
    QString location = "";  // 此处没有提供数据，留空
    QString currentAction = jsonObj["data"]["current_action"]["name"].toString();
    QString current_start = jsonObj["data"]["current_action"]["start_time"].toString();
    QString current_end = jsonObj["data"]["current_action"]["end_time"].toString();
    QString next_action = jsonObj["data"]["next_action"]["name"].toString();
    QString next_action_start = jsonObj["data"]["next_action"]["start_time"].toString();
    QString next_action_end = jsonObj["data"]["next_action"]["end_time"].toString();
    QString ip = jsonObj["data"]["ip"].toString();
    QString trafficStatistics = jsonObj["data"]["traffic_statistics"].toString();
    QString lastHeartbeat = jsonObj["timestamp"].toString();  // 使用 timestamp 作为最后心跳时间
    QString usedProcess = jsonObj["data"]["usedProcess"].toString();  // 使用 timestamp 作为最后心跳时间
    QString ProcessID = jsonObj["data"]["ProcessID"].toString();  // 使用 timestamp 作为最后心跳时间
    float temp = jsonObj["data"]["temperature"].toDouble(0.0);  // 使用 timestamp 作为最后心跳时间

    // 创建 DeviceStatus 实例并返回
    DeviceStatus deviceStatus(serialNumber, status, location, currentAction,
                              trafficStatistics, lastHeartbeat, ip,
                              current_start, current_end, next_action,
                              next_action_start, next_action_end,usedProcess,ProcessID,temp);
    return deviceStatus;
}
QString mqttclient::getMessageType(const QJsonObject& jsonObj) {
    // 检查 json 是否包含 messageType 字段
    if (jsonObj.contains("messageType") && !jsonObj["messageType"].toString().isEmpty()) {
        return jsonObj["messageType"].toString();
    } else {
        // 如果没有 messageType 或为空，则返回空字符串
        return "";
    }
}
