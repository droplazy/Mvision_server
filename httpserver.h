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

class HttpServer : public QTcpServer
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);
    void sendResponse(QTcpSocket *clientSocket, const QByteArray &json);
    void sendNotFound(QTcpSocket *clientSocket);

protected:
    void incomingConnection(qintptr socketDescriptor) override;

private slots:
    void onReadyRead();

private:
    void handleGetDevice(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handleGetProcess(QTcpSocket *clientSocket, const QUrlQuery &query);
    void handlePostDeviceCommand(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostDeviceAdd(QTcpSocket *clientSocket, const QUrlQuery &query, const QByteArray &body);
    void handlePostProcessCreate(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostProcessUpdate(QTcpSocket *clientSocket, const QUrlQuery &query, const QByteArray &body);
    void handlePostProcessDelete(QTcpSocket *clientSocket, const QByteArray &body);
    void handlePostAuthLogin(QTcpSocket *clientSocket, const QByteArray &body);
    QByteArray getContentType( QString& filePath);// 新增的静态文件处理方法
    void printStaticFiles(const QByteArray &htmlContent);
    void send404(QTcpSocket *clientSocket);
    void ShowHomepage(QTcpSocket *clientSocket, QByteArray request);
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
};

#endif // HTTPSERVER_H
