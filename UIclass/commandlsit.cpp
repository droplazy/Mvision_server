#include "commandlsit.h"
#include "ui_commandlsit.h"
#include <QMessageBox>
#include <QHeaderView>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QDir>
#include <QDesktopServices>

commandlsit::commandlsit(DatabaseManager* dbManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::commandlsit)
    , m_dbManager(dbManager)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("指令历史列表");
    setFixedSize(SUB_WIGET_WIDTH ,SUB_WIGET_HEITH );
    ui->tableWidget->setFixedSize(SUB_WIGET_TABLE_WIDTH,SUB_WIGET_TABLE_HEITH);    // 设置表格
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
        "开始时间", "结束时间", "完成度", "截图", "操作"
    });

    // 设置表格属性
    ui->tableWidget->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableWidget->setAlternatingRowColors(true);
    ui->tableWidget->setSortingEnabled(true);

    // 禁止编辑表格内容
    ui->tableWidget->setEditTriggers(QAbstractItemView::NoEditTriggers);

    // 设置列宽
    ui->tableWidget->setColumnWidth(0, 120);  // 指令ID
    ui->tableWidget->setColumnWidth(1, 80);   // 状态
    ui->tableWidget->setColumnWidth(2, 100);  // 动作
    ui->tableWidget->setColumnWidth(3, 100);  // 子动作
    ui->tableWidget->setColumnWidth(4, 120);  // 开始时间
    ui->tableWidget->setColumnWidth(5, 120);  // 结束时间
    ui->tableWidget->setColumnWidth(6, 80);   // 完成度
    ui->tableWidget->setColumnWidth(7, 80);   // 截图按钮列
    ui->tableWidget->setColumnWidth(8, 80);   // 操作列
}
void commandlsit::addViewButtonToRow(int row, const QString &imagePath)
{
    // 创建包含按钮的widget
    QWidget *widget = new QWidget();
    QPushButton *viewButton = new QPushButton("查看", widget);

    // 设置按钮样式
    viewButton->setStyleSheet(
        "QPushButton {"
        "    background-color: #4dabf7;"
        "    color: white;"
        "    border: none;"
        "    padding: 5px 10px;"
        "    border-radius: 3px;"
        "    font-size: 12px;"
        "}"
        "QPushButton:hover {"
        "    background-color: #339af0;"
        "}"
        "QPushButton:pressed {"
        "    background-color: #1c7ed6;"
        "}"
        "QPushButton:disabled {"
        "    background-color: #adb5bd;"
        "    color: #868e96;"
        "}"
        );

    // 设置按钮大小
    viewButton->setFixedSize(60, 25);

    // 如果图片路径为空或无效，禁用按钮
    if (imagePath.isEmpty() || imagePath == "无" || imagePath == "N/A") {
        viewButton->setEnabled(false);
        viewButton->setText("无图");
    } else {
        viewButton->setEnabled(true);
        viewButton->setText("查看");
    }

    // 布局
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(viewButton);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);

    // 将widget设置到表格
    ui->tableWidget->setCellWidget(row, 7, widget);

    // 连接按钮点击信号
    connect(viewButton, &QPushButton::clicked, [this, imagePath]() {
        onViewButtonClicked(imagePath);
    });
}
void commandlsit::onViewButtonClicked(const QString &imagePath)
{
    if (imagePath.isEmpty() || imagePath == "无" || imagePath == "N/A") {
        QMessageBox::information(this, "提示", "此指令没有截图");
        return;
    }

    // 构建完整的文件路径
    QString fullPath;

    // 处理不同的路径格式
    if (imagePath.startsWith("/")) {
        // 绝对路径或相对于根目录的路径
        fullPath = QDir::currentPath() + imagePath;
    } else if (imagePath.contains("/")) {
        // 相对路径
        fullPath = QDir::currentPath() + "/" + imagePath;
    } else {
        // 只有文件名，假设在当前目录
        fullPath = QDir::currentPath() + "/" + imagePath;
    }

    QFileInfo fileInfo(fullPath);

    // 检查文件是否存在
    if (!fileInfo.exists()) {
        QMessageBox::warning(this, "错误",
                             QString("文件不存在:\n%1").arg(fullPath));
        return;
    }

    // 检查是否是目录
    if (fileInfo.isDir()) {
        // 如果是目录，用文件管理器打开
        QUrl url = QUrl::fromLocalFile(fullPath);
        QDesktopServices::openUrl(url);
        qDebug() << "打开目录:" << fullPath;
    } else {
        // 如果是文件，打开所在目录并选中文件
        QUrl url = QUrl::fromLocalFile(fileInfo.absolutePath());
        QDesktopServices::openUrl(url);
        qDebug() << "打开文件所在目录:" << fileInfo.absolutePath();
    }
}

// 修改 loadCommandList 函数中的截图URL部分
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

        // 截图URL列改为查看按钮
        addViewButtonToRow(row, command.completed_url);

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

        // 截图URL列改为查看按钮
        addViewButtonToRow(row, command.completed_url);


        // 添加删除按钮
        addDeleteButtonToRow(row, command.commandId);

        row++;
    }

    // 更新标题显示指令数量
    setWindowTitle(QString("指令历史列表（共 %1 条指令，搜索到 %2 条）")
                       .arg(allCommands.size())
                       .arg(filteredCommands.size()));
}

void commandlsit::on_pushButton_clear_clicked()
{
    // 警告：清空所有历史记录
    int result = QMessageBox::warning(this, "清空所有记录",
                                      "⚠️ 警告：此操作将删除所有指令历史记录！\n"
                                      "此操作不可撤销！\n\n"
                                      "确定要继续吗？",
                                      QMessageBox::Yes | QMessageBox::No,
                                      QMessageBox::No);

    if (result == QMessageBox::Yes) {
        if (!m_dbManager) {
            QMessageBox::critical(this, "错误", "数据库未初始化");
            return;
        }

        // 二次确认
        int confirmResult = QMessageBox::warning(this, "再次确认",
                                                 "真的要删除所有指令历史记录吗？\n"
                                                 "共 " + QString::number(ui->tableWidget->rowCount()) + " 条记录将被删除！",
                                                 QMessageBox::Yes | QMessageBox::No,
                                                 QMessageBox::No);

        if (confirmResult == QMessageBox::Yes) {
            // 从数据库清空所有记录
            if (m_dbManager->clearAllCommandHistory()) {
                QMessageBox::information(this, "成功", "已清空所有指令历史记录");

                // 重新加载指令列表（会显示空列表）
                loadCommandList();

                qDebug() << "已清空所有指令历史记录";
            } else {
                QMessageBox::critical(this, "错误", "清空记录失败，请检查数据库连接");
            }
        }
    }
}
