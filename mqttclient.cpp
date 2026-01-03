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

void mqttclient::CommandMuiltSend(QJsonObject json)
{
    QJsonObject dataObj = json["data"].toObject();
    QString action = dataObj["action"].toString();
    QString sub_action = dataObj["sub_action"].toString();
    QString start_time = dataObj["start_time"].toString();
    QString end_time = dataObj["end_time"].toString();
    QString remark = dataObj["remark"].toString();
    QJsonArray serial_numbers = dataObj["serial_numbers"].toArray();

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

    // 插入新指令到数据库
    SQL_CommandHistory cmd1;
    cmd1.commandId = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz");
    cmd1.status = "todo";
    cmd1.action = action;
    cmd1.sub_action = sub_action;
    cmd1.start_time = start_time;
    cmd1.end_time = end_time;
    cmd1.remark = remark;
    cmd1.Completeness = "0%";
    cmd1.completed_url = "";
    dbManager->insertCommandHistory(cmd1);

    QByteArray message = QJsonDocument(newJson).toJson();

    for (const QJsonValue &serialNumber : std::as_const(serial_numbers)) {
        QString topic = "Device/Dispatch/" + serialNumber.toString();
        QMqttTopicName topicName(topic);
        publishMessage(topicName, message);
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

void mqttclient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    QJsonDocument doc = QJsonDocument::fromJson(message);
  //  qDebug() << topic << "\n" <<message;
    QJsonObject jsonObj = doc.object();

    if (!jsonObj.isEmpty()) {
        QString messageType = getMessageType(jsonObj);

        if (messageType == "") {
            qDebug() << "Error: messageType is empty!";
            return;
        } else if (messageType == "heart") {
            DeviceStatus deviceStatus = parseJsonHeartBeat(jsonObj);
            emit updateDeviceInfo(deviceStatus);
        } else {
            qDebug() << "Error: messageType is unknown!";
        }
    } else {
        qDebug() << "error: not a json ...";
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
    float temp = jsonObj["data"]["temperature"].toDouble(0.0);

    DeviceStatus deviceStatus(serialNumber, status, location, currentAction,
                              trafficStatistics, lastHeartbeat, ip,
                              current_start, current_end, next_action,
                              next_action_start, next_action_end, usedProcess, ProcessID, temp);
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
