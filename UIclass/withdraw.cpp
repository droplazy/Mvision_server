#include "withdraw.h"
#include "ui_withdraw.h"
#include <QPushButton>
#include <QHBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QDateTime>
#include "disposewithdrwaw.h"

withdraw::withdraw(QWidget *parent, DatabaseManager *dbManager)
    : QWidget(parent)
    , ui(new Ui::withdraw)
    , dbManager(dbManager)
    , tableModel(nullptr)
{
    ui->setupUi(this);

    // 初始化表格模型
    tableModel = new QStandardItemModel(this);
    ui->tableView->setModel(tableModel);

    // 设置表格属性
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setVisible(false);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置表格标题
    setupTableHeader();

    // 加载数据
    refreshWithdrawTable();
}

withdraw::~withdraw()
{
    delete ui;
    if (tableModel) {
        delete tableModel;
    }
}

void withdraw::setDatabaseManager(DatabaseManager *dbManager)
{
    this->dbManager = dbManager;
    if (dbManager) {
        refreshWithdrawTable();
    }
}

void withdraw::setupTableHeader()
{
    QStringList headers;
    headers << "提现ID" << "用户名" << "提现金额" << "支付宝账号"
            << "状态" << "申请时间" << "更新时间" << "备注" << "操作";
    tableModel->setHorizontalHeaderLabels(headers);

    // 设置列宽
    ui->tableView->setColumnWidth(0, 150);  // 提现ID
    ui->tableView->setColumnWidth(1, 100);  // 用户名
    ui->tableView->setColumnWidth(2, 100);  // 提现金额
    ui->tableView->setColumnWidth(3, 180);  // 支付宝账号
    ui->tableView->setColumnWidth(4, 80);   // 状态
    ui->tableView->setColumnWidth(5, 160);  // 申请时间
    ui->tableView->setColumnWidth(6, 160);  // 更新时间
    ui->tableView->setColumnWidth(7, 150);  // 备注
    ui->tableView->setColumnWidth(8, 80);   // 操作
}

void withdraw::refreshWithdrawTable()
{
    if (!dbManager) {
        qDebug() << "DatabaseManager 未设置";
        return;
    }

    // 清空表格
    tableModel->removeRows(0, tableModel->rowCount());

    // 加载提现记录
    loadWithdrawRecords();
}

void withdraw::loadWithdrawRecords(const QString &filterUsername)
{
    if (!dbManager) return;

    // 获取提现记录
    QList<SQL_WithdrawRecord> records;

    if (filterUsername.isEmpty()) {
        // 这里需要你实现 getAllWithdrawRecords() 方法
        // 或者我们可以遍历所有用户的记录
        records = dbManager->getAllWithdrawRecords();
    } else {
        // 使用现有的 getWithdrawRecordsByUsername 方法
        records = dbManager->getWithdrawRecordsByUsername(filterUsername);
    }

    qDebug() << "加载到" << records.size() << "条提现记录";

    // 填充表格数据
    for (int i = 0; i < records.size(); ++i) {
        const SQL_WithdrawRecord &record = records[i];

        QList<QStandardItem*> rowItems;

        // 提现ID
        QStandardItem *idItem = new QStandardItem(record.withdrawId);
        idItem->setData(record.withdrawId, Qt::UserRole);
        rowItems.append(idItem);

        // 用户名
        rowItems.append(new QStandardItem(record.username));

        // 提现金额
        rowItems.append(new QStandardItem(QString("¥%1").arg(record.amount, 0, 'f', 2)));

        // 支付宝账号
        rowItems.append(new QStandardItem(record.alipayAccount));

        // 状态（中文显示）
        QString statusText;
        if (record.status == "pending") statusText = "待处理";
        else if (record.status == "processing") statusText = "处理中";
        else if (record.status == "completed") statusText = "已完成";
        else if (record.status == "failed") statusText = "失败";
        else if (record.status == "cancelled") statusText = "已取消";
        else statusText = record.status;

        QStandardItem *statusItem = new QStandardItem(statusText);

        // 根据状态设置不同的颜色
        if (record.status == "pending") {
            statusItem->setForeground(QBrush(QColor(255, 165, 0)));  // 橙色
        } else if (record.status == "processing") {
            statusItem->setForeground(QBrush(QColor(0, 100, 255)));  // 蓝色
        } else if (record.status == "completed") {
            statusItem->setForeground(QBrush(QColor(0, 150, 0)));    // 绿色
        } else if (record.status == "failed") {
            statusItem->setForeground(QBrush(QColor(255, 0, 0)));    // 红色
        }

        rowItems.append(statusItem);

        // 申请时间（create_time）
        QString applyTime = record.createTime;
        rowItems.append(new QStandardItem(applyTime));

        // 更新时间（update_time）
        QString updateTime = record.updateTime;
        rowItems.append(new QStandardItem(updateTime));

        // 备注
        QString remark = record.remark;
        if (remark.isEmpty()) remark = "无";
        rowItems.append(new QStandardItem(remark));

        // 添加到表格
        tableModel->appendRow(rowItems);

        // 在最后一列添加"处理"按钮
        addProcessButtonToRow(i, record.withdrawId);
    }
}

void withdraw::addProcessButtonToRow(int row, const QString &withdrawId)
{
    // 创建按钮
    QPushButton *processButton = new QPushButton("处理");
    processButton->setProperty("withdrawId", withdrawId);
    processButton->setFixedSize(60, 25);

    // 获取当前行的状态
    QStandardItem *statusItem = tableModel->item(row, 4);  // 第5列是状态

    // 如果是已完成、失败或已取消的状态，禁用按钮
    if (statusItem) {
        QString status = statusItem->text();
        if (status == "已完成" || status == "失败" || status == "已取消") {
            processButton->setEnabled(false);
            processButton->setText("已处理");
        }
    }

    // 连接按钮点击信号
    connect(processButton, &QPushButton::clicked, this, &withdraw::on_processButtonClicked);

    // 将按钮添加到表格单元格
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(processButton);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(2, 2, 2, 2);
    widget->setLayout(layout);

    // 最后一列是操作列（第9列，索引8）
    ui->tableView->setIndexWidget(tableModel->index(row, 8), widget);
}

void withdraw::on_processButtonClicked()
{
    QPushButton *button = qobject_cast<QPushButton*>(sender());
    if (!button) return;

    QString withdrawId = button->property("withdrawId").toString();
    if (withdrawId.isEmpty()) return;

    // 获取当前行的提现信息
    int row = ui->tableView->currentIndex().row();
    if (row < 0) {
        // 如果没有选中行，查找按钮所在的行
        for (int i = 0; i < tableModel->rowCount(); ++i) {
            QWidget *widget = ui->tableView->indexWidget(tableModel->index(i, 8));
            if (widget && widget->findChild<QPushButton*>() == button) {
                row = i;
                break;
            }
        }
    }

    if (row < 0) {
        QMessageBox::warning(this, "错误", "无法找到对应的提现记录！");
        return;
    }

    // 获取提现详细信息
    QString username = tableModel->item(row, 1)->text();
    QString amountStr = tableModel->item(row, 2)->text().mid(1); // 去掉¥符号
    double amount = amountStr.toDouble();
    QString alipayAccount = tableModel->item(row, 3)->text();

    // 检查状态
    QString currentStatus = tableModel->item(row, 4)->text();
    if (currentStatus == "已完成" || currentStatus == "失败" || currentStatus == "已取消") {
        QMessageBox::information(this, "提示", "此提现记录已被处理！");
        return;
    }

    qDebug() << "====== 打开提现处理对话框 ======";
    qDebug() << "提现ID:" << withdrawId;
    qDebug() << "用户名:" << username;
    qDebug() << "金额:" << amount;
    qDebug() << "支付宝:" << alipayAccount;

    // 创建并显示处理对话框
    disposewithdrwaw *dialog = new disposewithdrwaw(this, dbManager, withdrawId);

    // 设置提现信息
    dialog->setWithdrawInfo(withdrawId, username, amount, alipayAccount);

    // 连接处理完成信号
    connect(dialog, &disposewithdrwaw::withdrawProcessed,
            this, [this](const QString &id, bool success) {
                if (success) {
                    refreshWithdrawTable();
                    QMessageBox::information(this, "成功", QString("提现ID %1 处理完成！").arg(id));
                }
            });

    // 模态显示对话框
    dialog->exec();

    // 清理对话框
    dialog->deleteLater();
}

void withdraw::processWithdrawRecord(const QString &withdrawId)
{
    if (!dbManager) {
        QMessageBox::warning(this, "错误", "数据库连接未设置");
        return;
    }

    qDebug() << "开始处理提现记录:" << withdrawId;

    // 更新提现状态为"processing"
    // 注意：这里需要你实现 updateWithdrawStatus 方法
    // 你可以先用一个简单的SQL更新

    QSqlQuery query;
    query.prepare(R"(
        UPDATE WithdrawRecords
        SET status = 'processing',
            update_time = datetime('now', 'localtime')
        WHERE withdraw_id = :withdraw_id
    )");
    query.bindValue(":withdraw_id", withdrawId);

    if (!query.exec()) {
        qDebug() << "提现记录状态更新失败:" << query.lastError().text();
        QMessageBox::warning(this, "失败", "更新提现记录状态失败");
        return;
    }

    qDebug() << "提现记录状态更新成功";

    // 刷新表格
    refreshWithdrawTable();

    QMessageBox::information(this, "成功", "提现记录状态已更新为处理中");
}

void withdraw::on_pushButton_clicked()
{
    QString filterText = ui->lineEdit_filter->text().trimmed();
    qDebug() << "搜索条件:" << filterText;

    if (filterText.isEmpty()) {
        // 如果搜索框为空，加载所有记录
        loadWithdrawRecords();
    } else {
        // 按用户名搜索
        loadWithdrawRecords(filterText);
    }
}
