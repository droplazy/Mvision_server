#include "HttpServer.h"
#include "qjsonarray.h"
#include <QJsonObject>
#include <QDebug>
#include <QUrlQuery>
#include <QFile>
#include <QTextStream>
#include <QDir>
#include <QApplication>

#define MANAGER_USERNAME "123"
#define MANAGER_PASSWD   "123"



HttpServer::HttpServer(DatabaseManager *db,QObject *parent) : QTcpServer(parent), dbManager(db)
{

    generateTextData();
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

    if ((path == "/home" || path == "/devices" ||path == "/process/new" ||path == ("/login")\
        ||path == "/process/center"||path == "/support" ) && request.startsWith("GET"))
    {
        path = "/index.html";
    }
    // 使用绝对路径返回文件
    QString basePath = "E:/qtpro/MuiltiControlSer/www/";


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
      //  qDebug() << "Received request:" << request;
        QList<QByteArray> lines = request.split('\n');
        if (lines.isEmpty()) return;

        // 解析请求行
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


        // 如果请求的是根目录，就返回 index.html
        //ShowHomepage(clientSocket, request);
#if 1
        qDebug() <<"GET REQ PATH: "<<  path <<"GET METHOD: "<< method;
        if (request.startsWith("GET")) {
            if (path == "/device") {
                handleGetDevice(clientSocket, query);
            } else if (path == "/process/get") {
                handleGetProcess(clientSocket, query);
            } else if (path == "/home" || path.contains(".css") || path.contains(".jpg") || path.contains("/login") \
                       || path.contains(".js")|| path.contains(".png") || path.contains(".html") \
                      || path.contains("/devices")|| path.contains("/process/new") || path.contains("/process/center") \
                      || path.contains("/support") || path.contains("/vite.svg") || path.contains("/favicon.ico")) {//静态文件会一直进入这里
               // serveStatic(clientSocket,path, "E:/qtpro/MuiltiControlSer/www/index.html");  // 处理 /home 请求，返回 index.html
                ShowHomepage(clientSocket, request);
            }
            /*else if (path == "/vite.svg"||"/favicon.ico ") {//静态文件会一直进入这里
                // serveStatic(clientSocket,path, "E:/qtpro/MuiltiControlSer/www/index.html");  // 处理 /home 请求，返回 index.html
                //ShowHomepage(clientSocket, request);
            } */else {
                sendNotFound(clientSocket);
            }
        } else if (request.startsWith("POST")) {
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
            } else if (path == "/auth/login") {
                handlePostAuthLogin(clientSocket, body);
            } else if (path == "/process/todev") {
                handlePostDeviceProcess(clientSocket, body);
            } else {
                qDebug()<<path << "[POST /process/create] body =" << body;

                sendNotFound(clientSocket);
            }
        } else {
            sendNotFound(clientSocket);
        }

#endif

        // 断开与客户端的连接
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
            deviceVector[i] = updatedDevice;
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
// 发送404响应


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

        // 打印响应
        //qDebug() << jsonResponse;
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
