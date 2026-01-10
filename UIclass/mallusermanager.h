#ifndef MALLUSERMANAGER_H
#define MALLUSERMANAGER_H

#include <QWidget>
#include <QStandardItemModel>
#include <QSortFilterProxyModel>

class DatabaseManager;

namespace Ui {
class mallusermanager;
}

class mallusermanager : public QWidget
{
    Q_OBJECT

public:
    explicit mallusermanager(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~mallusermanager();

private slots:
    void on_pushButton_clicked();  // 搜索按钮
    void on_lineEdit_filter_textChanged(const QString &text);  // 实时过滤

private:
    Ui::mallusermanager *ui;
    DatabaseManager *dbManager;
    QStandardItemModel *model;
    QSortFilterProxyModel *proxyModel;

    void setupTableView();          // 设置表格
    void loadUsers();               // 加载用户数据
    void filterUsers(const QString &filterText);  // 过滤用户
};

#endif // MALLUSERMANAGER_H
