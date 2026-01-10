#include "devicelistdialog.h"
#include "ui_devicelistdialog.h"
#include <QStandardItemModel>
#include <QMessageBox>
#include <./UIclass/insertdev.h>
#include <QHBoxLayout>
#include <QDateTime>
#include <algorithm>
#include <publicheader.h>
#include <QFile>
#include <QFileInfo>
#include <QDir>
devicelistdialog::devicelistdialog(DatabaseManager* dbManager, QVector<DeviceStatus> *deviceVector, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::devicelistdialog)
    , m_dbManager(dbManager)
    , m_deviceVector(deviceVector)
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

void devicelistdialog::updatedeviceinfo()
{
    // 检查设备容器是否有效
    if (!m_deviceVector) {
        return;
    }

    // 检查表格模型是否存在
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model) {
        return;
    }

    // 遍历所有行，更新对应的设备信息
    int rowCount = model->rowCount();
    for (int row = 0; row < rowCount; row++) {
        // 获取设备编号
        QModelIndex index = model->index(row, 0);
        QString serialNumber = model->data(index).toString();

        if (serialNumber.isEmpty()) {
            continue;
        }

        // 在容器中查找对应的设备
        DeviceStatus* device = findDeviceInVector(serialNumber);
        if (!device) {
            // 设备在容器中不存在，从表格中移除该行
            model->removeRow(row);
            row--;  // 调整索引
            rowCount--;
            continue;
        }

        // 更新校验码
        QStandardItem* checksumItem = model->item(row, 1);
        if (checksumItem) {
            checksumItem->setText(device->checksum);
        }

        // 更新状态
        QStandardItem* statusItem = model->item(row, 2);
        if (statusItem) {
            statusItem->setText(device->status);
            // 根据状态更新颜色
            if (device->status == "在线" || device->status.toLower() == "online") {
                statusItem->setForeground(Qt::darkGreen);
                statusItem->setText("在线");
            } else if (device->status == "离线" || device->status.toLower() == "offline") {
                statusItem->setForeground(Qt::darkRed);
                statusItem->setText("离线");
            } else if (device->status == "忙碌" || device->status.toLower() == "busy") {
                statusItem->setForeground(Qt::darkYellow);
                statusItem->setText("忙碌");
            } else {
                statusItem->setForeground(Qt::black);
            }
        }

        // 更新当前动作
        QStandardItem* actionItem = model->item(row, 3);
        if (actionItem) {
            actionItem->setText(device->currentAction.isEmpty() ? "空闲" : device->currentAction);
        }

        // 更新流量统计
        QStandardItem* flowItem = model->item(row, 4);
        if (flowItem) {
            flowItem->setText(device->trafficStatistics);
        }

        // 更新最后心跳
        QStandardItem* heartbeatItem = model->item(row, 5);
        if (heartbeatItem) {
            heartbeatItem->setText(device->lastHeartbeat);
        }

        // 更新IP地址
        QStandardItem* ipItem = model->item(row, 6);
        if (ipItem) {
            ipItem->setText(device->ip);
        }

        // 更新温度
        QStandardItem* tempItem = model->item(row, 7);
        if (tempItem) {
            tempItem->setText(QString::number(device->Temperature, 'f', 1) + "°C");
            // 根据温度更新颜色
            if (device->Temperature > 70.0) {
                tempItem->setForeground(Qt::red);
            } else if (device->Temperature > 50.0) {
                tempItem->setForeground(Qt::darkYellow);
            } else {
                tempItem->setForeground(Qt::black);
            }
        }

        // 更新硬件版本
        QStandardItem* hwItem = model->item(row, 8);
        if (hwItem) {
            hwItem->setText(device->hardversion);
        }

        // 更新使用流程ID
        QStandardItem* processIdItem = model->item(row, 9);
        if (processIdItem) {
            processIdItem->setText(device->usedProcessID);
        }
    }

    // 检查是否有新设备需要添加
    // 遍历容器中的所有设备
    for (const DeviceStatus& device : *m_deviceVector) {
        bool found = false;

        // 在表格中查找设备
        for (int row = 0; row < model->rowCount(); row++) {
            QStandardItem* item = model->item(row, 0);
            if (item && item->text() == device.serialNumber) {
                found = true;
                break;
            }
        }

        // 如果没找到，说明是新设备，添加到表格末尾
        if (!found) {
            int newRow = model->rowCount();

            // 添加新行
            QStandardItem* serialItem = new QStandardItem(device.serialNumber);
            serialItem->setEditable(false);
            model->setItem(newRow, 0, serialItem);

            QStandardItem* checksumItem = new QStandardItem(device.checksum);
            checksumItem->setEditable(false);
            model->setItem(newRow, 1, checksumItem);

            QStandardItem* statusItem = new QStandardItem(device.status);
            statusItem->setEditable(false);
            if (device.status == "在线" || device.status.toLower() == "online") {
                statusItem->setForeground(Qt::darkGreen);
                statusItem->setText("在线");
            } else if (device.status == "离线" || device.status.toLower() == "offline") {
                statusItem->setForeground(Qt::darkRed);
                statusItem->setText("离线");
            } else if (device.status == "忙碌" || device.status.toLower() == "busy") {
                statusItem->setForeground(Qt::darkYellow);
                statusItem->setText("忙碌");
            }
            model->setItem(newRow, 2, statusItem);

            // ... 其他列的初始化与 loadDeviceList 中类似 ...
            QStandardItem* actionItem = new QStandardItem(
                device.currentAction.isEmpty() ? "空闲" : device.currentAction);
            actionItem->setEditable(false);
            model->setItem(newRow, 3, actionItem);

            QStandardItem* flowItem = new QStandardItem(device.trafficStatistics);
            flowItem->setEditable(false);
            model->setItem(newRow, 4, flowItem);

            QStandardItem* heartbeatItem = new QStandardItem(device.lastHeartbeat);
            heartbeatItem->setEditable(false);
            model->setItem(newRow, 5, heartbeatItem);

            QStandardItem* ipItem = new QStandardItem(device.ip);
            ipItem->setEditable(false);
            model->setItem(newRow, 6, ipItem);

            QStandardItem* tempItem = new QStandardItem(QString::number(device.Temperature, 'f', 1) + "°C");
            tempItem->setEditable(false);
            if (device.Temperature > 70.0) {
                tempItem->setForeground(Qt::red);
            } else if (device.Temperature > 50.0) {
                tempItem->setForeground(Qt::darkYellow);
            }
            model->setItem(newRow, 7, tempItem);

            QStandardItem* hwItem = new QStandardItem(device.hardversion);
            hwItem->setEditable(false);
            model->setItem(newRow, 8, hwItem);

            QStandardItem* processIdItem = new QStandardItem(device.usedProcessID);
            processIdItem->setEditable(false);
            model->setItem(newRow, 9, processIdItem);

            // 添加删除按钮
            addDeleteButtonToRow(newRow, device.serialNumber);
        }
    }

    // 更新标题显示设备数量
    setWindowTitle(QString("设备列表（共 %1 台设备）最后更新：%2")
                       .arg(m_deviceVector->size())
                       .arg(QDateTime::currentDateTime().toString("hh:mm:ss")));
        updateVersionLabel();
}


void devicelistdialog::setupTableView()
{
    // 创建表格模型
    QStandardItemModel *model = new QStandardItemModel(this);

    // 设置表头 - 根据要求设置
    model->setHorizontalHeaderLabels({
        "设备编号", "校验码", "状态", "当前动作",
        "流量统计", "最后心跳", "IP地址", "温度",
        "硬件版本", "使用流程ID", "操作"
    });

    ui->tableView->setModel(model);

    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->setColumnWidth(0, 140);  // 设备编号 (原120+20=140)
    ui->tableView->setColumnWidth(1, 50);   // 校验码 (原100-20=80)
    ui->tableView->setColumnWidth(3, 50);  // 当前动作 (原120+30=150)
    ui->tableView->setColumnWidth(2, 80);   // 状态 (不变)
    ui->tableView->setColumnWidth(4, 100);  // 流量统计 (不变)
    ui->tableView->setColumnWidth(5, 150);  // 最后心跳 (不变)
    ui->tableView->setColumnWidth(6, 120);  // IP地址 (不变)
    ui->tableView->setColumnWidth(7, 80);   // 温度 (不变)
    ui->tableView->setColumnWidth(8, 100);  // 硬件版本 (不变)
    ui->tableView->setColumnWidth(9, 100);  // 使用流程ID (不变)
    ui->tableView->setColumnWidth(10, 80);  // 操作列 (不变)

    // 设置列宽自适应内容（可选）
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Interactive);
}

void devicelistdialog::loadDeviceList()
{
    if (!m_deviceVector) {
        return;
    }

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView->model());
    if (!model) return;

    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 从容器加载数据
    int row = 0;
    for (const DeviceStatus &device : *m_deviceVector) {
        // 设备编号
        QStandardItem *serialItem = new QStandardItem(device.serialNumber);
        serialItem->setEditable(false);
        model->setItem(row, 0, serialItem);

        // 校验码
        QStandardItem *checksumItem = new QStandardItem(device.checksum);
        checksumItem->setEditable(false);
        model->setItem(row, 1, checksumItem);

        // 状态
        QStandardItem *statusItem = new QStandardItem(device.status);
        statusItem->setEditable(false);
        // 根据状态设置颜色
        if (device.status == "在线" || device.status.toLower() == "online") {
            statusItem->setForeground(Qt::darkGreen);
            statusItem->setText("在线");
        } else if (device.status == "离线" || device.status.toLower() == "offline") {
            statusItem->setForeground(Qt::darkRed);
            statusItem->setText("离线");
        } else if (device.status == "忙碌" || device.status.toLower() == "busy") {
            statusItem->setForeground(Qt::darkYellow);
            statusItem->setText("忙碌");
        }
        model->setItem(row, 2, statusItem);

        // 当前动作
        QStandardItem *actionItem = new QStandardItem(
            device.currentAction.isEmpty() ? "空闲" : device.currentAction);
        actionItem->setEditable(false);
        model->setItem(row, 3, actionItem);

        // 流量统计
        QStandardItem *flowItem = new QStandardItem(device.trafficStatistics);
        flowItem->setEditable(false);
        model->setItem(row, 4, flowItem);

        // 最后心跳
        QStandardItem *heartbeatItem = new QStandardItem(device.lastHeartbeat);
        heartbeatItem->setEditable(false);
        model->setItem(row, 5, heartbeatItem);

        // IP地址
        QStandardItem *ipItem = new QStandardItem(device.ip);
        ipItem->setEditable(false);
        model->setItem(row, 6, ipItem);

        // 温度
        QStandardItem *tempItem = new QStandardItem(QString::number(device.Temperature, 'f', 1) + "°C");
        tempItem->setEditable(false);
        // 根据温度设置颜色
        if (device.Temperature > 70.0) {
            tempItem->setForeground(Qt::red);
        } else if (device.Temperature > 50.0) {
            tempItem->setForeground(Qt::darkYellow);
        }
        model->setItem(row, 7, tempItem);

        // 硬件版本
        QStandardItem *hwItem = new QStandardItem(device.hardversion);
        hwItem->setEditable(false);
        model->setItem(row, 8, hwItem);

        // 使用流程ID
        QStandardItem *processIdItem = new QStandardItem(device.usedProcessID);
        processIdItem->setEditable(false);
        model->setItem(row, 9, processIdItem);

        // 添加删除按钮到第10列
        addDeleteButtonToRow(row, device.serialNumber);

        row++;
    }

    // 更新标题显示设备数量
    setWindowTitle(QString("设备列表（共 %1 台设备）").arg(m_deviceVector->size()));
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
    ui->tableView->setIndexWidget(model->index(row, 10), widget);

    // 连接按钮点击信号
    connect(deleteButton, &QPushButton::clicked, [this, serialNumber]() {
        onDeleteButtonClicked(serialNumber);
    });
}

DeviceStatus* devicelistdialog::findDeviceInVector(const QString &serialNumber)
{
    if (!m_deviceVector) return nullptr;

    for (DeviceStatus &device : *m_deviceVector) {
        if (device.serialNumber == serialNumber) {
            return &device;
        }
    }
    return nullptr;
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
    if (!m_deviceVector) {
        QMessageBox::critical(this, "错误", "设备容器未初始化");
        return;
    }

    // 从容器中删除设备
    auto it = std::remove_if(m_deviceVector->begin(), m_deviceVector->end(),
                             [&serialNumber](const DeviceStatus& device) {
                                 return device.serialNumber == serialNumber;
                             });

    if (it != m_deviceVector->end()) {
        m_deviceVector->erase(it, m_deviceVector->end());

        // 从数据库删除（如果也需要）
        if (m_dbManager) {
            m_dbManager->deleteDevice(serialNumber);
        }

        QMessageBox::information(this, "成功", "设备删除成功");
        // 重新加载设备列表
        loadDeviceList();

        qDebug() << "设备删除成功：" << serialNumber;
    } else {
        QMessageBox::critical(this, "错误", "未找到要删除的设备");
    }
}

void devicelistdialog::on_button_search_clicked()
{
    if (!m_deviceVector) {
        return;
    }

    // 获取搜索关键词
    QString keyword = ui->lineEdit_search->text().trimmed();

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
    QVector<DeviceStatus> filteredDevices;

    // 过滤设备
    for (const DeviceStatus &device : *m_deviceVector) {
        if (device.serialNumber.toLower().contains(searchText) ||
            device.checksum.toLower().contains(searchText) ||
            device.status.toLower().contains(searchText) ||
            device.ip.toLower().contains(searchText) ||
            device.hardversion.toLower().contains(searchText) ||
            device.usedProcessID.toLower().contains(searchText)) {
            filteredDevices.append(device);
        }
    }

    // 填充过滤后的数据
    int row = 0;
    for (const DeviceStatus &device : filteredDevices) {
        // 设备编号
        QStandardItem *serialItem = new QStandardItem(device.serialNumber);
        serialItem->setEditable(false);
        model->setItem(row, 0, serialItem);

        // 校验码
        QStandardItem *checksumItem = new QStandardItem(device.checksum);
        checksumItem->setEditable(false);
        model->setItem(row, 1, checksumItem);

        // 状态
        QStandardItem *statusItem = new QStandardItem(device.status);
        statusItem->setEditable(false);
        if (device.status == "在线" || device.status.toLower() == "online") {
            statusItem->setForeground(Qt::darkGreen);
            statusItem->setText("在线");
        } else if (device.status == "离线" || device.status.toLower() == "offline") {
            statusItem->setForeground(Qt::darkRed);
            statusItem->setText("离线");
        } else if (device.status == "忙碌" || device.status.toLower() == "busy") {
            statusItem->setForeground(Qt::darkYellow);
            statusItem->setText("忙碌");
        }
        model->setItem(row, 2, statusItem);

        // 当前动作
        QStandardItem *actionItem = new QStandardItem(
            device.currentAction.isEmpty() ? "空闲" : device.currentAction);
        actionItem->setEditable(false);
        model->setItem(row, 3, actionItem);

        // 流量统计
        QStandardItem *flowItem = new QStandardItem(device.trafficStatistics);
        flowItem->setEditable(false);
        model->setItem(row, 4, flowItem);

        // 最后心跳
        QStandardItem *heartbeatItem = new QStandardItem(device.lastHeartbeat);
        heartbeatItem->setEditable(false);
        model->setItem(row, 5, heartbeatItem);

        // IP地址
        QStandardItem *ipItem = new QStandardItem(device.ip);
        ipItem->setEditable(false);
        model->setItem(row, 6, ipItem);

        // 温度
        QStandardItem *tempItem = new QStandardItem(QString::number(device.Temperature, 'f', 1) + "°C");
        tempItem->setEditable(false);
        if (device.Temperature > 70.0) {
            tempItem->setForeground(Qt::red);
        } else if (device.Temperature > 50.0) {
            tempItem->setForeground(Qt::darkYellow);
        }
        model->setItem(row, 7, tempItem);

        // 硬件版本
        QStandardItem *hwItem = new QStandardItem(device.hardversion);
        hwItem->setEditable(false);
        model->setItem(row, 8, hwItem);

        // 使用流程ID
        QStandardItem *processIdItem = new QStandardItem(device.usedProcessID);
        processIdItem->setEditable(false);
        model->setItem(row, 9, processIdItem);

        // 添加删除按钮
        addDeleteButtonToRow(row, device.serialNumber);

        row++;
    }

    // 更新标题
    setWindowTitle(QString("设备列表（共 %1 台设备，搜索到 %2 台）")
                       .arg(m_deviceVector->size())
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

        // 检查设备是否已存在（在容器中）
        if (findDeviceInVector(serialNumber) != nullptr) {
            QMessageBox::warning(this, "警告",
                                 QString("设备 %1 已存在！").arg(serialNumber));
            return;
        }

        // 插入新设备到数据库
        if (insertDevice(serialNumber, checksum, belongUser)) {
            // 同时添加到容器中
            // 创建一个临时的 DeviceStatus 对象
            DeviceStatus newDevice{
                serialNumber,              // sn
                "离线",                    // st
                "",                       // loc
                "",                       // action
                "0",                      // traffic
                "从未连接",                // heartbeat
                "unknown",                // ip
                "",                       // cs
                "",                       // ce
                "",                       // na
                "",                       // nas
                "",                       // nae
                "",                       // process
                "未使用",                  // processId
                0.0f                      // temperature
            };

            // 设置构造函数没有初始化的字段
            newDevice.checksum = checksum;
            newDevice.warningmsg = "";
            newDevice.newdev = true;
            newDevice.warining_ignore = false;
            newDevice.hardversion = "未知";

            if (m_deviceVector) {
                m_deviceVector->append(newDevice);
            }

            QMessageBox::information(this, "成功", "设备添加成功");
            // 重新加载设备列表
            loadDeviceList();
            // 清空搜索框
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

    // ====== 需要添加这里 ======
    // 初始化新增的社交媒体字段
    newDevice.tiktok = 0;      // 默认关闭
    newDevice.bilibili = 0;    // 默认关闭
    newDevice.xhs = 0;         // 默认关闭
    newDevice.weibo = 0;       // 默认关闭
    newDevice.kuaishou = 0;    // 默认关闭

    // 插入数据库
    bool ret = m_dbManager->insertDevice(newDevice);
    if(ret)
    {
        emit NewDeviceCallED(serialNumber);
        return true;
    }

    return false;


}
// 获取最新固件版本
QString devicelistdialog::getLatestFirmwareVersion()
{
    QDir downloadDir(QDir::current().filePath("Download"));

    // 如果Download目录不存在，返回空
    if (!downloadDir.exists()) {
        qDebug() << "Download目录不存在";
        return "";
    }

    // 设置过滤器，只查找firmware开头的.tar.gz文件
    QStringList filters;
    filters << "firmware_*.tar.gz";
    downloadDir.setNameFilters(filters);

    QFileInfoList fileList = downloadDir.entryInfoList(QDir::Files | QDir::NoDotAndDotDot);

    if (fileList.isEmpty()) {
        qDebug() << "Download目录中没有固件文件";
        return "";
    }

    QString latestVersion = "";
    QString latestFileName = "";

    foreach (const QFileInfo &fileInfo, fileList) {
        QString fileName = fileInfo.fileName();

        // 提取版本号，例如从"firmware_0.1.1.tar.gz"中提取"0.1.1"
        if (fileName.startsWith("firmware_") && fileName.endsWith(".tar.gz")) {
            QString versionStr = fileName.mid(9);  // 去掉"firmware_"
            versionStr = versionStr.left(versionStr.length() - 7);  // 去掉".tar.gz"

            // 比较版本号
            if (latestVersion.isEmpty() || compareVersions(versionStr, latestVersion) > 0) {
                latestVersion = versionStr;
                latestFileName = fileName;
            }
        }
    }

    return latestFileName;  // 返回完整的文件名
}
// 比较版本号的辅助函数
int devicelistdialog::compareVersions(const QString &v1, const QString &v2)
{
    QStringList v1Parts = v1.split('.');
    QStringList v2Parts = v2.split('.');

    int maxLength = qMax(v1Parts.size(), v2Parts.size());

    for (int i = 0; i < maxLength; i++) {
        int v1Num = (i < v1Parts.size()) ? v1Parts[i].toInt() : 0;
        int v2Num = (i < v2Parts.size()) ? v2Parts[i].toInt() : 0;

        if (v1Num > v2Num) return 1;
        if (v1Num < v2Num) return -1;
    }

    return 0;
}
// 更新版本标签
void devicelistdialog::updateVersionLabel()
{
    QString latestFirmware = getLatestFirmwareVersion();

    if (latestFirmware.isEmpty()) {
        ui->label_lastversion->setText("最新版本: 无");
        ui->label_lastversion->setStyleSheet("color: red;");
    } else {
        // 提取版本号用于显示
        QString versionNumber = latestFirmware.mid(9);  // 去掉"firmware_"
        versionNumber = versionNumber.left(versionNumber.length() - 7);  // 去掉".tar.gz"

        ui->label_lastversion->setText(QString("最新版本: v%1").arg(versionNumber));
        ui->label_lastversion->setStyleSheet("color: green;");

        // qDebug() << "最新固件文件:" << latestFirmware;
        // qDebug() << "版本号:" << versionNumber;
    }
}

void devicelistdialog::on_button_upgrade_clicked()
{
    qDebug() << "=== 开始固件升级处理 ===";

    // 1. 获取最新固件版本
    QString latestFirmware = getLatestFirmwareVersion();

    if (latestFirmware.isEmpty()) {
        QMessageBox::warning(this, "警告", "未找到固件文件，请将固件文件放在Download目录下");
        return;
    }

    qDebug() << "最新固件文件:" << latestFirmware;

    // 2. 查找需要升级的设备
    QStringList devicesToUpgrade;

    if (!m_deviceVector) {
        QMessageBox::warning(this, "警告", "设备列表为空");
        return;
    }

    // 解析最新版本号
    QString latestVersion = latestFirmware.mid(9);  // 去掉"firmware_"
    latestVersion = latestVersion.left(latestVersion.length() - 7);  // 去掉".tar.gz"

    qDebug() << "最新版本号:" << latestVersion;
    qDebug() << "筛选条件: 设备状态=空闲 且 版本号<" << latestVersion;

    foreach (const DeviceStatus &device, *m_deviceVector) {
        // 检查设备状态是否为"空闲"
        QString currentAction = device.currentAction.isEmpty() ? "空闲" : device.currentAction;

        if (currentAction != "空闲") {
            qDebug() << "设备" << device.serialNumber << "状态为" << currentAction << "，跳过";
            continue;
        }

        // 获取设备硬件版本
        QString deviceVersion = device.hardversion.trimmed();

        // 如果设备版本号未知或为空，假设需要升级
        bool needsUpgrade = deviceVersion.isEmpty() || deviceVersion == "未知";

        // 如果有版本号，进行比较
        if (!needsUpgrade && !deviceVersion.isEmpty()) {
            // 标准化版本号：去掉可能的"v"前缀
            if (deviceVersion.startsWith("v") || deviceVersion.startsWith("V")) {
                deviceVersion = deviceVersion.mid(1);
            }

            // 比较版本号
            if (compareVersions(deviceVersion, latestVersion) < 0) {
                needsUpgrade = true;
            }
        }

        if (needsUpgrade) {
            devicesToUpgrade.append(device.serialNumber);
            qDebug() << "设备" << device.serialNumber << "版本" << device.hardversion << "需要升级";
        } else {
            qDebug() << "设备" << device.serialNumber << "版本" << device.hardversion << "已是最新，跳过";
        }
    }

    if (devicesToUpgrade.isEmpty()) {
        QMessageBox::information(this, "提示", "没有需要升级的设备");
        return;
    }

    qDebug() << "需要升级的设备数量:" << devicesToUpgrade.size();
    qDebug() << "设备列表:" << devicesToUpgrade;

    // 3. 确认升级
    QString confirmMessage = QString("准备升级 %1 台设备\n\n固件文件: %2\n最新版本: v%3\n\n是否继续？")
                                 .arg(devicesToUpgrade.size())
                                 .arg(latestFirmware)
                                 .arg(latestVersion);

    int result = QMessageBox::question(this, "确认升级", confirmMessage,
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (result != QMessageBox::Yes) {
        qDebug() << "用户取消升级";
        return;
    }

    // 4. 构建升级列表并发送信号
    // 升级列表的第一个元素是固件文件名，后面是需要升级的设备列表
    QStringList upgradeList;
    upgradeList.append(latestFirmware);  // 第一个元素是固件文件名
    upgradeList.append(devicesToUpgrade);  // 后面是设备列表

    qDebug() << "发送升级信号，升级列表:" << upgradeList;

    // 发送升级信号
    emit deviceUpgrade(upgradeList);

    // 5. 显示升级提示
    QString successMessage = QString("升级指令已发送\n\n"
                                     "固件: %1\n"
                                     "版本: v%2\n"
                                     "设备数量: %3台\n\n"
                                     "设备列表:\n%4")
                                 .arg(latestFirmware)
                                 .arg(latestVersion)
                                 .arg(devicesToUpgrade.size())
                                 .arg(devicesToUpgrade.join("\n"));

    QMessageBox::information(this, "升级指令已发送", successMessage);

    qDebug() << "=== 固件升级处理完成 ===";
}
