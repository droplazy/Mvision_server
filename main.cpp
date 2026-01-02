#include "mainwindow.h"
#include <QMqttClient>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "loghandler.h"

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
    // ==================== 关键修复：添加这行 ====================
    // 安装全局消息处理器（必须放在QApplication创建后，MainWindow创建前）
   // qInstallMessageHandler(LogHandler::messageHandler);
    // ==========================================================
    //LogHandler::instance()->setLogFile("app_log.txt");
    MainWindow w;
    w.show();

    return a.exec();
}
