#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "loghandler.h"
#include <QMessageBox>
/****************************子窗口控件*/
#include "devicelistdialog.h"
#include "commandlsit.h"
#include "firmware.h"
#include "mallusermanager.h"  // 添加头文件
#include "mallproducts.h"
#include "orderlist.h"
#include "userappeal.h"
/*************************/
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 禁止窗口拉伸
    setWindowTitle("后台控制系统");
    setFixedSize(this->size());
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
    // 修改后 - 使用与 HttpServer 相同的基础路径
    QDir currentDir = QDir::current();  // 获取当前工作目录
   // QString currentDir = QCoreApplication::applicationDirPath();

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


void MainWindow::on_pushButton_cmdquery_clicked()
{
    if (!p_db) {
        QMessageBox::warning(this, "错误", "数据库未初始化");
        return;
    }

    commandlsit *dialog = new commandlsit(p_db, this);
    dialog->setAttribute(Qt::WA_DeleteOnClose);
    dialog->exec();
}


void MainWindow::on_pushButton_firmware_clicked()
{
    // 创建固件上传对话框
    firmware *dialog = new firmware(this);

    // 设置关闭时自动删除
    dialog->setAttribute(Qt::WA_DeleteOnClose);

    // 模态显示对话框
    dialog->exec();

    // 可以添加一些后续处理，比如刷新固件列表等
    qDebug() << "固件上传对话框已关闭";
}

void MainWindow::on_pushButton_malluser_clicked()
{
    qDebug() << "打开商城用户管理界面";

    // 创建商城用户管理对话框，传入数据库指针
    mallusermanager *managerDialog = new mallusermanager(p_db, this);

    // 设置模态对话框
    managerDialog->setModal(true);

    // 显示对话框
    managerDialog->exec();  // 使用exec()确保模态对话框

    // 对话框关闭后自动删除
    managerDialog->deleteLater();
}


void MainWindow::on_pushButton_products_clicked()
{
    // 创建商品管理对话框，传入数据库指针
    mallproducts *productsDialog = new mallproducts(p_db, this);

    // 设置模态对话框
    productsDialog->setModal(true);

    // 显示对话框
    productsDialog->exec();  // 使用exec()确保模态对话框

    // 对话框关闭后自动删除
    productsDialog->deleteLater();
}


void MainWindow::on_pushButton_order_clicked()
{
    // 创建订单查询窗口
    orderlist *orderWindow = new orderlist(this);
    orderWindow->setDatabaseManager(p_db);

    // 设置模态或非模态显示
    orderWindow->setAttribute(Qt::WA_DeleteOnClose);
    orderWindow->exec();  // 模态显示
}


void MainWindow::on_pushButton_appeal_clicked()
{
    // 创建投诉处理窗口
    userappeal *appealWindow = new userappeal(this);

    // 如果需要设置当前用户（从登录信息获取）
    // QString currentUser = getCurrentUser(); // 假设有这个函数
    // appealWindow->setCurrentUser(currentUser);

    // 设置窗口属性
    appealWindow->setAttribute(Qt::WA_DeleteOnClose);
    appealWindow->setWindowModality(Qt::ApplicationModal);

    // 显示窗口
    appealWindow->show();

}

