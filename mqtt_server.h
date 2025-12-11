#ifndef MQTT_SERVER_H
#define MQTT_SERVER_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <databasemanager.h>

class MQTT_server : public QObject
{
    Q_OBJECT
public:
    MQTT_server(DatabaseManager *db);;
    void startServer();
    void printConfigFile(const QString &filePath);
    void stopServer();

    QString PORT;
    QString log_path;
private:
    QProcess *process;  // 用于管理 mosquitto.exe 进程
    DatabaseManager *dbManager;
};

#endif // MQTT_SERVER_H
