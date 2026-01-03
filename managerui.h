#ifndef MANAGERUI_H
#define MANAGERUI_H

#include <QDialog>

namespace Ui {
class ManagerUI;
}

class DatabaseManager; // 前向声明

class ManagerUI : public QDialog
{
    Q_OBJECT

public:
    // 修改构造函数，接收数据库指针
    explicit ManagerUI(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~ManagerUI();

private slots:
    void on_pushButton_2_clicked();
    void on_pushButton_search_clicked();

private:
    Ui::ManagerUI *ui;
    DatabaseManager *m_dbManager; // 使用传入的数据库指针
};

#endif // MANAGERUI_H
