#include "mqtt_server.h"



void MQTT_server::printConfigFile(const QString &filePath)
{
    QFile file(filePath);
    if (!file.exists()) {
        qWarning() << "配置文件不存在!";
        return;
    }

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "无法打开配置文件!";
        return;
    }

    QTextStream in(&file);
    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty() || line.startsWith("#")) {
            // 忽略空行和以#开头的注释行
            continue;
        }
        qDebug() << line;  // 输出配置内容
    }
}

MQTT_server::MQTT_server(DatabaseManager *db):  dbManager(db)
{
    process = new QProcess(this);  // 初始化 QProcess

}

void MQTT_server::startServer()
{
    // 配置文件路径
    QString configPath = "D:/Program Files/mosquitto/mosquitto.conf";

    // 打印配置文件内容
    printConfigFile(configPath);

    // 设置启动命令
    QString program = "D:/Program Files/mosquitto/mosquitto.exe";
    QStringList arguments;
    arguments << "-c" << configPath << "-v";  // 添加命令行参数

    // 启动进程
    process->start(program, arguments);

    // 如果进程启动成功
    if (process->waitForStarted()) {
        qDebug() << "MQTT Server started successfully.";

        // 监听输出信息
        connect(process, &QProcess::readyReadStandardOutput, [this]() {
            QByteArray output = process->readAllStandardOutput();
            qDebug() << output;
        });

        // 监听进程结束
        connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished), [this](int exitCode, QProcess::ExitStatus exitStatus) {
            if (exitStatus == QProcess::CrashExit) {
                qWarning() << "WARNING: MQTT SERVER IS TERMINATED!";
            }
        });
    } else {
        qWarning() << "无法启动 MQTT Server!";
    }
}

// 新增的紧急停止服务接口
void MQTT_server::stopServer()
{
    if (process && process->state() == QProcess::Running) {
        qDebug() << "Stopping MQTT Server...";
        process->terminate();  // 尝试正常终止进程

        if (!process->waitForFinished(3000)) {  // 等待3秒，如果没有结束，再强制结束
            qWarning() << "正常终止失败，强制停止服务...";
            process->kill();  // 强制终止进程
        } else {
            qDebug() << "MQTT Server stopped successfully.";
        }
    } else {
        qWarning() << "MQTT Server is not running!";
    }
}
