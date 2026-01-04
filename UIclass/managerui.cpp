#include "managerui.h"
#include "ui_managerui.h"
#include "DatabaseManager.h"
#include <QSqlTableModel>
#include <QPushButton>
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>

ManagerUI::ManagerUI(DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::ManagerUI)
    , m_dbManager(dbManager)
{
    ui->setupUi(this);

    // 设置窗口标题
    this->setWindowTitle("用户管理");

    // 设置密码输入框的echoMode
  //  ui->lineEdit_3->setEchoMode(QLineEdit::Password);

    // 创建模型并设置到tableView
    QSqlTableModel *model = new QSqlTableModel(this);
    model->setTable("Users");

    // 设置表头
    model->setHeaderData(0, Qt::Horizontal, "账号");
    model->setHeaderData(1, Qt::Horizontal, "密码");
    model->setHeaderData(2, Qt::Horizontal, "手机号");
    model->setHeaderData(3, Qt::Horizontal, "邮箱");

    if (!model->select()) {
        QMessageBox::critical(this, "错误", "无法加载用户数据");
        return;
    }

    ui->tableView->setModel(model);

    // 设置表格属性
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
    ui->tableView->verticalHeader()->setVisible(false);

    // 添加删除按钮列
    for (int row = 0; row < model->rowCount(); ++row) {
        QPushButton *deleteBtn = new QPushButton("删除", this);
        deleteBtn->setProperty("row", row);

        // 连接删除按钮信号
        connect(deleteBtn, &QPushButton::clicked, [this, model, row]() {
            QString username = model->data(model->index(row, 0)).toString();

            if (QMessageBox::question(this, "确认删除",
                                      QString("确定要删除用户 '%1' 吗？").arg(username),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

                if (m_dbManager->deleteUser(username)) {
                    model->select(); // 刷新数据

                    // 重新为每一行创建删除按钮
                    for (int i = 0; i < model->rowCount(); ++i) {
                        QPushButton *newDeleteBtn = new QPushButton("删除", this);
                        newDeleteBtn->setProperty("row", i);

                        connect(newDeleteBtn, &QPushButton::clicked, [this, model, i]() {
                            QString delUsername = model->data(model->index(i, 0)).toString();

                            if (QMessageBox::question(this, "确认删除",
                                                      QString("确定要删除用户 '%1' 吗？").arg(delUsername),
                                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

                                if (m_dbManager->deleteUser(delUsername)) {
                                    model->select();
                                }
                            }
                        });

                        ui->tableView->setIndexWidget(model->index(i, 4), newDeleteBtn);
                    }
                }
            }
        });

        // 将按钮添加到表格的最后一列
        ui->tableView->setIndexWidget(model->index(row, 4), deleteBtn);
    }
}

ManagerUI::~ManagerUI()
{
    delete ui;
}

void ManagerUI::on_pushButton_2_clicked()
{
    QString username = ui->lineEdit_2->text().trimmed();
    QString password = ui->lineEdit_3->text().trimmed();
    QString phone = ui->lineEdit_4->text().trimmed();
    QString email = ui->lineEdit_5->text().trimmed();

    // 验证输入
    if (username.isEmpty() || password.isEmpty()) {
        QMessageBox::warning(this, "警告", "账号和密码不能为空！");
        return;
    }

    if (!m_dbManager) {
        QMessageBox::critical(this, "错误", "数据库连接无效！");
        return;
    }

    // 检查账号是否已存在
    SQL_User existingUser = m_dbManager->getUserByUsername(username);
    if (!existingUser.username.isEmpty()) {
        QMessageBox::warning(this, "警告", "账号已存在！");
        return;
    }

    // 创建用户对象
    SQL_User newUser;
    newUser.username = username;
    newUser.password = password;
    newUser.phone_number = phone;
    newUser.email = email;

    // 插入数据库
    if (m_dbManager->insertUser(newUser)) {
        QMessageBox::information(this, "成功", "用户添加成功！");

        // 清空输入框
        ui->lineEdit_2->clear();
        ui->lineEdit_3->clear();
        ui->lineEdit_4->clear();
        ui->lineEdit_5->clear();

        // 刷新表格
        QSqlTableModel *model = qobject_cast<QSqlTableModel*>(ui->tableView->model());
        if (model) {
            model->select();

            // 重新创建删除按钮
            for (int row = 0; row < model->rowCount(); ++row) {
                QPushButton *deleteBtn = new QPushButton("删除", this);
                deleteBtn->setProperty("row", row);

                connect(deleteBtn, &QPushButton::clicked, [this, model, row]() {
                    QString delUsername = model->data(model->index(row, 0)).toString();

                    if (QMessageBox::question(this, "确认删除",
                                              QString("确定要删除用户 '%1' 吗？").arg(delUsername),
                                              QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

                        if (m_dbManager->deleteUser(delUsername)) {
                            model->select();
                        }
                    }
                });

                ui->tableView->setIndexWidget(model->index(row, 4), deleteBtn);
            }
        }
    } else {
        QMessageBox::critical(this, "错误", "添加用户失败！");
    }
}

void ManagerUI::on_pushButton_search_clicked()
{
    QString searchText = ui->lineEdit->text().trimmed();
    QSqlTableModel *model = qobject_cast<QSqlTableModel*>(ui->tableView->model());

    if (!model) return;

    if (searchText.isEmpty()) {
        model->setFilter("");
    } else {
        QString filter = QString("username LIKE '%%1%' OR phone_number LIKE '%%1%' OR email LIKE '%%1%'")
        .arg(searchText);
        model->setFilter(filter);
    }

    if (!model->select()) {
        return;
    }

    // 更新删除按钮
    for (int row = 0; row < model->rowCount(); ++row) {
        QPushButton *deleteBtn = new QPushButton("删除", this);
        deleteBtn->setProperty("row", row);

        connect(deleteBtn, &QPushButton::clicked, [this, model, row]() {
            QString delUsername = model->data(model->index(row, 0)).toString();

            if (QMessageBox::question(this, "确认删除",
                                      QString("确定要删除用户 '%1' 吗？").arg(delUsername),
                                      QMessageBox::Yes | QMessageBox::No) == QMessageBox::Yes) {

                if (m_dbManager->deleteUser(delUsername)) {
                    model->select();
                }
            }
        });

        ui->tableView->setIndexWidget(model->index(row, 4), deleteBtn);
    }
}
