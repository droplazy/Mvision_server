#include "HttpServer.h"
#include "qjsonarray.h"
#include <QJsonObject>
#include <QDebug>
#include <QUrlQuery>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>
#include <QFileInfo>
#include <QMimeDatabase>
#include <QMimeType>
#include <QDateTime>
#define MANAGER_USERNAME "123"
#define MANAGER_PASSWD   "123"
#include <QRandomGenerator>
#include <QCryptographicHash>

HttpServer::HttpServer(DatabaseManager *db,QObject *parent) : QTcpServer(parent), dbManager(db)
{
    handleCreateTestOrdersSimple();
    generateTextData();
    createDownloadDirectoryIfNeeded();
    createUploadDirectoryIfNeeded();
}

void HttpServer::createDownloadDirectoryIfNeeded()
{
    QDir currentDir(QDir::currentPath());
    QString downloadPath = currentDir.filePath("Download");
  //  qDebug() << downloadPath << " is created ! ... ";
    if (!QDir(downloadPath).exists()) {
        QDir().mkdir(downloadPath);
    }
}
void HttpServer::createUploadDirectoryIfNeeded()
{
    QDir currentDir(QDir::currentPath());
    QString downloadPath = currentDir.filePath("Upload");
    if (!QDir(downloadPath).exists()) {
        QDir().mkdir(downloadPath);
    }
}

void HttpServer::generateTextData()
{
#if 0
    deviceVector.append(DeviceStatus("SN999321", "在线", "杭州", "休息", "56841.12MB",
                                     "2025-11-25T12:31:00Z", "192.168.10.111",
                                     "2025-10-25T13:00:12Z", "2025-10-25T14:00:12Z", "吃饭",
                                     "2025-10-25T15:00:12Z", "2025-10-25T16:00:12Z"));
    deviceVector.append(DeviceStatus("SN999322", "离线", "上海", "休息", "67234.98MB",
                                     "2025-11-25T12:35:00Z", "192.168.10.112",
                                     "2025-10-25T13:02:12Z", "2025-10-25T14:05:12Z", "洗澡",
                                     "2025-10-25T15:15:12Z", "2025-10-25T16:25:12Z"));
    deviceVector.append(DeviceStatus("SN999323", "在线", "广州", "休息", "78453.67MB",
                                     "2025-11-25T12:40:00Z", "192.168.10.113",
                                     "2025-10-25T13:10:12Z", "2025-10-25T14:20:12Z", "看电视",
                                     "2025-10-25T15:30:12Z", "2025-10-25T16:40:12Z"));
    deviceVector.append(DeviceStatus("SN999324", "离线", "北京", "休息", "91000.25MB",
                                     "2025-11-25T12:45:00Z", "192.168.10.114",
                                     "2025-10-25T13:20:12Z", "2025-10-25T14:30:12Z", "听音乐",
                                     "2025-10-25T15:40:12Z", "2025-10-25T16:50:12Z"));
    deviceVector.append(DeviceStatus("SN999325", "在线", "深圳", "休息", "12345.78MB",
                                     "2025-11-25T12:50:00Z", "192.168.10.115",
                                     "2025-10-25T13:30:12Z", "2025-10-25T14:40:12Z", "读书",
                                     "2025-10-25T15:50:12Z", "2025-10-25T16:55:12Z"));
    deviceVector.append(DeviceStatus("SN999326", "离线", "天津", "休息", "56789.99MB",
                                     "2025-11-25T12:55:00Z", "192.168.10.116",
                                     "2025-10-25T13:40:12Z", "2025-10-25T14:50:12Z", "打游戏",
                                     "2025-10-25T15:55:12Z", "2025-10-25T17:00:12Z"));
    deviceVector.append(DeviceStatus("SN999327", "在线", "成都", "休息", "23456.12MB",
                                     "2025-11-25T13:00:00Z", "192.168.10.117",
                                     "2025-10-25T13:50:12Z", "2025-10-25T14:55:12Z", "散步",
                                     "2025-10-25T16:00:12Z", "2025-10-25T17:05:12Z"));

    // 第一条流程
    Machine_Process_Total process1;
    process1.process_id = "1";
    process1.process_name = "小明创建的流程";
    process1.creation_time = "2025-11-11T12:00:00Z";
    process1.remark = "这个流程是给狗用的";

    // 添加子流程，调整action、sub_action等内容
    process1.Processes.append(Machine_Process_Single{"吃饭", "江边", "2025-11-11T12:00:00Z", "2025-11-11T12:30:00Z", "中午吃炒饭"});
    process1.Processes.append(Machine_Process_Single{"休息", "江边", "2025-11-11T12:40:00Z", "2025-11-11T13:00:00Z", "休息时间玩球"});
    process1.Processes.append(Machine_Process_Single{"洗澡", "江边", "2025-11-11T13:10:00Z", "2025-11-11T13:20:00Z", "准备洗澡，调节水温"});
    process1.Processes.append(Machine_Process_Single{"洗澡", "江边", "2025-11-11T13:30:00Z", "2025-11-11T14:00:00Z", "洗澡后擦干，准备穿衣服"});
    process1.Processes.append(Machine_Process_Single{"吃饭", "江边", "2025-11-11T14:30:00Z", "2025-11-11T14:50:00Z", "吃些零食"});
    process1.Processes.append(Machine_Process_Single{"散步", "江边", "2025-11-11T15:00:00Z", "2025-11-11T15:30:00Z", "带狗去附近散步"});
    process1.Processes.append(Machine_Process_Single{"游戏", "江边", "2025-11-11T16:00:00Z", "2025-11-11T16:30:00Z", "下午一起玩接飞盘游戏"});
    process1.Processes.append(Machine_Process_Single{"休息", "江边", "2025-11-11T16:40:00Z", "2025-11-11T17:00:00Z", "吃完饭后小憩片刻"});



    // 第二条流程
    Machine_Process_Total process2;
    process2.process_id = "2";
    process2.process_name = "小明创建的另一个流程";
    process2.creation_time = "2025-11-12T14:00:00Z";
    process2.remark = "这个流程是给猫用的";

    // 添加子流程
    process2.Processes.append(Machine_Process_Single{"散步", "躺平", "2025-11-12T14:00:00Z", "2025-11-12T14:30:00Z", "午饭吃饭"});
    process2.Processes.append(Machine_Process_Single{"吃屎", "躺平", "2025-11-12T15:00:00Z", "2025-11-12T15:30:00Z", "洗澡后吃鸡蛋"});

    // 第三条流程
    Machine_Process_Total process3;
    process3.process_id = "3";
    process3.process_name = "小明创建的特殊流程";
    process3.creation_time = "2025-11-13T16:00:00Z";
    process3.remark = "这个流程是给狗和猫用的";

    // 添加子流程
    process3.Processes.append(Machine_Process_Single{"骂人", "躺平", "2025-11-13T16:00:00Z", "2025-11-13T16:30:00Z", "下午吃饭"});
    process3.Processes.append(Machine_Process_Single{"杀人", "躺平", "2025-11-13T17:00:00Z", "2025-11-13T17:30:00Z", "洗澡后吃鸡蛋"});



    processVector.append(process1);
    processVector.append(process2);
    processVector.append(process3);



    for (const DeviceStatus& device : deviceVector)
    {
        //device.append(device.toJsonAll());
        SQL_Device device_preinsert;
        device_preinsert.device_status = device.status;
        device_preinsert.total_flow = device.trafficStatistics;
        device_preinsert.bound_user ="123";
        device_preinsert.checksum ="SSVGG";
        device_preinsert.serial_number =device.serialNumber;
        device_preinsert.ip_address =device.ip;
        device_preinsert.bound_time ="14点50分";
        dbManager.insertDevice(device_preinsert);
    }

    for (const Machine_Process_Total& processes : processVector)
    {
        dbManager.insertProcessSteps(processes);
    }


#endif
    QList<SQL_Device> devices = dbManager->getAllDevices();
    processVector = dbManager->getAllProcessSteps();
    for (const SQL_Device &device : devices) {
        deviceVector.append(DeviceStatus(device.serial_number, device.device_status, "未知", "未知", device.total_flow,
                                         "未知", device.ip_address,
                                         "未知", "未知", "未知",
                                         "未知", "未知","未知", "未知"));
    }
}


void HttpServer::incomingConnection(qintptr socketDescriptor) {
    QTcpSocket *clientSocket = new QTcpSocket();
    clientSocket->setSocketDescriptor(socketDescriptor);
    connect(clientSocket, &QTcpSocket::readyRead, this, &HttpServer::onReadyRead);
    connect(clientSocket, &QTcpSocket::disconnected, clientSocket, &QTcpSocket::deleteLater);


}


void HttpServer::ShowHomepage(QTcpSocket *clientSocket, QByteArray request)
{

    // 解析请求路径
    QStringList requestLines = QString(request).split("\r\n");
    QString path = requestLines.first().split(" ")[1];
    //路径加个修改区别商城
    if ((path == "/home" || path == "/devices" ||path == "/process/new" ||path == ("/login")\
         ||path == "/process/center"||path == "/support" ) && request.startsWith("GET"))
    {
        path = "/index.html";
    }
    // 使用绝对路径返回文件
  //  QString basePath = "E:/qtpro/MuiltiControlSer/www/";

    QDir currentDir(QDir::currentPath());
    QString basePath = currentDir.filePath("www/");

    QString filePath = basePath + path.mid(1);  // 去掉路径中的斜杠
    qDebug() << "Requested file: " << filePath;

    // 处理请求资源
    if ( QFile::exists(filePath)) {
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
}
void HttpServer::sendHttpResponse(QTcpSocket *clientSocket,
                                  int statusCode,
                                  const QString &statusText,
                                  const QByteArray &body)
{
    QString response;
    response += QString("HTTP/1.1 %1 %2\r\n").arg(statusCode).arg(statusText);
    response += "Content-Type: text/plain; charset=utf-8\r\n";
    response += QString("Content-Length: %1\r\n").arg(body.size());
    response += "Connection: close\r\n";
    response += "\r\n";

    clientSocket->write(response.toUtf8());
    clientSocket->write(body);
    clientSocket->disconnectFromHost();
}


void HttpServer::handleGetDownload(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    QString filename = query.queryItemValue("filename");

    // 1. 检查文件名是否为空
    if (filename.isEmpty()) {
        sendHttpResponse(clientSocket, 400, "Bad Request", "Missing filename parameter");
        return;
    }

    // 2. 安全检查：防止目录遍历攻击
    QFileInfo fileInfo(filename);
    QString baseName = fileInfo.fileName(); // 只获取文件名，去除路径
    if (baseName != filename) {
        // 如果用户尝试使用路径，拒绝请求
        sendHttpResponse(clientSocket, 403, "Forbidden", "Filename cannot contain path");
        return;
    }

    // 3. 防止特殊文件访问
    if (baseName == ".." || baseName == "." || baseName.contains("/") || baseName.contains("\\")) {
        sendHttpResponse(clientSocket, 403, "Forbidden", "Invalid filename");
        return;
    }

    // 4. 构建完整的文件路径
    QDir downloadDir(QDir::current().filePath("Download"));

    // 确保Download目录存在
    if (!downloadDir.exists()) {
        sendHttpResponse(clientSocket, 404, "Not Found", "Download directory not found");
        qDebug() <<"Download path  = " << downloadDir;
        return;
    }

    QString filePath = downloadDir.filePath(baseName);
    QFile file(filePath);

    // 5. 检查文件是否存在且可读
    if (!file.exists()) {
        sendHttpResponse(clientSocket, 404, "Not Found",
                         QString("File '%1' not found").arg(baseName).toUtf8());
        QStringList files = downloadDir.entryList(QDir::Files);
        for (const QString &filename : files) {
            qDebug() << "  -" << filename;
        }
        return;
    }

    if (!file.open(QIODevice::ReadOnly)) {
        sendHttpResponse(clientSocket, 403, "Forbidden",
                         QString("Cannot open file '%1'").arg(baseName).toUtf8());
        return;
    }

    // 6. 获取文件信息
    QFileInfo info(file);
    qint64 fileSize = info.size();
    QDateTime lastModified = info.lastModified();

    // 7. 获取MIME类型
    QMimeDatabase mimeDb;
    QMimeType mimeType = mimeDb.mimeTypeForFile(filePath);
    QString contentType = mimeType.name();

    // 8. 构建HTTP响应头
    QString headers;
    headers += "HTTP/1.1 200 OK\r\n";
    headers += QString("Content-Type: %1\r\n").arg(contentType);
    headers += QString("Content-Length: %1\r\n").arg(fileSize);
    headers += QString("Content-Disposition: attachment; filename=\"%1\"\r\n").arg(baseName);
    headers += QString("Last-Modified: %1\r\n")
                   .arg(lastModified.toString("ddd, dd MMM yyyy HH:mm:ss 'GMT'"));
    headers += "Accept-Ranges: bytes\r\n";
    headers += "Cache-Control: no-cache\r\n";
    headers += "Connection: close\r\n";
    headers += "\r\n"; // 空行分隔头部和正文

    // 9. 发送响应头
    clientSocket->write(headers.toUtf8());

    // 10. 分块发送文件内容（避免内存占用过大）
    const qint64 CHUNK_SIZE = 64 * 1024; // 64KB
    qint64 bytesRemaining = fileSize;
    char buffer[CHUNK_SIZE];

    while (bytesRemaining > 0 && clientSocket->state() == QAbstractSocket::ConnectedState) {
        qint64 bytesToRead = qMin(CHUNK_SIZE, bytesRemaining);
        qint64 bytesRead = file.read(buffer, bytesToRead);

        if (bytesRead <= 0) {
            break; // 读取错误或文件结束
        }

        qint64 bytesWritten = clientSocket->write(buffer, bytesRead);

        // 确保数据完全发送
        if (bytesWritten > 0) {
            clientSocket->flush();
        }

        if (bytesWritten != bytesRead) {
            // 写入失败，可能客户端断开连接
            break;
        }

        bytesRemaining -= bytesRead;
    }

    file.close();

    // 11. 关闭连接
    clientSocket->disconnectFromHost();
}

QJsonObject HttpServer::parseJsonData(const QString &jsonString) {
    // 解析 JSON 字符串
    QJsonDocument doc = QJsonDocument::fromJson(jsonString.toUtf8());

    if (doc.isNull()) {
        qDebug() << "Invalid JSON format!";
        return QJsonObject();
    }

    // 获取整个 JSON 对象
    QJsonObject jsonObject = doc.object();

    // 获取 "data" 部分并返回
    if (jsonObject.contains("data")) {
        return jsonObject.value("data").toObject();
    } else {
        qDebug() << "No 'data' key found!";
        return QJsonObject();
    }
}

void HttpServer::onReadyRead() {
    // 获取当前连接的客户端
    QTcpSocket *clientSocket = qobject_cast<QTcpSocket *>(sender());

    if (clientSocket) {
        // 读取请求
        QByteArray request = clientSocket->readAll();

        // 解析请求行
        QList<QByteArray> lines = request.split('\n');
        if (lines.isEmpty()) return;

        QByteArray requestLine = lines[0];
        QList<QByteArray> parts = requestLine.split(' ');
        if (parts.size() < 3) return;

        QByteArray method = parts[0];
        QByteArray fullPath = parts[1];

        // 分离 path 和 query
        QString path = QString(fullPath).split('?').first();
        QUrlQuery query((QUrl(fullPath)));

        // 提取 BODY
        QByteArray body;
        int emptyLineIndex = request.indexOf("\r\n\r\n");
        if (emptyLineIndex != -1) {
            body = request.mid(emptyLineIndex + 4);
        }

        // 解析头部到map
        QMap<QString, QString> headers;
        for (int i = 1; i < lines.size(); i++) {
            QByteArray line = lines[i].trimmed();
            if (line.isEmpty()) break; // 头部结束

            int colonPos = line.indexOf(':');
            if (colonPos > 0) {
                QString key = QString::fromUtf8(line.left(colonPos).trimmed());
                QString value = QString::fromUtf8(line.mid(colonPos + 1).trimmed());
                headers[key.toLower()] = value;
            }
        }

        qDebug() << "GET REQ PATH: " << path << "GET METHOD: " << method;

        if (method == "GET") {
            if (path == "/device") {
                handleGetDevice(clientSocket, query);
            } else if (path == "/mall/login/para") {
                handleGetLoginUI(clientSocket, query);
            }else if (path.contains( "/images")) {
                handleBGimagesGet(clientSocket, query);
            }else if (path == "/process/get") {
                handleGetProcess(clientSocket, query);
            } else if (path == "/download") {
                handleGetDownload(clientSocket, query);
            } else if (path == "/command/history") {
                handleGetCommandList(clientSocket, query);
            } else if (path == "/order/dispose/list") {
                handleGetOrderList(clientSocket, query);
            }else if (path == "/home" || path.contains(".css") /*|| path.contains(".jpg")*/ || path.contains("/login") \
                       || path.contains(".js") /*|| path.contains(".png")*/ || path.contains(".html") \
                       || path.contains("/devices") || path.contains("/process/new") || path.contains("/process/center") \
                       || path.contains("/support") || path.contains("/vite.svg") || path.contains("/favicon.ico")) {
                ShowHomepage(clientSocket, request);
            } else {
                sendNotFound(clientSocket);
            }
        } else if (method == "POST") {
            // 获取Content-Length
            qint64 contentLength = -1;
            if (headers.contains("content-length")) {
                bool ok;
                contentLength = headers["content-length"].toLongLong(&ok);
                if (!ok) contentLength = -1;
            }

            qDebug() << "Content-Length from headers:" << contentLength;

            // 如果是/dev/upload请求，确保读取完整body
            if (path == "/dev/upload") {
                // 如果已经接收的body不完整，继续读取
                if (contentLength > 0 && body.size() < contentLength) {
                    qDebug() << "Need to read more data. Current:" << body.size()
                    << "Expected:" << contentLength;

                    // 设置读取超时
                    int timeout = 5000; // 5秒
                    qint64 remaining = contentLength - body.size();

                    while (remaining > 0 && clientSocket->waitForReadyRead(timeout)) {
                        QByteArray moreData = clientSocket->readAll();
                        if (!moreData.isEmpty()) {
                            body.append(moreData);
                            remaining = contentLength - body.size();
                            // qDebug() << "Read additional" << moreData.size() << "bytes. Total:"
                            //          << body.size() << "Remaining:" << remaining;
                        } else {
                            break;
                        }
                    }

                    if (body.size() != contentLength) {
                        qDebug() << "Warning: Body incomplete! Expected:" << contentLength
                                 << "Actual:" << body.size();
                    }
                }

                qDebug() << "Before calling handlePostFileUpload, body size:" << body.size();
                handlePostFileUpload(clientSocket, query, body, "123654");
                qDebug() << "After handlePostFileUpload";

            } else {
                // 其他POST请求的处理
                if (path == "/device/command") {
                    handlePostDeviceCommand(clientSocket, body);
                } else if (path == "/device/add") {
                    handlePostDeviceAdd(clientSocket, query, body);
                } else if (path == "/process/create") {
                    handlePostProcessCreate(clientSocket, body);
                } else if (path == "/process/update") {
                    handlePostProcessUpdate(clientSocket, query, body);
                } else if (path == "/process/delete") {
                    handlePostProcessDelete(clientSocket, body);
                } else if (path == "/device/warning-ignore") {
                    handlePostWarningIgnore(clientSocket, body);
                } else if (path == "/auth/login") {
                    handlePostAuthLogin(clientSocket, body);
                }else if (path == "/auth/login") {
                    handlePostAuthLogin(clientSocket, body);
                }else if (path == "/process/todev") {
                    handlePostDeviceProcess(clientSocket, body);
                }else if (path == "/mall/login/info") {
                    handlePostMallLogin(clientSocket, body);
                }else if (path == "/mall/auth/register") {
                    handlePostMallRegist(clientSocket, body);
                }else {
                    qDebug() << path << "[POST /process/create] body =" << body;
                    sendNotFound(clientSocket);
                }
            }
        } else {
            sendNotFound(clientSocket);
        }

        // 断开与客户端的连接（对于keep-alive连接可能需要调整）
        clientSocket->disconnectFromHost();
    }
}
void HttpServer::onDeviceUpdata(DeviceStatus updatedDevice)
{
    // Traverse through the QVector to find the device with the same serial number
    for (int i = 0; i < deviceVector.size(); ++i) {
        if (deviceVector[i].serialNumber == updatedDevice.serialNumber) {
            // If the device with the matching serial number is found, update its details

            updatedDevice.lastHeartbeat = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            updatedDevice.status ="在线";
            updatedDevice.location ="杭州";
            //  updatedDevice.Temperature=
            //  deviceVector[i] = updatedDevice;
            deviceVector[i].usedProcessID =   updatedDevice.usedProcessID;
            deviceVector[i].usedProcess =   updatedDevice.usedProcess;
            deviceVector[i].hardversion =   updatedDevice.hardversion;
            deviceVector[i].current_end =   updatedDevice.current_end;
            deviceVector[i].current_start =   updatedDevice.current_start;
            deviceVector[i].ip =   updatedDevice.ip;
            deviceVector[i].lastHeartbeat =   updatedDevice.lastHeartbeat;
            deviceVector[i].trafficStatistics =   updatedDevice.trafficStatistics;
            deviceVector[i].checksum =   updatedDevice.checksum;
            deviceVector[i].Temperature =   updatedDevice.Temperature;
            deviceVector[i].status =   updatedDevice.status;

            //  qDebug() << "Device information updated for serial number: " << updatedDevice.serialNumber;
            return; // Exit after updating the device
        }
    }

    // If no device is found with the given serial number, you can optionally log it
    //   qDebug() << "Device with serial number " << updatedDevice.serialNumber << " not found!";
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
void HttpServer::handlePostAuthLogin(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "[POST /auth/login] body =" << body;

    QString qjson = QString(body);
    // 直接打印 body 数据
    qDebug() << "Raw JSON data:" << qjson;
    // QByteArray response;

    QJsonObject  result = parseJsonData(qjson);
    int code=400;
    QJsonObject  jsonObject;
    if(!result.isEmpty())
    {
        QString username = result.value("username").toString();
        QString password = result.value("password").toString();
        qDebug() << "Username:" << username;
        qDebug() << "Password:" << password;
        if(username == MANAGER_USERNAME && password == MANAGER_PASSWD)
        {
            code=200;
        }
        jsonObject= generateJson(username,code,"951");
    }
    QJsonDocument doc(jsonObject);
    QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    qDebug() << jsonData;

    sendResponse(clientSocket, jsonData);
}
void HttpServer::handlePostWarningIgnore(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "[POST /auth/login] body =" << body;

    QString qjson = QString(body);
    // 直接打印 body 数据
    qDebug() << "Raw JSON data:" << qjson;
    // QByteArray response;

    QJsonObject  result = parseJsonData(qjson);
    QJsonObject  jsonObject;
    if(!result.isEmpty())
    {
        QJsonObject rootObj = result;

        // 获取devices数组
        if (rootObj.contains("devices") && rootObj["devices"].isArray()) {
            QJsonArray devicesArray = rootObj["devices"].toArray();

            // 遍历数组
            for (int i = 0; i < devicesArray.size(); ++i) {
                QJsonObject deviceObj = devicesArray[i].toObject();

                // 获取serial_number
                QString serialNumber = deviceObj["serial_number"].toString();
                DeviceStatus* foundDevice = findDeviceBySerialNumber(deviceVector, serialNumber);
                if(nullptr != foundDevice)
                {
                    foundDevice->warining_ignore=true;
                    //    foundDevice->
                    qDebug() << "Device" << i << "serial number:" << serialNumber << "set ignore warning :" <<foundDevice->warining_ignore;

                }
                else
                {
                    qDebug() << "Device" << i << "serial number:" << serialNumber << "not found ";

                }
            }
        }
    }
    QByteArray json = "{\"code\":200,\"message\":\"ok\"}";
    sendResponse(clientSocket, json);
}
void HttpServer::handlePostFileUpload(QTcpSocket *clientSocket, QUrlQuery query, const QByteArray &body, QString verify)
{
    qDebug() << "=== handlePostFileUpload called ===";
    qDebug() << "Body size received:" << body.size() << "bytes";

    QString serial = query.queryItemValue("serial_number");
    QString verifycode = query.queryItemValue("verifycode");
    QString originalFilename = query.queryItemValue("filename");

    qDebug() << "Parameters - serial:" << serial
             << "verifycode:" << verifycode
             << "originalFilename:" << originalFilename;

    // 校验验证码
    if (verifycode != verify) {
        qDebug() << "Verifycode mismatch! Expected:" << verify << "Got:" << verifycode;
        sendHttpResponse(clientSocket, 403, "Forbidden", "Invalid verifycode");
        return;
    }

    if (serial.isEmpty()) {
        qDebug() << "Missing serial_number parameter";
        sendHttpResponse(clientSocket, 400, "Bad Request", "Missing serial_number");
        return;
    }

    // 创建目录
    QDir currentDir = QDir::current();
    QString uploadDirPath = currentDir.filePath("Upload");
    QString serialDirPath = QDir(uploadDirPath).filePath(serial);

    qDebug() << "Creating directories - Upload:" << uploadDirPath
             << "Serial:" << serialDirPath;

    if (!QDir(uploadDirPath).exists()) {
        if (QDir().mkpath(uploadDirPath)) {
            qDebug() << "Created Upload directory";
        } else {
            qDebug() << "Failed to create Upload directory";
            sendHttpResponse(clientSocket, 500, "Internal Error", "Cannot create Upload directory");
            return;
        }
    }

    if (!QDir(serialDirPath).exists()) {
        if (QDir().mkpath(serialDirPath)) {
            qDebug() << "Created serial directory";
        } else {
            qDebug() << "Failed to create serial directory";
            sendHttpResponse(clientSocket, 500, "Internal Error", "Cannot create serial directory");
            return;
        }
    }

    // 生成最终文件名
    QString finalFilename;
    if (!originalFilename.isEmpty()) {
        finalFilename = originalFilename;
        qDebug() << "Using original filename:" << finalFilename;
    } else {
        finalFilename = QString("%1_%2.dat")
        .arg(serial)
            .arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss"));
        qDebug() << "Generated filename:" << finalFilename;
    }

    // 保存文件
    QString filePath = QDir(serialDirPath).filePath(finalFilename);
    qDebug() << "Saving to path:" << filePath;

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing:" << file.errorString();
        sendHttpResponse(clientSocket, 500, "Internal Error",
                         QString("Cannot save file: %1").arg(file.errorString()).toUtf8());
        clientSocket->disconnectFromHost();
        return;
    }

    qDebug() << "File opened successfully, writing" << body.size() << "bytes...";
    qint64 bytesWritten = file.write(body);
    file.close();

    qDebug() << "Bytes written:" << bytesWritten << "/" << body.size();

    if (bytesWritten != body.size()) {
        qDebug() << "Warning: Incomplete write! Expected:" << body.size()
        << "Actual:" << bytesWritten;
    }

    // 验证文件大小
    QFileInfo fileInfo(filePath);
    qint64 actualFileSize = fileInfo.size();
    qDebug() << "Actual file size on disk:" << actualFileSize << "bytes";

    // 返回JSON响应
    QJsonObject json;
    json["success"] = true;
    json["message"] = "File uploaded successfully";
    json["serial"] = serial;
    json["filename"] = finalFilename;
    json["requested_size"] = static_cast<qint64>(body.size());
    json["written_size"] = bytesWritten;
    json["actual_size"] = actualFileSize;
    json["path"] = filePath;
    json["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);

    QJsonDocument doc(json);
    QByteArray jsonData = doc.toJson();

    qDebug() << "Sending JSON response:" << QString::fromUtf8(jsonData);

    QString response = QString("HTTP/1.1 200 OK\r\n"
                               "Content-Type: application/json\r\n"
                               "Content-Length: %1\r\n"
                               "Connection: close\r\n\r\n")
                           .arg(jsonData.size());

    qDebug() << "Writing HTTP response header...";

    if (clientSocket->write(response.toUtf8()) == -1) {
        qDebug() << "Failed to write response header:" << clientSocket->errorString();
    } else {
        clientSocket->flush();
        qDebug() << "Response header written, writing JSON data...";

        if (clientSocket->write(jsonData) == -1) {
            qDebug() << "Failed to write JSON data:" << clientSocket->errorString();
        } else {
            clientSocket->flush();
            qDebug() << "JSON data written successfully";
        }
    }

    qDebug() << "Disconnecting client socket...";
    clientSocket->disconnectFromHost();

    if (clientSocket->state() != QAbstractSocket::UnconnectedState) {
        qDebug() << "Waiting for socket to disconnect...";
        if (clientSocket->waitForDisconnected(3000)) {
            qDebug() << "Socket disconnected successfully";
        } else {
            qDebug() << "Socket disconnect timeout";
        }
    }

    qDebug() << "=== handlePostFileUpload finished ===";
    qDebug() << "";
}
// 辅助函数：获取HTTP头部值
QString HttpServer::getHeaderValue(QTcpSocket *clientSocket, const QString &headerName)
{
    // 这里需要从clientSocket的请求数据中解析头部
    // 假设你已经有一个存储请求头的数据结构
    // 或者可以从原始的请求数据中解析
    return "";
}
QByteArray HttpServer::getContentType(QString &filePath)
{
    QFileInfo fileInfo(filePath);
    QString extension = fileInfo.suffix().toLower();

    if (extension == "html") {
        return "text/html";
    } else if (extension == "css") {
        return "text/css";
    } else if (extension == "js") {
        return "application/javascript";
    } else if (extension == "jpg" || extension == "jpeg") {
        return "image/jpeg";
    } else if (extension == "png") {
        return "image/png";
    }
    return "application/octet-stream";  // 默认二进制类型
}

void HttpServer::printStaticFiles(const QByteArray &htmlContent) {
    // 使用正则表达式提取 CSS, JS, 图片等资源文件
    QRegularExpression cssRegex(R"(<link\s+[^>]*href=["']([^"']+\.css)["'][^>]*>)");
    QRegularExpression jsRegex(R"(<script\s+[^>]*src=["']([^"']+\.js)["'][^>]*>)");
    QRegularExpression imgRegex(R"(<img\s+[^>]*src=["']([^"']+\.(jpg|jpeg|png|gif))["'][^>]*>)");

    // 查找所有 CSS 文件
    QRegularExpressionMatchIterator cssIterator = cssRegex.globalMatch(htmlContent);
    while (cssIterator.hasNext()) {
        QRegularExpressionMatch match = cssIterator.next();
        QString cssFile = match.captured(1);
        qDebug() << "Found CSS file:" << cssFile;
    }

    // 查找所有 JS 文件
    QRegularExpressionMatchIterator jsIterator = jsRegex.globalMatch(htmlContent);
    while (jsIterator.hasNext()) {
        QRegularExpressionMatch match = jsIterator.next();
        QString jsFile = match.captured(1);
        qDebug() << "Found JS file:" << jsFile;
    }

    // 查找所有图片文件
    QRegularExpressionMatchIterator imgIterator = imgRegex.globalMatch(htmlContent);
    while (imgIterator.hasNext()) {
        QRegularExpressionMatch match = imgIterator.next();
        QString imgFile = match.captured(1);
        qDebug() << "Found image file:" << imgFile;
    }
}
void HttpServer::sendResponse(QTcpSocket *clientSocket, const QByteArray &json) {
    QByteArray responseHeader = "HTTP/1.1 200 OK\r\n";
    responseHeader += "Content-Type: application/json\r\n";
    responseHeader += "Content-Length: " + QByteArray::number(json.size()) + "\r\n";
    responseHeader += "Connection: close\r\n\r\n";

    clientSocket->write(responseHeader);
    clientSocket->write(json);
    clientSocket->flush();
    clientSocket->disconnectFromHost();
}

void HttpServer::sendNotFound(QTcpSocket *clientSocket) {
    QByteArray json = "{\"code\":404,\"message\":\"Not Found\"}";
    sendResponse(clientSocket, json);
}

void HttpServer::handleGetLoginUI(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    Q_UNUSED(query); // 这个接口可能不需要查询参数

    // 构建JSON响应
    QJsonObject jsonResponse;

    // 时间戳
    jsonResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 构建data对象
    QJsonObject dataObj;
    dataObj["backgroundpic"] = LOGIN_BACKGROUND_PIC;  // 使用宏
    dataObj["guide"] = LOGIN_GUIDE_TEXT;              // 使用宏
    dataObj["Slogan1"] = LOGIN_SLOGAN1;               // 使用宏
    dataObj["Slogan2"] = LOGIN_SLOGAN2;               // 使用宏

    jsonResponse["data"] = dataObj;

    // 转换为JSON字符串
    QJsonDocument jsonDoc(jsonResponse);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    // 发送响应
    sendResponse(clientSocket, jsonData);
}

void HttpServer::handleBGimagesGet(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    // 从查询参数获取文件名
    QString fileName = query.queryItemValue("filename");

    qDebug() << "Image request - filename:" << fileName;

    if (fileName.isEmpty()) {
        qDebug() << "Error: No filename specified";
        sendResponse(clientSocket, "HTTP/1.1 400 Bad Request\r\n\r\nNo filename specified");
        return;
    }

    // 安全检查：防止目录遍历攻击
    if (fileName.contains("..") || fileName.contains("/") || fileName.contains("\\")) {
        qDebug() << "Security check failed: Invalid filename" << fileName;
        sendResponse(clientSocket, "HTTP/1.1 403 Forbidden\r\n\r\nInvalid filename");
        return;
    }

    // 只允许图片文件扩展名
    QString extension = QFileInfo(fileName).suffix().toLower();
    QStringList allowedExtensions = {"jpg", "jpeg", "png", "gif", "bmp", "svg", "webp", "ico"};

    if (!allowedExtensions.contains(extension)) {
        qDebug() << "Security check failed: Invalid file extension" << extension;
        sendResponse(clientSocket, "HTTP/1.1 403 Forbidden\r\n\r\nInvalid file extension");
        return;
    }

    // 在当前应用程序目录的images子目录中查找文件
    // QString appDir = QCoreApplication::applicationDirPath();
    // QString imagePath = appDir + "/images/" + fileName;
    QDir currentDir(QDir::currentPath());
    QString imagePath = currentDir.filePath("images/"+fileName);
    qDebug() << "Looking for image at:" << imagePath;

    QFileInfo fileInfo(imagePath);
    if (!fileInfo.exists() || !fileInfo.isFile()) {
        qDebug() << "Image not found:" << imagePath;
        QString response = QString("HTTP/1.1 404 Not Found\r\n"
                                   "Content-Type: text/html\r\n\r\n"
                                   "<h1>404 Not Found</h1>"
                                   "<p>Image '%1' not found in images directory.</p>")
                               .arg(fileName);
        clientSocket->write(response.toUtf8());
        return;
    }

    // 读取文件
    QFile file(imagePath);
    if (!file.open(QIODevice::ReadOnly)) {
        qDebug() << "Cannot open file:" << imagePath << file.errorString();
        sendResponse(clientSocket, "HTTP/1.1 500 Internal Server Error\r\n\r\nCannot open file");
        return;
    }

    QByteArray fileData = file.readAll();
    file.close();

    // 确定MIME类型
    QString mimeType = "application/octet-stream";
    if (extension == "jpg" || extension == "jpeg") mimeType = "image/jpeg";
    else if (extension == "png") mimeType = "image/png";
    else if (extension == "gif") mimeType = "image/gif";
    else if (extension == "bmp") mimeType = "image/bmp";
    else if (extension == "svg") mimeType = "image/svg+xml";
    else if (extension == "webp") mimeType = "image/webp";
    else if (extension == "ico") mimeType = "image/x-icon";

    // 发送响应
    QString header = QString("HTTP/1.1 200 OK\r\n"
                             "Content-Type: %1\r\n"
                             "Content-Length: %2\r\n"
                             "Connection: close\r\n\r\n")
                         .arg(mimeType)
                         .arg(fileData.size());

    clientSocket->write(header.toUtf8());
    clientSocket->write(fileData);
    clientSocket->flush();

    qDebug() << "Image served successfully:" << fileName << "Size:" << fileData.size() << "bytes";
}
// ====== 处理接口 ======
void HttpServer::handleGetDevice(QTcpSocket *clientSocket, const QUrlQuery &query) {
    QString serial = query.queryItemValue("serial_number");
    //   qDebug() << "[GET /device] serial_number =" << serial;
    QByteArray jsonData;

    if(serial =="ALL")
    {
        QJsonArray devices;
        for (const DeviceStatus& device : deviceVector)
        {
            devices.append(device.toJsonAll());
        }
        // 调用接口生成响应
        QJsonObject response = generateDeviceResponse(devices);
        // 将 JSON 对象转化为格式化的 JSON 字符串
        QJsonDocument doc(response);
        jsonData = doc.toJson(QJsonDocument::Indented);
    }
    else if(serial =="WARNING")
    {
        QJsonArray devices;
        for ( DeviceStatus& device : deviceVector)
        {
            if(device.Temperature > 30 && !device.warining_ignore)
            {
                device.warningmsg = QString("设备高温，当前温度：%1").arg(device.Temperature);
                devices.append(device.toJsonWar());
                qDebug()<< "flag:" << device.warining_ignore;
            }
            else if(0)
            {
                device.warningmsg = QString("未知的新设备").arg(device.Temperature);
                devices.append(device.toJsonWar());
            }
            else
            {
                continue;
            }
        }
        // 调用接口生成响应
        QJsonObject response = generateDeviceResponse(devices);
        // 将 JSON 对象转化为格式化的 JSON 字符串
        QJsonDocument doc(response);
        jsonData = doc.toJson(QJsonDocument::Indented);
    }
    else if(!serial.isEmpty())
    {
        // QString targetSerialNumber = "SN999321";
        DeviceStatus* foundDevice = findDeviceBySerialNumber(deviceVector, serial);
        if(nullptr != foundDevice)
        {

            QJsonObject jsonResponse = generateJsonHearResponse(foundDevice->toJsonSingle());

            QJsonDocument doc(jsonResponse);
            jsonData =   doc.toJson(QJsonDocument::Indented);
        }
        else
        {

            QJsonObject response = generateFailureResponse();
            QJsonDocument doc(response);
            jsonData = doc.toJson(QJsonDocument::Indented);
        }
    }
    else
    {
        QJsonObject response = generateFailureResponse();
        QJsonDocument doc(response);
        jsonData = doc.toJson(QJsonDocument::Indented);
    }
    // QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    // qDebug() << jsonData;
    sendResponse(clientSocket, jsonData);
}
void HttpServer::extracted(QString &statusFilter, QString &userFilter,
                           QList<SQL_Order> &orders,
                           QList<SQL_Order> &allOrders) {
    for (const SQL_Order &order : allOrders) {
        if (order.status == statusFilter && order.user == userFilter) {
            orders.append(order);
        }
    }
}
void HttpServer::handleGetOrderList(QTcpSocket *clientSocket,
                                    const QUrlQuery &query) {
    // 打开数据库

    // 解析查询参数
    int limit = 1000; // 默认最大1000条
    int offset = 0;
    QString statusFilter;
    QString userFilter;

    QString limitStr = query.queryItemValue("limit");
    QString offsetStr = query.queryItemValue("offset");
    statusFilter = query.queryItemValue("status");
    userFilter = query.queryItemValue("user");

    // 处理分页参数
    if (!limitStr.isEmpty()) {
        bool ok;
        int requestedLimit = limitStr.toInt(&ok);
        if (ok && requestedLimit > 0 && requestedLimit <= 1000) {
            limit = requestedLimit;
        } else if (requestedLimit > 1000) {
            limit = 1000;
        }
    }

    if (!offsetStr.isEmpty()) {
        bool ok;
        offset = offsetStr.toInt(&ok);
        if (!ok || offset < 0)
            offset = 0;
    }

    // 获取订单数据
    QList<SQL_Order> orders;

    // 根据过滤条件获取数据
    if (!statusFilter.isEmpty() && !userFilter.isEmpty()) {
        // 先获取所有数据，然后在代码中过滤
        QList<SQL_Order> allOrders = dbManager->getAllOrders();
        extracted(statusFilter, userFilter, orders, allOrders);
    } else if (!statusFilter.isEmpty()) {
        orders = dbManager->getOrdersByStatus(statusFilter);
    } else if (!userFilter.isEmpty()) {
        orders = dbManager->getOrdersByUser(userFilter);
    } else {
        orders = dbManager->getAllOrders();
    }

    // 应用分页（限制最大返回数量）
    int totalCount = orders.size();
    int startIdx = qMin(offset, totalCount);
    int endIdx = qMin(offset + limit, totalCount);

    QList<SQL_Order> pagedOrders;
    for (int i = startIdx; i < endIdx; i++) {
        pagedOrders.append(orders[i]);
    }

    QString action = "action";        // $1
    QString subAction = "sub_action"; // $2
    QString startTime = "start_time"; // $3
    QString endTime = "end_time";     // $4


    // 构建JSON响应
    QJsonObject jsonResponse;
    jsonResponse["timestamp"] =
        QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonArray dataArray;

    for (const SQL_Order &order : pagedOrders) {
        // 获取产品名称（$5）
        QString productName = "todo";
        // 获取显示状态
        QString displayStatus = "待验证";

        // 获取验证人
        QString verifier = "todo";

        // 构建content文本（单行格式）
        QString content = QString("订单详情：商品名称：%1，下单单价：%"
                                  "2元，下单总数：%3，客户备注：%4")
                              .arg(productName) // $5
                              .arg(QString::number(order.unitPrice, 'f', 2))
                              .arg(order.quantity)
                              .arg(order.note.isEmpty() ? "无" : order.note);

        // 将createTime转换为ISO格式
        QString isoOrderTime = order.createTime;
        QDateTime createTime =
            QDateTime::fromString(order.createTime, "yyyy-MM-dd HH:mm:ss");
        if (createTime.isValid()) {
            isoOrderTime = createTime.toString(Qt::ISODate);
        } else {
            // 尝试其他常见格式
            createTime =
                QDateTime::fromString(order.createTime, "yyyy/MM/dd HH:mm:ss");
            if (createTime.isValid()) {
                isoOrderTime = createTime.toString(Qt::ISODate);
            }
            // 如果都不行，保持原样
        }

        // 构建订单对象
        QJsonObject orderObj;
        orderObj["orderId"] = order.orderId;
        orderObj["content"] = content;
        orderObj["user"] = order.user;
        orderObj["orderTime"] = isoOrderTime;
        orderObj["status"] = displayStatus;

        // 只有当有验证人时才添加verifier字段（匹配D00005的示例）
        if (!verifier.isEmpty()) {
            orderObj["verifier"] = verifier;
        }

        // 构建defaultParams对象
        QJsonObject defaultParams;
        defaultParams["action"] = action;          // $1
        defaultParams["subAction"] = subAction;    // $2
        defaultParams["devices"] = order.quantity; // devices = quantity
        defaultParams["startTime"] = startTime;    // $3
        defaultParams["endTime"] = endTime;        // $4
        defaultParams["remark"] = order.note;      // remark = note

        orderObj["defaultParams"] = defaultParams;

        dataArray.append(orderObj);
    }

    jsonResponse["data"] = dataArray;

    // 发送响应
    QJsonDocument jsonDoc(jsonResponse);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    sendResponse(clientSocket, jsonData);
}
void HttpServer::handleGetCommandList(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    // 1. 从数据库获取指令列表
    QList<SQL_CommandHistory> commands;

    // 检查是否有分页参数
    int limit = 1000; // 默认最大1000条
    int offset = 0;

    QString limitStr = query.queryItemValue("limit");
    QString offsetStr = query.queryItemValue("offset");

    if (!limitStr.isEmpty()) {
        bool ok;
        int requestedLimit = limitStr.toInt(&ok);
        if (ok && requestedLimit > 0 && requestedLimit <= 1000) {
            limit = requestedLimit;
        } else if (requestedLimit > 1000) {
            limit = 1000; // 强制限制最大1000条
        }
    }

    if (!offsetStr.isEmpty()) {
        bool ok;
        offset = offsetStr.toInt(&ok);
        if (!ok || offset < 0) {
            offset = 0;
        }
    }

    // 检查是否有过滤参数
    QString statusFilter = query.queryItemValue("status");
    QString actionFilter = query.queryItemValue("action");

    // 2. 获取数据（这里需要实现数据库查询逻辑）
    // 根据过滤条件获取数据
    if (!statusFilter.isEmpty()) {
        commands = dbManager->getCommandsByStatus(statusFilter);
    } else if (!actionFilter.isEmpty()) {
        commands = dbManager->getCommandsByAction(actionFilter);
    } else {
        // 获取所有指令，然后进行分页
        QList<SQL_CommandHistory> allCommands = dbManager->getAllCommands();

        // 实现分页
        int start = qMin(offset, allCommands.size());
        int end = qMin(offset + limit, allCommands.size());

        for (int i = start; i < end; i++) {
            commands.append(allCommands[i]);
        }
    }

    // 如果命令太多，限制为limit
    if (commands.size() > limit) {
        commands = commands.mid(0, limit);
    }

    // 3. 构建JSON响应
    QJsonObject jsonResponse;

    // 添加时间戳（ISO 8601格式，UTC）
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODateWithMs);
    jsonResponse["timestamp"] = timestamp;

    // 构建data数组
    QJsonArray dataArray;

    for (const SQL_CommandHistory &cmd : commands) {
        QJsonObject cmdObj;
        cmdObj["commandId"] = cmd.commandId;
        cmdObj["status"] = cmd.status;
        cmdObj["action"] = cmd.action;
        cmdObj["sub_action"] = cmd.sub_action;
        cmdObj["start_time"] = cmd.start_time;
        cmdObj["end_time"] = cmd.end_time;
        cmdObj["remark"] = cmd.remark;
        cmdObj["Completeness"] = cmd.Completeness;
        cmdObj["completed-url"] = cmd.completed_url;

        dataArray.append(cmdObj);
    }

    jsonResponse["data"] = dataArray;

    // 添加分页信息
    QJsonObject pagination;
    pagination["total"] = dbManager->getAllCommands().size(); // 总记录数
    pagination["limit"] = limit;
    pagination["offset"] = offset;
    pagination["returned"] = commands.size();
    jsonResponse["pagination"] = pagination;

    // 4. 将JSON转换为字节数组
    QJsonDocument jsonDoc(jsonResponse);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    // 5. 发送响应
    sendResponse(clientSocket, jsonData);
}

void HttpServer::handleGetProcess(QTcpSocket *clientSocket, const QUrlQuery &query) {
    QString processId = query.queryItemValue("process_id");
    //    qDebug() << "[GET /process/get] process_id =" << processId;
    QByteArray jsonData;
    if(processId == "ALL" || processId == "all")
    {
        QJsonArray data;
        for (const Machine_Process_Total& process : processVector)
        {
            data.append(process.toJsonAll());
        }
        // 调用接口生成响应
        QJsonObject response = generateProcessResponse(data);
        // 将 JSON 对象转化为格式化的 JSON 字符串
        QJsonDocument doc(response);
        jsonData = doc.toJson(QJsonDocument::Indented);
    }
    else if(!processId.isEmpty())
    {
        Machine_Process_Total* foundProcess = findProcessById(processVector, processId);
        if(nullptr != foundProcess)
        {
            QJsonObject data;


            // 将 Processes 转为 JSON 数组
            QJsonArray stepsArray;

            for (const Machine_Process_Single& process : foundProcess->Processes)
            {
                stepsArray.append(process.toJsonAll());
            }

            data["process_id"] = processId;
            data["steps"] = stepsArray;
            // 调用接口生成响应
            QJsonObject response = generateProcessSingleResponse(data);
            // 将 JSON 对象转化为格式化的 JSON 字符串
            QJsonDocument doc(response);
            jsonData = doc.toJson(QJsonDocument::Indented);
        }
        else
        {
            qDebug() << "havent found!" ;
            QJsonObject response = generateFailureResponse();
            QJsonDocument doc(response);
            jsonData = doc.toJson(QJsonDocument::Indented);
        }
    }
    else
    {
        QJsonObject response = generateFailureResponse();
        QJsonDocument doc(response);
        jsonData = doc.toJson(QJsonDocument::Indented);
    }
    // QByteArray jsonData = doc.toJson(QJsonDocument::Indented);
    // qDebug() << jsonData;
    sendResponse(clientSocket, jsonData);
}
void HttpServer::handlePostMallRegist(QTcpSocket *clientSocket, const QByteArray &body)
{
    // 解析JSON请求体
    QJsonDocument jsonDoc = QJsonDocument::fromJson(body);
    if (jsonDoc.isNull() || !jsonDoc.isObject()) {
        // 返回错误响应 - JSON格式错误
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject dataObj;
        dataObj["success"] = false;
        dataObj["message"] = "Invalid JSON format";
        errorResp["data"] = dataObj;

        sendHttpResponse(clientSocket, 400, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    QJsonObject jsonObj = jsonDoc.object();

    // 检查是否有data字段
    if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject dataObj;
        dataObj["success"] = false;
        dataObj["message"] = "Missing 'data' field in request";
        errorResp["data"] = dataObj;

        sendHttpResponse(clientSocket, 400, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    QJsonObject dataObj = jsonObj["data"].toObject();

    // 提取必填字段
    QString username = dataObj.value("username").toString();
    QString password = dataObj.value("password").toString();
    QString email = dataObj.value("email").toString();
    QString inviteCode = dataObj.value("inviteCode").toString();

    // 提取可选字段
    QString phone = dataObj.contains("phone") ? dataObj.value("phone").toString() : "";
    QString realName = dataObj.contains("realName") ? dataObj.value("realName").toString() : "";
    QString address = dataObj.contains("address") ? dataObj.value("address").toString() : "";

    // 验证必填字段
    if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Missing required fields (username, password, email)";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 400, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 用户名验证：3-20位，只允许字母、数字、下划线
    QRegularExpression usernameRegex("^[a-zA-Z0-9_]{3,20}$");
    if (!usernameRegex.match(username).hasMatch()) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Username must be 3-20 characters and contain only letters, numbers, and underscores";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 400, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 密码强度验证：至少6位，包含字母和数字
    if (password.length() < 6) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Password must be at least 6 characters";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 400, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 邮箱格式验证
    QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
    if (!emailRegex.match(email).hasMatch()) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Invalid email format";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 400, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 如果提供了手机号，验证格式
    if (!phone.isEmpty()) {
        QRegularExpression phoneRegex(R"(^1[3-9]\d{9}$)");
        if (!phoneRegex.match(phone).hasMatch()) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 400;

            QJsonObject respData;
            respData["success"] = false;
            respData["message"] = "Invalid phone number format (must be 11 digits starting with 1)";
            errorResp["data"] = respData;

            sendHttpResponse(clientSocket, 400, "application/json",
                             QJsonDocument(errorResp).toJson());
            return;
        }
    }

    // 检查用户名是否已存在
    if (dbManager->checkMallUserExists(username)) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 409;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Username already exists";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 409, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 检查邮箱是否已存在
    if (dbManager->checkEmailExists(email)) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 409;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Email already registered";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 409, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 如果提供了手机号，检查手机号是否已存在
    if (!phone.isEmpty() && dbManager->checkPhoneExists(phone)) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 409;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Phone number already registered";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 409, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    // 邀请码相关逻辑
    QString inviterUsername = "";

    // 如果提供了邀请码，验证邀请码是否存在并找到邀请人
    if (!inviteCode.isEmpty()) {
        if (!dbManager->checkInviteCodeExists(inviteCode)) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 400;

            QJsonObject respData;
            respData["success"] = false;
            respData["message"] = "Invalid invite code";
            errorResp["data"] = respData;

            sendHttpResponse(clientSocket, 400, "application/json",
                             QJsonDocument(errorResp).toJson());
            return;
        }

        // 查找拥有该邀请码的用户
        SQL_MallUser inviterUser = dbManager->getMallUserByInviteCode(inviteCode);
        if (inviterUser.username.isEmpty()) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 400;

            QJsonObject respData;
            respData["success"] = false;
            respData["message"] = "Could not find user with the provided invite code";
            errorResp["data"] = respData;

            sendHttpResponse(clientSocket, 400, "application/json",
                             QJsonDocument(errorResp).toJson());
            return;
        }

        inviterUsername = inviterUser.username;
        qDebug() << "Found inviter user:" << inviterUsername << "for invite code:" << inviteCode;
    }

    // 为新用户生成邀请码
    QString userInviteCode = generateInviteCode();

    // 确保邀请码唯一
    int retryCount = 0;
    while (dbManager->checkInviteCodeExists(userInviteCode) && retryCount < 10) {
        userInviteCode = generateInviteCode();
        retryCount++;
        qDebug() << "Invite code collision, generating new one:" << userInviteCode;
    }

    if (retryCount >= 10) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 500;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = "Failed to generate unique invite code after multiple attempts";
        errorResp["data"] = respData;

        sendHttpResponse(clientSocket, 500, "application/json",
                         QJsonDocument(errorResp).toJson());
        return;
    }

    try {
        // 创建商城用户对象
        SQL_MallUser newUser;
        newUser.username = username;
        newUser.password = password;  // 明文保存（已按您的要求）
        newUser.email = email;
        newUser.phone = phone;
        newUser.inviteCode = userInviteCode;  // 使用生成的邀请码
        newUser.inviterUsername = inviterUsername;  // 设置邀请人账号（如果有）
        newUser.userLevel = 1;  // 默认用户等级
        newUser.balance = 0.0;  // 初始余额为0
        newUser.points = 0;     // 初始积分为0

        // 设置创建时间
        newUser.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        qDebug() << "Registering user with details:";
        qDebug() << "  Username:" << username;
        qDebug() << "  Email:" << email;
        qDebug() << "  Generated invite code:" << userInviteCode;
        qDebug() << "  Received invite code:" << inviteCode;
        qDebug() << "  Inviter username:" << inviterUsername;

        // 插入用户到数据库
        if (!dbManager->insertMallUser(newUser)) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 500;

            QJsonObject respData;
            respData["success"] = false;
            respData["message"] = "Failed to create user account";
            errorResp["data"] = respData;

            sendHttpResponse(clientSocket, 500, "application/json",
                             QJsonDocument(errorResp).toJson());
            return;
        }

        // 注册成功，返回成功响应（包含详细调试信息）
        QJsonObject successResp;
        successResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        successResp["code"] = 200;

        QJsonObject successData;
        successData["success"] = true;
        successData["message"] = "User registered successfully";
        successData["username"] = username;
        successData["userLevel"] = newUser.userLevel;
        successData["balance"] = newUser.balance;
        successData["points"] = newUser.points;
        successData["inviteCode"] = userInviteCode;  // 返回生成的邀请码
        successData["receivedInviteCode"] = inviteCode;  // 返回接收到的邀请码
        successData["inviterUsername"] = inviterUsername;  // 返回邀请人账号
        successData["registerTime"] = newUser.createTime;  // 返回注册时间

        // 调试信息
        QJsonObject debugInfo;
        debugInfo["passwordSavedAs"] = "plaintext";  // 显示密码保存方式
        debugInfo["inviteCodeGenerated"] = userInviteCode;
        debugInfo["inviteCodeReceived"] = inviteCode.isEmpty() ? "none" : inviteCode;
        debugInfo["inviterFound"] = !inviterUsername.isEmpty();
        debugInfo["inviterUsername"] = inviterUsername.isEmpty() ? "none" : inviterUsername;
        successData["debugInfo"] = debugInfo;

        successResp["data"] = successData;

        qDebug() << "User registered successfully:" << username;
        qDebug() << "Generated invite code:" << userInviteCode;
        if (!inviterUsername.isEmpty()) {
            qDebug() << "Invited by:" << inviterUsername;
        }

        sendHttpResponse(clientSocket, 200, "application/json",
                         QJsonDocument(successResp).toJson());

    } catch (const std::exception &e) {
        // 捕获并处理任何异常
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 500;

        QJsonObject respData;
        respData["success"] = false;
        respData["message"] = QString("Registration failed: %1").arg(e.what());
        errorResp["data"] = respData;

        qDebug() << "Registration exception:" << e.what();
        sendHttpResponse(clientSocket, 500, "application/json",
                         QJsonDocument(errorResp).toJson());
    }
}
void HttpServer::handlePostDeviceCommand(QTcpSocket *clientSocket, const QByteArray &body) {
    qDebug() << "[POST /device/command] body =" << body;
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isNull()) {
        QJsonObject jsonObj = doc.object();
        //  qDebug() << "Parsed JSON body:" << jsonObj;
        emit  devCommadSend(jsonObj);
    }
    sendResponse(clientSocket, "{\"code\":200,\"msg\":\"POST /process/command success\"}");
}
// 添加生成邀请码的辅助函数（需要在头文件中声明）
QString HttpServer::generateInviteCode()
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString inviteCode;
    QRandomGenerator *generator = QRandomGenerator::global();

    // 生成8位邀请码
    for (int i = 0; i < 6; i++) {
        int index = generator->bounded(chars.length());
        inviteCode.append(chars.at(index));
    }

    // 添加前缀，格式如：INV-XXXXXX
    return inviteCode;
}
void HttpServer::handlePostMallLogin(QTcpSocket *clientSocket, const QByteArray &body) {
    qDebug() << "[POST /mall/login] body =" << QString::fromUtf8(body);

    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);

    if (doc.isNull() || !doc.isObject()) {
        qDebug() << "JSON parse error:" << parseError.errorString();
        sendResponse(clientSocket, "{\"code\":400,\"msg\":\"Invalid JSON format\"}");
        return;
    }

    QJsonObject jsonObj = doc.object();

    // 检查是否有data字段
    if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
        qDebug() << "Missing data field";
        sendResponse(clientSocket, "{\"code\":400,\"msg\":\"Missing data field\"}");
        return;
    }

    QJsonObject dataObj = jsonObj["data"].toObject();

    // 获取用户名和密码
    QString username = dataObj["username"].toString();
    QString password = dataObj["password"].toString();

    qDebug() << "Login attempt - Username:" << username << "Password:" << password;

    // 检查是否为空
    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "Username or password is empty";
        sendResponse(clientSocket, "{\"code\":400,\"msg\":\"Username and password cannot be empty\"}");
        return;
    }

    // 检查dbManager是否已初始化
    if (!dbManager) {
        qDebug() << "Database manager not initialized";
        sendResponse(clientSocket, "{\"code\":500,\"msg\":\"Database error\"}");
        return;
    }

    // 打开数据库连接（如果尚未打开）
    if (!dbManager->openDatabase("your_database.db")) {
        qDebug() << "Failed to open database";
        sendResponse(clientSocket, "{\"code\":500,\"msg\":\"Database connection failed\"}");
        return;
    }



    // 验证用户登录
    bool loginSuccess = dbManager->validateMallUserLogin(username, password);

    if (!loginSuccess) {
        // 兼容旧的admin/admin方式（如果需要）
        if (username == "admin" && password == "admin") {
            loginSuccess = true;
            qDebug() << "Login successful for admin (legacy)";

            // 如果admin用户不存在于数据库，创建它
            if (!dbManager->checkMallUserExists("admin")) {
                SQL_MallUser adminUser;
                adminUser.username = "admin";
                adminUser.password = "admin";
                adminUser.email = "admin@example.com";
                adminUser.inviteCode = "ADMIN001";
                adminUser.userLevel = 10; // 管理员级别
                adminUser.balance = 1000.0;
                adminUser.points = 1000;

                dbManager->insertMallUser(adminUser);
                qDebug() << "Admin user created in database";
            }
        }
    }

    // 构建响应
    QJsonObject responseObj;
    responseObj["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    if (loginSuccess) {
        // 生成token
        QString token = generateToken(username);

        // 获取用户信息
        SQL_MallUser user = dbManager->getMallUserByUsername(username);

        QJsonObject responseData;
        responseData["username"] = user.username;
        responseData["nickname"] = user.username; // 可以使用昵称字段，这里先用用户名
        responseData["token"] = token;
        responseData["userLevel"] = user.userLevel;
        responseData["balance"] = user.balance;
        responseData["points"] = user.points;
        responseData["email"] = user.email;
        responseData["phone"] = user.phone;
        responseData["createTime"] = user.createTime;


        responseObj["code"] = 200;
        responseObj["msg"] = "Login successful";
        responseObj["data"] = responseData;

        qDebug() << "Login successful for user:" << username;
    } else {
        responseObj["code"] = 401;
        responseObj["msg"] = "Invalid username or password";
        responseObj["data"] = QJsonObject();

        qDebug() << "Login failed for user:" << username;
    }

    // 转换为JSON并发送
    QJsonDocument responseDoc(responseObj);
    sendResponse(clientSocket, responseDoc.toJson());
}
// 生成token的辅助函数
QString HttpServer::generateToken(const QString &username)
{
    // 简单的方法：用户名 + 时间戳 + 随机数
    QString timestamp = QString::number(QDateTime::currentMSecsSinceEpoch());
    QString random = QString::number(QRandomGenerator::global()->bounded(1000, 9999));

    // 创建一个简单的token
    QString token = QString("%1_%2_%3")
                        .arg(username)
                        .arg(timestamp)
                        .arg(random);

    // 计算MD5哈希（可选，增加安全性）
    QByteArray hash = QCryptographicHash::hash(token.toUtf8(), QCryptographicHash::Md5);
    QString tokenHash = hash.toHex();

    // 添加前缀，便于识别
    return "mall_token_" + tokenHash;
}
void HttpServer::handlePostDeviceProcess(QTcpSocket *clientSocket, const QByteArray &body) {
    // 1. 解析 JSON 请求体
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (doc.isNull()) {
        sendResponse(clientSocket, "{\"code\":400,\"msg\":\"Invalid JSON\"}");
        return;
    }
    QJsonObject jsonObj = doc.object();

    // 获取传入的 data 和 process_id
    QJsonObject data = jsonObj.value("data").toObject();
    QString processId = data.value("process_id").toString();
    QString timestamp = jsonObj.value("timestamp").toString();
    QString serial_number = data.value("serial_number").toString();

    // 2. 查找对应的 process_id
    bool found = false;
    //   QJsonArray responseData;

    for (const Machine_Process_Total &process : std::as_const(processVector)) {
        if (process.process_id == processId) {
            found = true;

            // 3. 构建响应 JSON
            QJsonObject responseObj;
            responseObj["messageType"] = "process";
            responseObj["password"] = "securePassword123";  // 假设为固定值，可以根据需要动态生成
            responseObj["timestamp"] = timestamp;
            responseObj["username"] = "user123";  // 假设为固定值，可以根据需要动态生成
            responseObj["processId"] = processId;
            responseObj["serial_number"] = serial_number;

            // 4. 构建数据部分
            QJsonArray dataArray;
            int stepCounter = 1; // 用于生成步骤编号

            for (const Machine_Process_Single &singleProcess : process.Processes) {
                QJsonObject stepObj = singleProcess.toJsonAll();  // 将单个步骤转化为 JSON
                stepObj["step"] = stepCounter++;  // 添加步骤号
                dataArray.append(stepObj);
            }

            responseObj["data"] = dataArray;

            // 5. 将构建好的 JSON 对象转化为字符串并发送给客户端
            QJsonDocument responseDoc(responseObj);
            emit devProcessSend(responseObj);
            sendResponse(clientSocket, "{\"code\":200,\"msg\":\"流程绑定成功\"}");
            qDebug() << responseDoc.object();
            break;
        }
    }

    if (!found) {
        sendResponse(clientSocket, "{\"code\":404,\"msg\":\"process_id not found\"}");
    }
}

void HttpServer::handlePostDeviceAdd(QTcpSocket *clientSocket, const QUrlQuery &query, const QByteArray &body) {
    QString serial = query.queryItemValue("serial_number");
    QString code = query.queryItemValue("verification_code");
    qDebug() << "[POST /device/add] serial_number =" << serial << ", verification_code =" << code;
    qDebug() << "Body =" << body;

    // 解析 JSON BODY
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isNull()) {
        QJsonObject rootObj = doc.object();
        qDebug() << "Parsed JSON body:" << rootObj;
        // 获取并打印 timestamp
        if (rootObj.contains("timestamp")) {
            QString timestamp = rootObj["timestamp"].toString();
            qDebug() << "Timestamp: " << timestamp;
        } else {
            qWarning() << "Missing 'timestamp' field.";
            sendResponse(clientSocket, "{\"code\":400,\"msg\":\"error\"}");

            return ;
        }
        QString serialNumber;
        QString verificationCode;
        // 获取并处理 'data' 对象
        if (rootObj.contains("data") && rootObj["data"].isObject())
        {
            QJsonObject dataObj = rootObj["data"].toObject();

            // 提取 serial_number 和 verification_code
            if (dataObj.contains("serial_number") && dataObj["serial_number"].isString()) {
                serialNumber = dataObj["serial_number"].toString();
                qDebug() << "Serial Number: " << serialNumber;
            } else {
                qWarning() << "Missing or invalid 'serial_number' field.";
                sendResponse(clientSocket, "{\"code\":400,\"msg\":\"error\"}");

                return ;
            }

            if (dataObj.contains("verification_code") && dataObj["verification_code"].isString()) {
                verificationCode = dataObj["verification_code"].toString();
                qDebug() << "Verification Code: " << verificationCode;
            } else {
                qWarning() << "Missing or invalid 'verification_code' field.";
                sendResponse(clientSocket, "{\"code\":400,\"msg\":\"error\"}");
                return ;
            }

            if(findDeviceBySerialNumber(deviceVector, serialNumber))
            {
                sendResponse(clientSocket, "{\"code\":400,\"msg\":\"设备已经存在\"}");
            }
            else
            {
                deviceVector.append(DeviceStatus(serialNumber, "离线", "未知", "未知 ","0",
                                                 "未知", "未知",
                                                 "未知", "未知", "未知",
                                                 "未知", "未知","未知", "未知"));
                SQL_Device insert_dev;
                insert_dev.device_status = "离线";
                insert_dev.serial_number = serialNumber;
                insert_dev.bound_time =  QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
                insert_dev.checksum = verificationCode;
                insert_dev.ip_address="unknow";
                insert_dev.total_flow="0";
                insert_dev.bound_user = "123";//todo
                dbManager->insertDevice(insert_dev);
                sendResponse(clientSocket, "{\"code\":200,\"msg\":\"设备添加成功\"}");
                emit  NewDeviceCall(serialNumber);
            }
        }

    }

}

bool HttpServer::deleteProcessByProcessId(const QString& process_id) {
    for (int i = 0; i < processVector.size(); ++i) {
        if (processVector[i].process_id == process_id) {
            // 找到并删除该节点
            processVector.removeAt(i);
            dbManager->deleteProcessSteps(process_id);
            return true;
        }
    }
    return false; // 没找到该 process_id
}

// bool HttpServer::deleteProcessByIdFromTotal(const QString& process_id, const QString& target_id) {
//     for (auto& process : totalProcesses) {
//         if (process.process_id == process_id) {
//             return process.deleteProcessById(target_id);
//         }
//     }
//     return false; // 没找到对应的 Process
// }

void HttpServer::handlePostProcessCreate(QTcpSocket *clientSocket, const QByteArray &body) {
    //  qDebug() << "[POST /process/create] body =" << body;
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isNull()) {
        QJsonObject jsonObj = doc.object();
        parseJsonGenerateNode(jsonObj,processVector);
        sendResponse(clientSocket, "{\"code\":200,\"msg\":\"POST /process/create success\"}");

    }
    else
    {
        sendResponse(clientSocket, "{\"code\":400,\"msg\":\"POST /process/create failed\"}");

    }
}

void HttpServer::handlePostProcessUpdate(QTcpSocket *clientSocket, const QUrlQuery &query, const QByteArray &body) {
    QString processId = query.queryItemValue("process_id");
    qDebug() << "[POST /process/update] process_id =" << processId;
    QJsonDocument doc = QJsonDocument::fromJson(body);
    QByteArray jsonData;
    if(!processId.isEmpty())
    {
        QJsonObject jsonObj = doc.object();

        // 调用解析函数
        if (parseMachineProcess(jsonObj, processVector)) {
            // 打印解析结果
            for (const Machine_Process_Total &process : processVector) {
                qDebug() << "Process ID:" << process.process_id;
                qDebug() << "Creation Time:" << process.creation_time;

                for (const Machine_Process_Single &single : process.Processes) {
                    qDebug() << "  Action:" << single.action;
                    qDebug() << "  Sub Action:" << single.sub_action;
                    qDebug() << "  Start Time:" << single.start_time;
                    qDebug() << "  End Time:" << single.end_time;
                    qDebug() << "  Remark:" << single.remark;
                }
            }
            sendResponse(clientSocket, "{\"code\":200,\"msg\":\"POST /process/update success\"}");
        } else {
            qDebug() << "Failed to parse the JSON data.";
        }

    }
    else
    {
        sendResponse(clientSocket, "{\"code\":400,\"msg\":\"POST /process/update failed\"}");
    }
}
bool HttpServer::processDeleteRequest(const QJsonObject &rootObj) {
    // 验证根对象中是否包含 "data" 字段
    if (!rootObj.contains("data") || !rootObj["data"].isObject()) {
        qWarning() << "Missing 'data' field or it is not an object.";
        return false;
    }

    QJsonObject dataObj = rootObj["data"].toObject();

    // 验证是否包含 "process_id" 字段，并确保它是一个数组
    if (!dataObj.contains("process_id") || !dataObj["process_id"].isArray()) {
        qWarning() << "Missing 'process_id' field or it is not an array.";
        return false;
    }

    QJsonArray processIdsArray = dataObj["process_id"].toArray();

    // 遍历 process_id 数组并调用 deleteProcessByProcessId 方法
    for (const QJsonValue &processIdValue : processIdsArray) {
        if (processIdValue.isString()) {
            QString processId = processIdValue.toString();
            deleteProcessByProcessId(processId);
        } else {
            qWarning() << "Invalid process_id value: not a string.";
        }
    }

    return true;
}
void HttpServer::handlePostProcessDelete(QTcpSocket *clientSocket, const QByteArray &body) {
    qDebug() << "[POST /process/delete] body =" << body;
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isNull()) {
        QJsonObject jsonObj = doc.object();
        processDeleteRequest(jsonObj);
        // qDebug() << "Parsed JSON body:" << jsonObj;
    }
    sendResponse(clientSocket, "{\"code\":200,\"msg\":\"POST /process/delete success\"}");
}
QJsonObject HttpServer::generateJson(const QString &username, int code, const QString &token) {
    // 获取当前时间戳（格式化为 ISO 8601）
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 创建 data 子对象
    QJsonObject dataObj;
    dataObj.insert("username", username);
    if (!token.isEmpty()) {
        dataObj.insert("token", token);
    }

    // 创建根对象
    QJsonObject rootObj;
    rootObj.insert("timestamp", timestamp);
    rootObj.insert("code", code);
    rootObj.insert("data", dataObj);

    return rootObj;
}
QJsonObject HttpServer::generateDeviceResponse(const QJsonArray &devices) {
    // 获取当前时间戳（ISO 8601 格式）
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 构建 JSON 响应
    QJsonObject response;
    response["timestamp"] = timestamp;
    response["code"] = 200;

    // 创建 "data" 对象
    QJsonObject data;

    // 设置 "total_devices" 为设备数组的长度
    data["total_devices"] = devices.size();

    // 将传入的 "devices" 数组添加到 "data" 中
    data["devices"] = devices;

    // 将 "data" 添加到响应对象
    response["data"] = data;

    return response;
}
QJsonObject HttpServer::generateProcessResponse(const QJsonArray &data) {
    // 获取当前时间戳（ISO 8601 格式）
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 构建 JSON 响应
    QJsonObject response;
    response["timestamp"] = timestamp;
    response["code"] = 200;



    // 将 "data" 添加到响应对象
    response["data"] = data;

    return response;
}

QJsonObject HttpServer::generateProcessSingleResponse(const QJsonObject &data) {
    // 获取当前时间戳（ISO 8601 格式）
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 构建 JSON 响应
    QJsonObject response;
    response["timestamp"] = timestamp;
    response["code"] = 200;



    // 将 "data" 添加到响应对象
    response["data"] = data;

    return response;
}
// 创建一个失败的接口，返回固定的 400 错误码
QJsonObject HttpServer::generateFailureResponse() {
    // 获取当前时间戳（ISO 8601 格式）
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 构建 JSON 响应
    QJsonObject response;
    response["timestamp"] = timestamp;
    response["code"] = 400;

    return response;
}
DeviceStatus* HttpServer::findDeviceBySerialNumber(QVector<DeviceStatus>& devices, const QString& serialNumber) {
    for (auto& device : devices) {
        if (device.serialNumber == serialNumber) {
            //   device.printInfo();
            return &device;  // 找到并返回指针
        }
    }
    return nullptr;  // 如果没有找到返回 nullptr
}
Machine_Process_Total* HttpServer::findProcessById(QVector<Machine_Process_Total>& processes, const QString& id) {
    for (auto& process : processes) {
        if (process.process_id == id) {
            return &process;  // 找到并返回指针
        }
    }
    return nullptr;  // 如果没有找到返回 nullptr
}
// 生成带有指定 data 的 JSON
QJsonObject HttpServer::generateJsonHearResponse(const QJsonObject& data) {
    QJsonObject response;

    // 获取当前的时间戳，格式化为 ISO 8601 格式
    QString timestamp = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 设置固定字段
    response["timestamp"] = timestamp;
    response["code"] = 200;

    // 设置 data 字段为传入的 QJsonObject
    response["data"] = data;

    return response;
}
QString HttpServer::parseJsonGenerateNode(const QJsonObject &rootObj, QVector<Machine_Process_Total> &processVector) {
    // Extract timestamp (if needed)
    QString timestamp = rootObj["timestamp"].toString();
    QString retId;
    // Extract the data array from the root object
    QJsonArray dataArray = rootObj["data"].toArray();

    // Determine the starting process_id (max + 1)
    int maxProcessId = 0;
    for (const Machine_Process_Total &existingProcess : processVector) {
        bool ok;
        int currentId = existingProcess.process_id.toInt(&ok);
        if (ok && currentId > maxProcessId) {
            maxProcessId = currentId;
        }
    }

    // Iterate through the data array and populate the processVector
    for (const QJsonValue &dataValue : dataArray) {
        QJsonObject dataObj = dataValue.toObject();

        Machine_Process_Total processTotal;

        // Generate new process_id based on the existing max process_id
        processTotal.process_id = QString::number(maxProcessId + 1);  // New process_id is max + 1
        retId = processTotal.process_id;
        maxProcessId++;  // Increment maxProcessId for the next process

        // Print the process_id to debug output
        qDebug() << "Generated process_id:" << processTotal.process_id;
        qDebug() <<dataObj;
        processTotal.process_name = dataObj["process_name"].toString();
        processTotal.creation_time = QDateTime::currentDateTime().toString(Qt::ISODate);  // Optionally set creation time if needed
        processTotal.remark = dataObj["remark"].toString();; // Set this field if available in the data

        QJsonArray stepsArray = dataObj["steps"].toArray();

        // Iterate through steps and populate Processes
        for (const QJsonValue &stepValue : stepsArray) {
            QJsonObject stepObj = stepValue.toObject();

            Machine_Process_Single processSingle;
            processSingle.action = stepObj["action"].toString();
            processSingle.sub_action = stepObj["sub_action"].toString();
            processSingle.start_time = stepObj["start_time"].toString();
            processSingle.end_time = stepObj["end_time"].toString();
            processSingle.remark = stepObj["remark"].toString();

            processTotal.Processes.append(processSingle);
        }

        // Add processTotal to the vector
        processVector.append(processTotal);
        dbManager->insertProcessSteps(processTotal);
    }

    return retId;
}
// 解析 JSON 数据并将其插入到 processVector 中
// 解析 JSON 数据并将其插入到 processVector 中
bool HttpServer::parseMachineProcess(const QJsonObject &rootObj, QVector<Machine_Process_Total> &processVector) {
    // 验证根对象中是否包含 "data" 字段
    if (!rootObj.contains("data") || !rootObj["data"].isObject()) {
        qWarning() << "Missing 'data' field or it is not an object.";
        return false;
    }

    QJsonObject dataObj = rootObj["data"].toObject();

    // 提取 process_id 和 process_name
    QString processId = dataObj["process_id"].toString();

    // 查找是否已经有该 process_id 的节点
    Machine_Process_Total* processTotal = findProcessById(processVector, processId);

    if (processTotal == nullptr) {
        // 如果没有找到对应的 process_id，就创建一个新的节点
        processTotal = new Machine_Process_Total;
        processTotal->process_id = processId;
        processTotal->process_name = "Process Name";  // 假设是一个固定值
        //    processTotal->creation_time = rootObj["timestamp"].toString();
        //    processTotal->remark = "";  // 可以根据需要填充
        processVector.append(*processTotal);
    }

    // 清空现有步骤数据
    processTotal->Processes.clear();

    // 解析步骤
    if (dataObj.contains("steps") && dataObj["steps"].isArray()) {
        QJsonArray stepsArray = dataObj["steps"].toArray();

        // 遍历每个步骤并填充 Machine_Process_Single
        for (const QJsonValue &stepValue : stepsArray) {
            if (stepValue.isObject()) {
                QJsonObject stepObj = stepValue.toObject();
                Machine_Process_Single processSingle;

                // 提取每个步骤的信息
                if (stepObj.contains("action") && stepObj["action"].isString()) {
                    processSingle.action = stepObj["action"].toString();
                }
                if (stepObj.contains("sub_action") && stepObj["sub_action"].isString()) {
                    processSingle.sub_action = stepObj["sub_action"].toString();
                }
                if (stepObj.contains("start_time") && stepObj["start_time"].isString()) {
                    processSingle.start_time = stepObj["start_time"].toString();
                }
                if (stepObj.contains("end_time") && stepObj["end_time"].isString()) {
                    processSingle.end_time = stepObj["end_time"].toString();
                }
                if (stepObj.contains("remark") && stepObj["remark"].isString()) {
                    processSingle.remark = stepObj["remark"].toString();
                }

                // 将步骤添加到 Process 中
                processTotal->Processes.append(processSingle);
                dbManager->deleteProcessSteps(processId);
                dbManager->insertProcessSteps(*processTotal);

            }
        }
    }

    return true;
}
void HttpServer::handleCreateTestOrdersSimple()
{
    // Q_UNUSED(query);

    // DatabaseManager dbManager;
    // if (!dbManager.openDatabase("your_database.db")) {
    //     sendResponse(clientSocket, "{\"code\":500,\"msg\":\"Database error\"}");
    //     return;
    // }

    // 确保表存在
    //dbManager.createTable5();

    QList<SQL_Order> orders;
    QDateTime now = QDateTime::currentDateTime();

    // 生成10个测试订单
    for (int i = 1; i <= 10; i++) {
        SQL_Order order;
        order.orderId = QString("TEST%1").arg(now.toString("yyyyMMdd")) +
                        QString::number(i).rightJustified(3, '0');
        order.productId = QString("PROD%1").arg(QRandomGenerator::global()->bounded(1, 6), 3, 10, QChar('0'));
        order.unitPrice = QRandomGenerator::global()->bounded(100, 1001);
        order.quantity = QRandomGenerator::global()->bounded(1, 6);
        order.totalPrice = order.unitPrice * order.quantity;
        order.user = QString("用户%1").arg(QRandomGenerator::global()->bounded(1, 11));
        order.contactInfo = QString("138%1").arg(QRandomGenerator::global()->bounded(10000000, 99999999));

        // 随机状态
        QStringList statusOptions = {"pending", "paid", "completed", "cancelled"};
        order.status = statusOptions[QRandomGenerator::global()->bounded(statusOptions.size())];

        order.note = QString("测试订单%1，备注信息").arg(i);
        order.createTime = now.addDays(-QRandomGenerator::global()->bounded(30)).toString("yyyy-MM-dd HH:mm:ss");

        orders.append(order);

        // 插入数据库
        dbManager->insertOrder(order);
    }


}
