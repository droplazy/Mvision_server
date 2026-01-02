#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QThread>
#include <QtMqtt>
#include "publicheader.h"
#include <QJsonDocument>
#include <QJsonObject>
#include <QString>
#include <QDebug>
#include <databasemanager.h>


class mqttclient : public QThread
{
    Q_OBJECT
public:
 //   explicit  mqttclient(DatabaseManager *db, QObject *parent = nullptr);//禁止隐式转换
    explicit mqttclient(DatabaseManager *db);
    virtual void run() override;  // 重载 run 函数
    void connectToBroker();  // 连接到MQTT代理
    void publishMessage(QMqttTopicName topicName, QByteArray message);   // 发布消息


public slots:
    void ADDsubscribeTopic(QString device);//添加主题并且订阅
    void CommandMuiltSend(QJsonObject json);
    void ProcessDevtSend(QJsonObject json);

private slots:
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);  // 消息接收槽
    void onStateChanged(QMqttClient::ClientState state);
private:
    QMqttClient *mqttClient; // MQTT客户端对象
    QStringList topicList;
    void subscribeToTopic(QString topic); // 订阅主题
    void extracted();
    void subscribeALLTopic(); // 订阅所有主题

    DeviceStatus parseJsonHeartBeat(const QJsonObject &jsonObj);
    QString getMessageType(const QJsonObject &jsonObj);
   // int scribeInt =0;
    DatabaseManager *dbManager;
signals:
    void updateDeviceInfo(DeviceStatus);
    void mqttclientconnted(bool);

};

#endif // MQTTCLIENT_H
