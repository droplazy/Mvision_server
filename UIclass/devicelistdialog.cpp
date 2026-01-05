// devicelistdialog.cpp
#include "devicelistdialog.h"
#include "ui_devicelistdialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <./UIclass/insertdev.h>
#include <QHBoxLayout>



devicelistdialog::devicelistdialog(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::devicelistdialog)
    , m_dbManager(dbManager)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("设备列表");
    setFixedSize(this->size());
    // 设置表格
    setupTableView();

    // 加载设备列表
    loadDeviceList();
}

devicelistdialog::~devicelistdialog()
{
    delete ui;
}

void devicelistdialog::setupTableView()
{
    // 创建表格模型
    QStandardItemModel *model = new QStandardItemModel(this);

    // 设置表头 - 最后一列添加"操作"
    model->setHorizontalHeaderLabels({
        "设备编号", "IP地址", "状态", "绑定用户",
        "累计流量", "校验码", "绑定时间", "操作"
    });

    ui->tableView->setModel(model);

    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);

    // 禁止编辑表格内容
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置选择模式
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置列宽
    ui->tableView->setColumnWidth(0, 120);  // 设备编号
    ui->tableView->setColumnWidth(1, 120);  // IP地址
    ui->tableView->setColumnWidth(2, 80);   // 状态
    ui->tableView->setColumnWidth(3, 100);  // 绑定用户
    ui->tableView->setColumnWidth(4, 100);  // 累计流量
    ui->tableView->setColumnWidth(5, 100);  // 校验码
    ui->tableView->setColumnWidth(6, 150);  // 绑定时间
    ui->tableView->setColumnWidth(7, 80);   // 操作列
}
void devicelistdialog::loadDeviceList()
{
    if (!m_dbManager) {
        return;
    }

    // 获取设备列表
    QList<SQL_Device> devices = m_dbManager->getAllDevices();

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model) return;

    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 填充数据
    int row = 0;
    for (const SQL_Device &device : devices) {
        // 设备编号
        QStandardItem *serialItem = new QStandardItem(device.serial_number);
        serialItem->setEditable(false);
        model->setItem(row, 0, serialItem);

        // IP地址
        QStandardItem *ipItem = new QStandardItem(device.ip_address);
        ipItem->setEditable(false);
        model->setItem(row, 1, ipItem);

        // 状态
        QStandardItem *statusItem = new QStandardItem(device.device_status);
        statusItem->setEditable(false);
        // 根据状态设置颜色
        if (device.device_status == "在线" || device.device_status.toLower() == "online") {
            statusItem->setForeground(Qt::darkGreen);
            statusItem->setText("在线");
        } else if (device.device_status == "离线" || device.device_status.toLower() == "offline") {
            statusItem->setForeground(Qt::darkRed);
            statusItem->setText("离线");
        } else if (device.device_status == "忙碌" || device.device_status.toLower() == "busy") {
            statusItem->setForeground(Qt::darkYellow);
            statusItem->setText("忙碌");
        }
        model->setItem(row, 2, statusItem);

        // 绑定用户
        QStandardItem *userItem = new QStandardItem(
            device.bound_user.isEmpty() ? "未绑定" : device.bound_user);
        userItem->setEditable(false);
        model->setItem(row, 3, userItem);

        // 累计流量
        QStandardItem *flowItem = new QStandardItem(device.total_flow);
        flowItem->setEditable(false);
        model->setItem(row, 4, flowItem);

        // 校验码
        QStandardItem *checksumItem = new QStandardItem(device.checksum);
        checksumItem->setEditable(false);
        model->setItem(row, 5, checksumItem);

        // 绑定时间
        QStandardItem *timeItem = new QStandardItem(
            device.bound_time.isEmpty() ? "未绑定" : device.bound_time);
        timeItem->setEditable(false);
        model->setItem(row, 6, timeItem);

        // 添加删除按钮到第7列
        addDeleteButtonToRow(row, device.serial_number);

        row++;
    }

    // 更新标题显示设备数量
    setWindowTitle(QString("设备列表（共 %1 台设备）").arg(devices.size()));
}

void devicelistdialog::addDeleteButtonToRow(int row, const QString &serialNumber)
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model) return;

    // 创建包含按钮的widget
    QWidget *widget = new QWidget();
    QPushButton *deleteButton = new QPushButton("删除", widget);

    // 设置按钮样式
    deleteButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #ff6b6b;"
        "    color: white;"
        "    border: none;"
        "    padding: 5px 10px;"
        "    border-radius: 3px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #ff5252;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #ff3838;"
        "}"
        );

    // 设置按钮大小
    deleteButton->setFixedSize(60, 25);

    // 布局
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(deleteButton);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);

    // 将widget设置到表格
    ui->tableView->setIndexWidget(model->index(row, 7), widget);

    // 连接按钮点击信号，使用lambda传递设备编号
    connect(deleteButton, &QPushButton::clicked, [this, serialNumber]() {
        onDeleteButtonClicked(serialNumber);
    });
}


void devicelistdialog::onDeleteButtonClicked(const QString &serialNumber)
{
    if (serialNumber.isEmpty()) return;

    // 确认删除
    int result = QMessageBox::question(this, "确认删除",
                                       QString("确定要删除设备 <b>%1</b> 吗？\n此操作不可撤销！").arg(serialNumber),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (result == QMessageBox::Yes) {
        deleteDevice(serialNumber);
    }
}

void devicelistdialog::deleteDevice(const QString &serialNumber)
{
    if (!m_dbManager) {
        QMessageBox::critical(this, "错误", "数据库未初始化");
        return;
    }

    // 从数据库删除
    if (m_dbManager->deleteDevice(serialNumber)) {
        QMessageBox::information(this, "成功", "设备删除成功");

        // 重新加载设备列表
        loadDeviceList();
        emit deviceListUpdated();

        qDebug() << "设备删除成功：" << serialNumber;
    } else {
        QMessageBox::critical(this, "错误", "设备删除失败，请检查数据库连接");
    }
}
void devicelistdialog::on_button_search_clicked()
{
    if (!m_dbManager) {
        return;
    }

    // 获取搜索关键词
    QString keyword = ui->lineEdit_search->text().trimmed();

    // 获取所有设备
    QList<SQL_Device> allDevices = m_dbManager->getAllDevices();

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model) return;

    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 如果没有关键词，显示所有设备
    if (keyword.isEmpty()) {
        loadDeviceList();
        return;
    }

    QString searchText = keyword.toLower();
    QList<SQL_Device> filteredDevices;

    // 过滤设备
    for (const SQL_Device &device : allDevices) {
        // 搜索设备编号、IP地址、状态、绑定用户、校验码
        if (device.serial_number.toLower().contains(searchText) ||
            device.ip_address.toLower().contains(searchText) ||
            device.device_status.toLower().contains(searchText) ||
            device.bound_user.toLower().contains(searchText) ||
            device.checksum.toLower().contains(searchText)) {
            filteredDevices.append(device);
        }
    }

    // 填充过滤后的数据
    int row = 0;
    for (const SQL_Device &device : filteredDevices) {
        // 设备编号
        QStandardItem *serialItem = new QStandardItem(device.serial_number);
        serialItem->setEditable(false);
        model->setItem(row, 0, serialItem);

        // IP地址
        QStandardItem *ipItem = new QStandardItem(device.ip_address);
        ipItem->setEditable(false);
        model->setItem(row, 1, ipItem);

        // 状态
        QStandardItem *statusItem = new QStandardItem(device.device_status);
        statusItem->setEditable(false);
        // 根据状态设置颜色
        if (device.device_status == "在线" || device.device_status.toLower() == "online") {
            statusItem->setForeground(Qt::darkGreen);
            statusItem->setText("在线");
        } else if (device.device_status == "离线" || device.device_status.toLower() == "offline") {
            statusItem->setForeground(Qt::darkRed);
            statusItem->setText("离线");
        } else if (device.device_status == "忙碌" || device.device_status.toLower() == "busy") {
            statusItem->setForeground(Qt::darkYellow);
            statusItem->setText("忙碌");
        }
        model->setItem(row, 2, statusItem);

        // 绑定用户
        QStandardItem *userItem = new QStandardItem(
            device.bound_user.isEmpty() ? "未绑定" : device.bound_user);
        userItem->setEditable(false);
        model->setItem(row, 3, userItem);

        // 累计流量
        QStandardItem *flowItem = new QStandardItem(device.total_flow);
        flowItem->setEditable(false);
        model->setItem(row, 4, flowItem);

        // 校验码
        QStandardItem *checksumItem = new QStandardItem(device.checksum);
        checksumItem->setEditable(false);
        model->setItem(row, 5, checksumItem);

        // 绑定时间
        QStandardItem *timeItem = new QStandardItem(
            device.bound_time.isEmpty() ? "未绑定" : device.bound_time);
        timeItem->setEditable(false);
        model->setItem(row, 6, timeItem);

        // 添加删除按钮
        addDeleteButtonToRow(row, device.serial_number);

        row++;
    }

    // 更新标题显示设备数量
    setWindowTitle(QString("设备列表（共 %1 台设备，搜索到 %2 台）")
                       .arg(allDevices.size())
                       .arg(filteredDevices.size()));
}



void devicelistdialog::on_button_adddev_clicked()
{
    if (!m_dbManager) {
        QMessageBox::warning(this, "错误", "数据库未初始化");
        return;
    }

    // 创建插入设备对话框
    insertdev dialog(this);
    dialog.setModal(true);

    // 显示对话框并等待用户输入
    if (dialog.exec() == QDialog::Accepted) {
        // 获取用户输入的数据
        QString serialNumber = dialog.getSerialNumber();
        QString checksum = dialog.getChecksum();
        QString belongUser = dialog.getBelongUser();

        // 验证数据
        if (serialNumber.isEmpty() || checksum.isEmpty()) {
            QMessageBox::warning(this, "警告", "设备编号和校验码不能为空");
            return;
        }

        // 检查设备是否已存在
        SQL_Device existingDevice = m_dbManager->getDeviceBySerialNumber(serialNumber);
        if (!existingDevice.serial_number.isEmpty()) {
            QMessageBox::warning(this, "警告",
                                 QString("设备 %1 已存在！").arg(serialNumber));
            return;
        }

        // 插入新设备
        if (insertDevice(serialNumber, checksum, belongUser)) {
            QMessageBox::information(this, "成功", "设备添加成功");

            // 重新加载设备列表
            loadDeviceList();
            emit deviceListUpdated();
            // 清空搜索框（如果有）
            ui->lineEdit_search->clear();
        } else {
            QMessageBox::critical(this, "错误", "设备添加失败");
        }
    }
}


bool devicelistdialog::insertDevice(const QString &serialNumber, const QString &checksum, const QString &belongUser)
{
    // 创建设备对象
    SQL_Device newDevice;
    newDevice.serial_number = serialNumber;
    newDevice.checksum = checksum;
    newDevice.total_flow = "0";
    newDevice.ip_address = "unknown";
    newDevice.device_status = "离线";
    newDevice.bound_user = belongUser.isEmpty() ? "未绑定" : belongUser;
    newDevice.bound_time = belongUser.isEmpty() ? "" : QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    // 插入数据库
    return m_dbManager->insertDevice(newDevice);
}
