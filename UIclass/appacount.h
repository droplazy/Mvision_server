#ifndef APPACOUNT_H
#define APPACOUNT_H

#include <QWidget>
#include <QColor>
#include <QPointer>
#include <QMap>
#include <QTimer>

class applogin2;

namespace Ui {
class appacount;
}

class DatabaseManager;  // 前向声明
class SQL_Device;       // 前向声明

class appacount : public QWidget
{
    Q_OBJECT

public:
    // 通过构造函数传递数据库指针
    explicit appacount(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~appacount();

    // 公共方法：刷新表格数据
    void refreshTableData();

signals:
    // 新增的信号，用于转发JSON到MainWindow
    void forwardJsonToMQTT(const QString& deviceSerial, const QString& jsonString);

public slots:
    void on_tableWidget_cellDoubleClicked(int row, int column);

    // 接收登录结果的槽函数 - 修改为bool参数
    void onAppLoginStatusReceived(const QString& commandId, bool success);
    void updateCRcode(const QString &commandId);
private slots:
    void on_pushButton_search_clicked();

private:
    Ui::appacount *ui;
    DatabaseManager* m_dbManager;  // 保存数据库指针
    QMap<QString, QPointer<applogin2>> m_loginDialogs;  // 保存登录对话框指针，按命令ID索引
    QMap<QString, QString> m_commandDeviceMap;  // 保存命令ID与设备号的映射

    // 初始化表格
    void initTable();

    // 加载所有设备
    void loadAllDevices();

    // 添加设备到表格
    void addDeviceToTable(const SQL_Device& device);

    // 根据状态文本返回颜色
    QColor getStatusColor(const QString& status);

    // 更新设备状态
    void updateDeviceStatus(const QString& deviceSerial, int platformColumn, const QString& newStatus);
};

#endif // APPACOUNT_H
