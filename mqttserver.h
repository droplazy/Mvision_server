#ifndef MQTTSERVER_H
#define MQTTSERVER_H

#include <QObject>
#include <QThread>
#include <QtMqtt>
class mqttserver:QThread
{
    Q_OBJECT
public:
    mqttserver();
    virtual void run() override;  // 重载 run 函数
};

#endif // MQTTSERVER_H
