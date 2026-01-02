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
    p_email =new EmailSender();

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
    p_email->setSmtpServer("smtp.yeah.net", 465);
    // 登录
    if (p_email->login("zwdz668@yeah.net", "XFaYuyxRWqQXJp7w"))
    {
        qDebug() << "登录成功";
    }
    connect(p_http, &HttpServer::devCommadSend, p_mqtt_cli, &mqttclient::CommandMuiltSend);  // 下发设备命令信号
    connect(p_http, &HttpServer::devProcessSend, p_mqtt_cli, &mqttclient::ProcessDevtSend);  // 下发流程命令
    connect(p_http, &HttpServer::sendemail, p_email, &EmailSender::onSendEmailRequested);  // 发送邮箱信号
    connect(p_http, &HttpServer::NewDeviceCall, p_mqtt_cli, &mqttclient::ADDsubscribeTopic);  // 新设备接入
    connect(p_mqtt_cli, &mqttclient::updateDeviceInfo, p_http, &HttpServer::onDeviceUpdata);  // 更新设备信息



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
