#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QTcpServer>
#include <QTcpSocket>
#include <QUrlQuery>
#include <QJsonDocument>
#include <QByteArray>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include "publicheader.h"
#include "databasemanager.h"
#include <QTimer>


class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HttpServer(DatabaseManager *db, QObject *parent = nullptr);


    void sendResponse(QTcpSocket *clientSocket, const QByteArray &json);
    void sendNotFound(QTcpSocket *clientSocket);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();
public slots:
    void onDeviceUpdata(DeviceStatus updatedDevice);

private:
    void handleGetOrderQuery(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handlePostDeviceProcess(QTcpSocket *clientSocket, const QByteArray &body);
    void handleGetDevice(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleGetProcess(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handlePostDeviceCommand(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostDeviceAdd(QTcpSocket *clientSocket, const QUrlQuery &query, const QByteArray &body);
    void handlePostProcessCreate(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostProcessUpdate(QTcpSocket *clientSocket, const QUrlQuery &query, const QByteArray &body);
    void handlePostProcessDelete(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostAuthLogin(QTcpSocket *clientSocket, const QByteArray &body);
    //void handlePostFileUpload(QTcpSocket *clientSocket,QUrlQuery query, const QByteArray &body);
    void handlePostFileUpload(QTcpSocket *clientSocket, QUrlQuery query, const QByteArray &body, QString verify);

    QByteArray getContentType( QString& filePath);// 新增的静态文件处理方法
    void printStaticFiles(const QByteArray &htmlContent);
    void send404(QTcpSocket *clientSocket);
    void ShowHomepage(QTcpSocket *clientSocket, QByteArray request);
    void handleGetDownload(QTcpSocket *clientSocket, const QUrlQuery &query);
    QJsonObject parseJsonData(const QString &jsonString);
    QJsonObject generateJson(const QString &username, int code, const QString &token = "");

    QJsonObject generateDeviceResponse(const QJsonArray &devices);
    QJsonObject generateFailureResponse();
    DeviceStatus *findDeviceBySerialNumber(QVector<DeviceStatus> &devices, const QString &serialNumber);
    QJsonObject generateJsonHearResponse(const QJsonObject &data);
    void generateTextData();

    // 创建 DeviceStatus 对象的 QVector
    QVector<DeviceStatus> deviceVector;
    QVector<Machine_Process_Total> processVector;


    Machine_Process_Total *findProcessById(QVector<Machine_Process_Total> &processes, const QString &id);
    QJsonObject generateProcessResponse(const QJsonArray &data);
    QJsonObject generateProcessSingleResponse(const QJsonObject &data);
    bool deleteProcessByProcessId(const QString &process_id);

    QString parseJsonGenerateNode(const QJsonObject &rootObj, QVector<Machine_Process_Total> &processVector);
    bool parseMachineProcess(const QJsonObject &rootObj, QVector<Machine_Process_Total> &processVector);
    bool processDeleteRequest(const QJsonObject &rootObj);

    DatabaseManager *dbManager;
    void sendHttpResponse(QTcpSocket *clientSocket, int statusCode, const QString &statusText, const QByteArray &body);
    void createDownloadDirectoryIfNeeded();
    void createUploadDirectoryIfNeeded();
    QString getHeaderValue(QTcpSocket *clientSocket, const QString &headerName);
    void handlePostWarningIgnore(QTcpSocket *clientSocket, const QByteArray &body);
    void handleGetCommandList(QTcpSocket *clientSocket, const QUrlQuery &query);
    void extracted(QString &statusFilter, QString &userFilter,
                   QList<SQL_Order> &orders, QList<SQL_Order> &allOrders);
    void handleGetOrderList(QTcpSocket *clientSocket, const QUrlQuery &query);



    //订单数据库测试接口
    void handleCreateTestOrdersSimple();
    void handleGetLoginUI(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleBGimagesGet(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handlePostMallLogin(QTcpSocket *clientSocket, const QByteArray &body);
    QString generateToken(const QString &username);
    void handlePostMallRegist(QTcpSocket *clientSocket, const QByteArray &body);
    QString generateInviteCode();
    void handlePostMallPasswdReset(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostMallSendMailCode(QTcpSocket *clientSocket, const QByteArray &body);
    void handleGetAuthInfo(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleGetAuthPromote(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handlePostMallSendwithdraw(QTcpSocket *clientSocket, const QByteArray &body);
    QString generateWithdrawId();
    void handlePostMallOrderCheckout(QTcpSocket *clientSocket, const QByteArray &body);
    QString generatePaymentCode();
    QString generateOrderId();


    // 待支付订单容器（订单ID -> 订单对象）
    QMap<QString, SQL_Order> pendingOrders;

    // 订单超时定时器
    QTimer *orderTimeoutTimer;

    // 订单过期时间（秒）
    static const int ORDER_EXPIRY_SECONDS = 60; // 30分钟
    bool completeOrderPayment(const QString &orderId);
    QList<SQL_Order> getUserPendingOrders(const QString &username);
    bool removePendingOrder(const QString &orderId);
    SQL_Order getPendingOrder(const QString &orderId);
    void cleanupExpiredOrders();
    void initOrderTimer();
    void handleGetpaidOK(QTcpSocket *clientSocket, const QUrlQuery &query);
    QString mapOrderStatusToApi(const QString &dbStatus);
    QString formatOrderTime(const QString &dbTime);
    void sendJsonResponse(QTcpSocket *clientSocket, int statusCode, const QJsonObject &json);
    void handlePostMallUserAppealtext(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    void handlePostMallUserAppealPic(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    void createAppealDirectoryIfNeeded();
    QByteArray parseMultipartData(const QByteArray &body, const QByteArray &boundary);
    void handleGetOrderAppeal(QTcpSocket *clientSocket, const QUrlQuery &query);
signals:
    void NewDeviceCall(QString);
    void devCommadSend(QJsonObject);
    void devProcessSend(QJsonObject);

};

#endif // HTTPSERVER_H
