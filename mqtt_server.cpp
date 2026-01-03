#include "mqtt_server.h"
#include <QDir>
#include <QThread>
#include <QStringConverter>

MQTT_server::MQTT_server(DatabaseManager *db, QObject *parent)
    : QObject(parent)
    , dbManager(db)
    , process(nullptr)
{
    process = new QProcess(this);
}

MQTT_server::~MQTT_server()
{
    stopServer();
    if (process) {
        process->deleteLater();
    }
}

bool MQTT_server::startServer(const QString &ip, int port)
{
    // 获取当前工作目录
    QDir currentDir = QDir::current();

    // 构建 mosquitto 目录路径
    QString mosquittoDirPath = currentDir.filePath("mosquitto");

    // 检查 mosquitto 目录是否存在
    QDir mosquittoDir(mosquittoDirPath);
    if (!mosquittoDir.exists()) {
        qCritical() << "✗ mosquitto directory not found:" << mosquittoDirPath;
        qCritical() << "Please create mosquitto directory and put mosquitto.exe in it";
        return false;
    }

    // 构建可执行文件和配置文件的完整路径
    QString exePath = mosquittoDir.filePath("mosquitto.exe");
    QString configPath = mosquittoDir.filePath("mosquitto.conf");
    QString logPath = mosquittoDir.filePath("mosquitto.log");

    qDebug() << "========================================";
    qDebug() << "Starting MQTT Server...";
    qDebug() << "Bind IP:" << ip;
    qDebug() << "Port:" << port;
    qDebug() << "Mosquitto directory:" << mosquittoDirPath;
    qDebug() << "Configuration path:" << configPath;
    qDebug() << "========================================";

    // 1. 检查 mosquitto.exe 是否存在
    if (!QFile::exists(exePath)) {
        qCritical() << "✗ mosquitto.exe not found in:" << mosquittoDirPath;
        qCritical() << "Please ensure mosquitto.exe is in the mosquitto directory";
        return false;
    }

    // 2. 检查并处理配置文件
    if (!QFile::exists(configPath)) {
        qWarning() << "! mosquitto.conf not found, creating default configuration...";
        if (!createDefaultConfig(configPath, logPath, ip, port)) {
            qCritical() << "✗ Failed to create default configuration file";
            return false;
        }
    } else {
        if (!validateAndFixConfig(configPath, logPath, ip, port)) {
            qWarning() << "! Configuration file validation failed, recreating...";
            if (!createDefaultConfig(configPath, logPath, ip, port)) {
                qCritical() << "✗ Failed to recreate configuration file";
                return false;
            }
        }
    }

    // 3. 终止现有的 mosquitto.exe 进程
    qDebug() << "Terminating existing mosquitto.exe processes...";
    QString killCommand = "taskkill /F /IM mosquitto.exe";
    QProcess killProcess;
    killProcess.start(killCommand);
    killProcess.waitForFinished(5000);

    QThread::msleep(500);

    // 4. 启动 MQTT 服务器
    qDebug() << "Starting mosquitto.exe...";
    QStringList arguments;
    arguments << "-c" << configPath << "-v";

    if (!process) {
        process = new QProcess(this);
    }

    process->start(exePath, arguments);

    if (!process->waitForStarted(3000)) {
        qCritical() << "✗ Failed to start MQTT Server!";
        return false;
    }

    // 启动成功
    qDebug() << "✓ MQTT Server STARTED SUCCESSFULLY!";
    qDebug() << "✓ PID:" << process->processId();
    qDebug() << "✓ Bind IP:" << ip;
    qDebug() << "✓ Port:" << port;

    // 监听输出信息
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

    return true;
}

bool MQTT_server::createDefaultConfig(const QString &configPath, const QString &logPath,
                                      const QString &ip, int port)
{
    QFile configFile(configPath);
    if (!configFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qCritical() << "Cannot open config file for writing:" << configPath;
        return false;
    }

    QTextStream out(&configFile);
#if QT_VERSION >= QT_VERSION_CHECK(6, 0, 0)
    out.setEncoding(QStringConverter::Utf8);
#else
    out.setCodec("UTF-8");
#endif

    out << "# MQTT Server Configuration - Auto Generated\n";
    out << "\n";
    out << "# Network settings\n";
    out << "listener " << port << " " << ip << "\n";
    out << "\n";
    out << "# Security settings\n";
    out << "allow_anonymous true\n";
    out << "\n";
    out << "# Logging settings\n";
    out << "log_type all\n";
    out << "log_dest file " << QDir::toNativeSeparators(logPath) << "\n";
    out << "log_timestamp true\n";
    out << "\n";
    out << "# Connection settings\n";
    out << "max_connections -1\n";
    out << "persistence true\n";
    out << "persistence_file mosquitto.db\n";

    configFile.close();

    if (configFile.error() != QFile::NoError) {
        qCritical() << "Error writing config file:" << configFile.errorString();
        return false;
    }

    return true;
}

bool MQTT_server::validateAndFixConfig(const QString &configPath, const QString &logPath,
                                       const QString &ip, int port)
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
    QString expectedListener = QString("listener %1 %2").arg(port).arg(ip);
    if (!configContent.contains(expectedListener, Qt::CaseInsensitive)) {
        qWarning() << "Configuration does not match expected listener setting";
        return false;
    }

    return true;
}

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
            continue;
        }
        qDebug() << line;
    }
}

void MQTT_server::stopServer()
{
    if (process && process->state() == QProcess::Running) {
        qDebug() << "Stopping MQTT Server...";
        process->terminate();

        if (!process->waitForFinished(3000)) {
            qWarning() << "正常终止失败，强制停止服务...";
            process->kill();
        } else {
            qDebug() << "MQTT Server stopped successfully.";
        }
    } else {
        qWarning() << "MQTT Server is not running!";
    }
}
