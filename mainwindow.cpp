#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    p_db = new DatabaseManager();
    p_mqttt_ser = new MQTT_server(p_db);
    p_mqttt_ser->startServer();
   // return ;

    // 获取当前目录
    QString currentDir = QCoreApplication::applicationDirPath();

    // 设置数据库文件名，确保数据库文件创建在当前目录
    QString dbName = QDir(currentDir).filePath("system.db");

    // 调用createDatabase方法并传入数据库文件路径
    if (p_db->createDatabase(dbName)) {
        qDebug() << "Database and tables created successfully!";
    } else {
        qDebug() << "Failed to create database or tables.";
    }
    p_http = new HttpServer(p_db);
    p_mqtt_cli = new mqttclient(p_db);

   // p_mqtt_ser
    connect(p_http, &HttpServer::devCommadSend, p_mqtt_cli, &mqttclient::CommandMuiltSend);  // 连接状态变化信号onDeviceUpdata
    connect(p_http, &HttpServer::NewDeviceCall, p_mqtt_cli, &mqttclient::ADDsubscribeTopic);  // 连接状态变化信号onDeviceUpdata
    connect(p_mqtt_cli, &mqttclient::updateDeviceInfo, p_http, &HttpServer::onDeviceUpdata);  // 连接状态变化信号 void devCommadSend(QJsonObject);



    if (!p_http->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Server could not start!";
    } else {
        qDebug() << "Server started on port 8080...";
    }
    p_mqtt_cli->start();

}

MainWindow::~MainWindow()
{
    delete ui;
}
