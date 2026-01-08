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
    void updateNewrqeInfo(QString info);
private slots:
    void on_pushButton_devlist_clicked();

    void on_pushButton_cmdquery_clicked();

    void on_pushButton_firmware_clicked();

    void on_pushButton_malluser_clicked();

    void on_pushButton_products_clicked();

    void on_pushButton_order_clicked();

    void on_pushButton_appeal_clicked();

    void on_pushButton_webUI_clicked();

    //void on_pushButton_openhttp_clicked();

    void on_pushButton_openmqtt_clicked();

    void updateHttpDeviceContainer();
    void on_pushButton_contorluser_clicked();

    void on_pushButton_cmddispatch_clicked();

private:
    Ui::MainWindow *ui;

    QString IP;
    QString MQTT_PORT;
    QString HTTP_PORT;

    class HttpServer *p_http;
    class mqttclient *p_mqtt_cli;
    class MQTT_server *p_mqttt_ser;
    class DatabaseManager *p_db;
    class EmailSender *p_email;

    void updateSystemTime();
  //  void initDatabaseAndServices();
    void initUIStatus();
    void initDatabase();
    void startMqttClient(const QString &ip, int port);
    void initEmailService();
    void connectHttpToMqtt();
    void stopMqttServices();
    bool isMqttServerRunning();
    bool validateNetworkConfig();
    bool checkMqttServerRunning();
    void connectEmailSignals();
    void disconnectHttpConnections();
    void disconnectMqttClientSignals();
    void connectHttpSignals();
    void enableNetworkControls(bool enable);
};
#endif // MAINWINDOW_H
