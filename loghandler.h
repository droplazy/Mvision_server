#ifndef LOGHANDLER_H
#define LOGHANDLER_H

#include <QObject>
#include <QTextEdit>
#include <QMutex>
#include <QFile>
#include <QDateTime>

class LogHandler : public QObject
{
    Q_OBJECT

public:
    static LogHandler* instance();

    void setOutputWidget(QTextEdit *widget);
    void setLogFile(const QString &filename);
    void setMaxLines(int maxLines);

    static void messageHandler(QtMsgType type,
                               const QMessageLogContext &context,
                               const QString &msg);

    void clearLog();
    void saveToFile(const QString &filename);

signals:
    void logMessageReceived(const QString &message);

private:
    explicit LogHandler(QObject *parent = nullptr);
    ~LogHandler();

    static LogHandler* m_instance;
    QTextEdit* m_outputWidget = nullptr;
    QFile m_logFile;
    QMutex m_mutex;
    int m_maxLines = 1000; // 默认最大行数
    QStringList m_logBuffer; // 用于保存日志以便导出
};

#endif // LOGHANDLER_H
