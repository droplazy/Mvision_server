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
        // 读取请求
        QByteArray request = clientSocket->readAll();
        qDebug() << "Received request:" << request;

        // 使用绝对路径返回文件
        QString basePath = "E:/qtpro/MuiltiControlSer/";

        // 解析请求路径
        QStringList requestLines = QString(request).split("\r\n");
        QString path = requestLines.first().split(" ")[1];

        // 如果请求的是根目录，就返回 index.html
        if (path == "/") {
            path = "/index.html";
        }

        QString filePath = basePath + path.mid(1);  // 去掉路径中的斜杠
        qDebug() << "Requested file: " << filePath;

        // 处理请求资源
        if (request.startsWith("GET") && QFile::exists(filePath)) {
            QFile file(filePath);
            if (file.open(QIODevice::ReadOnly)) {
                QByteArray fileContent = file.readAll();

                // 根据文件类型设置响应头
                QByteArray responseHeader = "HTTP/1.1 200 OK\r\n";
                QByteArray contentType = getContentType(filePath);
                responseHeader += "Content-Type: " + contentType + "\r\n";
                responseHeader += "Content-Length: " + QByteArray::number(fileContent.size()) + "\r\n";
                responseHeader += "Connection: close\r\n\r\n";

                // 发送响应
                clientSocket->write(responseHeader);
                clientSocket->write(fileContent);
                clientSocket->flush();
            } else {
                send404(clientSocket);  // 如果文件无法打开，返回404
            }
        } else {
            send404(clientSocket);  // 如果文件不存在，返回404
        }

        // 断开与客户端的连接
        clientSocket->disconnectFromHost();
    }
}

// 获取文件类型
QByteArray HttpServer::getContentType(const QString &filePath) {
    if (filePath.endsWith(".html")) {
        return "text/html";
    } else if (filePath.endsWith(".css")) {
        return "text/css";
    } else if (filePath.endsWith(".js")) {
        return "application/javascript";
    } else if (filePath.endsWith(".jpg") || filePath.endsWith(".jpeg")) {
        return "image/jpeg";
    } else if (filePath.endsWith(".png")) {
        return "image/png";
    } else if (filePath.endsWith(".gif")) {
        return "image/gif";
    } else {
        return "application/octet-stream";  // 默认文件类型
    }
}

// 发送404响应
void HttpServer::send404(QTcpSocket *clientSocket) {
    QByteArray errorResponse = "HTTP/1.1 404 Not Found\r\n";
    errorResponse += "Content-Type: text/html\r\n";
    errorResponse += "Connection: close\r\n\r\n";
    errorResponse += "<html><body><h1>404 Not Found</h1></body></html>";
    clientSocket->write(errorResponse);
    clientSocket->flush();
}

void HttpServer::onDisconnected() {
    // 客户端断开连接
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());
    if (clientSocket) {
        qDebug() << "Client disconnected!";
        clientSocket->deleteLater();
    }
}
