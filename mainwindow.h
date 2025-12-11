#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include  <QtSql>
#include <QMainWindow>
#include "mqttclient.h"
#include "httpserver.h"
#include "mqtt_server.h"




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

private:
    Ui::MainWindow *ui;

    class HttpServer *p_http;
    class mqttclient *p_mqtt_cli;
    class MQTT_server *p_mqttt_ser;
    class DatabaseManager *p_db;
};
#endif // MAINWINDOW_H
