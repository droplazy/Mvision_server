#ifndef MQTTCLIENT_H
#define MQTTCLIENT_H

#include <QObject>
#include <QThread>
#include <QtMqtt>

class mqttclient : public QThread
{
    Q_OBJECT
public:
    mqttclient();
    virtual void run() override;  // 重载 run 函数
    void connectToBroker();  // 连接到MQTT代理
    void subscribeToTopic(); // 订阅主题
    void publishMessage();   // 发布消息

private slots:
    void onMessageReceived(const QByteArray &message, const QMqttTopicName &topic);  // 消息接收槽

private:
    QMqttClient *mqttClient; // MQTT客户端对象
};

#endif // MQTTCLIENT_H
