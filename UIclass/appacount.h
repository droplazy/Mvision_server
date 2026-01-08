#ifndef APPACOUNT_H
#define APPACOUNT_H

#include <QDialog>
#include <QColor>
#include <QPointer>  // 添加这行
namespace Ui {
class appacount;
}

class DatabaseManager;  // 前向声明
class SQL_Device;       // 前向声明

class appacount : public QDialog
{
    Q_OBJECT

public:
    // 通过构造函数传递数据库指针
    explicit appacount(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~appacount();
public slots:
    void on_tableWidget_cellDoubleClicked(int row, int column);

private slots:
    void on_pushButton_search_clicked();

private:
    Ui::appacount *ui;
    DatabaseManager* m_dbManager;  // 保存数据库指针

    // 初始化表格
    void initTable();

    // 加载所有设备
    void loadAllDevices();

    // 添加设备到表格
    void addDeviceToTable(const SQL_Device& device);

    // 根据状态文本返回颜色
    QColor getStatusColor(const QString& status);
};

#endif // APPACOUNT_H
