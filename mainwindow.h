#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include  <QtSql>
#include <QMainWindow>
#include "mqttclient.h"
#include "httpserver.h"





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
    class mqttclient *p_mqtt;
    class DatabaseManager *p_db;
};
#endif // MAINWINDOW_H
