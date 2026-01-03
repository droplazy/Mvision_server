#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QSqlError>
#include <QString>
#include <QDebug>
#include "publicheader.h"
// 可以在头文件中定义这些状态常量
namespace AppealStatus {
// 总体状态
const QString PENDING = "待处理";
const QString PROCESSING = "处理中";
const QString REVIEWING = "复核中";
const QString COMPLETED = "已完成";
const QString REJECTED = "已拒绝";
const QString CANCELLED = "已取消";

// 处理状态分类
const QString NEED_FURTHER_INFO = "需要更多信息";
const QString WAITING_FOR_RESPONSE = "等待用户回复";
const QString TECHNICAL_ISSUE = "技术问题";
const QString CUSTOMER_SERVICE = "客服处理中";
const QString REFUND_PROCESSING = "退款处理中";
const QString QUALITY_INSPECTION = "质量检查中";

// 优先级
const QString URGENT = "urgent";
const QString HIGH = "high";
const QString NORMAL = "normal";
const QString LOW = "low";
}

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

    // 商城用户管理
    bool insertMallUser(const SQL_MallUser &user);
    bool updateMallUser(const SQL_MallUser &user);
    bool deleteMallUser(const QString &username);
    bool updateMallUserPassword(const QString &username, const QString &newPassword);
    bool updateMallUserLastLogin(const QString &username);
    bool updateMallUserBalance(const QString &username, double amount);
    bool updateMallUserPoints(const QString &username, int points);

    // 查询功能
    QList<SQL_MallUser> getAllMallUsers();
    SQL_MallUser getMallUserByUsername(const QString &username);
    SQL_MallUser getMallUserByEmail(const QString &email);
    SQL_MallUser getMallUserByPhone(const QString &phone);
    SQL_MallUser getMallUserByInviteCode(const QString &inviteCode);
    QList<SQL_MallUser> getMallUsersByLevel(int level);
    QList<SQL_MallUser> getMallUsersByStatus(const QString &status);
    QList<SQL_MallUser> getMallUsersByTimeRange(const QString &startTime, const QString &endTime);
    QList<SQL_MallUser> getMallUsersByInviter(const QString &inviterUsername);
    // 验证功能
    bool validateMallUserLogin(const QString &username, const QString &password);
    bool checkMallUserExists(const QString &username);
    bool checkEmailExists(const QString &email);
    bool checkPhoneExists(const QString &phone);
    bool checkInviteCodeExists(const QString &inviteCode);

    // 统计功能
    int getMallUserCount();
    int getMallUserCountByLevel(int level);
    int getMallUserCountByStatus(const QString &status);
    double getTotalMallUserBalance();
    int getTotalMallUserPoints();

    // 批量操作
    bool batchInsertMallUsers(const QList<SQL_MallUser> &users);
    bool batchUpdateMallUsers(const QList<SQL_MallUser> &users);
    bool createWithdrawRecord(const QString &withdrawId, const QString &username, double amount, const QString &alipayAccount, const QString &remark = "");
    QList<SQL_WithdrawRecord> getWithdrawRecordsByUsername(const QString &username);
    QList<SQL_Order> getOrdersByCommandId(const QString &commandId);
    SQL_Order getFirstOrderByCommandId(const QString &commandId);
    bool hasOrdersForCommand(const QString &commandId);
    int getOrderCountByCommandId(const QString &commandId);
    QList<SQL_Order> getUserOrdersWithSnapshots(const QString &username);
    SQL_Order getOrderWithSnapshot(const QString &orderId);
  /*  bool insertUserAppeal(const QString &username, const QString &orderId,
                          const QString &appealType, const QString &contentPath,
                          const QString &textContent);

    bool insertUserAppeal(const QString &username, const QString &orderId,
                          const QString &appealType, const QString &contentPath);*/
    QList<SQL_AppealRecord> getAppealsByPriority(const QString &priority);
    QList<SQL_AppealRecord> getAppealsByProcessingStatus(const QString &processingStatus);
    bool updateAppealPriority(int appealId, const QString &priority);
    bool updateAppealProcessingStatus(int appealId, const QString &processingStatus);
    bool insertUserAppeal(const QString &username, const QString &orderId,
                          const QString &appealType, const QString &contentPath,
                          const QString &textContent,
                          const QString &priority = "normal",
                          int appealLevel = 1);QList<SQL_AppealRecord> getAppealsByUser(const QString &username);

    bool insertProduct(const SQL_Product &product);

    bool updateProduct(const SQL_Product &product);
    bool deleteProduct(const QString &productId);
    SQL_Product getProductById(const QString &productId);
    QList<SQL_Product> getAllProducts();
    QList<SQL_Product> getProductsByCategory(const QString &categoryId);
    QList<SQL_Product> getProductsByStatus(const QString &status);
    QList<SQL_Product> searchProducts(const QString &keyword);
    bool updateProductStock(const QString &productId, int quantity, bool increment);
    bool updateProductSales(const QString &productId, int quantity);
    bool updateProductRating(const QString &productId, double newRating);
    QList<SQL_Product> getHotProducts(int limit);
    SQL_Product extractProductFromQuery(const QSqlQuery &query);
    QList<SQL_Product> getNewProducts(int limit);
    bool batchUpdateProducts(const QList<SQL_Product> &products);
    bool batchInsertProducts(const QList<SQL_Product> &products);
    int getInvitedUserCount(const QString &inviterUsername);
    double getInvitedUsersTotalConsumption(const QString &inviterUsername);

    bool deleteUserToken(const QString &username);
    bool cleanExpiredTokens();
    QString getUsernameByToken(const QString &token);
    bool validateToken(const QString &token);
    bool saveUserToken(const QString &username, const QString &token);
private:
    QSqlDatabase db;

    bool createTable1(); //设备表
    bool createTable2(); //后台用户表
    bool createTable3(); //流程表
    bool createTable4(); //指令表
    bool createTable5(); //订单表
    bool createTable6();  // 商城用户表
    bool createAppealTable();

    SQL_CommandHistory extractCommandFromQuery(const QSqlQuery &query);
    SQL_Order extractOrderFromQuery(const QSqlQuery &query);
    SQL_MallUser extractMallUserFromQuery(const QSqlQuery &query);
    bool createWithdrawTable();
    SQL_AppealRecord extractAppealFromQuery(const QSqlQuery &query);
    bool createTable7();//商品表单
    bool createTokenTable();
};

#endif // DATABASEMANAGER_H
