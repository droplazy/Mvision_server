#ifndef ORDERLIST_H
#define ORDERLIST_H

#include <QWidget>
#include <QSqlQueryModel>
#include <QSortFilterProxyModel>
#include "DatabaseManager.h"

namespace Ui {
class orderlist;
}

class orderlist : public QWidget
{
    Q_OBJECT

public:
    explicit orderlist(QWidget *parent = nullptr);
    ~orderlist();

    // 设置数据库管理器
    void setDatabaseManager(DatabaseManager* dbManager);

private slots:
    void on_pushButton_search_clicked();

    // 添加双击表格的信号槽
    void on_tableView_doubleClicked(const QModelIndex &index);

private:
    // 初始化表格
    void initTableView();

    // 加载订单数据
    void loadOrders(const QString &keyword = "");

    // 显示订单详情
    void showOrderDetail(const QString &orderId);

private:
    Ui::orderlist *ui;
    DatabaseManager* m_dbManager;
    QSqlQueryModel* m_model;
    QSortFilterProxyModel* m_proxyModel;
};

#endif // ORDERLIST_H
