#ifndef COMMANDDEV_H
#define COMMANDDEV_H

#include <QWidget  >
#include <QVector>
#include <QJsonObject>
#include "publicheader.h"  // 包含DeviceStatus结构体

namespace Ui {
class commanddev;
}

class commanddev : public QWidget
{
    Q_OBJECT

public:
    explicit commanddev(class DatabaseManager *db, QVector<DeviceStatus> *devices, QWidget *parent = nullptr);
    ~commanddev();

    // 获取生成的指令
    QJsonObject getGeneratedCommand() const;
    QList<QString> getSelectedDevices() const;

private slots:
    void on_checkBox_direct_send_stateChanged(int state);
    void on_pushButton_autoselect_clicked();
    void on_pushButton_allselect_clicked();
    void on_pushButton_command_clicked();
  //  void on_comboBox_order_currentIndexChanged(int index);
    void on_comboBox_act_currentTextChanged(const QString &text);
    void onOrderListViewDoubleClicked(const QModelIndex &index);  // 新增槽函数
signals:
    void dCommadSend(QJsonObject);

private:
    Ui::commanddev *ui;
    DatabaseManager *m_db;
    QVector<DeviceStatus> *m_devices;
    QJsonObject m_command;
    QList<QString> m_selectedDevices;

    void initOrderListView();
    void initActionComboBox();
    void updateSubActionComboBox(const QString &mainAction);
    void updateOrderInfo(const QString &orderId);
    void updateDeviceTable();
    void setupDeviceTable();
};

#endif // COMMANDDEV_H
