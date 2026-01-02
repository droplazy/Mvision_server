#include "loghandler.h"
#include <QTextStream>
#include <QApplication>
#include <QScrollBar>
#include <QDebug>

LogHandler* LogHandler::m_instance = nullptr;

LogHandler* LogHandler::instance()
{
    static QMutex mutex;
    if (!m_instance) {
        mutex.lock();
        if (!m_instance) {
            m_instance = new LogHandler();
        }
        mutex.unlock();
    }
    return m_instance;
}

LogHandler::LogHandler(QObject *parent) : QObject(parent)
{
    // 连接信号，确保日志更新在 GUI 线程执行
    connect(this, &LogHandler::logMessageReceived,
            this, [this](const QString &msg) {
                if (m_outputWidget) {
                    // 添加到文本编辑框
                    m_outputWidget->append(msg);

                    // 限制最大行数
                    QStringList lines = m_outputWidget->toPlainText().split("\n");
                    if (lines.size() > m_maxLines) {
                        lines = lines.mid(lines.size() - m_maxLines);
                        m_outputWidget->setPlainText(lines.join("\n"));
                    }

                    // 自动滚动到底部
                    QScrollBar *scrollBar = m_outputWidget->verticalScrollBar();
                    scrollBar->setValue(scrollBar->maximum());
                }
            });
}

LogHandler::~LogHandler()
{
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }
}

void LogHandler::setOutputWidget(QTextEdit *widget)
{
    m_outputWidget = widget;
    if (m_outputWidget) {
        m_outputWidget->setReadOnly(true);
        m_outputWidget->setWordWrapMode(QTextOption::NoWrap); // 不自动换行
    }
}

void LogHandler::setLogFile(const QString &filename)
{
    m_mutex.lock();
    if (m_logFile.isOpen()) {
        m_logFile.close();
    }

    m_logFile.setFileName(filename);
    if (m_logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
        QTextStream stream(&m_logFile);
        stream << "\n=== Log started at "
               << QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")
               << " ===\n";
        stream.flush();
    }
    m_mutex.unlock();
}

void LogHandler::setMaxLines(int maxLines)
{
    m_maxLines = maxLines;
}

void LogHandler::messageHandler(QtMsgType type,
                                const QMessageLogContext &context,
                                const QString &msg)
{
    Q_UNUSED(context);

    QString timestamp = QDateTime::currentDateTime().toString("hh:mm:ss.zzz");
    QString level;

    switch (type) {
    case QtDebugMsg:
        level = "DEBUG";
        break;
    case QtInfoMsg:
        level = "INFO ";
        break;
    case QtWarningMsg:
        level = "WARN ";
        break;
    case QtCriticalMsg:
        level = "ERROR";
        break;
    case QtFatalMsg:
        level = "FATAL";
        break;
    }

    QString logLine = QString("[%1][%2] %3")
                          .arg(timestamp, level, msg);

    LogHandler::instance()->m_mutex.lock();

    // 保存到缓冲区以便导出
    LogHandler::instance()->m_logBuffer.append(logLine);

    // 写入文件
    if (LogHandler::instance()->m_logFile.isOpen()) {
        QTextStream fileStream(&LogHandler::instance()->m_logFile);
        fileStream << logLine << "\n";
        fileStream.flush();
    }

    LogHandler::instance()->m_mutex.unlock();

    // 发送到 GUI（线程安全）
    emit LogHandler::instance()->logMessageReceived(logLine);
}

void LogHandler::clearLog()
{
    if (m_outputWidget) {
        m_outputWidget->clear();
    }
    m_logBuffer.clear();
}

void LogHandler::saveToFile(const QString &filename)
{
    QFile file(filename);
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream stream(&file);
        for (const QString &line : m_logBuffer) {
            stream << line << "\n";
        }
        file.close();
    }
}
