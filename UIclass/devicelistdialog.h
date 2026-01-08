// devicelistdialog.h
#ifndef DEVICELISTDIALOG_H
#define DEVICELISTDIALOG_H

#include <QDialog>
#include "DatabaseManager.h"
namespace Ui {
class devicelistdialog;
}

class devicelistdialog : public QDialog
{
    Q_OBJECT

public:
    explicit devicelistdialog(DatabaseManager* dbManager, QVector<DeviceStatus> *deviceVector, QWidget *parent = nullptr);
    ~devicelistdialog();
public slots:
    void updatedeviceinfo();
private slots:
    void on_button_search_clicked();
    void on_button_adddev_clicked();
    void onDeleteButtonClicked(const QString &serialNumber);

    void on_button_upgrade_clicked();

private:
    Ui::devicelistdialog *ui;
    DatabaseManager* m_dbManager;
    QVector<DeviceStatus> *m_deviceVector;

    void loadDeviceList();
    void setupTableView();
    bool insertDevice(const QString &serialNumber, const QString &checksum, const QString &belongUser);

    void addDeleteButtonToRow(int row, const QString &serialNumber);  // 添加删除按钮
    void deleteDevice(const QString &serialNumber);  // 删除设备

    // 辅助函数
    DeviceStatus* findDeviceInVector(const QString &serialNumber);
    // 新增：固件升级相关函数
    QString getLatestFirmwareVersion();  // 获取最新固件版本
    void updateVersionLabel();           // 更新版本标签
    int compareVersions(const QString &v1, const QString &v2);
signals:
    void deviceUpgrade(QStringList upgradeList);  // 新增：设备升级信号，第一个元素是固件文件名，后面是设备列表
    void NewDeviceCallED(QString);

};


#endif // DEVICELISTDIALOG_H
