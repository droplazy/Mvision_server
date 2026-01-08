#include "commandlsit.h"
#include "ui_commandlsit.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>

commandlsit::commandlsit(DatabaseManager* dbManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::commandlsit)
    , m_dbManager(dbManager)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("指令历史列表");
    setFixedSize(this->size());
    // 设置表格
    setupTableWidget();

    // 连接搜索按钮
    connect(ui->pushButton, &QPushButton::clicked, this, &commandlsit::on_pushButton_clicked);

    // 加载指令列表
    loadCommandList();
}

commandlsit::~commandlsit()
{
    delete ui;
}

void commandlsit::setupTableWidget()
{
    // 设置表格属性
    ui->tableWidget->setColumnCount(9); // 8个数据列 + 1个操作列
    ui->tableWidget->setHorizontalHeaderLabels({
        "指令ID", "状态", "动作", "子动作",
        "开始时间", "结束时间", "完成度", "截图URL", "操作"
    });

    // 设置表格属性
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setSortingEnabled(true);

    // 禁止编辑表格内容
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置列宽
    ui->tableWidget->setColumnWidth(0, 150);  // 指令ID
    ui->tableWidget->setColumnWidth(1, 80);   // 状态
    ui->tableWidget->setColumnWidth(2, 100);  // 动作
    ui->tableWidget->setColumnWidth(3, 100);  // 子动作
    ui->tableWidget->setColumnWidth(4, 150);  // 开始时间
    ui->tableWidget->setColumnWidth(5, 150);  // 结束时间
    ui->tableWidget->setColumnWidth(6, 80);   // 完成度
    ui->tableWidget->setColumnWidth(7, 200);  // 截图URL
    ui->tableWidget->setColumnWidth(8, 80);   // 操作列
}

void commandlsit::loadCommandList()
{
    if (!m_dbManager) {
        QMessageBox::warning(this, "错误", "数据库未初始化");
        return;
    }

    // 获取所有指令
    QList<SQL_CommandHistory> commands = m_dbManager->getAllCommands();

    // 清空表格
    ui->tableWidget->setRowCount(0);

    if (commands.isEmpty()) {
        // 显示空提示
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setItem(0, 0, new QTableWidgetItem("没有找到指令数据"));
        setWindowTitle("指令历史列表（无指令）");
        return;
    }

    // 设置行数
    ui->tableWidget->setRowCount(commands.size());

    // 填充数据
    int row = 0;
    for (const SQL_CommandHistory &command : commands) {
        // 指令ID
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(command.commandId));

        // 状态
        QTableWidgetItem *statusItem = new QTableWidgetItem(command.status);
        // 根据状态设置颜色
        if (command.status.toLower() == "completed" || command.status == "完成") {
            statusItem->setForeground(Qt::darkGreen);
        } else if (command.status.toLower() == "failed" || command.status == "失败") {
            statusItem->setForeground(Qt::darkRed);
        } else if (command.status.toLower() == "pending" || command.status == "进行中") {
            statusItem->setForeground(Qt::darkYellow);
        }
        ui->tableWidget->setItem(row, 1, statusItem);

        // 动作
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(command.action));

        // 子动作
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(command.sub_action));

        // 开始时间
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(command.start_time));

        // 结束时间
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(command.end_time));

        // 完成度
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(command.completeness));

        // 截图URL（截断显示）
        QString url = command.completed_url;
        if (url.length() > 30) {
            url = url.left(27) + "...";
        }
        ui->tableWidget->setItem(row, 7, new QTableWidgetItem(url));
        ui->tableWidget->item(row, 7)->setToolTip(command.completed_url);

        // 添加删除按钮
        addDeleteButtonToRow(row, command.commandId);

        row++;
    }

    // 更新标题显示指令数量
    setWindowTitle(QString("指令历史列表（共 %1 条指令）").arg(commands.size()));
}

void commandlsit::addDeleteButtonToRow(int row, const QString &commandId)
{
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
    ui->tableWidget->setCellWidget(row, 8, widget);

    // 连接按钮点击信号
    connect(deleteButton, &QPushButton::clicked, [this, commandId]() {
        onDeleteButtonClicked(commandId);
    });
}

void commandlsit::onDeleteButtonClicked(const QString &commandId)
{
    if (commandId.isEmpty()) return;

    // 确认删除
    int result = QMessageBox::question(this, "确认删除",
                                       QString("确定要删除指令 <b>%1</b> 吗？\n此操作不可撤销！").arg(commandId),
                                       QMessageBox::Yes | QMessageBox::No,
                                       QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (!m_dbManager) {
            QMessageBox::critical(this, "错误", "数据库未初始化");
            return;
        }

        // 从数据库删除
        if (m_dbManager->deleteCommandHistory(commandId)) {
            QMessageBox::information(this, "成功", "指令删除成功");

            // 重新加载指令列表
            loadCommandList();

            qDebug() << "指令删除成功：" << commandId;
        } else {
            QMessageBox::critical(this, "错误", "指令删除失败，请检查数据库连接");
        }
    }
}

void commandlsit::on_pushButton_clicked()
{
    QString keyword = ui->lineEdit->text().trimmed();
    filterCommands(keyword);
}

void commandlsit::filterCommands(const QString &keyword)
{
    if (!m_dbManager) {
        QMessageBox::warning(this, "错误", "数据库未初始化");
        return;
    }

    // 获取所有指令
    QList<SQL_CommandHistory> allCommands = m_dbManager->getAllCommands();

    // 清空表格
    ui->tableWidget->setRowCount(0);

    // 如果没有关键词，显示所有指令
    if (keyword.isEmpty()) {
        loadCommandList();
        return;
    }

    QString searchText = keyword.toLower();
    QList<SQL_CommandHistory> filteredCommands;

    // 过滤指令
    for (const SQL_CommandHistory &command : allCommands) {
        // 搜索指令ID、状态、动作、子动作
        if (command.commandId.toLower().contains(searchText) ||
            command.status.toLower().contains(searchText) ||
            command.action.toLower().contains(searchText) ||
            command.sub_action.toLower().contains(searchText) ||
            command.completeness.toLower().contains(searchText) ||
            command.completed_url.toLower().contains(searchText)) {
            filteredCommands.append(command);
        }
    }

    if (filteredCommands.isEmpty()) {
        // 显示空提示
        ui->tableWidget->setRowCount(1);
        ui->tableWidget->setItem(0, 0, new QTableWidgetItem("未找到匹配的指令"));
        setWindowTitle(QString("指令历史列表（共 %1 条指令，搜索到 0 条）").arg(allCommands.size()));
        return;
    }

    // 设置行数
    ui->tableWidget->setRowCount(filteredCommands.size());

    // 填充过滤后的数据
    int row = 0;
    for (const SQL_CommandHistory &command : filteredCommands) {
        // 指令ID
        ui->tableWidget->setItem(row, 0, new QTableWidgetItem(command.commandId));

        // 状态
        QTableWidgetItem *statusItem = new QTableWidgetItem(command.status);
        // 根据状态设置颜色
        if (command.status.toLower() == "completed" || command.status == "完成") {
            statusItem->setForeground(Qt::darkGreen);
        } else if (command.status.toLower() == "failed" || command.status == "失败") {
            statusItem->setForeground(Qt::darkRed);
        } else if (command.status.toLower() == "pending" || command.status == "进行中") {
            statusItem->setForeground(Qt::darkYellow);
        }
        ui->tableWidget->setItem(row, 1, statusItem);

        // 动作
        ui->tableWidget->setItem(row, 2, new QTableWidgetItem(command.action));

        // 子动作
        ui->tableWidget->setItem(row, 3, new QTableWidgetItem(command.sub_action));

        // 开始时间
        ui->tableWidget->setItem(row, 4, new QTableWidgetItem(command.start_time));

        // 结束时间
        ui->tableWidget->setItem(row, 5, new QTableWidgetItem(command.end_time));

        // 完成度
        ui->tableWidget->setItem(row, 6, new QTableWidgetItem(command.completeness));

        // 截图URL（截断显示）
        QString url = command.completed_url;
        if (url.length() > 30) {
            url = url.left(27) + "...";
        }
        ui->tableWidget->setItem(row, 7, new QTableWidgetItem(url));
        ui->tableWidget->item(row, 7)->setToolTip(command.completed_url);

        // 添加删除按钮
        addDeleteButtonToRow(row, command.commandId);

        row++;
    }

    // 更新标题显示指令数量
    setWindowTitle(QString("指令历史列表（共 %1 条指令，搜索到 %2 条）")
                       .arg(allCommands.size())
                       .arg(filteredCommands.size()));
}
