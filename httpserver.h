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
#include <QMutex>
#include <QTcpSocket>
#include <QPointer>
#include "wechatpay.h"





struct FrontendTask {
    QString taskId;          // 任务ID
    QString platform;        // 平台 AAA/BBB/CCC等
    QString account;         // 账号
    QString deviceSerial;    // 设备序列号
    QString status;          // 状态: pending, processing, success, failed
    QString createTime;      // 创建时间
    QString updateTime;      // 更新时间
    QString remark;          // 备注信息
    QString commandid;
    QString username;
    QString method;

    bool postCRcode=false;
    bool postResult=false;


    //QTcpSocket *clientinfo;
    QPointer<QTcpSocket> clientinfo;
    // qintptr socketDescriptor;  // 这个最安全
    void print() {
        qDebug() << "FrontendTask:";
        qDebug() << "  taskId:" << taskId;
        qDebug() << "  platform:" << platform;
        qDebug() << "  account:" << account;
        qDebug() << "  deviceSerial:" << deviceSerial;
        qDebug() << "  status:" << status;
        qDebug() << "  createTime:" << createTime;
        qDebug() << "  updateTime:" << updateTime;
        qDebug() << "  remark:" << remark;
        qDebug() << "  commandid:" << commandid;
        qDebug() << "  username:" << username;
    }
};
class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HttpServer(DatabaseManager *db, QObject *parent = nullptr);
    
    QString LOGIN_GUIDE_TEXT ="佳荣电子商城";              // 使用宏
    QString LOGIN_SLOGAN1 = "支持软件硬件研发打样一站式服务";               // 使用宏
    QString LOGIN_SLOGAN2 =  "已经服务江浙沪上百家企业";             // 使用宏
    
    void sendResponse(QTcpSocket *clientSocket, const QByteArray &json);
    void sendNotFound(QTcpSocket *clientSocket);
    QString askDeepSeek(const QString &question);

    // 创建 DeviceStatus 对象的 QVector
    QVector<DeviceStatus> deviceVector;
    QVector<Machine_Process_Total> processVector;
    
protected:
    void incomingConnection(qintptr socketDescriptor) override;
    
private slots:
    void onReadyRead();
    void cleanupExpiredCodes();                    // 清理过期验证码
public slots:
    void onDeviceUpdata(DeviceStatus updatedDevice);
    void handleAppLoginStatus(QString commid, bool isSuccess);
    void tenSecondTimerFunction(); //todo 设备上报时间格式有错误
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
    void handlePostFileUpload(QTcpSocket *clientSocket, QUrlQuery query, const QByteArray &body);
    
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
    
    // 初始化定时器
    void initTimer();
    QTimer *m_tenSecondTimer;  // 十秒定时器
    Machine_Process_Total *findProcessById(QVector<Machine_Process_Total> &processes, const QString &id);
    QJsonObject generateProcessResponse(const QJsonArray &data);
    QJsonObject generateProcessSingleResponse(const QJsonObject &data);
    bool deleteProcessByProcessId(const QString &process_id);
    //WeChatPay *pay;
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
    QVector<FrontendTask> frontendTasks;  // 前端任务容器
    // 订单过期时间（秒）
    static const int ORDER_EXPIRY_SECONDS = 180; // 30分钟
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
    void handlePostOrderVerify(QTcpSocket *clientSocket, const QByteArray &body);
    QStringList generateRandomSerialNumbers(int count);
    QString mapStatusToChinese(const QString &status);
    void handleCreateProductDebug();
    // void handlePostMallProducts(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    void handleGetMallProducts(QTcpSocket *clientSocket, const QUrlQuery &query);
    // void handlePostVerifyCode(QTcpSocket *clientSocket, const QByteArray &body);
    
    
    void initVerificationSystem();// 初始化验证码系统
    void addVerificationCode(const QString &code, const QString &username, const QString &email); // 添加验证码到容器
    bool verifyCode(const QString &username, const QString &code);// 验证验证码
    QString getLatestCode(const QString &username); // 获取用户最新的有效验证码
    QVector<VerificationCode> verificationCodes;  // 验证码容器
    QTimer *cleanupTimer;                         // 清理定时器
    QMutex codeMutex;                             // 线程安全锁
    void printVerificationCodes();
    //  bool checkVerificationCodeFrequency(const QString &username, const QString &email, int minIntervalSeconds);
    void sendUnauthorized(QTcpSocket *clientSocket);
    void saveDeviceStatusToDatabase(const DeviceStatus &deviceStatus);
    void markDeviceOffline(DeviceStatus &device);
    QDateTime parseHeartbeatTime(const QString &timeStr);
    void handleGetIPTEST(QTcpSocket *clientSocket, const QString &IP);
    void handleGetWithDraw(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handlePostWithDrawPic(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    void handleGetidleDev(QTcpSocket *clientSocket);
    void handlePostPlatformReq(QTcpSocket *clientSocket, const QByteArray &body);
    QString findAvailableDevice(const QString &platform);
    void handlePostPlatformReqSendcode(QTcpSocket *clientSocket, const QByteArray &body);
    void handleGetTaskSta(QTcpSocket *clientSocket, const QUrlQuery &query);
    void updateTaskStatusByCommandId(const QString &commandid, const QString &sta);
    void handleGetCRCODE(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleGetProductsPic(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleGetPersonInfo(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleGetCancelDev(QTcpSocket *clientSocket, const QUrlQuery &query);
    void sendErrorImage(QTcpSocket *clientSocket, int errorCode, const QString &errorMsg);
    void sendErrorResponse(QTcpSocket *clientSocket, int errorCode, const QString &errorMsg);
    QString getHttpStatusText(int code);
    void handlecmdSnap(QTcpSocket *clientSocket, const QUrlQuery &query);
    QJsonObject productToJson(const SQL_Product &product);
  //  void sendJsonResponse(QTcpSocket *clientSocket, int statusCode, const QJsonObject &json, bool keepAlive = false);
    void handlePostXUNFEIocr(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    void handlePostXUNFEIocrLvingroom(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    FrontendTask *findTaskById(const QString &taskId);
    bool QueryCRcodePic(FrontendTask task);
    QTcpSocket *getSocketByDescriptor(qintptr descriptor);
    bool isSocketValid(const FrontendTask &task);
    //QPixmap createQRCode(const QString &text);
    QPixmap wechatPay(const SQL_Order &order);
    void wechatpay_closeOrder(QString outTradeNo);
    void handleGetPayQRCode(QTcpSocket *clientSocket, const QString path);
    void handlePostWechatPayOK(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query);
    QByteArray aes256GcmDecrypt(const QByteArray &ciphertext, const QByteArray &key, const QByteArray &nonce, const QByteArray &aad);
signals:
    void NewDeviceCall(QString);
    void devCommadSend(QJsonObject);
    void devProcessSend(QJsonObject);
    void sendemail(EmailInfo);
    void sendreqInfo(QString);
    void updateDev();
    void getCRcodeImg(QString);
    void forwardJsonToMQTT(const QString& deviceSerial, const QString& jsonString);
    void ORCbraggerresult(const QString& commandid, const QStringList& text);
    
};

#endif // HTTPSERVER_H
