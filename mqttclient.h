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
#include <QTimer>


class mqttclient : public QThread
{
    Q_OBJECT
public:
    explicit mqttclient(DatabaseManager *db,
                        const QString &hostname = "192.168.10.103",
                        int port = 1883,
                        const QString &clientId = "pcpcpc222333");
    virtual void run() override;
    void connectToBroker();  // 连接到MQTT代理
    void publishMessage(QMqttTopicName topicName, QByteArray message);

    // 设置连接参数
    void setConnectionParams(const QString &hostname, int port, const QString &clientId);

public slots:
    void ADDsubscribeTopic(QString device);
    void CommandMuiltSend(QJsonObject json);
    void ProcessDevtSend(QJsonObject json);
    void reconnectBroker();  // 重新连接
    void devUpgrade(QStringList devList);
    // 十秒定时函数
    void tenSecondTimerFunction();
    void SingleTopicPub(QString topic, QString msg);
private slots:
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);
    void onStateChanged(QMqttClient::ClientState state);

private:
    QMqttClient *mqttClient;
    QStringList topicList;
    QString m_hostname;
    int m_port;
    QString m_clientId;
    // 初始化定时器
        QTimer *m_tenSecondTimer;  // 十秒定时器
    void initTimer();
    void subscribeToTopic(QString topic);
    void subscribeALLTopic();
    DeviceStatus parseJsonHeartBeat(const QJsonObject &jsonObj);
    QString getMessageType(const QJsonObject &jsonObj);
    DatabaseManager *dbManager;

    bool verifyDeviceChecksum(const QString &serialNumber, const QString &verificationCode);
    void handleApplicationStatus(const QJsonObject &jsonObj);
    QString extractAccountUsingSplit(const QString &remark);
signals:
    void updateDeviceInfo(DeviceStatus);
    void mqttclientconnted(bool);
    void applogginstatus(QString commid,bool);
     void programInfoGenerated(const ProgramInfo &programInfo);

};

#endif // MQTTCLIENT_H
