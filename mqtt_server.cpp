#include "mqtt_server.h"
#include <QDir>
#include <QApplication>
#include <QThread>

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

bool MQTT_server::startServer()
{
    // 获取当前应用程序目录
    QString currentDir = QCoreApplication::applicationDirPath()+"/mosquitto/";

    // 构建可执行文件和配置文件的完整路径
    QString exePath = QDir(currentDir).filePath("mosquitto.exe");
    QString configPath = QDir(currentDir).filePath("mosquitto.conf");
    QString logPath = QDir(currentDir).filePath("mosquitto.log");

    qDebug() << "========================================";
    qDebug() << "Starting MQTT Server...";
    qDebug() << "Current directory:" << currentDir;
    qDebug() << "mosquitto.exe path:" << exePath;
    qDebug() << "Configuration path:" << configPath;
    qDebug() << "Log file path:" << logPath;
    qDebug() << "========================================";

    // 1. 检查 mosquitto.exe 是否存在
    if (!QFile::exists(exePath)) {
        qCritical() << "✗ mosquitto.exe not found in:" << currentDir;
        qCritical() << "Please ensure mosquitto.exe is in the application directory";
        return false;
    } else {
        qDebug() << "✓ mosquitto.exe found";
    }

    // 2. 检查并处理配置文件
    if (!QFile::exists(configPath)) {
        qWarning() << "! mosquitto.conf not found, creating default configuration...";
        if (!createDefaultConfig(configPath, logPath)) {
            qCritical() << "✗ Failed to create default configuration file";
            return false;
        }
        qDebug() << "✓ Default configuration created successfully";
    } else {
        qDebug() << "✓ Configuration file found";
        // 检查现有配置文件内容
        if (!validateAndFixConfig(configPath, logPath)) {
            qWarning() << "! Configuration file validation failed, recreating...";
            if (!createDefaultConfig(configPath, logPath)) {
                qCritical() << "✗ Failed to recreate configuration file";
                return false;
            }
            qDebug() << "✓ Configuration file recreated successfully";
        } else {
            qDebug() << "✓ Configuration file validation passed";
        }
    }

    // 3. 终止现有的 mosquitto.exe 进程
    qDebug() << "Terminating existing mosquitto.exe processes...";
    QString killCommand = "taskkill /F /IM mosquitto.exe";
    QProcess killProcess;
    killProcess.start(killCommand);

    if (!killProcess.waitForFinished(5000)) {
        qWarning() << "! Warning: Could not terminate existing mosquitto.exe processes";
    } else {
        QString output = killProcess.readAllStandardOutput();
        QString error = killProcess.readAllStandardError();

        if (output.contains("SUCCESS") || output.contains("terminated")) {
            qDebug() << "✓ Existing mosquitto.exe processes terminated";
        } else if (error.contains("not found") || output.contains("not found")) {
            qDebug() << "✓ No existing mosquitto.exe processes found";
        } else {
            qDebug() << "✓ Process termination command executed";
        }
    }

    // 等待一会儿确保进程完全退出
    QThread::msleep(500);

    // 4. 打印配置文件内容（调试用）
    printConfigFile(configPath);

    // 5. 启动 MQTT 服务器
    qDebug() << "Starting mosquitto.exe...";
    QStringList arguments;
    arguments << "-c" << configPath << "-v";

    process->start(exePath, arguments);

    // 等待进程启动
    if (!process->waitForStarted(3000)) {
        qCritical() << "✗ Failed to start MQTT Server!";
        qCritical() << "Error:" << process->errorString();
        return false;
    }

    // 启动成功，打印详细信息
    qDebug() << "========================================";
    qDebug() << "✓ MQTT Server STARTED SUCCESSFULLY!";
    qDebug() << "✓ PID:" << process->processId();
    qDebug() << "✓ Port: 1883";
    qDebug() << "✓ Listening on: 0.0.0.0";
    qDebug() << "✓ Anonymous access: DISABLED";
    qDebug() << "✓ Config file: " << configPath;
    qDebug() << "✓ Log file: " << logPath;
    qDebug() << "========================================";

    // 6. 监听输出信息
    connect(process, &QProcess::readyReadStandardOutput, [this]() {
        QByteArray output = process->readAllStandardOutput();
        QString outputStr = QString::fromLocal8Bit(output).trimmed();
        if (!outputStr.isEmpty()) {
            qDebug() << "[MOSQUITTO]" << outputStr;
        }
    });

    connect(process, &QProcess::readyReadStandardError, [this]() {
        QByteArray error = process->readAllStandardError();
        QString errorStr = QString::fromLocal8Bit(error).trimmed();
        if (!errorStr.isEmpty()) {
            qWarning() << "[MOSQUITTO]" << errorStr;
        }
    });

    // 7. 监听进程结束
    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            [this](int exitCode, QProcess::ExitStatus exitStatus) {
                if (exitStatus == QProcess::CrashExit) {
                    qCritical() << "✗ MQTT Server CRASHED with exit code:" << exitCode;
                    qCritical() << "Please check the log file for details";
                } else {
                    qWarning() << "! MQTT Server terminated with exit code:" << exitCode;
                }
            });

    return true;
}

bool MQTT_server::createDefaultConfig(const QString &configPath, const QString &logPath)
{
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open config file for writing:" << configPath;
        return false;
    }

    QTextStream out(&configFile);
out.setEncoding(QStringConverter::Utf8);
    // 编写配置文件内容
    out << "# MQTT Server Configuration - Auto Generated\n";
    out << "\n";
    out << "# Network settings\n";
    out << "listener 1883 0.0.0.0\n";
    out << "\n";
    out << "# Security settings - Disable anonymous access\n";
    out << "allow_anonymous true\n";
    out << "\n";
    out << "# Logging settings\n";
    out << "log_type all\n";
    out << "log_dest file " << QDir::toNativeSeparators(logPath) << "\n";
    out << "log_type error\n";
    out << "log_type warning\n";
    out << "log_type notice\n";
    out << "log_type information\n";
    out << "log_timestamp true\n";
    out << "\n";
    out << "# Connection settings\n";
    out << "max_connections -1\n";
    out << "persistence true\n";
    out << "persistence_file mosquitto.db\n";
    out << "\n";
    out << "# QoS settings\n";
    out << "max_inflight_messages 20\n";
    out << "max_queued_messages 1000\n";

    configFile.close();

    if (configFile.error() != QFile::NoError) {
        qCritical() << "Error writing config file:" << configFile.errorString();
        return false;
    }

    qDebug() << "Default configuration created at:" << configPath;
    return true;
}

bool MQTT_server::validateAndFixConfig(const QString &configPath, const QString &logPath)
{
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Cannot open config file for reading:" << configPath;
        return false;
    }

    QTextStream in(&configFile);
    QString configContent = in.readAll();
    configFile.close();

    // 检查关键配置项
    bool hasListener = configContent.contains("listener", Qt::CaseInsensitive);
    bool hasAllowAnonymous = configContent.contains("allow_anonymous", Qt::CaseInsensitive);
    bool hasLogDest = configContent.contains("log_dest", Qt::CaseInsensitive);

    // 如果缺少关键配置项，需要重新创建
    if (!hasListener || !hasAllowAnonymous || !hasLogDest) {
        qWarning() << "Configuration file missing critical settings";
        return false;
    }

    // 检查是否允许匿名访问
    if (configContent.contains("allow_anonymous false", Qt::CaseInsensitive)) {
        qWarning() << "Configuration allows anonymous access, need to fix";
        return false;
    }

    // 检查日志路径是否在当前目录
    if (!configContent.contains(QDir::toNativeSeparators(logPath))) {
        qWarning() << "Log destination is not set to current directory";
        return false;
    }

    return true;
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
