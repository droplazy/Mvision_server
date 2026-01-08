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
    : QDialog(parent)
    , ui(new Ui::appacount)
    , m_dbManager(dbManager)  // 保存数据库指针
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("设备社交媒体状态管理");

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
    } else {
        QMessageBox::critical(this, "错误", "数据库连接无效！");
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
    // 设置表格列数和标题
    ui->tableWidget->setColumnCount(6);
    QStringList headers;
    headers << "设备号" << "TikTok" << "Bilibili" << "小红书" << "微博" << "快手";
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
// 添加双击单元格事件处理函数（在文件末尾添加）



void appacount::on_tableWidget_cellDoubleClicked(int row, int column)
{
    // 获取行名（设备号）
    QTableWidgetItem* rowItem = ui->tableWidget->item(row, 0);
    if (!rowItem) return;

    QString deviceSerial = rowItem->text();  // 设备号

    // 获取列名（平台名称）
    QString platformName = "";
    switch(column) {
    case 1: platformName = "TikTok"; break;
    case 2: platformName = "Bilibili"; break;
    case 3: platformName = "小红书"; break;
    case 4: platformName = "微博"; break;
    case 5: platformName = "快手"; break;
    default: platformName = "设备信息"; break;
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
    currentDialog = new applogin2(deviceSerial, platformName, currentStatus, this);
    currentDialog->setAttribute(Qt::WA_DeleteOnClose);  // 关闭时自动删除

    // 连接信号（如果需要处理）
    connect(currentDialog, &applogin2::loginRequested,
            [this, deviceSerial, platformName](const QString& device,
                                               const QString& platform,
                                               const QString& account,
                                               const QString& password,
                                               const QString& verifyCode) {
                qDebug() << "收到登录请求信号:";
                qDebug() << "设备:" << device;
                qDebug() << "平台:" << platform;
                qDebug() << "账号:" << account;
                qDebug() << "验证码:" << verifyCode;

                // 这里可以添加实际的登录处理逻辑
                // 例如: 更新数据库、调用API等
            });

    connect(currentDialog, &applogin2::sendCodeRequested,
            [this, deviceSerial, platformName](const QString& device,
                                               const QString& platform,
                                               const QString& account,
                                               const QString& password) {
                qDebug() << "收到发送验证码请求信号:";
                qDebug() << "设备:" << device;
                qDebug() << "平台:" << platform;
                qDebug() << "账号:" << account;

                // 这里可以添加实际的验证码发送逻辑
            });

    // 非模态显示
    currentDialog->show();
}
