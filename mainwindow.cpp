#include "mainwindow.h"
#include "ui_mainwindow.h"
//#include "loghandler.h"
#include <QMenu>
#include <QAction>
#include <QMessageBox>
#include <QVBoxLayout>
#include <QEvent>
#include <QCloseEvent>
#include <QSystemTrayIcon>
#include "ai_bragger.h"

/****************************子窗口控件*/
#include "UIclass/devicelistdialog.h"
#include "UIclass/commandlsit.h"
#include "UIclass/firmware.h"
#include "UIclass/mallusermanager.h"  // 添加头文件
#include "UIclass/mallproducts.h"
#include "UIclass/orderlist.h"
#include "UIclass/userappeal.h"
#include "UIclass/guidetextset.h"
#include "UIclass/managerui.h"
#include "UIclass/commanddev.h"
#include "UIclass/appacount.h"
#include "UIclass/withdraw.h"
#include "UIclass/livingcontrol.h"

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
    livingControlWindow = nullptr;
    // 调试：检查初始状态
    qDebug() << "=== MainWindow 初始化开始 ===";
    qDebug() << "应用程序图标状态（初始化前）:";
    qDebug() << "  - 是否为空:" << qApp->windowIcon().isNull();

    // 设置窗口标题
    setWindowTitle("后台控制系统");
    setFixedSize(this->size());

    // 初始化系统托盘（先创建，但不立即显示）
    createTrayIcon();
    createTrayMenu();

    // 连接托盘图标激活信号
    connect(trayIcon, &QSystemTrayIcon::activated,
            this, &MainWindow::onTrayIconActivated);

    // 检查图标是否已正确设置，然后才显示
    if (!trayIcon->icon().isNull()) {
        trayIcon->show();
        qDebug() << "托盘图标已显示";
    } else {
        qWarning() << "托盘图标未设置，延迟显示...";
        // 延迟显示，确保图标已加载
        QTimer::singleShot(100, this, [this]() {
            if (!trayIcon->icon().isNull()) {
                trayIcon->show();
                qDebug() << "延迟显示托盘图标成功";
            } else {
                qCritical() << "无法显示托盘图标：图标未设置";
                // 即使没有图标也显示（系统可能会提供默认图标）
                trayIcon->show();
            }
        });
    }

    // 创建定时器用于显示系统时间
    QTimer *timer = new QTimer(this);
    connect(timer, &QTimer::timeout, this, &MainWindow::updateSystemTime);
    timer->start(1000);
    updateSystemTime();

    // 初始化UI状态
    initUIStatus();

    // 初始化数据库对象
    initDatabase();


    QObject::connect(&manager, &MediaMTX_Manager::serverStarted, [](const QString &url) {
        qDebug() << "服务器启动成功";
        qDebug() << "RTSP地址:" << url;
    });

    QObject::connect(&manager, &MediaMTX_Manager::error, [](const QString &msg) {
        qDebug() << "错误:" << msg;
    });

    // 启动服务器
    QString mediamtxPath = "mediamtx/mediamtx.exe";
    if (manager.startServer(mediamtxPath)) {
        qDebug() << "服务器运行中...按Ctrl+C停止";
    } else {
        qDebug() << "启动失败";
    }

    p_ai =new AI_bragger();
    p_ai->start();

    // 创建1秒定时器
     m_oneSecondTimer = new QTimer(this);

    // 设置定时器间隔为1000毫秒（1秒）
    m_oneSecondTimer->setInterval(1000);

    // 连接定时器的timeout信号到槽函数
    connect(m_oneSecondTimer, &QTimer::timeout,
            this, &MainWindow::onOneSecondTimerTimeout);
    m_oneSecondTimer->start();
    qDebug() << "定时器已启动";

    qDebug() << "=== MainWindow 初始化完成 ===";
}

void MainWindow::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::WindowStateChange) {
        // 当窗口状态改变时
        if (isMinimized()) {
            // 窗口最小化时，可以选择隐藏窗口并显示在托盘
            hide();

            // 显示提示信息
            if (trayIcon) {
                trayIcon->showMessage(
                    "提示",
                    "程序已最小化到托盘",
                    QSystemTrayIcon::Information,
                    1500
                    );
            }
        }
    }

    QMainWindow::changeEvent(event);
}
void MainWindow::createTrayIcon()
{
    trayIcon = new QSystemTrayIcon(this);



    // 方法1：直接使用应用程序图标
    QIcon icon = qApp->windowIcon();

    if (icon.isNull()) {

        // 方法2：尝试从文件加载
        if (QFile::exists("rainbow.ico")) {
            icon = QIcon("rainbow.ico");
        }
        // 方法3：尝试从资源文件加载
        else if (QFile::exists(":/icons/app_icon.png")) {
            icon = QIcon(":/icons/app_icon.png");
        }
        // 方法4：创建默认图标
        else {
            // 创建一个简单的默认图标
            QPixmap pixmap(32, 32);
            pixmap.fill(QColor(0, 120, 215));  // 蓝色
            icon = QIcon(pixmap);
        }
    }
    // 设置图标
    trayIcon->setIcon(icon);

    // 验证图标是否已设置
    if (trayIcon->icon().isNull()) {
        qCritical() << "警告：托盘图标设置失败！";
        // 创建一个应急图标
        QPixmap emergencyPixmap(32, 32);
        emergencyPixmap.fill(Qt::red);
        trayIcon->setIcon(QIcon(emergencyPixmap));
    }

    // 设置提示文本
    trayIcon->setToolTip("后台控制系统");

}
void MainWindow::createTrayMenu()
{
    // 创建托盘菜单
    trayMenu = new QMenu(this);

    // 创建菜单项
    restoreAction = new QAction("显示主窗口", this);
    quitAction = new QAction("退出", this);

    // 连接菜单项信号
    connect(restoreAction, &QAction::triggered, this, &MainWindow::onRestoreAction);
    connect(quitAction, &QAction::triggered, this, &MainWindow::onQuitAction);

    // 添加菜单项
    trayMenu->addAction(restoreAction);
    trayMenu->addSeparator(); // 分隔线
    trayMenu->addAction(quitAction);

    // 设置托盘菜单
    trayIcon->setContextMenu(trayMenu);
}

void MainWindow::onTrayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    switch (reason) {
    case QSystemTrayIcon::DoubleClick:
        // 双击托盘图标恢复窗口
        onRestoreAction();
        break;
    case QSystemTrayIcon::Trigger:
        // 单击托盘图标（可选的附加功能）
        // trayIcon->showMessage("提示", "双击显示主窗口");
        break;
    default:
        break;
    }
}

void MainWindow::onRestoreAction()
{
    // 恢复窗口显示
    showNormal();
    activateWindow();
    raise();
}

void MainWindow::onQuitAction()
{
    // 退出程序
    trayIcon->hide(); // 隐藏托盘图标
    qApp->quit();     // 退出应用程序
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
    ui->lineEdit_ip->setText("192.168.10.103");
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

            if (p_mqtt_cli) {
                connect(p_mqtt_cli, &mqttclient::programInfoGenerated,p_ai, &AI_bragger::onProgramInfoGenerated);
                connect(p_ai, &AI_bragger::sCommadSend,p_mqtt_cli, &mqttclient::devcommandsend);
                p_ai->sethostpath(ip,"8554");


                qDebug() << "成功连接 programInfoGenerated 信号";
            } else {
                qDebug() << "警告：p_mqtt_cli 为空，无法连接信号";
            }
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

    connect(p_http, &HttpServer::forwardJsonToMQTT,
            this, [this](const QString& deviceSerial, const QString& jsonString) {
                if (p_mqtt_cli) {
                    p_mqtt_cli->SingleTopicPub(deviceSerial, jsonString);
                }
            });
    // 尝试普通连接方式（去掉UniqueConnection测试）
    bool connected = connect(p_mqtt_cli, &mqttclient::applogginstatus,
                             p_http, &HttpServer::handleAppLoginStatus,
                             Qt::AutoConnection);

    qDebug() << "信号槽连接结果:" << connected;

    // 连接邮件信号（如果邮件服务已启动）
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

void MainWindow::closeEvent(QCloseEvent *event)
{

    if (trayIcon && trayIcon->isVisible()) {
        // 隐藏窗口
        hide();

        // 显示通知消息（可选）
        trayIcon->showMessage(
            "应用程序",
            "程序已最小化到系统托盘，双击图标恢复窗口",
            QSystemTrayIcon::Information,
            2000
            );

        // 忽略关闭事件
        event->ignore();
    } else {
        // 正常关闭
        event->accept();
    }
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

void MainWindow::on_pushButton_clicked()
{
    // 创建测试数据
    AIpost a;
    a.theme = "旅游体验";
    a.text = "最近去了一趟黄山，看到了美丽的云海和奇特的松树，山上空气清新，风景如画";
    a.motion = "积极正面";
    a.scene = "旅游社交平台分享";
    a.num = 3;
    a.commandid = "TEST_001";
    xunfeiAIprase(a);
}

#include <QCryptographicHash>
#include <QMessageAuthenticationCode>

QString generateSignature(const QString &apiSecret, const QString &date, const QString &method, const QString &path)
{
    // 1. 构建签名字符串
    QString signatureOrigin = QString("host: spark-api-open.xf-yun.com\ndate: %1\n%2 %3 HTTP/1.1")
                                  .arg(date).arg(method).arg(path);

    // 2. 计算HMAC-SHA256
    QMessageAuthenticationCode hmac(QCryptographicHash::Sha256);
    hmac.setKey(apiSecret.toUtf8());
    hmac.addData(signatureOrigin.toUtf8());
    QByteArray signature = hmac.result();

    // 3. Base64编码
    return signature.toBase64();
}

void MainWindow::on_pushButton_livingcontrol_clicked()
{
    // 清理子窗口
    if (ui->sub_widget->layout()) {
        QLayoutItem* item;
        while ((item = ui->sub_widget->layout()->takeAt(0)) != nullptr) {
            if (item->widget()) {
                item->widget()->deleteLater();
            }
            delete item;
        }
    } else {
        QVBoxLayout* layout = new QVBoxLayout(ui->sub_widget);
        layout->setContentsMargins(0, 0, 0, 0);
    }

    // 创建直播控制界面并传入p_ai
    livingcontrol *liveControl = new livingcontrol(ui->sub_widget, p_ai);
    ui->sub_widget->layout()->addWidget(liveControl);

    // 显示
    ui->sub_widget->setVisible(true);

    qDebug() << "直播控制界面已创建";
}
void MainWindow::onOneSecondTimerTimeout()
{
    checkAndStartProgramSpeechRecognition();
    checkAndGenerateBragger();
}

void MainWindow::checkAndStartProgramSpeechRecognition()
{
    if (!p_ai) return;

    QVector<ProgramInfo>& programList = p_ai->ProgramList;

    for (ProgramInfo& program : programList) {
        QString commandId = program.commandId;

        if (!program.rtspurl.isEmpty() &&
            program.isStreaming &&
            program.voicetotext.isEmpty() &&
            !program.isListen &&
            !m_programSpeechRecognizers.contains(commandId)) {

            program.isListen = true;

            RealtimeSpeechRecognizer* recognizer = new RealtimeSpeechRecognizer(this);

            RealtimeSpeechRecognizer::Config config;
            config.appId = "318eeb03";
            config.apiKey = "0731bdabe8a186215737d1edeb15b9ea";
            config.apiSecret = "MGM2NGNlYWM4NTA3Mzc3ZmY4ODIzZmZh";
            config.ffmpegPath = QDir::current().filePath("ffmpeg/bin/ffmpeg.exe");

            recognizer->setConfig(config);

            // 为每个节目创建两个临时变量
            QString* currentSentence = new QString();  // 当前句子
            QString* historyText = new QString();      // 历史文本

            connect(recognizer, &RealtimeSpeechRecognizer::textReceived,
                    this, [this, commandId, currentSentence, historyText](const QString &text) {
                        // 判断是否新句子开始（比当前句子短）
                        if (text.length() < currentSentence->length()) {
                            // 将当前句子拼接到历史文本
                            if (!currentSentence->isEmpty()) {
                                if (!historyText->isEmpty()) {
                                    *historyText += "，";
                                }
                                *historyText += *currentSentence;
                            }
                            // 开始新句子
                            *currentSentence = text;
                        }
                        else {
                            // 当前句子的修正
                            *currentSentence = text;
                        }
                    });

            connect(recognizer, &RealtimeSpeechRecognizer::sessionCompleted,
                    this, [this, commandId, currentSentence, historyText]() {
                        // 处理最后一个句子
                        if (!currentSentence->isEmpty()) {
                            if (!historyText->isEmpty()) {
                                *historyText += "，";
                            }
                            *historyText += *currentSentence;
                        }

                        // 生成最终文本
                        QString finalText = *historyText;

                        // 只在有文本内容时才更新program
                        if (!finalText.isEmpty()) {
                            updateProgramVoiceText(commandId, finalText);
                        }

                        // 设置isListen为false
                        for (ProgramInfo& prog : p_ai->ProgramList) {
                            if (prog.commandId == commandId) {
                                prog.isListen = false;
                                break;
                            }
                        }

                        // 清理识别器
                        if (m_programSpeechRecognizers.contains(commandId)) {
                            RealtimeSpeechRecognizer* rec = m_programSpeechRecognizers[commandId];
                            rec->deleteLater();
                            m_programSpeechRecognizers.remove(commandId);
                        }

                        delete currentSentence;
                        delete historyText;
                    });

            connect(recognizer, &RealtimeSpeechRecognizer::errorOccurred,
                    this, [this, commandId, currentSentence, historyText](const QString &error) {
                        // 出错时保存已有文本
                        QString finalText = *historyText;
                        if (!finalText.isEmpty() && !currentSentence->isEmpty()) {
                            finalText += "，";
                        }
                        finalText += *currentSentence;

                        if (!finalText.isEmpty()) {
                            updateProgramVoiceText(commandId, finalText);
                        }

                        for (ProgramInfo& prog : p_ai->ProgramList) {
                            if (prog.commandId == commandId) {
                                prog.isListen = false;
                                break;
                            }
                        }

                        if (m_programSpeechRecognizers.contains(commandId)) {
                            RealtimeSpeechRecognizer* rec = m_programSpeechRecognizers[commandId];
                            rec->deleteLater();
                            m_programSpeechRecognizers.remove(commandId);
                        }

                        delete currentSentence;
                        delete historyText;
                    });

            if (recognizer->startRecognition(program.rtspurl)) {
                m_programSpeechRecognizers[commandId] = recognizer;
            } else {
                program.isListen = false;
                delete currentSentence;
                delete historyText;
                recognizer->deleteLater();
            }
        }
    }
}
// updateProgramVoiceText函数保持不变
void MainWindow::updateProgramVoiceText(const QString &commandId, const QString &text)
{
    if (!p_ai) return;

    for (ProgramInfo& prog : p_ai->ProgramList) {
        if (prog.commandId == commandId) {
            prog.voicetotext = text;
            break;
        }
    }
}

void MainWindow::xunfeiAIprase(const AIpost &aiPost)
{
    QString commandId = aiPost.commandid;

    qDebug() << "AI生成请求 - 节目:" << commandId;
    qDebug() << "参数: 主题=" << aiPost.theme
             << " 场景=" << aiPost.scene
             << " 情绪=" << aiPost.motion
             << " 条数=" << aiPost.num;

    // 构建更丰富的AI提示词
    QString prompt;

    if (!aiPost.guide.isEmpty()) {
        // 使用引导词作为开头
        prompt = aiPost.guide + "\n\n";
    } else {
        // 默认开头
    }

    prompt = "请帮我生成用户评论：\n\n";

    // 添加主题要求
    if (!aiPost.theme.isEmpty()) {
        prompt += QString("【主题要求】%1\n").arg(aiPost.theme);
    }

    // 添加场景要求
    if (!aiPost.scene.isEmpty()) {
        prompt += QString("【使用场景】%1\n").arg(aiPost.scene);
    }

    // 添加情绪要求
    if (!aiPost.motion.isEmpty()) {
        prompt += QString("【情感基调】%1\n").arg(aiPost.motion);
    }
    // 添加情绪要求
    if (!aiPost.guide.isEmpty()) {
        prompt += QString("【其他参考】%1\n").arg(aiPost.guide);
    }
    // 添加内容参考
    prompt += QString("【视频内容】%1\n\n").arg(aiPost.text);

    // 添加数量要求
    prompt += QString("【生成数量】%1条\n\n").arg(aiPost.num);

    // 通用要求
    prompt += "【具体要求】\n";
    prompt += "1. 每条评论都要独特、不重复\n";
    prompt += "2. 语言自然口语化，像真人写的\n";
    prompt += "3. 每条评论长度8-20字\n";
    prompt += "4. 符合指定的主题、场景和情感基调\n\n";

    // 格式要求
    prompt += "【回复格式】\n";
    prompt += "请严格按照以下格式回复，每条评论用方括号包裹：\n";
    prompt += "[第一条评论内容]\n";
    prompt += "[第二条评论内容]\n";
    prompt += "[...]\n";

    // 连接信号槽
    connect(&ai, &SimpleXFAI::responseReceived,
            this, [this, commandId](const QString &response) {
                qDebug() << "收到AI回复，节目:" << commandId;

                // 解析评论
                //QStringList comments = parseBracketComments(response);

                if (!response.isEmpty()) {
                   // QString allComments = comments.join("；");
                    updateProgramBragger(commandId, response);
                  //  qDebug() << "生成" << comments.size() << "条评论";
                } else {
                    updateProgramBragger(commandId, response);
                    qDebug() << "未能解析出格式，使用原始回复";
                }

                // 断开连接
                disconnect(&ai, &SimpleXFAI::responseReceived, this, 0);
            });

    connect(&ai, &SimpleXFAI::errorOccurred,
            this, [this, commandId](const QString &error) {
                qDebug() << "AI请求错误，节目:" << commandId << "错误:" << error;
                resetProgramGenerating(commandId);
            });


    // 发送请求
    qDebug() << "AI post" << prompt;
    ai.askQuestion(prompt);
}
void MainWindow::resetProgramGenerating(const QString &commandId)
{
    if (!p_ai) return;

    for (ProgramInfo &program : p_ai->ProgramList) {
        if (program.commandId == commandId) {
            program.isGenerating = false;
            qDebug() << "✅ 重置节目" << commandId << "的isGenerating状态";
            break;
        }
    }
}

QStringList MainWindow::parseBracketComments(const QString &response)
{
    QStringList comments;
    QRegularExpression regex("\\[(.*?)\\]");
    QRegularExpressionMatchIterator it = regex.globalMatch(response);

    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString comment = match.captured(1).trimmed();
        if (!comment.isEmpty()) comments << comment;
    }
    return comments;
}

void MainWindow::updateProgramBragger(const QString &commandId, const QString &comments)
{
    if (!p_ai) return;
    for (ProgramInfo &program : p_ai->ProgramList) {
        if (program.commandId == commandId) {
            program.bragger = comments;
            program.isGenerating = false;
            break;
        }
    }
}
void MainWindow::checkAndGenerateBragger()
{
    if (!p_ai) return;

    QVector<ProgramInfo>& programList = p_ai->ProgramList;

// 调试开关 - 完成调试后把这行注释掉
//#define DEBUG_AI_GENERATE

#ifdef DEBUG_AI_GENERATE
    qDebug() << "=== AI生成检查开始 ===";
#endif

    for (ProgramInfo& program : programList) {
        QString commandId = program.commandId;

#ifdef DEBUG_AI_GENERATE
        bool condition1 = !program.voicetotext.isEmpty();
        bool condition2 = program.bragger.isEmpty();
        bool condition3 = !program.isGenerating;
        bool condition4 = !program.isListen;
        bool allConditions = condition1 && condition2 && condition3 && condition4;

        qDebug() << QString("节目[%1]: voicetotext=%2 bragger=%3 isGen=%4 isListen=%5")
                        .arg(commandId)
                        .arg(condition1 ? "有" : "无")
                        .arg(condition2 ? "空" : "有")
                        .arg(condition3 ? "否" : "是")
                        .arg(condition4 ? "否" : "是");
#endif

        if (!program.voicetotext.isEmpty() &&
            program.bragger.isEmpty() &&
            !program.isGenerating && !program.isListen) {

            if (program.voicetotext.trimmed().length() < 15) {
#ifdef DEBUG_AI_GENERATE
                qDebug() << QString("  长度不足: %1 < 15").arg(program.voicetotext.trimmed().length());
#endif
                program.voicetotext.clear();
                program.bragger.clear();
                program.isListen =false;
                program.isGenerating =false;
                qDebug() << "重置数据";

                continue;
            }

#ifdef DEBUG_AI_GENERATE
            qDebug() << QString("  ✓ 满足条件，开始生成AI评论");
#endif

            program.isGenerating = true;

            AIpost aiRequest;
            aiRequest.commandid = commandId;
            aiRequest.text = program.voicetotext;
            aiRequest.theme = program.theme;
            aiRequest.scene = program.scene;
            aiRequest.motion = program.motion;
            aiRequest.guide = program.guideword;
            aiRequest.num = program.deviceList.size();

#ifdef DEBUG_AI_GENERATE
            qDebug() << QString("  设备数: %1, 主题: %2").arg(aiRequest.num).arg(aiRequest.theme);
#endif

            xunfeiAIprase(aiRequest);
            break;
        }
    }

#ifdef DEBUG_AI_GENERATE
    qDebug() << "=== AI生成检查结束 ===\n";
#endif
}
