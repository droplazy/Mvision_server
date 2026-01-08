#include "commanddev.h"
#include "ui_commanddev.h"
#include "DatabaseManager.h"
#include <QSqlQuery>
#include <QSqlError>
#include <QTableWidgetItem>
#include <QCheckBox>
#include <QHeaderView>
#include <QDateTime>
#include <QStandardItemModel>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDebug>

commanddev::commanddev(DatabaseManager *db, QVector<DeviceStatus> *devices, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::commanddev)
    , m_db(db)
    , m_devices(devices)
{
    ui->setupUi(this);

    // 初始化订单列表
    initOrderListView();
    initActionComboBox();

    // 设置时间控件默认值
    ui->timeEdit_start->setTime(QTime(0, 0, 0));
    ui->timeEdit_end->setTime(QTime(23, 59, 59));

    // 初始化设备表格
    setupDeviceTable();
    updateDeviceTable();

    // 连接信号槽
    connect(ui->listView_order, &QListView::doubleClicked,
            this, &commanddev::onOrderListViewDoubleClicked);
    connect(ui->comboBox_act, &QComboBox::currentTextChanged,
            this, &::commanddev::on_comboBox_act_currentTextChanged);
    // 连接复选框状态变化信号
    connect(ui->checkBox_direct_send, &QCheckBox::checkStateChanged,
            this, &commanddev::on_checkBox_direct_send_stateChanged);

    // 初始时，如果不勾选直接发送，则需要订单，所以订单列表默认可用
    on_checkBox_direct_send_stateChanged(ui->checkBox_direct_send->checkState());
}

commanddev::~commanddev()
{
    delete ui;
}

void commanddev::initOrderListView()
{
    if (!m_db) return;

    // 创建模型
    QStandardItemModel *orderModel = new QStandardItemModel(this);
    ui->listView_order->setModel(orderModel);

    // 获取状态为'paid'的订单
    QList<SQL_Order> orders = m_db->getOrdersByStatus("paid");

    for (const SQL_Order &order : orders) {
        // 只显示订单号
        QStandardItem *item = new QStandardItem(order.orderId);
        // 将订单ID存储在item的user role中
        item->setData(order.orderId, Qt::UserRole);
        orderModel->appendRow(item);
    }

    // 如果没有订单，显示提示
    if (orders.isEmpty()) {
        QStandardItem *item = new QStandardItem("暂无已支付订单");
        item->setEnabled(false);
        item->setSelectable(false);
        orderModel->appendRow(item);
    }

    // 设置简单的样式
    ui->listView_order->setAlternatingRowColors(true);
    ui->listView_order->setFont(QFont("Microsoft YaHei", 10));
}

void commanddev::initActionComboBox()
{
    ui->comboBox_act->clear();
    ui->comboBox_subact->clear();

    // 添加预设动作
    ui->comboBox_act->addItem("抖音", "抖音");
    ui->comboBox_act->addItem("BB", "BB");
    ui->comboBox_act->addItem("CC", "CC");

    // 默认选中第一个并更新子动作
    if (ui->comboBox_act->count() > 0) {
        on_comboBox_act_currentTextChanged(ui->comboBox_act->itemData(0).toString());
    }
}

void commanddev::updateSubActionComboBox(const QString &mainAction)
{
    ui->comboBox_subact->clear();

    if (mainAction == "抖音") {

        // ui->comboBox_subact->addItem("评论");
        // ui->comboBox_subact->addItem("直播");
        // ui->comboBox_subact->addItem("弹幕");
        // ui->comboBox_subact->addItem("私信");
        // ui->comboBox_subact->addItem("退出");
        // ui->comboBox_subact->addItem("登录");

        ui->comboBox_subact->addItem("评论", "评论");//点赞评论转发收藏
        ui->comboBox_subact->addItem("直播", "直播");
        ui->comboBox_subact->addItem("弹幕", "弹幕");//1 明文 2 传输文本
        ui->comboBox_subact->addItem("私信", "私信");//1 明文 2 传输文本
        ui->comboBox_subact->addItem("退出", "退出");//1 明文 2 传输文本
        ui->comboBox_subact->addItem("登录", "登录");//1 明文 2 传输文本

    }
    else if (mainAction == "BB") {
        ui->comboBox_subact->addItem("BB_1", "BB_1");
        ui->comboBox_subact->addItem("BB_2", "BB_2");
        ui->comboBox_subact->addItem("BB_3", "BB_3");
    }
    else if (mainAction == "CC") {
        ui->comboBox_subact->addItem("CC_1", "CC_1");
        ui->comboBox_subact->addItem("CC_2", "CC_2");
        ui->comboBox_subact->addItem("CC_3", "CC_3");
    }

    if (ui->comboBox_subact->count() > 0) {
        ui->comboBox_subact->setCurrentIndex(0);
    }
}

void commanddev::setupDeviceTable()
{
    // 设置表格属性
    ui->tableView_dev->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView_dev->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tableView_dev->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 创建模型并设置列
    QStandardItemModel *model = new QStandardItemModel(this);
    model->setColumnCount(7); // 包括复选框列

    // 设置列标题 - 将"状态"列改为"动作"
    model->setHorizontalHeaderLabels(QStringList() << "选择" << "设备名" << "动作" << "IP地址"
                                                   << "最后心跳" << "位置" << "状态");

    ui->tableView_dev->setModel(model);

    // 调整列宽
    ui->tableView_dev->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView_dev->horizontalHeader()->setSectionResizeMode(0, QHeaderView::Fixed);
    ui->tableView_dev->setColumnWidth(0, 60);
}

void commanddev::updateDeviceTable()
{
    if (!m_devices) return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView_dev->model());
    if (!model) return;

    model->removeRows(0, model->rowCount());

    for (int i = 0; i < m_devices->size(); ++i) {
        const DeviceStatus &device = m_devices->at(i);

        // 创建行
        QList<QStandardItem*> rowItems;

        // 第1列：复选框
        QStandardItem *checkItem = new QStandardItem();
        checkItem->setCheckable(true);
        checkItem->setCheckState(Qt::Unchecked);
        checkItem->setTextAlignment(Qt::AlignCenter);
        rowItems.append(checkItem);

        // 第2列：设备序列号
        QStandardItem *nameItem = new QStandardItem(device.serialNumber);
        rowItems.append(nameItem);

        // 第3列：动作（原状态列改为动作列）
        QStandardItem *actionItem = new QStandardItem(device.currentAction);
        // 为不同动作设置不同颜色
        if (device.currentAction.isEmpty()) {
            actionItem->setText("空闲");
            actionItem->setForeground(QBrush(Qt::darkGreen));
        } else if (device.currentAction.contains("错误", Qt::CaseInsensitive)) {
            actionItem->setForeground(QBrush(Qt::darkRed));
        } else if (device.currentAction.contains("警告", Qt::CaseInsensitive)) {
            actionItem->setForeground(QBrush(Qt::darkYellow));
        }
        rowItems.append(actionItem);

        // 第4列：IP地址
        QStandardItem *ipItem = new QStandardItem(device.ip);
        rowItems.append(ipItem);

        // 第5列：最后心跳
        QStandardItem *heartbeatItem = new QStandardItem(device.lastHeartbeat);
        rowItems.append(heartbeatItem);

        // 第6列：位置
        QStandardItem *locationItem = new QStandardItem(device.location);
        rowItems.append(locationItem);

        // 第7列：状态（新增状态列，显示device.status）
        QStandardItem *statusItem = new QStandardItem(device.status);
        // 为不同状态设置不同颜色
        if (device.status == "空闲") {
            statusItem->setForeground(QBrush(Qt::darkGreen));
        } else if (device.status == "忙碌") {
            statusItem->setForeground(QBrush(Qt::darkRed));
        } else if (device.status == "警告") {
            statusItem->setForeground(QBrush(Qt::darkYellow));
        }
        rowItems.append(statusItem);

        model->appendRow(rowItems);
    }
}

void commanddev::onOrderListViewDoubleClicked(const QModelIndex &index)
{
    if (!m_db) return;

    // 获取选中的item
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->listView_order->model());
    if (!model) return;

    QStandardItem *item = model->itemFromIndex(index);
    if (!item || !item->isEnabled()) return;

    // 从item中获取订单ID
    QString orderId = item->text();
    if (orderId.isEmpty() || orderId == "暂无已支付订单") return;

    // 从数据库获取订单详细信息
    SQL_Order order = m_db->getOrderById(orderId);
    if (order.orderId.isEmpty()) {
        ui->order_info->setText("订单不存在或已删除");
        return;
    }

    // 构建显示文本
    QString infoText = QString("订单ID: %1\n"
                               "商品名称: %2\n"
                               "商品ID: %3\n"
                               "数量: %4\n"
                               "单价: ¥%5\n"
                               "总价: ¥%6\n"
                               "用户: %7\n"
                               "创建时间: %8\n"
                               "备注: %9")
                           .arg(order.orderId)
                           .arg(order.productName.isEmpty() ? "未知商品" : order.productName)
                           .arg(order.productId)
                           .arg(order.quantity)
                           .arg(order.unitPrice, 0, 'f', 2)
                           .arg(order.totalPrice, 0, 'f', 2)
                           .arg(order.user.isEmpty() ? "未知用户" : order.user)
                           .arg(order.createTime)
                           .arg(order.note.isEmpty() ? "无备注" :
                                    (order.note.length() > 100 ?
                                         order.note.left(100) + "..." : order.note));

    ui->order_info->setText(infoText);
}

void commanddev::on_comboBox_act_currentTextChanged(const QString &text)
{
    updateSubActionComboBox(text);
}
void commanddev::on_checkBox_direct_send_stateChanged(int state)
{
    bool isDirectSend = (state == Qt::Checked);

    // 根据复选框状态启用/禁用订单相关控件
    ui->listView_order->setEnabled(!isDirectSend);
    ui->order_info->setEnabled(!isDirectSend);

    // 更新标签提示
    if (isDirectSend) {
        ui->order_info->setText("直接发送模式：不绑定订单，直接发送指令到设备");
        ui->listView_order->setToolTip("直接发送模式下订单选择被禁用");
    } else {
        ui->order_info->setText("未选择订单");
        ui->listView_order->setToolTip("请双击选择订单");
    }

    qDebug() << "直接发送模式:" << (isDirectSend ? "启用" : "禁用");
}
void commanddev::on_pushButton_autoselect_clicked()
{
    if (!m_devices) return;

    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView_dev->model());
    if (!model) return;

    int selectedCount = 0;

    // 遍历所有行，自动选择状态为"空闲"的设备（检查第7列的状态）
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *statusItem = model->item(row, 6); // 第7列：状态列
        QStandardItem *actionItem = model->item(row, 2); // 第3列：动作列
        QStandardItem *checkItem = model->item(row, 0);  // 第1列：复选框列

        // 调试输出
        QString deviceName = model->item(row, 1)->text();
        QString status = statusItem ? statusItem->text() : "未知";
        QString action = actionItem ? actionItem->text() : "未知";

        qDebug() << QString("设备: %1, 状态: %2, 动作: %3").arg(deviceName).arg(status).arg(action);

        // 检查设备是否空闲：状态为"空闲"或动作为空/包含"空闲"
        bool isFree = (status == "空闲") ||
                      (action.isEmpty()) ||
                      (action == "空闲") ||
                      (action.contains("空闲", Qt::CaseInsensitive));

        if (statusItem && checkItem && isFree) {
            checkItem->setCheckState(Qt::Checked);
            selectedCount++;
            qDebug() << "  -> 选中";
        } else if (checkItem) {
            checkItem->setCheckState(Qt::Unchecked);
            qDebug() << "  -> 未选中";
        }
    }

    QMessageBox::information(this, "自动选择",
                             QString("已自动选择 %1 个空闲设备").arg(selectedCount));
}

void commanddev::on_pushButton_allselect_clicked()
{
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView_dev->model());
    if (!model) return;

    // 遍历所有行，全部选中
    for (int row = 0; row < model->rowCount(); ++row) {
        QStandardItem *checkItem = model->item(row, 0);
        if (checkItem) {
            checkItem->setCheckState(Qt::Checked);
        }
    }

    QMessageBox::information(this, "全选", "已选择所有设备");
}

void commanddev::on_pushButton_command_clicked()
{
    qDebug() << "====== 开始处理指令 ======";

    // 1. 获取选中的设备
    m_selectedDevices.clear();
    QStandardItemModel *model = qobject_cast<QStandardItemModel*>(ui->tableView_dev->model());

    if (model) {
        for (int row = 0; row < model->rowCount(); ++row) {
            QStandardItem *checkItem = model->item(row, 0);
            QStandardItem *nameItem = model->item(row, 1);

            if (checkItem && nameItem && checkItem->checkState() == Qt::Checked) {
                m_selectedDevices.append(nameItem->text());
            }
        }
    }

    // 打印选中的设备序列号
    qDebug() << "=== 选中的设备序列号 ===";
    for (int i = 0; i < m_selectedDevices.size(); ++i) {
        qDebug() << QString("  %1. %2").arg(i+1).arg(m_selectedDevices[i]);
    }

    if (m_selectedDevices.isEmpty()) {
        QMessageBox::warning(this, "警告", "请至少选择一个设备");
        return;
    }

    // 检查是否勾选直接发送
    bool isDirectSend = ui->checkBox_direct_send->isChecked();
    QString orderId;
    QString commandId;

    if (!isDirectSend) {
        // 绑定订单模式
        qDebug() << "=== 绑定订单模式 ===";

        // 2. 检查是否选中了订单
        QModelIndexList selectedIndexes = ui->listView_order->selectionModel()->selectedIndexes();
        if (selectedIndexes.isEmpty()) {
            QMessageBox::warning(this, "警告", "请先在订单列表中双击选择一个订单");
            return;
        }

        // 获取选中的订单ID
        QModelIndex selectedIndex = selectedIndexes.first();
        QStandardItemModel *orderModel = qobject_cast<QStandardItemModel*>(ui->listView_order->model());
        QStandardItem *orderItem = orderModel->itemFromIndex(selectedIndex);
        orderId = orderItem->text();

        if (orderId == "暂无已支付订单") {
            QMessageBox::warning(this, "警告", "请选择有效的订单");
            return;
        }

        qDebug() << "选中的订单ID:" << orderId;

        // 3. 检查数据库指针
        if (!m_db) {
            QMessageBox::critical(this, "错误", "数据库连接不可用");
            return;
        }

        // 4. 检查订单是否存在和状态
        SQL_Order existingOrder = m_db->getOrderById(orderId);
        if (existingOrder.orderId.isEmpty()) {
            QMessageBox::warning(this, "警告", QString("订单不存在: %1").arg(orderId));
            return;
        }

        // 检查订单状态是否可以执行
        if (existingOrder.status != "paid" && existingOrder.status != "已支付") {
            QString statusText = existingOrder.status.isEmpty() ? "未知状态" : existingOrder.status;
            QMessageBox::warning(this, "警告",
                                 QString("订单状态不允许执行\n订单ID: %1\n当前状态: %2\n要求状态: 已支付")
                                     .arg(orderId)
                                     .arg(statusText));
            return;
        }

        qDebug() << "订单验证通过:";
        qDebug() << "  订单ID:" << existingOrder.orderId;
        qDebug() << "  状态:" << existingOrder.status;
        qDebug() << "  商品:" << existingOrder.productName;
        qDebug() << "  数量:" << existingOrder.quantity;

        // 5. 更新订单状态为execing
        qDebug() << "更新订单状态为execing...";
        bool orderStatusUpdated = m_db->updateOrderStatus(orderId, "execing");
        if (!orderStatusUpdated) {
            qDebug() << "更新订单状态失败";
            QMessageBox::critical(this, "错误", "更新订单状态失败，请检查数据库连接");
            return;
        }

        // 验证状态是否真的更新了
        SQL_Order updatedOrder = m_db->getOrderById(orderId);
        if (updatedOrder.status != "execing") {
            qDebug() << "❌ 订单状态更新验证失败!";
            qDebug() << "期望状态: execing";
            qDebug() << "实际状态:" << updatedOrder.status;

            // 回滚到原始状态
            m_db->updateOrderStatus(orderId, existingOrder.status);
            QMessageBox::critical(this, "错误",
                                  QString("订单状态更新失败，当前状态: %1").arg(updatedOrder.status));
            return;
        }

        qDebug() << "✅ 订单状态更新成功";
        qDebug() << "  原状态:" << existingOrder.status << " -> 新状态:" << updatedOrder.status;
    } else {
        // 直接发送模式
        qDebug() << "=== 直接发送模式（不绑定订单）===";
        qDebug() << "跳过订单验证和数据库更新";
    }

    // 6. 生成指令ID
    commandId = QString("CMD_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz"));
    qDebug() << "生成的指令ID:" << commandId;

    // 7. 获取输入参数
    QString action = ui->comboBox_act->currentData().toString();
    QString subAction = ui->comboBox_subact->currentData().toString();
    QString startTimeStr = ui->timeEdit_start->time().toString("HH:mm:ss");
    QString endTimeStr = ui->timeEdit_end->time().toString("HH:mm:ss");

    // 获取输入控件内容
    QString idText = ui->lineEdit_id->text().trimmed();
    QString linkText = ui->lineEdit_link->text().trimmed();
    QString msgText = ui->textEdit_msg->toPlainText().trimmed();
    QString markText = ui->lineEdit_mark->text().trimmed();

    // 构建remark字符串
    QString remark = QString("ID:%1:ID LINK:%2:LINK MSG:%3:MSG MARK:%4:MARK")
                         .arg(idText)
                         .arg(linkText)
                         .arg(msgText)
                         .arg(markText);

    // 处理时间格式
    QString start_time, end_time;

    // 使用输入的时间字符串
    if (!startTimeStr.isEmpty()) {
        start_time = startTimeStr;
    } else {
        // 如果没有开始时间，使用当前时间
        start_time = QDateTime::currentDateTime().toString("HH:mm:ss");
    }

    if (!endTimeStr.isEmpty()) {
        end_time = endTimeStr;
    } else {
        // 如果没有结束时间，设置为开始时间后1小时
        if (!startTimeStr.isEmpty()) {
            QTime startTime = QTime::fromString(startTimeStr, "HH:mm:ss");
            if (startTime.isValid()) {
                end_time = startTime.addSecs(3600).toString("HH:mm:ss");
            } else {
                end_time = QDateTime::currentDateTime().addSecs(3600).toString("HH:mm:ss");
            }
        } else {
            end_time = QDateTime::currentDateTime().addSecs(3600).toString("HH:mm:ss");
        }
    }

    // 8. 输出详细信息
    qDebug() << "=== 指令详细信息 ===";
    qDebug() << "指令ID:" << commandId;
    qDebug() << "动作:" << (action.isEmpty() ? "default" : action);
    qDebug() << "子动作:" << (subAction.isEmpty() ? "default" : subAction);
    qDebug() << "开始时间:" << start_time;
    qDebug() << "结束时间:" << end_time;
    qDebug() << "备注:" << remark;
    qDebug() << "序列号列表:";
    for (int i = 0; i < m_selectedDevices.size(); i++) {
        qDebug() << QString("  %1. %2").arg(i+1).arg(m_selectedDevices[i]);
    }

    if (!isDirectSend) {
        // 绑定订单模式：创建指令并保存到数据库
        SQL_CommandHistory cmd;
        cmd.commandId = commandId;
        cmd.status = "execing";
        cmd.action = action.isEmpty() ? "default" : action;
        cmd.sub_action = subAction.isEmpty() ? "default" : subAction;
        cmd.start_time = start_time;
        cmd.end_time = end_time;
        cmd.remark = remark;
        cmd.Completeness = "0%";
        cmd.completed_url = "";

        if (!m_db->insertCommandHistory(cmd)) {
            qDebug() << "插入指令历史记录失败";

            // 指令插入失败，回滚订单状态到原来的状态
            if (!orderId.isEmpty()) {
                SQL_Order existingOrder = m_db->getOrderById(orderId);
                m_db->updateOrderStatus(orderId, existingOrder.status);
                qDebug() << "订单状态已回滚到:" << existingOrder.status;
            }

            QMessageBox::critical(this, "错误", "插入指令历史记录失败，已回滚订单状态");
            return;
        }

        qDebug() << "指令历史记录插入成功";

        // 更新订单的指令ID
        if (!orderId.isEmpty()) {
            SQL_Order updatedOrder = m_db->getOrderById(orderId);
            updatedOrder.commandId = commandId;
            if (!m_db->updateOrder(updatedOrder)) {
                qDebug() << "更新订单指令ID失败";
                // 这里虽然失败，但订单状态已经是execing，指令也已经创建
                // 所以不进行回滚，只是记录日志
            } else {
                qDebug() << "订单指令ID更新成功";
            }
        }
    } else {
        qDebug() << "直接发送模式：跳过数据库操作";
    }

    // 9. 构建指令JSON（用于发送给设备） - 关键修改在这里！
    QJsonObject dataObj;
    dataObj["command_id"] = commandId;
    dataObj["action"] = action;
    dataObj["sub_action"] = subAction;
    dataObj["start_time"] = start_time;
    dataObj["end_time"] = end_time;
    dataObj["remark"] = remark;

    // 添加设备序列号数组 - 这是关键修改！
    QJsonArray serialNumbersArray;
    for (const QString &serialNumber : m_selectedDevices) {
        serialNumbersArray.append(serialNumber);
    }
    dataObj["serial_numbers"] = serialNumbersArray;  // 添加设备序列号数组

    m_command = QJsonObject();
    m_command["data"] = dataObj;
    m_command["messageType"] = "command";
    m_command["password"] = "securePassword123";
    m_command["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    m_command["username"] = "user123";

    // 10. 打印明文JSON（包含设备序列号）
    QJsonDocument doc(m_command);
    QString jsonString = doc.toJson(QJsonDocument::Indented);

    qDebug() << "\n=== 生成的指令JSON（包含设备序列号） ===";
    qDebug() << jsonString;

    // 11. 发送信号 - 将指令发送出去
    qDebug() << "\n=== 发送设备指令信号 ===";
    qDebug() << "设备数量:" << m_selectedDevices.size();
    qDebug() << "第一个设备:" << (m_selectedDevices.isEmpty() ? "无" : m_selectedDevices.first());

    emit dCommadSend(m_command);
    qDebug() << "指令信号已发送";

    // 12. 显示成功信息
    if (!isDirectSend) {
        QMessageBox::information(this, "指令生成成功",
                                 QString("指令生成完成！\n\n"
                                         "订单信息:\n"
                                         "  ID: %1\n"
                                         "  状态: execing\n"
                                         "  指令ID: %2\n\n"
                                         "设备信息:\n"
                                         "  已选择 %3 个设备\n\n"
                                         "指令信息:\n"
                                         "  动作: %4\n"
                                         "  子动作: %5\n"
                                         "  时间: %6 ~ %7\n\n"
                                         "备注: %8\n\n"
                                         "指令已发送到MQTT，等待处理...")
                                     .arg(orderId)
                                     .arg(commandId)
                                     .arg(m_selectedDevices.size())
                                     .arg(action)
                                     .arg(subAction)
                                     .arg(start_time)
                                     .arg(end_time)
                                     .arg(remark.isEmpty() ? "无" : remark));
    } else {
        QMessageBox::information(this, "指令生成成功",
                                 QString("直接发送模式指令生成完成！\n\n"
                                         "设备信息:\n"
                                         "  已选择 %1 个设备\n\n"
                                         "指令信息:\n"
                                         "  指令ID: %2\n"
                                         "  动作: %3\n"
                                         "  子动作: %4\n"
                                         "  时间: %5 ~ %6\n\n"
                                         "备注: %7\n\n"
                                         "指令已发送到MQTT，等待处理...")
                                     .arg(m_selectedDevices.size())
                                     .arg(commandId)
                                     .arg(action)
                                     .arg(subAction)
                                     .arg(start_time)
                                     .arg(end_time)
                                     .arg(remark.isEmpty() ? "无" : remark));
    }

    qDebug() << "====== 指令处理完成 ======";
}
QJsonObject commanddev::getGeneratedCommand() const
{
    return m_command;
}

QList<QString> commanddev::getSelectedDevices() const
{
    return m_selectedDevices;
}
