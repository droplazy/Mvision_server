#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDebug>
#include "publicheader.h"


class DatabaseManager : public QObject
{
    Q_OBJECT
public:
    explicit DatabaseManager(QObject *parent = nullptr);
    ~DatabaseManager();

    bool createDatabase(const QString &dbName);
    bool createTables();
    bool openDatabase(const QString &dbName);

    //设备增删改查
    SQL_Device getDeviceBySerialNumber(const QString &serial_number);
    QList<SQL_Device> getAllDevices();
    bool deleteDevice(const QString &serial_number);
    bool updateDevice(const SQL_Device &device);
    bool insertDevice(const SQL_Device &device);
    //用户名增删改查
    SQL_User getUserByUsername(const QString &username);
    QList<SQL_User> getAllUsers();
    bool deleteUser(const QString &username);
    bool updateUser(const SQL_User &user);
    bool insertUser(const SQL_User &user);
    //流程增删改查

    bool deleteProcessSteps(const QString &process_id);
    bool updateProcessSteps(const Machine_Process_Total &process);
    bool insertProcessSteps(const Machine_Process_Total &process);
    QList<Machine_Process_Total> getAllProcessSteps();
private:
    QSqlDatabase db;

    bool createTable1();
    bool createTable2();
    bool createTable3();
};

#endif // DATABASEMANAGER_H
