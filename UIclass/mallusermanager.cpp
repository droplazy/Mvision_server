#include "mallusermanager.h"
#include "ui_mallusermanager.h"
#include "DatabaseManager.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>

mallusermanager::mallusermanager(DatabaseManager *dbManager, QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::mallusermanager)
    , dbManager(dbManager)
    , model(new QStandardItemModel(this))
    , proxyModel(new QSortFilterProxyModel(this))
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("商城用户管理");
    setFixedSize(SUB_WIGET_WIDTH ,SUB_WIGET_HEITH );
    setFixedSize(SUB_WIGET_WIDTH ,SUB_WIGET_HEITH );
    ui->tableView->setFixedSize(SUB_WIGET_TABLE_WIDTH,SUB_WIGET_TABLE_HEITH);    // 禁止窗口拉伸
    setFixedSize(this->size());
    if (!dbManager) {
        QMessageBox::critical(this, "错误", "数据库管理器为空！");
        return;
    }

    // 设置表格
    setupTableView();

    // 加载用户数据
    loadUsers();

    // 连接信号槽
    connect(ui->lineEdit_filter, &QLineEdit::textChanged,
            this, &mallusermanager::on_lineEdit_filter_textChanged);
}

mallusermanager::~mallusermanager()
{
    delete ui;
}

void mallusermanager::setupTableView()
{
    // 设置表头 - 调整为9列（去掉了状态列）
    QStringList headers;
    headers << "用户名" << "邮箱" << "手机号" << "邀请码"
            << "邀请人" << "等级" << "余额" << "积分" << "注册时间";

    model->setHorizontalHeaderLabels(headers);

    // 设置代理模型
    proxyModel->setSourceModel(model);
    proxyModel->setFilterCaseSensitivity(Qt::CaseInsensitive);

    // 设置表格属性
    ui->tableView->setModel(proxyModel);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);

    // 设置列宽 - 调整为9列
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setColumnWidth(0, 100);  // 用户名
    ui->tableView->setColumnWidth(1, 150);  // 邮箱
    ui->tableView->setColumnWidth(2, 120);  // 手机号
    ui->tableView->setColumnWidth(3, 100);  // 邀请码
    ui->tableView->setColumnWidth(4, 100);  // 邀请人
    ui->tableView->setColumnWidth(5, 60);   // 等级
    ui->tableView->setColumnWidth(6, 80);   // 余额
    ui->tableView->setColumnWidth(7, 60);   // 积分
    ui->tableView->setColumnWidth(8, 150);  // 注册时间
}

void mallusermanager::loadUsers()
{
    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 从数据库获取所有用户
    QList<SQL_MallUser> users = dbManager->getAllMallUsers();

    // 填充表格
    for (const SQL_MallUser &user : users) {
        QList<QStandardItem*> rowItems;

        // 用户名
        rowItems.append(new QStandardItem(user.username));

        // 邮箱
        rowItems.append(new QStandardItem(user.email.isEmpty() ? "未设置" : user.email));

        // 手机号
        rowItems.append(new QStandardItem(user.phone.isEmpty() ? "未设置" : user.phone));

        // 邀请码
        rowItems.append(new QStandardItem(user.inviteCode.isEmpty() ? "未设置" : user.inviteCode));

        // 邀请人
        rowItems.append(new QStandardItem(user.inviterUsername.isEmpty() ? "无" : user.inviterUsername));

        // 等级
        rowItems.append(new QStandardItem(QString::number(user.userLevel)));

        // 余额
        rowItems.append(new QStandardItem(QString::number(user.balance, 'f', 2)));

        // 积分
        rowItems.append(new QStandardItem(QString::number(user.points)));

        // 注册时间
        QString createTime = user.createTime.isEmpty() ? "未知" : user.createTime;
        rowItems.append(new QStandardItem(createTime));

        model->appendRow(rowItems);
    }

    qDebug() << "加载了" << users.size() << "个商城用户";
}

void mallusermanager::filterUsers(const QString &filterText)
{
    if (filterText.isEmpty()) {
        // 如果过滤文本为空，显示所有数据
        proxyModel->setFilterFixedString("");
    } else {
        // 在所有列中搜索
        proxyModel->setFilterKeyColumn(-1);  // -1 表示在所有列中搜索
        proxyModel->setFilterFixedString(filterText);
    }
}

void mallusermanager::on_pushButton_clicked()
{
    // 搜索按钮功能
    QString searchText = ui->lineEdit_filter->text().trimmed();
    filterUsers(searchText);
}

void mallusermanager::on_lineEdit_filter_textChanged(const QString &text)
{
    // 实时过滤功能
    filterUsers(text);
}
