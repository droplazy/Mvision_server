#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include "mqttclient.h"
#include "httpserver.h"
#include "mqtt_server.h"
#include "emailsender.h"



QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

public slots:
    void DispayMqttclientStatus(bool status);
private slots:
    void on_pushButton_devlist_clicked();

    void on_pushButton_cmdquery_clicked();

    void on_pushButton_firmware_clicked();

    void on_pushButton_malluser_clicked();

private:
    Ui::MainWindow *ui;

    class HttpServer *p_http;
    class mqttclient *p_mqtt_cli;
    class MQTT_server *p_mqttt_ser;
    class DatabaseManager *p_db;
    class EmailSender *p_email;

    void updateSystemTime();
};
#endif // MAINWINDOW_H
