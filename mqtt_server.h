#ifndef MQTT_SERVER_H
#define MQTT_SERVER_H

#include <QObject>
#include <QProcess>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include "databasemanager.h"  // 注意引号

class MQTT_server : public QObject
{
    Q_OBJECT
public:
    explicit MQTT_server(DatabaseManager *db, QObject *parent = nullptr);  // 添加explicit和parent参数
    ~MQTT_server();  // 添加析构函数声明

    bool startServer(const QString &ip = "0.0.0.0", int port = 1883);
    void stopServer();

    QString PORT;
    QString log_path;

private:
    QProcess *process;
    DatabaseManager *dbManager;

    bool createDefaultConfig(const QString &configPath, const QString &logPath,
                             const QString &ip, int port);
    bool validateAndFixConfig(const QString &configPath, const QString &logPath,
                              const QString &ip, int port);
    void printConfigFile(const QString &filePath);
};

#endif // MQTT_SERVER_H
