#include "httpserver.h"
#include <QTcpSocket>
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <QUrlQuery>

HttpServer::HttpServer(QObject *parent) : QObject(parent), server(new QTcpServer(this)) {
    // 连接信号和槽
    connect(server, &QTcpServer::newConnection, this, &HttpServer::onNewConnection);
}

void HttpServer::startServer(quint16 port) {
    // 开始监听指定端口
    if (!server->listen(QHostAddress::Any, port)) {
        qDebug() << "Error: Unable to start server.";
    } else {
        qDebug() << "Server is running on port:" << port;
    }
}

void HttpServer::onNewConnection() {
    // 获取新的连接
    QTcpSocket *clientSocket = server->nextPendingConnection();

    // 连接信号和槽
    connect(clientSocket, &QTcpSocket::readyRead, this, &HttpServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, this, &HttpServer::onDisconnected);

    qDebug() << "New client connected!";
}

void HttpServer::onReadyRead() {
    // 获取当前连接的客户端
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if (clientSocket) {
        // 读取数据
        QByteArray request = clientSocket->readAll();
        qDebug() << "Received request:" << request;

        // 检查请求路径并根据路径读取相应的HTML文件
        QString filePath;
        QByteArray responseBody;  // 用于存储响应体的内容

        // 处理 GET 请求
        if (request.startsWith("GET / HTTP/1.1") /*|| request.startsWith("GET /login HTTP/1.1")*/) {
            filePath = "E:/htmldoc/login.html";
        }
        // 处理 POST 请求，提交表单数据
        else if (request.startsWith("POST /login HTTP/1.1")) {
            // 提取 POST 请求中的表单数据
            int contentLength = 0;
            QByteArray formData;

            // 查找 Content-Length 头部并解析表单数据
            QList<QByteArray> headers = request.split('\r\n');
            for (const QByteArray &header : headers) {
                if (header.startsWith("Content-Length:")) {
                    contentLength = header.mid(15).trimmed().toInt();
                }
            }

            // 获取 POST 请求体中的表单数据（需要跳过头部）
            int bodyStartIndex = request.indexOf("\r\n\r\n") + 4;
            formData = request.mid(bodyStartIndex);

            // 打印出表单数据
            qDebug() << "Form data received:" << formData;

            // 如果内容是 URL 编码的，进行解码
            QUrlQuery urlQuery(QString::fromUtf8(formData));
            QString username = urlQuery.queryItemValue("username");
            QString password = urlQuery.queryItemValue("password");

            qDebug() << "Username:" << username;
            qDebug() << "Password:" << password;

            // 设置返回的页面内容，可以根据需要定制
            filePath = "E:\qtpro\MuiltiControlSer/htmldoc/logsuccess.html";  // 登录成功后显示的页面
        }
        // 处理 GET 请求的其他路径
        else if (request.startsWith("GET /forgot-password HTTP/1.1")) {
            filePath = "E:/htmldoc/forget.html";
        } else if (request.startsWith("GET /register HTTP/1.1")) {
            filePath = "E:/htmldoc/register.html";
        } else {
            // 如果没有匹配到路径，返回404
            QByteArray errorResponse = "HTTP/1.1 404 Not Found\r\n";
            errorResponse += "Content-Type: text/html\r\n";
            errorResponse += "Connection: close\r\n\r\n";
            errorResponse += "<html><body><h1>404 Not Found</h1></body></html>";
            clientSocket->write(errorResponse);
            clientSocket->flush();
            clientSocket->disconnectFromHost();
            return;
        }

        // 打开文件并读取内容
        QFile file(filePath);
        if (file.exists() && file.open(QIODevice::ReadOnly)) {
            QByteArray htmlContent = file.readAll();

            // 构建响应内容
            QByteArray response = "HTTP/1.1 200 OK\r\n";
            response += "Content-Type: text/html\r\n";  // 设置返回类型为HTML
            response += "Content-Length: " + QByteArray::number(htmlContent.size()) + "\r\n";
            response += "Connection: close\r\n\r\n";  // 设置连接关闭
            response += htmlContent;

            // 发送响应
            clientSocket->write(response);
            clientSocket->flush();
        } else {
            // 如果文件无法打开，返回404错误
            QByteArray errorResponse = "HTTP/1.1 404 Not Found\r\n";
            errorResponse += "Content-Type: text/html\r\n";
            errorResponse += "Connection: close\r\n\r\n";
            errorResponse += "<html><body><h1>404 Not Found</h1></body></html>";
            clientSocket->write(errorResponse);
            clientSocket->flush();
        }

        clientSocket->disconnectFromHost();
    }
}

void HttpServer::onDisconnected() {
    // 客户端断开连接
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        qDebug() << "Client disconnected!";
        clientSocket->deleteLater();
    }
}
