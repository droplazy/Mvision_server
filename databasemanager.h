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
    bool createTables();// 订单表
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

    // 指令历史记录操作
    bool insertCommandHistory(const SQL_CommandHistory &command);
    bool updateCommandHistory(const SQL_CommandHistory &command);
    bool deleteCommandHistory(const QString &commandId);
    bool deleteCommandHistoryByStatus(const QString &status);
    bool deleteCommandHistoryByTimeRange(const QString &startTime, const QString &endTime);
    QList<SQL_CommandHistory> getAllCommands();
    QList<SQL_CommandHistory> getCommandsByStatus(const QString &status);
    QList<SQL_CommandHistory> getCommandsByAction(const QString &action);
    QList<SQL_CommandHistory> getCommandsByTimeRange(const QString &startTime, const QString &endTime);
    SQL_CommandHistory getCommandById(const QString &commandId);

    int getCommandCountByStatus(const QString &status);
    QList<QString> getDistinctActions();

    // 批量操作
    bool batchInsertCommands(const QList<SQL_CommandHistory> &commands);
    bool batchUpdateCommands(const QList<SQL_CommandHistory> &commands);
    // 订单管理
    bool insertOrder(const SQL_Order &order);
    bool updateOrder(const SQL_Order &order);
    bool deleteOrder(const QString &orderId);
    bool updateOrderStatus(const QString &orderId, const QString &status);

    QList<SQL_Order> getAllOrders();
    QList<SQL_Order> getOrdersByUser(const QString &user);
    QList<SQL_Order> getOrdersByStatus(const QString &status);
    QList<SQL_Order> getOrdersByProduct(const QString &productId);
    QList<SQL_Order> getOrdersByTimeRange(const QString &startTime, const QString &endTime);
    SQL_Order getOrderById(const QString &orderId);

    // 统计功能
    int getOrderCountByStatus(const QString &status);
    double getTotalSales();
    double getTotalSalesByProduct(const QString &productId);
    QMap<QString, int> getOrderStatistics(); // 按状态统计

    // 批量操作
    bool batchInsertOrders(const QList<SQL_Order> &orders);
    bool batchUpdateOrders(const QList<SQL_Order> &orders);
private:
    QSqlDatabase db;

    bool createTable1();
    bool createTable2();
    bool createTable3();
    bool createTable4();
    bool createTable5();

    SQL_CommandHistory extractCommandFromQuery(const QSqlQuery &query);
    SQL_Order extractOrderFromQuery(const QSqlQuery &query);
};

#endif // DATABASEMANAGER_H
