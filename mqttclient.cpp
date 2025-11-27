#include "mqttclient.h"
#include <QTimer>
#include <QtMqtt/QMqttClient>
#include <QtMqtt/QMqttTopicName>
#include "httpserver.h"


mqttclient::mqttclient()
    : mqttClient(new QMqttClient(this)) // 初始化MQTT客户端
{
    connect(mqttClient, &QMqttClient::messageReceived, this, &mqttclient::onMessageReceived);
}

void mqttclient::run()
{
    connectToBroker();   // 连接到 MQTT 代理
    subscribeToTopic();  // 订阅主题
    // while(1)
    // {
    //     if( msgFlag)
    //     {
    //         publishMessage();
    //     }
    //     sleep(1);
    // }
    // // 每3秒发布一次消息
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &mqttclient::publishMessage);
    timer->start(200);  // 设置定时器每3秒触发一次
}

void mqttclient::connectToBroker()
{
    mqttClient->setHostname("test.mosquitto.org");  // 设置MQTT代理地址
    mqttClient->setPort(1883);  // 设置MQTT代理端口
    mqttClient->connectToHost();  // 连接到代理
}

void mqttclient::subscribeToTopic()
{
    if (mqttClient->state() == QMqttClient::Connected) {
        QMqttTopicFilter topicFilter("test/topic123");  // 使用QMqttTopicFilter类型的主题过滤器
        mqttClient->subscribe(topicFilter);  // 订阅主题
    }
}


void mqttclient::publishMessage()
{
    if (mqttClient->state() == QMqttClient::Connected  ) {

        QByteArray message = "来自PC端";  // 消息内容
        QMqttTopicName topicName("test/topic123");
        mqttClient->publish(topicName, message,0,false);  // 发布消息到 test/topic123 主题
        qDebug() << "消息已经发送";

        qDebug() << "没有消息可以发送";
    }
}

void mqttclient::onMessageReceived(const QByteArray &message, const QMqttTopicName &topic)
{
    // 处理收到的消息
    qDebug() << "Received message on topic" << topic.name() << ":" << message;
}
