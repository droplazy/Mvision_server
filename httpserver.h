#ifndef HTTPSERVER_H
#define HTTPSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>

class HttpServer : public QObject
{
    Q_OBJECT
public:
    explicit HttpServer(QObject *parent = nullptr);

    void startServer(quint16 port);

    QString getRequestParameter(const QByteArray &request, const QString &param);
private slots:
    void onNewConnection();
    void onReadyRead();
    void onDisconnected();

private:
    QTcpServer *server;

    // 新增的辅助函数
    QString parsePostData(const QByteArray &request);  // 解析POST数据
    QString getValueFromFormData(const QString &data, const QString &key);  // 获取表单数据中的特定字段
};

#endif // HTTPSERVER_H
