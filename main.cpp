#include "mainwindow.h"
#include <QMqttClient>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "httpserver.h"

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

    HttpServer server;
    server.startServer(8080);


    return a.exec();
}
