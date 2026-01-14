#include "mainwindow.h"
#include <QMqttClient>
#include <QApplication>
#include <QLocale>
#include <QTranslator>
#include "loghandler.h"
#include "privilegehelper.h"  // 添加这行
#include <QMessageBox>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 检查管理员权限
    if (!PrivilegeHelper::requireAdministrator()) {
        // 用户选择不以管理员运行
        QMessageBox::warning(nullptr,
                             QObject::tr("警告"),
                             QObject::tr("程序将以普通用户权限运行，某些功能可能受限。"));
    }
    // else
    // {
    //     QMessageBox::information(nullptr, "程序启动","IP请填写实际地址而非全零地址");

    // }
    QTranslator translator;
    const QStringList uiLanguages = QLocale::system().uiLanguages();
    for (const QString &locale : uiLanguages) {
        const QString baseName = "MuiltiControlSer_" + QLocale(locale).name();
        if (translator.load(":/i18n/" + baseName)) {
            a.installTranslator(&translator);
            break;
        }
    }

    // qInstallMessageHandler(LogHandler::messageHandler);
    // LogHandler::instance()->setLogFile("app_log.txt");
    MainWindow w;
    w.show();

    return a.exec();
}
