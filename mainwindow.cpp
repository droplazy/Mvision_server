#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "loghandler.h"
#include <QMessageBox>
#include <QVBoxLayout>
/****************************子窗口控件*/
#include "./UIclass/devicelistdialog.h"
#include "./UIclass/commandlsit.h"
#include "./UIclass/firmware.h"
#include "./UIclass/mallusermanager.h"  // 添加头文件
#include "./UIclass/mallproducts.h"
#include "./UIclass/orderlist.h"
#include "./UIclass/userappeal.h"
#include "./UIclass/guidetextset.h"
#include "./UIclass/managerui.h"
#include "./UIclass/commanddev.h"
#include "./UIclass/appacount.h"
#include "UIclass/withdraw.h"


MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , p_db(nullptr)
    , p_http(nullptr)
    , p_mqtt_cli(nullptr)
    , p_mqttt_ser(nullptr)
    , p_email(nullptr)
{
    ui->setupUi(this);

    // 禁止窗口拉伸
    setWindowTitle("后台控制系统");
    setFixedSize(this->size());

    // 设置日志输出到 textEdit_console
    // if (ui->textEdit_console) {
    //     LogHandler::instance()->setOutputWidget(ui->textEdit_console);
    //     LogHandler::instance()->setMaxLines(500);
    // }

    // 创建定时器用于显示系统时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSystemTime);
    timer->start(1000);
    updateSystemTime();

    // 初始化UI状态
    initUIStatus();

    // 初始化数据库对象
    initDatabase();

    qDebug() << "MainWindow初始化完成";
}
// 初始化数据库
void MainWindow::initDatabase()
{
    p_db = new DatabaseManager();

    // 获取当前目录
    QDir currentDir = QDir::current();
    QString dbName = currentDir.filePath("system.db");

    // 创建数据库和表
    if (p_db->createDatabase(dbName)) {
        qDebug() << "数据库和表创建成功！";
    } else {
        qDebug() << "数据库创建失败！";
        QMessageBox::warning(this, "数据库错误", "数据库初始化失败，部分功能可能无法使用。");
    }
}
// 初始化UI状态
void MainWindow::initUIStatus()
{
    // 设置默认IP和端口
    ui->lineEdit_ip->setText("填写实际IP而非0.0.0.0地址");
    ui->lineEdit_mqtport->setText("1883");
    ui->lineEdit_httpport->setText("8080");
    // 设置服务器按钮初始状态
    ui->pushButton_openmqtt->setText("开启服务器");
    ui->pushButton_openmqtt->setStyleSheet("background-color: green; color: white;");
    // 设置状态标签为未启动
    ui->label_edit_http->setText("未启动");
    ui->label_edit_http->setStyleSheet("background-color: gray; color: white; padding: 2px;");

    ui->label_edit_mqttser->setText("未启动");
    ui->label_edit_mqttser->setStyleSheet("background-color: gray; color: white; padding: 2px;");

    ui->label_edit_mqttsubcrib->setText("未连接");
    ui->label_edit_mqttsubcrib->setStyleSheet("background-color: gray; color: white; padding: 2px;");

    ui->label_edit_smtp->setText("未连接");
    ui->label_edit_smtp->setStyleSheet("background-color: gray; color: white; padding: 2px;");

    // 初始状态允许编辑
    enableNetworkControls(true);
}
// 启用或禁用网络控制控件
void MainWindow::enableNetworkControls(bool enable)
{
    ui->lineEdit_ip->setEnabled(enable);
    ui->lineEdit_mqtport->setEnabled(enable);
    ui->lineEdit_httpport->setEnabled(enable);
    ui->pushButton_openmqtt->setEnabled(enable);

    if (enable) {
        ui->lineEdit_ip->setStyleSheet("");
        ui->lineEdit_mqtport->setStyleSheet("");
        ui->lineEdit_httpport->setStyleSheet("");
        ui->pushButton_openmqtt->setText("启动MQTT服务");
    } else {
        ui->lineEdit_ip->setStyleSheet("background-color: #f0f0f0; color: #666;");
        ui->lineEdit_mqtport->setStyleSheet("background-color: #f0f0f0; color: #666;");
        ui->lineEdit_httpport->setStyleSheet("background-color: #f0f0f0; color: #666;");

    }
}
// 验证网络配置
bool MainWindow::validateNetworkConfig()
{
    QString ip = ui->lineEdit_ip->text().trimmed();
    QString mqttPortStr = ui->lineEdit_mqtport->text().trimmed();
    QString httpPortStr = ui->lineEdit_httpport->text().trimmed();

    // 检查IP地址
    if (ip.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入IP地址！");
        ui->lineEdit_ip->setFocus();
        return false;
    }

    QHostAddress address;
    if (!address.setAddress(ip)) {
        QMessageBox::warning(this, "输入错误", "请输入有效的IP地址！");
        ui->lineEdit_ip->setFocus();
        return false;
    }

    // 检查MQTT端口
    if (mqttPortStr.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入MQTT端口号！");
        ui->lineEdit_mqtport->setFocus();
        return false;
    }

    int mqttPort = mqttPortStr.toInt();
    if (mqttPort < 1 || mqttPort > 65535) {
        QMessageBox::warning(this, "输入错误", "MQTT端口号必须在1-65535范围内！");
        ui->lineEdit_mqtport->setFocus();
        return false;
    }

    // 检查HTTP端口
    if (httpPortStr.isEmpty()) {
        QMessageBox::warning(this, "输入错误", "请输入HTTP端口号！");
        ui->lineEdit_httpport->setFocus();
        return false;
    }

    int httpPort = httpPortStr.toInt();
    if (httpPort < 1 || httpPort > 65535) {
        QMessageBox::warning(this, "输入错误", "HTTP端口号必须在1-65535范围内！");
        ui->lineEdit_httpport->setFocus();
        return false;
    }

    return true;
}

// 统一的服务器管理按钮点击事件
void MainWindow::on_pushButton_openmqtt_clicked()
{
    // 获取按钮当前状态
    bool isHttpRunning = (p_http && p_http->isListening());
    bool isMqttRunning = checkMqttServerRunning();
    bool anyServerRunning = isHttpRunning || isMqttRunning;

    // 如果没有任何服务器运行，则启动所有服务
    if (!anyServerRunning) {
        if (!validateNetworkConfig()) {
            return;
        }

        QString ip = ui->lineEdit_ip->text().trimmed();
        int httpPort = ui->lineEdit_httpport->text().toInt();
        int mqttPort = ui->lineEdit_mqtport->text().toInt();

        bool allStarted = true;
        QString errorMessages;

        // 1. 启动HTTP服务器
        if (!p_http) {
            p_http = new HttpServer(p_db, this);
        }

        if (p_http->listen(QHostAddress("0.0.0.0"), httpPort)) {
            qDebug() << "HTTP服务器已启动 - IP:" << "0.0.0.0" << "端口:" << httpPort;
            ui->label_edit_http->setText(QString::number(httpPort));
            ui->label_edit_http->setStyleSheet("background-color: green; color: white; padding: 2px;");

            // 连接HTTP信号
            connectHttpSignals();
        } else {
            qDebug() << "HTTP服务器启动失败！";
            ui->label_edit_http->setText("失败");
            ui->label_edit_http->setStyleSheet("background-color: red; color: white; padding: 2px;");
            errorMessages += QString("HTTP服务器启动失败 (端口%1)\n").arg(httpPort);
            allStarted = false;
        }

        // 2. 启动MQTT服务器
        if (allStarted) {
            if (!p_mqttt_ser) {
                p_mqttt_ser = new MQTT_server(p_db, this);
            }

            if (p_mqttt_ser->startServer("0.0.0.0", mqttPort)) {
                qDebug() << "MQTT服务器已启动 - IP:" << "0.0.0.0" << "端口:" << mqttPort;
                ui->label_edit_mqttser->setText(QString::number(mqttPort));
                ui->label_edit_mqttser->setStyleSheet("background-color: green; color: white; padding: 2px;");

                // 等待服务器启动
                QThread::msleep(1000);

                // 启动MQTT客户端
                startMqttClient(ip, mqttPort);
                qDebug() << "MQTT客户端已经监听 - IP:" << ip << "端口:" << mqttPort;

                // 连接HTTP和MQTT
                if (p_http && p_http->isListening() && p_mqtt_cli) {
                    connectHttpToMqtt();
                }
            } else {
                qDebug() << "MQTT服务器启动失败！";
                ui->label_edit_mqttser->setText("失败");
                ui->label_edit_mqttser->setStyleSheet("background-color: red; color: white; padding: 2px;");
                errorMessages += QString("MQTT服务器启动失败 (端口%1)\n").arg(mqttPort);

                // MQTT启动失败时，也停止HTTP（如果需要）
                if (p_http && p_http->isListening()) {
                    p_http->close();
                    ui->label_edit_http->setText("未启动");
                    ui->label_edit_http->setStyleSheet("background-color: gray; color: white; padding: 2px;");
                }
                allStarted = false;
            }
        }

        // 3. 初始化邮件服务
        if (allStarted) {
            initEmailService();
        }

        // 更新UI状态
        if (allStarted) {
            // 禁用所有网络输入控件
            ui->lineEdit_ip->setEnabled(false);
            ui->lineEdit_httpport->setEnabled(false);
            ui->lineEdit_mqtport->setEnabled(false);

            // 设置样式
            ui->lineEdit_ip->setStyleSheet("background-color: #f0f0f0; color: #666;");
            ui->lineEdit_httpport->setStyleSheet("background-color: #f0f0f0; color: #666;");
            ui->lineEdit_mqtport->setStyleSheet("background-color: #f0f0f0; color: #666;");

            // 更新按钮状态
            ui->pushButton_openmqtt->setText("关闭服务器");
            ui->pushButton_openmqtt->setStyleSheet("background-color: red; color: white;");

            // 显示成功信息
            QString successInfo = QString("所有服务器启动成功！\n\nHTTP服务：\n"
                                          "• 地址：http://%1:%2\n\n"
                                          "MQTT服务：\n"
                                          "• 地址：%1:%3")
                                      .arg(ip).arg(httpPort).arg(mqttPort);
            QMessageBox::information(this, "服务器启动成功", successInfo);
        } else {
            // 有服务启动失败
            QMessageBox::critical(this, "服务器启动失败",
                                  QString("部分服务器启动失败：\n\n%1").arg(errorMessages));
        }

    } else {
        // 已经有服务在运行，停止所有服务
        //bool allStopped = true;

        // 1. 停止HTTP服务器
        if (p_http && p_http->isListening()) {
            disconnectHttpConnections();
            p_http->close();
            ui->label_edit_http->setText("未启动");
            ui->label_edit_http->setStyleSheet("background-color: gray; color: white; padding: 2px;");
            qDebug() << "HTTP服务器已停止";
        }

        // 2. 停止MQTT服务
        stopMqttServices();
        ui->label_edit_mqttser->setText("未启动");
        ui->label_edit_mqttser->setStyleSheet("background-color: gray; color: white; padding: 2px;");
        ui->label_edit_mqttsubcrib->setText("未连接");
        ui->label_edit_mqttsubcrib->setStyleSheet("background-color: gray; color: white; padding: 2px;");

        // 3. 重置邮件服务状态
        if (p_email) {
            ui->label_edit_smtp->setText("未连接");
            ui->label_edit_smtp->setStyleSheet("background-color: gray; color: white; padding: 2px;");
        }

        // 启用所有网络输入控件
        ui->lineEdit_ip->setEnabled(true);
        ui->lineEdit_httpport->setEnabled(true);
        ui->lineEdit_mqtport->setEnabled(true);

        // 重置样式
        ui->lineEdit_ip->setStyleSheet("");
        ui->lineEdit_httpport->setStyleSheet("");
        ui->lineEdit_mqtport->setStyleSheet("");

        // 更新按钮状态
        ui->pushButton_openmqtt->setText("开启服务器");
        ui->pushButton_openmqtt->setStyleSheet("background-color: green; color: white;");

        QMessageBox::information(this, "服务器已停止", "所有服务器已成功停止 程序将退出");
        QApplication::exit(0);
    }
}
// 启动MQTT客户端
void MainWindow::startMqttClient(const QString &ip, int port)
{
    // 停止现有的MQTT客户端
    if (p_mqtt_cli) {
        disconnectMqttClientSignals();
        if (p_mqtt_cli->isRunning()) {
            p_mqtt_cli->quit();
            p_mqtt_cli->wait(3000);
        }
        p_mqtt_cli->deleteLater();
        p_mqtt_cli = nullptr;
        qDebug() << "停止现有MQTT客户端";
    }

    // 创建新的MQTT客户端
    p_mqtt_cli = new mqttclient(p_db, ip, port, "pcpcpc222333");

    // 连接客户端状态信号（使用Qt::UniqueConnection避免重复连接）
    connect(p_mqtt_cli, &mqttclient::mqttclientconnted,
            this, &MainWindow::DispayMqttclientStatus,
            Qt::UniqueConnection);

    // 启动客户端
    p_mqtt_cli->start();
    qDebug() << "MQTT客户端已启动，连接到：" << ip << ":" << port;
}

void MainWindow::initEmailService()
{
    if (!p_email) {
        p_email = new EmailSender(this);

        QString email, password;

        // 尝试从文件读取配置
        if (readSmtpConfig(email, password)) {
            qDebug() << "从配置文件读取SMTP配置成功";
        } else {
            // 如果文件不存在或读取失败，使用硬编码值
            email = "zwdz668@yeah.net";
            password = "FDcbqh9sg4Gh9cp8";
            qDebug() << "使用默认SMTP配置";
        }

        p_email->setSmtpServer("smtp.yeah.net", 465);

        if (p_email->login(email, password)) {
            qDebug() << "SMTP登录成功";
            ui->label_edit_smtp->setText("就绪");
            ui->label_edit_smtp->setStyleSheet("background-color: green; color: white; padding: 2px;");
            connectEmailSignals();
        } else {
            qDebug() << "SMTP登录失败";
            ui->label_edit_smtp->setText("失败");
            ui->label_edit_smtp->setStyleSheet("background-color: red; color: white; padding: 2px;");
        }
    }
}
// 读取SMTP配置的辅助函数
bool MainWindow::readSmtpConfig(QString &email, QString &password)
{
    QDir currentDir = QDir::current();
    QString configPath = currentDir.filePath("smtp.txt");

    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开配置文件:" << configPath;
        return false;
    }

    QTextStream in(&configFile);
    QString content = in.readAll();
    configFile.close();

    // 简单解析 EMAIL: 和 CODE: 行
    QStringList lines = content.split('\n');
    for (const QString &line : lines) {
        QString trimmed = line.trimmed();
        if (trimmed.startsWith("EMAIL:")) {
            email = trimmed.mid(6).trimmed();
        } else if (trimmed.startsWith("CODE:")) {
            password = trimmed.mid(5).trimmed();
        }
    }

    if (email.isEmpty() || password.isEmpty()) {
        qWarning() << "配置文件格式不正确";
        return false;
    }

    return true;
}
// 连接HTTP信号
void MainWindow::connectHttpSignals()
{
    if (!p_http) return;

    // 断开现有连接
    disconnectHttpConnections();

    // 连接请求信息信号
    connect(p_http, &HttpServer::sendreqInfo,
            this, &MainWindow::updateNewrqeInfo,
            Qt::UniqueConnection);

    // 连接邮件信号（如果邮件服务已启动）
    if (p_email) {
        connectEmailSignals();
    }

    // 连接HTTP和MQTT信号（如果MQTT已启动）
    if (p_mqtt_cli) {
        connectHttpToMqtt();
    }
}

// 连接HTTP和MQTT之间的信号
void MainWindow::connectHttpToMqtt()
{
    if (!p_http || !p_mqtt_cli) return;

    // 使用Qt::UniqueConnection避免重复连接
    connect(p_http, &HttpServer::devCommadSend,
            p_mqtt_cli, &mqttclient::CommandMuiltSend);

    connect(p_http, &HttpServer::devProcessSend,
            p_mqtt_cli, &mqttclient::ProcessDevtSend,
            Qt::UniqueConnection);

    connect(p_http, &HttpServer::NewDeviceCall,
            p_mqtt_cli, &mqttclient::ADDsubscribeTopic,
            Qt::UniqueConnection);

    connect(p_mqtt_cli, &mqttclient::updateDeviceInfo,
            p_http, &HttpServer::onDeviceUpdata,
            Qt::UniqueConnection);

    qDebug() << "HTTP与MQTT客户端信号连接成功";
}

// 连接邮件信号
void MainWindow::connectEmailSignals()
{
    if (!p_http || !p_email) return;

    connect(p_http, &HttpServer::sendemail,
            p_email, &EmailSender::onSendEmailRequested,
            Qt::UniqueConnection);
}

// 断开HTTP连接
void MainWindow::disconnectHttpConnections()
{
    if (!p_http) return;

    disconnect(p_http, &HttpServer::sendreqInfo, this, &MainWindow::updateNewrqeInfo);
    disconnect(p_http, &HttpServer::sendemail, nullptr, nullptr);

    if (p_mqtt_cli) {
        disconnect(p_http, &HttpServer::devCommadSend, p_mqtt_cli, &mqttclient::CommandMuiltSend);
        disconnect(p_http, &HttpServer::devProcessSend, p_mqtt_cli, &mqttclient::ProcessDevtSend);
        disconnect(p_http, &HttpServer::NewDeviceCall, p_mqtt_cli, &mqttclient::ADDsubscribeTopic);
    }

    // HTTP停止时，如果MQTT不在运行，重新启用所有控件
    if (!checkMqttServerRunning()) {
        enableNetworkControls(true);
    } else {
        // MQTT还在运行，只启用HTTP相关控件
        ui->lineEdit_ip->setEnabled(false); // IP仍然禁用
        ui->lineEdit_httpport->setEnabled(true);
        ui->lineEdit_httpport->setStyleSheet("");
    }
}
// 断开MQTT客户端信号
void MainWindow::disconnectMqttClientSignals()
{
    if (!p_mqtt_cli) return;

    disconnect(p_mqtt_cli, &mqttclient::mqttclientconnted, this, &MainWindow::DispayMqttclientStatus);
    disconnect(p_mqtt_cli, &mqttclient::updateDeviceInfo, nullptr, nullptr);
}

// 停止MQTT服务
void MainWindow::stopMqttServices()
{
    // 停止MQTT客户端
    if (p_mqtt_cli) {
        disconnectMqttClientSignals();

        if (p_mqtt_cli->isRunning()) {
            p_mqtt_cli->quit();
            if (!p_mqtt_cli->wait(3000)) {
                qWarning() << "MQTT客户端停止超时";
            }
        }

        p_mqtt_cli->deleteLater();
        p_mqtt_cli = nullptr;
        qDebug() << "MQTT客户端已停止";
    }

    // 停止MQTT服务器
    if (p_mqttt_ser) {
        p_mqttt_ser->stopServer();
        qDebug() << "MQTT服务器已停止";
    }
}

// 检查MQTT服务器是否在运行
bool MainWindow::checkMqttServerRunning()
{
    // 通过检查状态标签判断
    QString status = ui->label_edit_mqttser->text();
    return (status != "未启动" && status != "失败" && status != "已停止");
}
void MainWindow::updateNewrqeInfo(QString info)
{
    // 获取listView_ipclick模型
    QStandardItemModel* model = qobject_cast<QStandardItemModel*>(ui->listView_ipclick->model());
    if (!model) {
        model = new QStandardItemModel(this);
        ui->listView_ipclick->setModel(model);
    }

    // 添加时间戳
    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss");
    QString displayText = QString("[%1] %2").arg(timestamp).arg(info);

    // 创建新的列表项
    QStandardItem* item = new QStandardItem(displayText);

    // 限制显示的条目数量（比如最多100条）
    if (model->rowCount() >= 100) {
        model->removeRow(0); // 移除最旧的一条
    }

    // 添加到列表底部
    model->appendRow(item);

    // 自动滚动到最后一条
    QModelIndex lastIndex = model->index(model->rowCount() - 1, 0);
    ui->listView_ipclick->scrollTo(lastIndex);

    // 可选：高亮显示新条目
    ui->listView_ipclick->setCurrentIndex(lastIndex);
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

    if (!p_db || !p_http) {
        QMessageBox::warning(this, "服务器未打开", "请先打开服务器");
        return;
    }


    // 清理并显示
    if (!ui->sub_widget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    QLayout* layout = ui->sub_widget->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 创建设备列表界面
    devicelistdialog *devWidget = new devicelistdialog(p_db, &p_http->deviceVector, ui->sub_widget);

    // 连接信号
    connect(devWidget, &devicelistdialog::deviceUpgrade,
            p_mqtt_cli, &mqttclient::devUpgrade);

    connect(p_http, &HttpServer::updateDev,
            devWidget, &devicelistdialog::updatedeviceinfo,
            Qt::UniqueConnection);

    connect(devWidget, &devicelistdialog::NewDeviceCallED,
            p_mqtt_cli, &mqttclient::ADDsubscribeTopic,
            Qt::UniqueConnection);

    layout->addWidget(devWidget);
    ui->sub_widget->setVisible(true);
}
void MainWindow::updateHttpDeviceContainer()
{
    if (!p_db || !p_http) {
        QMessageBox::warning(this, "服务器未打开", "请先打开服务器");
        return;
    }


    qDebug() << "数据库已更新，开始同步到HTTP容器...";

    // 清空HTTP容器
    p_http->deviceVector.clear();

    // 从数据库获取设备列表
    QList<SQL_Device> devices = p_db->getAllDevices();

    // 转换为DeviceStatus并添加到HTTP容器
    for (const SQL_Device &sqlDevice : devices) {
        DeviceStatus deviceStatus(
            "",      // serialNumber
            "",      // status
            "",      // location
            "",      // currentAction
            "",      // trafficStatistics
            "",      // lastHeartbeat
            "",      // ip
            "",      // current_start
            "",      // current_end
            "",      // next_action
            "",      // next_action_start
            "",      // next_action_end
            "",      // usedProcess
            "",      // usedProcessID
            0        // Temperature
            );

        // 对号入座：SQL_Device -> DeviceStatus
        deviceStatus.serialNumber = sqlDevice.serial_number;
        deviceStatus.checksum = sqlDevice.checksum;
        deviceStatus.status = sqlDevice.device_status;
        deviceStatus.ip = sqlDevice.ip_address;
        // trafficStatistics 可以从 total_flow 转换
        deviceStatus.trafficStatistics = sqlDevice.total_flow;
        // bound_user 可以放在 location 或其他字段
        deviceStatus.location = sqlDevice.bound_user;
        // bound_time 可以作为 lastHeartbeat
        deviceStatus.lastHeartbeat = sqlDevice.bound_time;

        // 添加到HTTP容器
        p_http->deviceVector.append(deviceStatus);
    }

    qDebug() << "HTTP容器已更新，当前设备数量:" << p_http->deviceVector.size();
}

void MainWindow::on_pushButton_cmdquery_clicked()
{
    if (!p_db) return;

    // 清理并显示
    if (!ui->sub_widget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    QLayout* layout = ui->sub_widget->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 创建指令查询界面
    commandlsit *cmdWidget = new commandlsit(p_db, ui->sub_widget);
    layout->addWidget(cmdWidget);
    ui->sub_widget->setVisible(true);
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
    if (!p_db) return;

    // 清理并显示
    if (!ui->sub_widget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    QLayout* layout = ui->sub_widget->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 创建商城用户管理界面
    mallusermanager *managerWidget = new mallusermanager(p_db, ui->sub_widget);
    layout->addWidget(managerWidget);
    ui->sub_widget->setVisible(true);

    qDebug() << "商城用户管理界面已嵌入到子窗口";
}
void MainWindow::on_pushButton_products_clicked()
{
    if (!p_db) return;

    // 清理并显示
    if (!ui->sub_widget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    QLayout* layout = ui->sub_widget->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 创建商品管理界面
    mallproducts *productsWidget = new mallproducts(p_db, ui->sub_widget);
    layout->addWidget(productsWidget);
    ui->sub_widget->setVisible(true);
}

void MainWindow::on_pushButton_order_clicked()
{
    if (!p_db || !p_http) {
        QMessageBox::warning(this, "服务器未打开", "请先打开服务器");
        return;
    }


    // 1. 如果sub_widget已有布局，先清空内容
    if (ui->sub_widget->layout()) {
        QLayoutItem* item;
        while ((item = ui->sub_widget->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        // 清空后布局还在，但内容是空的
    } else {
        // 如果没有布局，创建一个
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    // 2. 创建订单查询界面（作为子部件）
    orderlist *orderWidget = new orderlist(ui->sub_widget);  // 父对象改为 ui->sub_widget
    orderWidget->setDatabaseManager(p_db);  // 设置数据库管理器

    // 3. 添加到现有布局
    ui->sub_widget->layout()->addWidget(orderWidget);

    // 4. 显示（嵌入模式不需要exec()）
    ui->sub_widget->setVisible(true);

    qDebug() << "订单查询界面已嵌入到子窗口";
}

void MainWindow::on_pushButton_appeal_clicked()
{
    // 清理并显示
    if (!ui->sub_widget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    QLayout* layout = ui->sub_widget->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 创建投诉处理界面
    userappeal *appealWidget = new userappeal(ui->sub_widget);

    // 如果需要设置当前用户
    // appealWidget->setCurrentUser(currentUser);

    layout->addWidget(appealWidget);
    ui->sub_widget->setVisible(true);
}

void MainWindow::on_pushButton_webUI_clicked()
{
    if (!p_db || !p_http) {
        QMessageBox::warning(this, "服务器未打开", "请先打开服务器");
        return;
    }


    // 清理并显示
    if (!ui->sub_widget->layout()) {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    QLayout* layout = ui->sub_widget->layout();
    while (QLayoutItem* item = layout->takeAt(0)) {
        if (item->widget()) item->widget()->deleteLater();
        delete item;
    }

    // 创建引导文本设置界面
    guidetextset *guideWidget = new guidetextset(ui->sub_widget);

    // 传递当前的值给设置窗口
    if (p_http) {
        guideWidget->setTextValues(&p_http->LOGIN_GUIDE_TEXT,
                                   &p_http->LOGIN_SLOGAN1,
                                   &p_http->LOGIN_SLOGAN2);
    }

    layout->addWidget(guideWidget);
    ui->sub_widget->setVisible(true);
}


void MainWindow::on_pushButton_contorluser_clicked()
{
    if (!p_db || !p_http) {
        QMessageBox::warning(this, "服务器未打开", "请先打开服务器");
        return;
    }


    // 1. 如果sub_widget已有布局，先清空内容
    if (ui->sub_widget->layout()) {
        QLayoutItem* item;
        while ((item = ui->sub_widget->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        // 清空后布局还在，但内容是空的
    } else {
        // 如果没有布局，创建一个
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    // 2. 创建用户管理界面（作为子部件）
    ManagerUI *managerWidget = new ManagerUI(p_db, ui->sub_widget);  // 父对象改为 ui->sub_widget

    // 3. 添加到现有布局
    ui->sub_widget->layout()->addWidget(managerWidget);

    // 4. 显示（嵌入模式不需要exec()）
    ui->sub_widget->setVisible(true);

    qDebug() << "用户管理界面已嵌入到子窗口";

    // 注意：如果ManagerUI需要连接信号，在这里添加connect语句
    // 例如：connect(managerWidget, &ManagerUI::someSignal, ...);
}

void MainWindow::on_pushButton_cmddispatch_clicked()
{
    if (!p_db || !p_http) {
        QMessageBox::warning(this, "服务器未打开", "请先打开服务器");
        return;
    }

    // 1. 如果sub_widget已有布局，先清空内容
    if (ui->sub_widget->layout()) {
        QLayoutItem* item;
        while ((item = ui->sub_widget->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        // 清空后布局还在，但内容是空的
    } else {
        // 如果没有布局，创建一个
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    // 2. 创建指令分发界面（作为子部件）
    commanddev *cmdWidget = new commanddev(p_db, &p_http->deviceVector, ui->sub_widget);

    // 3. 添加到现有布局（不要重新创建布局！）
    ui->sub_widget->layout()->addWidget(cmdWidget);  // 关键修改

    // 4. 连接信号 - 当用户点击"发送指令"按钮时会发射此信号
    connect(cmdWidget, &commanddev::dCommadSend,
            p_mqtt_cli, &mqttclient::CommandMuiltSend);

    // 5. 显示（嵌入模式不需要exec()）
    ui->sub_widget->setVisible(true);

    qDebug() << "指令分发界面已嵌入到子窗口";
}
void MainWindow::on_pushButton_appcount_clicked()
{
    // 1. 如果sub_widget已有布局，先清空内容
    if (ui->sub_widget->layout()) {
        QLayoutItem* item;
        while ((item = ui->sub_widget->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        // 重要：清空后布局还在，但内容是空的
    } else {
        // 如果没有布局，创建一个
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    qDebug() << "adssdad";

    // 2. 创建子界面（不需要WA_DeleteOnClose）
    appacount *p_appacount = new appacount(p_db, ui->sub_widget);

    // 3. 添加到现有布局（不要重新创建布局！）
    ui->sub_widget->layout()->addWidget(p_appacount);  // 关键修改

    // 4. 连接信号
    connect(p_appacount, &appacount::forwardJsonToMQTT,
            this, [this](const QString& deviceSerial, const QString& jsonString) {
                if (p_mqtt_cli) {
                    p_mqtt_cli->SingleTopicPub(deviceSerial, jsonString);
                }
            });
    connect(p_mqtt_cli, &mqttclient::applogginstatus,
            p_appacount, &appacount::onAppLoginStatusReceived);
    connect(p_http, &HttpServer::getCRcodeImg,
            p_appacount, &appacount::updateCRcode);

    // 5. 显示（不需要exec()）
    ui->sub_widget->setVisible(true);
}

void MainWindow::on_pushButton_withdraw_clicked()
{
    // 1. 如果sub_widget已有布局，先清空内容
    if (ui->sub_widget->layout()) {
        QLayoutItem* item;
        while ((item = ui->sub_widget->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
        // 重要：清空后布局还在，但内容是空的
    } else {
        // 如果没有布局，创建一个
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    qDebug() << "提现管理按钮被点击";

    // 2. 创建提现管理子界面
    withdraw *p_withdraw = new withdraw(ui->sub_widget, p_db);

    // 3. 添加到现有布局（不要重新创建布局！）
    ui->sub_widget->layout()->addWidget(p_withdraw);

    // 4. 显示
    ui->sub_widget->setVisible(true);
}
