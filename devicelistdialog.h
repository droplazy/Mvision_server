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
    explicit devicelistdialog(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~devicelistdialog();

private slots:
    void on_button_search_clicked();

    void on_button_adddev_clicked();

    void onDeleteButtonClicked(const QString &serialNumber);
private:
    Ui::devicelistdialog *ui;
    DatabaseManager* m_dbManager;

    void loadDeviceList();
    void setupTableView();
    bool insertDevice(const QString &serialNumber, const QString &checksum, const QString &belongUser);

    void addDeleteButtonToRow(int row, const QString &serialNumber);  // 添加删除按钮
    void deleteDevice(const QString &serialNumber);  // 删除设备
};

#endif // DEVICELISTDIALOG_H
