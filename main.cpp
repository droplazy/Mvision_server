#include "mainwindow.h"
#include <QMqttClient>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MuiltiControlSer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }
    MainWindow w;
   // w.show();


    // 创建DatabaseManager实例

    // mqttclient mqtt_cli;
    // // mqtt_cli.start();

    return a.exec();
}
