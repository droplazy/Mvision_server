#include "appacount.h"
#include "ui_appacount.h"
#include "DatabaseManager.h"
#include "publicheader.h"  // 包含SQL_Device定义
#include <QDebug>
#include <QMessageBox>
#include <QTableWidgetItem>
#include "applogin2.h"

// 构造函数，接收数据库指针
appacount::appacount(DatabaseManager* dbManager, QWidget *parent)
    : QWidget(parent)  // ✅ 修改3：改为QWidget
    , ui(new Ui::appacount)
    , m_dbManager(dbManager)
{
    ui->setupUi(this);

    // 可选：如果你还想显示标题，可以这样设置
   // ui->label_title->setText("设备社交媒体状态管理");  // 假设UI中有label_title

    // 或者作为tooltip提示
    setToolTip("设备社交媒体状态管理");

    // 初始化表格
    initTable();

    // 连接搜索按钮信号
    connect(ui->pushButton_search, &QPushButton::clicked, this, &appacount::on_pushButton_search_clicked);
    // 连接表格双击事件
    connect(ui->tableWidget, &QTableWidget::cellDoubleClicked,
            this, &appacount::on_tableWidget_cellDoubleClicked);

    // 初始加载所有设备
    if (m_dbManager) {
        loadAllDevices();
    }
}


// 析构函数
appacount::~appacount()
{
    delete ui;
}

// 初始化表格
void appacount::initTable()
{
     setFixedSize(1000,620);
    // 设置表格列数和标题
    ui->tableWidget->setColumnCount(10);
    QStringList headers;
    headers << "设备号" << "TikTok" << "Bilibili" << "小红书" << "微博" << "快手"<<"预留"<<"预留"<<"预留"<<"预留";
    ui->tableWidget->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableWidget->horizontalHeader()->setStretchLastSection(true);
    ui->tableWidget->verticalHeader()->setVisible(false);

    // 设置列宽
    ui->tableWidget->setColumnWidth(0, 150);  // 设备号列宽
    for (int i = 1; i < 6; i++) {
        ui->tableWidget->setColumnWidth(i, 100);
    }
}

// 加载所有设备
void appacount::loadAllDevices()
{
    // 清空表格
    ui->tableWidget->setRowCount(0);

    if (!m_dbManager) {
        return;
    }

    // 获取所有设备
    QList<SQL_Device> devices = m_dbManager->getAllDevices();

    qDebug() << "获取到设备数量:" << devices.size();

    // 遍历设备并添加到表格
    for (const SQL_Device& device : devices) {
        addDeviceToTable(device);
    }
}

// 刷新表格数据
void appacount::refreshTableData()
{
    QString searchText = ui->lineEdit_search->text().trimmed();

    if (searchText.isEmpty()) {
        // 如果搜索框为空，加载所有设备
        loadAllDevices();
    } else {
        // 如果有搜索文本，重新执行搜索
        on_pushButton_search_clicked();
    }
}

// 添加设备到表格
void appacount::addDeviceToTable(const SQL_Device& device)
{
    int row = ui->tableWidget->rowCount();
    ui->tableWidget->insertRow(row);

    // 设备号
    QTableWidgetItem* serialItem = new QTableWidgetItem(device.serial_number);
    serialItem->setTextAlignment(Qt::AlignCenter);
    ui->tableWidget->setItem(row, 0, serialItem);

    // TikTok状态
    QString tiktokStatus = device.tiktok.isEmpty() ? "未登录" : device.tiktok;
    QTableWidgetItem* tiktokItem = new QTableWidgetItem(tiktokStatus);
    tiktokItem->setTextAlignment(Qt::AlignCenter);
    tiktokItem->setForeground(getStatusColor(tiktokStatus));
    ui->tableWidget->setItem(row, 1, tiktokItem);

    // Bilibili状态
    QString bilibiliStatus = device.bilibili.isEmpty() ? "未登录" : device.bilibili;
    QTableWidgetItem* bilibiliItem = new QTableWidgetItem(bilibiliStatus);
    bilibiliItem->setTextAlignment(Qt::AlignCenter);
    bilibiliItem->setForeground(getStatusColor(bilibiliStatus));
    ui->tableWidget->setItem(row, 2, bilibiliItem);

    // 小红书状态
    QString xhsStatus = device.xhs.isEmpty() ? "未登录" : device.xhs;
    QTableWidgetItem* xhsItem = new QTableWidgetItem(xhsStatus);
    xhsItem->setTextAlignment(Qt::AlignCenter);
    xhsItem->setForeground(getStatusColor(xhsStatus));
    ui->tableWidget->setItem(row, 3, xhsItem);

    // 微博状态
    QString weiboStatus = device.weibo.isEmpty() ? "未登录" : device.weibo;
    QTableWidgetItem* weiboItem = new QTableWidgetItem(weiboStatus);
    weiboItem->setTextAlignment(Qt::AlignCenter);
    weiboItem->setForeground(getStatusColor(weiboStatus));
    ui->tableWidget->setItem(row, 4, weiboItem);

    // 快手状态
    QString kuaishouStatus = device.kuaishou.isEmpty() ? "未登录" : device.kuaishou;
    QTableWidgetItem* kuaishouItem = new QTableWidgetItem(kuaishouStatus);
    kuaishouItem->setTextAlignment(Qt::AlignCenter);
    kuaishouItem->setForeground(getStatusColor(kuaishouStatus));
    ui->tableWidget->setItem(row, 5, kuaishouItem);
}

// 根据状态文本返回颜色
QColor appacount::getStatusColor(const QString& status)
{
    // 定义状态颜色映射
    if (status == "未登录" || status == "登录失败" || status == "失败" ||
        status == "error" || status == "fail" || status == "stopped") {
        return Qt::red;  // 红色表示失败/未登录
    }
    else if (status == "已登录" || status == "登录成功" || status == "成功" ||
             status == "success" || status == "completed" || status == "running") {
        return Qt::green;  // 绿色表示成功/已登录
    }
    else if (status == "登录中" || status == "处理中" || status == "进行中" ||
             status == "processing" || status == "running") {
        return Qt::blue;  // 蓝色表示进行中
    }
    else {
        return Qt::black;  // 默认黑色
    }
}

// 搜索按钮点击事件
void appacount::on_pushButton_search_clicked()
{
    QString searchText = ui->lineEdit_search->text().trimmed();

    if (searchText.isEmpty()) {
        // 如果搜索框为空，加载所有设备
        loadAllDevices();
        return;
    }

    // 清空表格
    ui->tableWidget->setRowCount(0);

    if (!m_dbManager) {
        return;
    }

    // 获取所有设备
    QList<SQL_Device> allDevices = m_dbManager->getAllDevices();

    // 根据搜索文本过滤设备
    int foundCount = 0;
    for (const SQL_Device& device : allDevices) {
        // 搜索设备号（模糊匹配）
        if (device.serial_number.contains(searchText, Qt::CaseInsensitive)) {
            addDeviceToTable(device);
            foundCount++;
        }
    }
}

// 更新设备状态
void appacount::updateDeviceStatus(const QString& deviceSerial, int platformColumn, const QString& newStatus)
{
    // 查找设备所在行
    for (int row = 0; row < ui->tableWidget->rowCount(); row++) {
        QTableWidgetItem* serialItem = ui->tableWidget->item(row, 0);
        if (serialItem && serialItem->text() == deviceSerial) {
            // 找到设备，更新对应平台的状态
            QTableWidgetItem* statusItem = ui->tableWidget->item(row, platformColumn);
            if (statusItem) {
                statusItem->setText(newStatus);
                statusItem->setForeground(getStatusColor(newStatus));
                qDebug() << QString("更新设备状态: %1, 平台列: %2, 新状态: %3")
                                .arg(deviceSerial)
                                .arg(platformColumn)
                                .arg(newStatus);
            }
            break;
        }
    }
}

void appacount::on_tableWidget_cellDoubleClicked(int row, int column)
{
    // 如果是第一列（设备号列），则不执行任何操作
    if (column == 0) {
        return;  // 直接返回，不弹出对话框
    }

    // 获取行名（设备号）
    QTableWidgetItem* rowItem = ui->tableWidget->item(row, 0);
    if (!rowItem) return;

    QString deviceSerial = rowItem->text();  // 设备号

    // 获取列名（平台名称）
    QString platformName = "";
    int platformColumn = -1;
    switch(column) {
    case 1:
        platformName = "TikTok";
        platformColumn = 1;
        break;
    case 2:
        platformName = "Bilibili";
        platformColumn = 2;
        break;
    case 3:
        platformName = "小红书";
        platformColumn = 3;
        break;
    case 4:
        platformName = "微博";
        platformColumn = 4;
        break;
    case 5:
        platformName = "快手";
        platformColumn = 5;
        break;
    default:
        platformName = "设备信息";
        break;
    }

    // 获取当前单元格的状态
    QTableWidgetItem* cellItem = ui->tableWidget->item(row, column);
    QString currentStatus = cellItem ? cellItem->text() : "未登录";

    qDebug() << QString("双击: 行%1, 列%2, 设备: %3, 平台: %4, 当前状态: %5")
                    .arg(row)
                    .arg(column)
                    .arg(deviceSerial)
                    .arg(platformName)
                    .arg(currentStatus);

    // 防止重复打开（检查是否已经有窗口在显示）
    static QPointer<applogin2> currentDialog = nullptr;
    if (currentDialog && currentDialog->isVisible()) {
        currentDialog->activateWindow();
        currentDialog->raise();
        return;
    }

    // 创建并显示登录对话框
    QString commandId = QString("CMD_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz"));

    currentDialog = new applogin2(deviceSerial, platformName, currentStatus, commandId,m_dbManager, this);
    currentDialog->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动删除

    // 保存对话框指针到映射中
    m_loginDialogs[commandId] = currentDialog;

    // 保存命令ID与设备号、平台列的映射
    if (platformColumn != -1) {
        m_commandDeviceMap[commandId] = QString("%1|%2").arg(deviceSerial).arg(platformColumn);
    }

    // 连接信号 - 新增的JSON发送信号
    connect(currentDialog, &applogin2::sendJsonToMQTT,
            [this](const QString& device, const QString& jsonString) {
                qDebug() << "收到JSON发送信号:";
                qDebug() << "设备:" << device;
                qDebug() << "JSON:" << jsonString;
                // 转发信号给MainWindow
                emit forwardJsonToMQTT(device, jsonString);
            });

    // 连接登录结果信号
    connect(currentDialog, &applogin2::destroyed, [this, commandId]() {
        // 对话框销毁时从映射中移除
        m_loginDialogs.remove(commandId);
        m_commandDeviceMap.remove(commandId);
    });

    // 非模态显示
    currentDialog->show();
}

// 修改接收登录结果的槽函数
void appacount::onAppLoginStatusReceived(const QString& commandId, bool success)
{
    qDebug() << "appacount收到登录结果 - 命令ID:" << commandId
             << "成功:" << success;

    // 查找对应的登录对话框
    if (m_loginDialogs.contains(commandId)) {
        QPointer<applogin2> dialog = m_loginDialogs[commandId];
        if (dialog) {
            // 根据布尔值转换状态信息
            QString status = success ? "success" : "failed";
            QString message = success ? "登录成功" : "登录失败";

            // 转发结果给登录对话框
            dialog->onLoginResultReceived(commandId, status, message);

            // 如果登录成功，更新表格中的状态
            if (success && m_commandDeviceMap.contains(commandId)) {
                QString deviceInfo = m_commandDeviceMap[commandId];
                QStringList parts = deviceInfo.split("|");
                if (parts.size() == 2) {
                    QString deviceSerial = parts[0];
                    int platformColumn = parts[1].toInt();

                    // 更新表格中该设备对应平台的状态为"已登录"
                    updateDeviceStatus(deviceSerial, platformColumn, "已登录");

                    // 同时从数据库重新加载数据以确保数据一致性
                    QTimer::singleShot(100, this, [this]() {
                        refreshTableData();
                        qDebug() << "登录成功，刷新表格数据";
                    });
                }
            }
        } else {
            // 对话框已销毁，从映射中移除
            m_loginDialogs.remove(commandId);
            m_commandDeviceMap.remove(commandId);
        }
    }
}

void appacount::updateCRcode(const QString& commandId)
{
    qDebug() << "appacount收到二维码图片 - 命令ID:" << commandId ;

    // 查找对应的登录对话框
    if (m_loginDialogs.contains(commandId)) {
        QPointer<applogin2> dialog = m_loginDialogs[commandId];
        if (dialog) {
            // 转发给对应的applogin2对话框
            dialog->onCRcodeImgReceived(commandId);


            qDebug() << "已转发二维码图片给applogin2对话框";
        } else {
            // 对话框已销毁，从映射中移除
            m_loginDialogs.remove(commandId);
            m_commandDeviceMap.remove(commandId);
            qDebug() << "applogin2对话框已销毁，无法转发二维码图片";
        }
    } else {
        qDebug() << "未找到命令ID对应的applogin2对话框:" << commandId;
    }
}
