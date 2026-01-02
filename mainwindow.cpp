#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loghandler.h"  // <=== 新增这行
#include <QMessageBox>
#include "devicelistdialog.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);


    // 设置日志输出到 textEdit_console
    if (ui->textEdit_console) {
        LogHandler::instance()->setOutputWidget(ui->textEdit_console);
        LogHandler::instance()->setMaxLines(500);
    }

    // 创建定时器用于显示系统时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSystemTime);

    // 每秒更新一次时间
    timer->start(1000);

    // 立即更新一次时间
    updateSystemTime();



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
    if (!p_http->listen(QHostAddress::Any, 8080)) {
        qDebug() << "Server could not start!";
        ui->label_edit_http->setText("Failed");
        ui->label_edit_http->setStyleSheet("background-color: red; color: white; padding: 2px;");
    } else {
        qDebug() << "Server started on port 8080...";
        ui->label_edit_http->setText("8080");
        ui->label_edit_http->setStyleSheet("background-color: green; color: white; padding: 2px;");
    }
    p_mqtt_cli = new mqttclient(p_db);
    // 启动MQTT服务器并设置状态显示
    if (p_mqttt_ser->startServer()) {
        qDebug() << "MQTT Server started successfully on port 1883";
        ui->label_edit_mqttser->setText("1883");
        ui->label_edit_mqttser->setStyleSheet("background-color: green; color: white; padding: 2px;");
    } else {
        qDebug() << "MQTT Server failed to start!";
        ui->label_edit_mqttser->setText("Failed");
        ui->label_edit_mqttser->setStyleSheet("background-color: red; color: white; padding: 2px;");
    }

    p_email->setSmtpServer("smtp.yeah.net", 465);
    // 登录
    if (p_email->login("zwdz668@yeah.net", "XFaYuyxRWqQXJp7w"))
    {
        qDebug() << "SMTP登录成功";
        ui->label_edit_smtp->setText("Ready");
        ui->label_edit_smtp->setStyleSheet("background-color: green; color: white; padding: 2px;");
    }
    else
    {
        qDebug() << "SMTP登录失败";
        ui->label_edit_smtp->setText("Failed");
        ui->label_edit_smtp->setStyleSheet("background-color: red; color: white; padding: 2px;");
    }
    connect(p_http, &HttpServer::devCommadSend, p_mqtt_cli, &mqttclient::CommandMuiltSend);  // 下发设备命令信号
    connect(p_http, &HttpServer::devProcessSend, p_mqtt_cli, &mqttclient::ProcessDevtSend);  // 下发流程命令
    connect(p_http, &HttpServer::sendemail, p_email, &EmailSender::onSendEmailRequested);  // 发送邮箱信号
    connect(p_http, &HttpServer::NewDeviceCall, p_mqtt_cli, &mqttclient::ADDsubscribeTopic);  // 新设备接入
    connect(p_mqtt_cli, &mqttclient::updateDeviceInfo, p_http, &HttpServer::onDeviceUpdata);  // 更新设备信息mqttclientconnted
    connect(p_mqtt_cli, &mqttclient::mqttclientconnted, this, &MainWindow::DispayMqttclientStatus);  // 更新设备信息




    p_mqtt_cli->start();

}

// 更新系统时间的槽函数
void MainWindow::updateSystemTime()
{
    // 获取当前系统时间
    QDateTime currentDateTime = QDateTime::currentDateTime();

    // 格式化为"2026年1月2日 17:57:00"格式
    QString timeStr = currentDateTime.toString("yyyy年M月d日 hh:mm:ss");

    // 在label_edit_systime中显示（假设UI中有一个名为label_edit_systime的QLabel）
    ui->label_edit_systime->setText(timeStr);
}
// 更新系统时间的槽函数
void MainWindow::DispayMqttclientStatus(bool status)
{
    // 登录
    if (status)
    {
        ui->label_edit_mqttsubcrib->setText("Ready");
        ui->label_edit_mqttsubcrib->setStyleSheet("background-color: green; color: white; padding: 2px;");
    }
    else
    {
        ui->label_edit_mqttsubcrib->setText("Failed");
        ui->label_edit_mqttsubcrib->setStyleSheet("background-color: red; color: white; padding: 2px;");
    }
}
MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::on_pushButton_devlist_clicked()
{
    if (!p_db) {
        qDebug() << "数据库未初始化，无法显示设备列表";
        QMessageBox::warning(this, "错误", "数据库未初始化，请检查数据库连接");
        return;
    }

    qDebug() << "正在打开设备列表对话框...";

    // 创建并显示设备列表对话框
    devicelistdialog *dialog = new devicelistdialog(p_db, this);

    // 设置模态对话框
    dialog->setModal(true);

    // 设置关闭时自动删除
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 显示对话框（模态方式）
    dialog->exec();

    qDebug() << "设备列表对话框已关闭";
}

