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
#include <QRandomGenerator>
#include <QCryptographicHash>
#include <QSslServer>  // 修改：使用QSslServer
#include <QSslSocket>  // 修改：使用QSslSocket
HttpServer::HttpServer(DatabaseManager *db,QObject *parent) : QTcpServer(parent), dbManager(db)
{
    //handleCreateTestOrdersSimple();
    handleCreateProductDebug();
    initVerificationSystem();//邮箱验证码生成
    generateTextData();
    createDownloadDirectoryIfNeeded();
    createUploadDirectoryIfNeeded();
    createAppealDirectoryIfNeeded();
    // 初始化订单超时定时器
    initOrderTimer();

    qDebug() << "HttpServer initialized with pending order container";
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

// 初始化验证码系统
void HttpServer::initVerificationSystem()
{
    // 创建定时器，每分钟清理一次过期验证码
    cleanupTimer = new QTimer(this);
    cleanupTimer->setInterval(60000); // 1分钟

    // 连接定时器信号
    connect(cleanupTimer, &QTimer::timeout, this, &HttpServer::cleanupExpiredCodes);

    // 启动定时器
    cleanupTimer->start();

    qDebug() << "验证码系统初始化完成，定时清理已启动";
}

// 清理过期验证码
void HttpServer::cleanupExpiredCodes()
{
    QMutexLocker locker(&codeMutex);

    int oldSize = verificationCodes.size();

    // 移除过期验证码
    verificationCodes.erase(
        std::remove_if(verificationCodes.begin(), verificationCodes.end(),
                       [](const VerificationCode &vc) {
                           return vc.isExpired();
                       }),
        verificationCodes.end()
        );

    int newSize = verificationCodes.size();
    int removed = oldSize - newSize;

    if (removed > 0) {
        qDebug() << "清理过期验证码:" << removed << "个，剩余:" << newSize;
    }
}

// 添加验证码到容器
void HttpServer::addVerificationCode(const QString &code, const QString &username, const QString &email)
{
    QMutexLocker locker(&codeMutex);

    // 先清理用户旧的验证码
    verificationCodes.erase(
        std::remove_if(verificationCodes.begin(), verificationCodes.end(),
                       [username](const VerificationCode &vc) {
                           return vc.username == username;
                       }),
        verificationCodes.end()
        );

    // 添加新验证码
    VerificationCode vc(code, username, email);
    verificationCodes.append(vc);

    qDebug() << "添加验证码: " << code
             << " 用户: " << username
             << " 邮箱: " << email
             << " 过期时间: " << vc.expireTime.toString("HH:mm:ss");
    qDebug() << "当前验证码数量:" << verificationCodes.size();
}

// 验证验证码
bool HttpServer::verifyCode(const QString &username, const QString &code)
{
    QMutexLocker locker(&codeMutex);

    for (auto &vc : verificationCodes) {
        if (vc.username == username && vc.code == code && vc.isValid()) {
            vc.verified = true;
            qDebug() << "验证码验证成功:" << code << " 用户:" << username;
            return true;
        }
    }

    qDebug() << "验证码验证失败:" << code << " 用户:" << username;
    return false;
}

// 获取用户最新的有效验证码
QString HttpServer::getLatestCode(const QString &username)
{
    QMutexLocker locker(&codeMutex);

    QString latestCode;
    QDateTime latestTime;

    for (const auto &vc : verificationCodes) {
        if (vc.username == username && vc.isValid()) {
            if (vc.createTime > latestTime) {
                latestTime = vc.createTime;
                latestCode = vc.code;
            }
        }
    }

    return latestCode;
}
void HttpServer::createUploadDirectoryIfNeeded()
{
    QDir currentDir(QDir::currentPath());
    QString downloadPath = currentDir.filePath("Upload");
    if (!QDir(downloadPath).exists()) {
        QDir().mkdir(downloadPath);
    }
}
void HttpServer::createAppealDirectoryIfNeeded()
{
    QDir currentDir(QDir::currentPath());
    QString UserApealPath = currentDir.filePath("UserAppeal");
    if (!QDir(UserApealPath).exists()) {
        QDir().mkdir(UserApealPath);
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
void HttpServer::handlePostMallSendMailCode(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "====== 处理发送邮箱验证码请求 ======";

    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    try {
        // 解析JSON请求体
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(body, &parseError);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            qDebug() << "JSON解析失败:" << parseError.errorString();
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "无效的JSON格式";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        // 检查是否有data字段
        if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
            qDebug() << "请求缺少data字段";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "请求缺少data字段";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject dataObj = jsonObj["data"].toObject();

        // 提取字段
        QString username = dataObj.value("username").toString();
        QString email = dataObj.value("email").toString();

        qDebug() << "请求发送验证码:";
        qDebug() << "  用户名:" << username;
        qDebug() << "  邮箱:" << email;

        // 验证必填字段
        if (username.isEmpty() || email.isEmpty()) {
            qDebug() << "用户名或邮箱为空";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "用户名和邮箱不能为空";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查用户是否存在
        if (!dbManager->checkMallUserExists(username)) {
            qDebug() << "用户不存在:" << username;
            response["code"] = 404;
            response["success"] = false;
            response["message"] = "用户不存在";
            sendJsonResponse(clientSocket, 404, response);
            return;
        }

        // 获取用户信息，验证邮箱是否匹配
        SQL_MallUser user = dbManager->getMallUserByUsername(username);

        if (user.email.isEmpty()) {
            qDebug() << "用户未绑定邮箱:" << username;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "用户未绑定邮箱";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        if (user.email != email) {
            qDebug() << "邮箱不匹配:";
            qDebug() << "  提供的邮箱:" << email;
            qDebug() << "  注册的邮箱:" << user.email;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "邮箱和账号不匹配";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查是否有未过期的验证码，且生成时间在30秒内
        QString existingCode;
        QDateTime existingCreateTime;
        bool hasRecentCode = false;

        {
            QMutexLocker locker(&codeMutex);

            for (const auto &vc : verificationCodes) {
                if (vc.username == username && vc.email == email && !vc.verified && !vc.isExpired()) {
                    qint64 secondsSinceCreated = vc.createTime.secsTo(QDateTime::currentDateTime());
                    qDebug() << "找到现有验证码:" << vc.code << "创建于" << secondsSinceCreated << "秒前";

                    if (secondsSinceCreated < 60*5) {
                        existingCode = vc.code;
                        existingCreateTime = vc.createTime;
                        hasRecentCode = true;
                        break;
                    }
                }
            }
        }

        QString verificationCode;
        bool isNewCode = false;

        if (hasRecentCode) {
            // 30秒内已发送过验证码，直接使用现有的
            verificationCode = existingCode;
            qint64 secondsSinceCreated = existingCreateTime.secsTo(QDateTime::currentDateTime());
            qDebug() << "验证码已存在且未超过30秒(" << secondsSinceCreated << "秒)，不再重新生成";
            qDebug() << "使用现有验证码:" << verificationCode;
        } else {
            // 超过30秒或没有验证码，清理旧验证码并生成新验证码
            {
                QMutexLocker locker(&codeMutex);
                // 清理该用户的所有验证码
                verificationCodes.erase(
                    std::remove_if(verificationCodes.begin(), verificationCodes.end(),
                                   [username](const VerificationCode &vc) {
                                       return vc.username == username;
                                   }),
                    verificationCodes.end()
                    );
                qDebug() << "已清理用户" << username << "的旧验证码";
            }

            // 生成新的4位随机验证码（1000-9999）
            verificationCode = QString::number(QRandomGenerator::global()->bounded(1000, 10000));
            isNewCode = true;
            qDebug() << "生成新验证码:" << verificationCode;

            // 保存验证码到容器
            addVerificationCode(verificationCode, username, email);
        }

        // 创建邮件信息结构体
        EmailInfo emailInfo;
        emailInfo.toEmail = email;
        emailInfo.subject = "验证码通知";
        emailInfo.message = QString(
                                "尊敬的 %1：\n\n"
                                "您的验证码是：%2\n\n"
                                "验证码有效期为5分钟，请尽快使用。\n\n"
                                "如非本人操作，请忽略此邮件。\n"
                                ).arg(username).arg(verificationCode);

        // 发送邮件信号（如果是新验证码才发送邮件）
        if (isNewCode) {
            qDebug() << "发送邮件信号...";
            emit sendemail(emailInfo);
            qDebug() << "新验证码邮件已发送到:" << email;
        } else {
            qDebug() << "使用现有验证码，不再重复发送邮件";
        }

        qDebug() << "验证码:" << verificationCode;
        qDebug() << "生成时间:" << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        qDebug() << "过期时间:" << QDateTime::currentDateTime().addSecs(300).toString("yyyy-MM-dd HH:mm:ss");
        qDebug() << "状态:" << (isNewCode ? "新生成的验证码" : "使用现有验证码");

        // 构建成功响应
        response["code"] = 200;
        response["success"] = true;
        response["message"] = "验证码已发送";

// 调试模式下返回验证码
#if 1
        response["debug_verification_code"] = verificationCode;
        response["debug_is_new_code"] = isNewCode;
#endif

        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "====== 验证码发送完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "发送验证码异常:" << e.what();
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "发送验证码未知异常";
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
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
    qDebug() <<"Download path  = " << downloadDir;

    // 确保Download目录存在
    if (!downloadDir.exists()) {
        sendHttpResponse(clientSocket, 404, "Not Found", "Download directory not found");
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
        // 获取客户端IP地址
        QString clientIp = clientSocket->peerAddress().toString();
        // 读取请求
        QByteArray request = clientSocket->readAll();
        // 发送请求信息信号（在完整处理之前）
        QString reqInfo = QString("IP: %1").arg(clientIp);
        emit sendreqInfo(reqInfo);
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
        // 发送包含完整信息的信号
        QString fullReqInfo = QString("IP: %1 | Path: %2 | Method: %3").arg(clientIp).arg(path).arg(QString(method));
        emit sendreqInfo(fullReqInfo);

        // 获取token参数
        QString token = query.queryItemValue("token");
        qDebug() << "token:" << token;
#if 0
        // Token验证（排除登录接口）
        bool isLoginPath = (path == "/auth/login" || path == "/mall/login/info"|| (path == "/home" || path.contains(".css") || path.contains("/login") \
                                                                                   || path.contains(".js") || path.contains(".html") \
                                                                                   || path.contains("/devices") || path.contains("/process/new") || path.contains("/process/center") \
                                                                                   || path.contains("/support") || path.contains("/vite.svg") || path.contains("/favicon.ico")));
        if (!isLoginPath) {
            // 非登录接口需要验证token
            if (token.isEmpty() || !dbManager || !dbManager->validateToken(token)) {
                qDebug() << "Token验证失败或不存在，返回401";
                sendUnauthorized(clientSocket);
                clientSocket->disconnectFromHost();
                return;
            }
        }
#endif
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
            // GET请求处理...
            if (path == "/device") {
                handleGetDevice(clientSocket, query);
            } else if (path == "/mall/login/para") {
                handleGetLoginUI(clientSocket, query);
            } else if (path.contains("/images")) {
                handleBGimagesGet(clientSocket, query);
            } else if (path == "/process/get") {
                handleGetProcess(clientSocket, query);
            } else if (path == "/download") {
                handleGetDownload(clientSocket, query);
            } else if (path == "/command/history") {
                handleGetCommandList(clientSocket, query);
            } else if (path == "/order/dispose/list") {
                handleGetOrderList(clientSocket, query);
            } else if (path == "/mall/auth/info") {
                handleGetAuthInfo(clientSocket, query);
            } else if (path == "/debug/orderPaid") {
                handleGetpaidOK(clientSocket, query);
            } else if (path == "/mall/product/item") {
                handleGetMallProducts(clientSocket);
            } else if (path == "/mall/auth/promot") {
                handleGetAuthPromote(clientSocket, query);
            } else if (path == "/mall/auth/order/query") {
                handleGetOrderQuery(clientSocket, query);
            } else if (path == "/mall/auth/order/recheck-response") {
                handleGetOrderAppeal(clientSocket, query);
            } else if (path == "/home" || path.contains(".css") || path.contains("/login") \
                       || path.contains(".js") || path.contains(".html") \
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
                handlePostFileUpload(clientSocket, query, body);
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
                } else if (path == "/process/todev") {
                    handlePostDeviceProcess(clientSocket, body);
                } else if (path == "/mall/login/info") {
                    handlePostMallLogin(clientSocket, body);
                } else if (path == "/mall/auth/register") {
                    handlePostMallRegist(clientSocket, body);
                } else if (path == "/mall/auth/passwd-reset/reset") {
                    handlePostMallPasswdReset(clientSocket, body);
                } else if (path == "/mall/auth/passwd-reset/sendemail") {
                    handlePostMallSendMailCode(clientSocket, body);
                } else if (path == "/mall/auth/promot/withdraw") {
                    handlePostMallSendwithdraw(clientSocket, body);
                } else if (path == "/mall/product/order-checkout") {
                    handlePostMallOrderCheckout(clientSocket, body);
                } else if (path == "/order/dispose/Verify") {
                    handlePostOrderVerify(clientSocket, body);
                } else if (path == "/mall/auth/order/appeal/text") {
                    handlePostMallUserAppealtext(clientSocket, body, query);
                } else if (path == "/mall/auth/order/appeal/picture") {
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
                            } else {
                                break;
                            }
                        }

                        if (body.size() != contentLength) {
                            qDebug() << "Warning: Body incomplete! Expected:" << contentLength
                                     << "Actual:" << body.size();
                        }
                    }

                    qDebug() << "Before calling handlePostMallUserAppealPic, body size:" << body.size();
                    handlePostMallUserAppealPic(clientSocket, body, query);
                    qDebug() << "After handlePostFileUpload";
                } else {
                    qDebug() << path << "[POST /process/create] body =" << body;
                    sendNotFound(clientSocket);
                }
            }
        } else {
            sendNotFound(clientSocket);
        }

        // 断开与客户端的连接
        clientSocket->disconnectFromHost();
    }
}


void HttpServer::sendUnauthorized(QTcpSocket *clientSocket)
{
    QJsonObject response;
    response["code"] = 401;
    response["message"] = "Unauthorized: Token missing or invalid";
    response["timestamp"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    QJsonDocument doc(response);
    QByteArray jsonData = doc.toJson();

    QString responseStr = QString(
        "HTTP/1.1 401 Unauthorized\r\n"
        "Content-Type: application/json\r\n"
        "Content-Length: %1\r\n"
        "\r\n"
        "%2"
    ).arg(jsonData.size()).arg(QString::fromUtf8(jsonData));

    clientSocket->write(responseStr.toUtf8());
    clientSocket->flush();
}
void HttpServer::onDeviceUpdata(DeviceStatus updatedDevice)
{
    // 更新本地QVector中的设备信息
    for (int i = 0; i < deviceVector.size(); ++i) {
        if (deviceVector[i].serialNumber == updatedDevice.serialNumber) {
            // 更新设备信息
          //  updatedDevice.lastHeartbeat = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            updatedDevice.status = "在线";
           // updatedDevice.location = "杭州";
           // updatedDevice.printInfo();
            // 更新设备向量中的各个字段
            deviceVector[i].usedProcessID = updatedDevice.usedProcessID;
            deviceVector[i].usedProcess = updatedDevice.usedProcess;
            deviceVector[i].hardversion = updatedDevice.hardversion;
            deviceVector[i].current_end = updatedDevice.current_end;
            deviceVector[i].current_start = updatedDevice.current_start;
            deviceVector[i].currentAction = updatedDevice.currentAction;
            deviceVector[i].ip = updatedDevice.ip;
            deviceVector[i].lastHeartbeat = updatedDevice.lastHeartbeat;
            deviceVector[i].trafficStatistics = updatedDevice.trafficStatistics;
            deviceVector[i].checksum = updatedDevice.checksum;
            deviceVector[i].Temperature = updatedDevice.Temperature;
            deviceVector[i].status = updatedDevice.status;

            // 同时保存到数据库中
            saveDeviceStatusToDatabase(deviceVector[i]);
            emit updateDev();//发送信号到客户端界面  如果界面被打开的化

            return;
        }
    }

    // 如果设备不存在于向量中，可能是新设备
    // 更新传入的设备信息
    // updatedDevice.lastHeartbeat = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
    // updatedDevice.status = "在线";
    // updatedDevice.location = "杭州";
    // updatedDevice.warningmsg="新设备";
    // // 添加到向量
    // deviceVector.append(updatedDevice);
    // // 保存新设备到数据库
    // saveDeviceStatusToDatabase(updatedDevice);
}

void HttpServer::saveDeviceStatusToDatabase(const DeviceStatus &deviceStatus)
{

    // 将 DeviceStatus 转换为 SQL_Device
    SQL_Device sqlDevice;
    sqlDevice.serial_number = deviceStatus.serialNumber;
    sqlDevice.checksum = deviceStatus.checksum;

    // 流量统计：这里需要根据你的业务逻辑转换，假设trafficStatistics是已用流量
    // 如果是总流量，可能需要从数据库中读取原有的总流量然后加上新增流量
    sqlDevice.total_flow = deviceStatus.trafficStatistics;

    sqlDevice.ip_address = deviceStatus.ip;
    sqlDevice.device_status = deviceStatus.status;

    // bound_user 和 bound_time 在DeviceStatus中没有对应字段
    // 这里需要从数据库中查询已有的设备信息，如果有绑定信息则保留
    SQL_Device existingDevice = dbManager->getDeviceBySerialNumber(deviceStatus.serialNumber);
    if (!existingDevice.serial_number.isEmpty()) {
        // 如果设备已存在，保留原有的绑定信息
        sqlDevice.bound_user = existingDevice.bound_user;
        sqlDevice.bound_time = existingDevice.bound_time;
    } else {
        // 新设备，设置默认值或留空
        sqlDevice.bound_user = "";
        sqlDevice.bound_time = "";
    }

    // 检查设备是否已存在
    if (dbManager->getDeviceBySerialNumber(deviceStatus.serialNumber).serial_number.isEmpty()) {
        // 设备不存在，插入新记录
        if (!dbManager->insertDevice(sqlDevice)) {
            qDebug() << "Failed to insert device:" << deviceStatus.serialNumber;
        } else {
            qDebug() << "Inserted new device:" << deviceStatus.serialNumber;
        }
    } else {
        // 设备已存在，更新记录
        if (!dbManager->updateDevice(sqlDevice)) {
            qDebug() << "Failed to update device:" << deviceStatus.serialNumber;
        }
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
void HttpServer::handlePostAuthLogin(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "[POST /auth/login] body =" << body;

    QString qjson = QString(body);
    qDebug() << "Raw JSON data:" << qjson;

    QJsonObject result = parseJsonData(qjson);
    int code = 400;
    QString token = "";
    QString username = "";

    QJsonObject jsonObject;

    if(!result.isEmpty())
    {
        username = result.value("username").toString();
        QString password = result.value("password").toString();
        qDebug() << "Username:" << username;
        qDebug() << "Password:" << password;

        // 使用数据库指针验证用户
        if (dbManager) {
            // 从Users表中查询用户
            SQL_User user = dbManager->getUserByUsername(username);

            if (!user.username.isEmpty()) {
                // 用户存在，验证密码
                if (user.password == password) {
                    code = 200; // 登录成功

                    // 生成token
                    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789";
                    const int tokenLength = 32;

                    QRandomGenerator *generator = QRandomGenerator::global();
                    for(int i = 0; i < tokenLength; ++i) {
                        int index = generator->bounded(chars.length());
                        token.append(chars.at(index));
                    }

                    qDebug() << "Generated token:" << token;

                    // 保存token到数据库
                    dbManager->saveUserToken(username, token);
                } else {
                    qDebug() << "Password incorrect";
                    code = 401; // 密码错误
                }
            } else {
                qDebug() << "User not found";
                code = 404; // 用户不存在
            }
        } else {
            qDebug() << "Database manager is null";
            code = 500; // 服务器错误
        }

        // 生成返回JSON
        jsonObject = generateJson(username, code, token);
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

void HttpServer::handlePostFileUpload(QTcpSocket *clientSocket, QUrlQuery query, const QByteArray &body)
{
    qDebug() << "=== handlePostFileUpload called ===";
    qDebug() << "Body size received:" << body.size() << "bytes";

    QString serial = query.queryItemValue("serial_number");
    QString commandId = query.queryItemValue("commandid");
    QString verifycode = query.queryItemValue("verifycode");

    qDebug() << "Parameters - serial:" << serial
             << "commandid:" << commandId
             << "verifycode:" << verifycode;

    if (serial.isEmpty()) {
        qDebug() << "Missing serial_number parameter";
        sendHttpResponse(clientSocket, 400, "Bad Request", "Missing serial_number");
        return;
    }

    if (commandId.isEmpty()) {
        qDebug() << "Missing commandid parameter";
        sendHttpResponse(clientSocket, 400, "Bad Request", "Missing commandid");
        return;
    }

    // 解析multipart/form-data数据
    QString originalFilename;
    QByteArray fileContent;

    // 查找boundary（通常是第一行）
    QByteArray boundary;
    int boundaryStart = body.indexOf("\r\n");
    if (boundaryStart != -1) {
        boundary = body.left(boundaryStart).trimmed();
        qDebug() << "Found boundary:" << boundary;
    } else {
        qDebug() << "Cannot find boundary in multipart data";
        sendHttpResponse(clientSocket, 400, "Bad Request", "Invalid multipart data format");
        return;
    }

    // 查找文件名
    int filenameStart = body.indexOf("filename=\"");
    if (filenameStart != -1) {
        int filenameEnd = body.indexOf("\"", filenameStart + 10);
        if (filenameEnd != -1) {
            originalFilename = QString::fromUtf8(body.mid(filenameStart + 10, filenameEnd - filenameStart - 10));
            qDebug() << "Extracted original filename from multipart:" << originalFilename;
        }
    }

    if (originalFilename.isEmpty()) {
        qDebug() << "Cannot find filename in multipart data";
        sendHttpResponse(clientSocket, 400, "Bad Request", "No filename in multipart data");
        return;
    }

    // 提取文件后缀
    QString fileExtension;
    QFileInfo fileInfo(originalFilename);
    fileExtension = fileInfo.suffix();
    if (fileExtension.isEmpty()) {
        fileExtension = "dat"; // 默认后缀
    }

    // 使用serial_number作为文件名
    QString finalFilename = serial + "." + fileExtension;
    qDebug() << "Using final filename:" << finalFilename;

    // 查找文件内容的开始位置（在Content-Type之后有两个空行）
    QString contentTypeMarker = "Content-Type:";
    int contentTypePos = body.indexOf(contentTypeMarker.toUtf8());
    if (contentTypePos != -1) {
        int contentStart = body.indexOf("\r\n\r\n", contentTypePos);
        if (contentStart != -1) {
            contentStart += 4; // 跳过两个空行
            // 查找文件内容的结束位置（下一个boundary）
            int contentEnd = body.indexOf(boundary, contentStart);
            if (contentEnd != -1) {
                // 减去最后两个换行符（\r\n）
                contentEnd -= 2;
                fileContent = body.mid(contentStart, contentEnd - contentStart);
                qDebug() << "Extracted file content:" << fileContent.size() << "bytes";
            }
        }
    }

    if (fileContent.isEmpty()) {
        qDebug() << "Cannot extract file content from multipart data";
        sendHttpResponse(clientSocket, 400, "Bad Request", "No file content in multipart data");
        return;
    }

    // 创建目录（在当前目录下的Upload/commandid文件夹）
    QDir currentDir = QDir::current();
    QString uploadDirPath = currentDir.filePath("Upload");
    QString commandIdDirPath = currentDir.filePath("Upload/" + commandId);

    qDebug() << "Creating directories - Upload:" << uploadDirPath
             << "CommandId:" << commandIdDirPath;

    if (!QDir(uploadDirPath).exists()) {
        if (QDir().mkpath(uploadDirPath)) {
            qDebug() << "Created Upload directory";
        } else {
            qDebug() << "Failed to create Upload directory";
            sendHttpResponse(clientSocket, 500, "Internal Error", "Cannot create Upload directory");
            return;
        }
    }

    if (!QDir(commandIdDirPath).exists()) {
        if (QDir().mkpath(commandIdDirPath)) {
            qDebug() << "Created commandId directory";
        } else {
            qDebug() << "Failed to create commandId directory";
            sendHttpResponse(clientSocket, 500, "Internal Error", "Cannot create commandId directory");
            return;
        }
    }

    // 保存文件到当前目录下的Upload/commandid文件夹
    QString filePath = QDir(commandIdDirPath).filePath(finalFilename);
    qDebug() << "Saving to path:" << filePath;

    QFile file(filePath);

    if (!file.open(QIODevice::WriteOnly)) {
        qDebug() << "Cannot open file for writing:" << file.errorString();
        sendHttpResponse(clientSocket, 500, "Internal Error",
                         QString("Cannot save file: %1").arg(file.errorString()).toUtf8());
        clientSocket->disconnectFromHost();
        return;
    }

    qDebug() << "File opened successfully, writing" << fileContent.size() << "bytes...";
    qint64 bytesWritten = file.write(fileContent);
    file.close();

    qDebug() << "Bytes written:" << bytesWritten << "/" << fileContent.size();

    if (bytesWritten != fileContent.size()) {
        qDebug() << "Warning: Incomplete write! Expected:" << fileContent.size()
        << "Actual:" << bytesWritten;
    }

    // 验证文件大小
    QFileInfo savedFileInfo(filePath);
    qint64 actualFileSize = savedFileInfo.size();
    qDebug() << "Actual file size on disk:" << actualFileSize << "bytes";

    // 返回JSON响应
    QJsonObject json;
    json["success"] = true;
    json["message"] = "File uploaded successfully";
    json["serial"] = serial;
    json["commandid"] = commandId;
    json["filename"] = finalFilename;
    json["original_filename"] = originalFilename;
    json["extension"] = fileExtension;
    json["requested_size"] = static_cast<qint64>(fileContent.size());
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

void HttpServer::handleGetAuthPromote(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    qDebug() << "====== 处理获取推广信息请求 ======";

    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 获取username参数
    QString username = query.queryItemValue("username");

    qDebug() << "查询用户名:" << username;

    // 验证必填字段
    if (username.isEmpty()) {
        qDebug() << "用户名参数为空";
        response["success"] = false;
        response["message"] = "用户名不能为空";
        sendJsonResponse(clientSocket, 400, response);
        return;
    }

    try {
        // 检查用户是否存在
        if (!dbManager->checkMallUserExists(username)) {
            qDebug() << "用户不存在:" << username;
            response["success"] = false;
            response["message"] = "用户不存在";
            sendJsonResponse(clientSocket, 404, response);
            return;
        }

        // 获取用户信息
        SQL_MallUser user = dbManager->getMallUserByUsername(username);

        // 获取推广人数（邀请的下级用户数量）
        QList<SQL_MallUser> invitedUsers = dbManager->getMallUsersByInviter(username);
        int promotionCount = invitedUsers.size();

        // 计算累计收益（可以添加更复杂的逻辑）
        double earnedAmount = user.balance;

        // 获取下级用户消费总额（如果有对应函数）
        // double invitedConsumption = dbManager->getInvitedUsersTotalConsumption(username);
        double invitedConsumption = 0.0;  // 暂时设为0

        // 推广公告
        QString announcement = "欢迎使用推广系统";
        if (user.userLevel >= 2) {
            announcement = "VIP会员享受额外推广奖励";
        }
        if (promotionCount > 0) {
            if (promotionCount >= 10) {
                announcement = QString("恭喜您已成功邀请%1位用户！").arg(promotionCount);
            }
        }

        // 打印重要信息到调试台
        qDebug() << "=== 推广信息详情 ===";
        qDebug() << "用户名:" << username;
        qDebug() << "用户等级:" << user.userLevel;
        qDebug() << "用户余额:" << QString::number(user.balance, 'f', 2) << "元";
        qDebug() << "邀请码:" << user.inviteCode;
        qDebug() << "推广人数:" << promotionCount << "人";

        if (!invitedUsers.isEmpty()) {
            qDebug() << "邀请的用户列表:";
            int count = qMin(5, invitedUsers.size());
            for (int i = 0; i < count; i++) {
                const SQL_MallUser &invitedUser = invitedUsers[i];
                qDebug() << QString("  %1. %2 (等级:%3, 注册时间:%4)")
                                .arg(i+1)
                                .arg(invitedUser.username)
                                .arg(invitedUser.userLevel)
                                .arg(invitedUser.createTime);
            }

            // 统计用户等级
            int level1Count = 0, level2Count = 0, level3Count = 0, vipCount = 0;
            for (const SQL_MallUser &invitedUser : invitedUsers) {
                if (invitedUser.userLevel == 1) level1Count++;
                else if (invitedUser.userLevel == 2) level2Count++;
                else if (invitedUser.userLevel == 3) level3Count++;
                else if (invitedUser.userLevel >= 10) vipCount++;
            }

            qDebug() << "下级用户等级分布:";
            qDebug() << "  普通用户:" << level1Count << "人";
            if (level2Count > 0) qDebug() << "  Lv2用户:" << level2Count << "人";
            if (level3Count > 0) qDebug() << "  Lv3用户:" << level3Count << "人";
            if (vipCount > 0) qDebug() << "  VIP用户:" << vipCount << "人";
        }

        qDebug() << "累计收益:" << QString::number(earnedAmount, 'f', 2) << "元";
        qDebug() << "下级用户消费总额:" << QString::number(invitedConsumption, 'f', 2) << "元";
        qDebug() << "推广公告:" << announcement;

        if (!user.inviterUsername.isEmpty()) {
            qDebug() << "我的邀请人:" << user.inviterUsername;
            SQL_MallUser inviter = dbManager->getMallUserByUsername(user.inviterUsername);
            if (!inviter.username.isEmpty()) {
                qDebug() << "邀请人信息:";
                qDebug() << "  用户名:" << inviter.username;
                qDebug() << "  等级:" << inviter.userLevel;
                qDebug() << "  邀请码:" << inviter.inviteCode;
            }
        }
        qDebug() << "=========================";

        // 构建响应数据
        QJsonObject dataObj;
        dataObj["promotionCount"] = promotionCount;
        dataObj["earnedAmount"] = earnedAmount;
        dataObj["announcement"] = announcement;
        dataObj["currentBalance"] = user.balance;
        dataObj["inviteCode"] = user.inviteCode;

        // 构建完整响应
        response["success"] = true;
        response["message"] = "获取成功";
        response["data"] = dataObj;

        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "====== 推广信息查询完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "获取推广信息异常:" << e.what();
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "获取推广信息未知异常";
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
}
void HttpServer::handleGetpaidOK(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    qDebug() << "[GET /debug/orderPaid] query =" << query.toString();

    // 获取orderId参数
    QString orderId = query.queryItemValue("orderId");

    qDebug() << "Order payment test for order ID:" << orderId;

    // 验证必填字段
    if (orderId.isEmpty()) {
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 400;

        QJsonObject dataObj;
        dataObj["success"] = false;
        dataObj["message"] = "Missing required parameter: orderId";
        errorResp["data"] = dataObj;

        sendResponse(clientSocket, QJsonDocument(errorResp).toJson());
        return;
    }

    try {
        // 检查订单是否在待支付容器中
        if (!pendingOrders.contains(orderId)) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 404;

            QJsonObject dataObj;
            dataObj["success"] = false;
            dataObj["message"] = "订单不存在或已过期";
            dataObj["orderId"] = orderId;
            dataObj["pendingOrdersCount"] = static_cast<int>(pendingOrders.size());

            // 列出当前所有待支付订单ID
            QJsonArray pendingOrderIds;
            for (auto it = pendingOrders.begin(); it != pendingOrders.end(); ++it) {
                pendingOrderIds.append(it.key());
            }
            dataObj["allPendingOrderIds"] = pendingOrderIds;

            errorResp["data"] = dataObj;

            qDebug() << "Order not found in pending orders:" << orderId;
            qDebug() << "Current pending orders:" << pendingOrderIds;

            sendResponse(clientSocket, QJsonDocument(errorResp).toJson());
            return;
        }

        // 获取订单信息
        SQL_Order order = pendingOrders.value(orderId);

        qDebug() << "Processing payment for order:";
        qDebug() << "  Order ID:" << order.orderId;
        qDebug() << "  User:" << order.user;
        qDebug() << "  Product:" << order.productId;
        qDebug() << "  Amount:" << order.totalPrice;
        qDebug() << "  Status:" << order.status;
        qDebug() << "  Created:" << order.createTime;

        // 模拟支付成功，将订单保存到数据库
        if (!dbManager) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 500;

            QJsonObject dataObj;
            dataObj["success"] = false;
            dataObj["message"] = "Database manager not available";
            errorResp["data"] = dataObj;

            sendResponse(clientSocket, QJsonDocument(errorResp).toJson());
            return;
        }

        // 更新订单状态为已支付
        order.status = "paid";
        order.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // 将订单保存到数据库
        if (!dbManager->insertOrder(order)) {
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["code"] = 500;

            QJsonObject dataObj;
            dataObj["success"] = false;
            dataObj["message"] = "Failed to save order to database";
            dataObj["orderId"] = orderId;
            errorResp["data"] = dataObj;

            sendResponse(clientSocket, QJsonDocument(errorResp).toJson());
            return;
        }

        // 从待支付容器中移除
        pendingOrders.remove(orderId);

        // 构建成功响应
        QJsonObject successResp;
        successResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        successResp["code"] = 200;

        QJsonObject successData;
        successData["success"] = true;
        successData["message"] = "订单支付成功，已保存到数据库";
        successData["orderId"] = orderId;
        successData["paymentStatus"] = "paid";
        successData["savedToDatabase"] = true;
        successData["paymentTime"] = order.updateTime;

        // 订单详情
        QJsonObject orderDetails;
        orderDetails["productId"] = order.productId;
        orderDetails["unitPrice"] = order.unitPrice;
        orderDetails["quantity"] = order.quantity;
        orderDetails["totalPrice"] = order.totalPrice;
        orderDetails["note"] = order.note;
        orderDetails["user"] = order.user;
        orderDetails["contactInfo"] = order.contactInfo;
        orderDetails["createTime"] = order.createTime;
        successData["orderDetails"] = orderDetails;

        // 调试信息
        QJsonObject debugInfo;
        debugInfo["orderFoundInPending"] = true;
        debugInfo["databaseSaved"] = true;
        debugInfo["pendingOrdersBefore"] = static_cast<int>(pendingOrders.size()) + 1; // +1 因为已经移除了
        debugInfo["pendingOrdersAfter"] = static_cast<int>(pendingOrders.size());
        debugInfo["paymentMethod"] = "debug_test";
        debugInfo["testUrl"] = "This is a test payment via /debug/orderPaid endpoint";
        successData["debugInfo"] = debugInfo;

        successResp["data"] = successData;

        qDebug() << "Order payment test successful:";
        qDebug() << "  Order ID:" << orderId;
        qDebug() << "  User:" << order.user;
        qDebug() << "  Amount:" << order.totalPrice;
        qDebug() << "  Saved to database: yes";
        qDebug() << "  Remaining pending orders:" << pendingOrders.size();

        sendResponse(clientSocket, QJsonDocument(successResp).toJson());

    } catch (const std::exception &e) {
        // 捕获并处理任何异常
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["code"] = 500;

        QJsonObject dataObj;
        dataObj["success"] = false;
        dataObj["message"] = QString("订单支付测试失败: %1").arg(e.what());
        errorResp["data"] = dataObj;

        qDebug() << "Order payment test exception:" << e.what();
        sendResponse(clientSocket, QJsonDocument(errorResp).toJson());
    }
}

void HttpServer::handleGetAuthInfo(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    qDebug() << "====== 处理获取用户信息请求 ======";

    // 获取username参数
    QString username = query.queryItemValue("username");

    qDebug() << "查询用户名:" << username;

    // 验证必填字段
    if (username.isEmpty()) {
        qDebug() << "用户名参数为空";
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["success"] = false;
        errorResp["message"] = "用户名不能为空";
        sendJsonResponse(clientSocket, 400, errorResp);
        return;
    }

    try {
        // 检查用户是否存在
        if (!dbManager->checkMallUserExists(username)) {
            qDebug() << "用户不存在:" << username;
            QJsonObject errorResp;
            errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResp["success"] = false;
            errorResp["message"] = "用户不存在";
            sendJsonResponse(clientSocket, 404, errorResp);
            return;
        }

        // 获取用户信息
        SQL_MallUser user = dbManager->getMallUserByUsername(username);

        // 构建用户等级字符串
        QString accountLevel;
        switch (user.userLevel) {
        case 1:
            accountLevel = "lv1";
            break;
        case 2:
            accountLevel = "lv2";
            break;
        case 3:
            accountLevel = "lv3";
            break;
        case 10:
            accountLevel = "admin";
            break;
        default:
            accountLevel = QString("lv%1").arg(user.userLevel);
            break;
        }

        // 打印重要信息到调试台
        qDebug() << "=== 用户信息详情 ===";
        qDebug() << "用户名:" << user.username;
        qDebug() << "邮箱:" << user.email;
        qDebug() << "手机号:" << user.phone;
        qDebug() << "用户等级:" << user.userLevel << "(" << accountLevel << ")";
        qDebug() << "邀请码:" << user.inviteCode;
        qDebug() << "余额:" << QString::number(user.balance, 'f', 2) << "元";
        qDebug() << "积分:" << user.points << "分";
        qDebug() << "创建时间:" << user.createTime;
        qDebug() << "最后登录:" << user.lastLoginTime;
  //      qDebug() << "状态:" << user.status;
        qDebug() << "密码:" << user.password;

        if (!user.inviterUsername.isEmpty()) {
            qDebug() << "邀请人:" << user.inviterUsername;
            SQL_MallUser inviter = dbManager->getMallUserByUsername(user.inviterUsername);
            if (!inviter.username.isEmpty()) {
                qDebug() << "邀请人详情:";
                qDebug() << "  用户名:" << inviter.username;
                qDebug() << "  邮箱:" << inviter.email;
                qDebug() << "  等级:" << inviter.userLevel;
                qDebug() << "  邀请码:" << inviter.inviteCode;
            }
        }
        qDebug() << "=========================";

        // 构建响应数据
        QJsonObject dataObj;
        dataObj["accountLevel"] = accountLevel;
        dataObj["myInviteCode"] = user.inviteCode.isEmpty() ? "未设置" : user.inviteCode;
        dataObj["myEmail"] = user.email.isEmpty() ? "未绑定" : user.email;

        // 构建完整响应
        QJsonObject successResp;
        successResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        successResp["success"] = true;
        successResp["message"] = "获取成功";
        successResp["data"] = dataObj;

        qDebug() << "返回API响应";
        sendJsonResponse(clientSocket, 200, successResp);
        qDebug() << "====== 用户信息查询完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "获取用户信息异常:" << e.what();
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["success"] = false;
        errorResp["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, errorResp);
    } catch (...) {
        qDebug() << "获取用户信息未知异常";
        QJsonObject errorResp;
        errorResp["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResp["success"] = false;
        errorResp["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, errorResp);
    }
}
void HttpServer::handleGetOrderAppeal(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    qDebug() << "Handling GET order appeal response request";

    // 解析查询参数
    QString username = query.queryItemValue("username");

    if (username.isEmpty()) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Missing required parameter: username";
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }



    // 获取用户的所有投诉记录
    QList<SQL_AppealRecord> appeals;
    try {
        // 这里需要实现getAppealsByUser方法
        appeals = dbManager->getAppealsByUser(username);

        if (appeals.isEmpty()) {
            qDebug() << "No appeals found for user:" << username;
        }
    } catch (const std::exception &e) {
        qDebug() << "Database error:" << e.what();

        QJsonObject errorResponse;
        errorResponse["error"] = "Database error";
        errorResponse["message"] = QString(e.what());
        errorResponse["code"] = 500;
        sendJsonResponse(clientSocket, 500, errorResponse);
        return;
    }

    // 构建API响应
    QJsonObject response;

    // 设置时间戳
    QDateTime currentTime = QDateTime::currentDateTimeUtc();
    response["timestamp"] = currentTime.toString(Qt::ISODate);

    // 构建数据数组
    QJsonArray dataArray;

    // 映射数据库状态到API的verificationResult
    auto mapStatusToResult = [](const QString &dbStatus, const QString &processingStatus) -> QString {
        // 如果有处理状态，优先使用处理状态
        if (!processingStatus.isEmpty()) {
            return processingStatus;
        }

        // 否则使用总体状态
        if (dbStatus == "已完成") return "处理完成";
        if (dbStatus == "已拒绝") return "投诉驳回";
        if (dbStatus == "处理中") return "正在处理";
        if (dbStatus == "复核中") return "复核中";
        if (dbStatus == "待处理") return "等待处理";
        if (dbStatus == "已取消") return "已取消";

        // 如果数据库有result字段，使用result
        return dbStatus.isEmpty() ? "等待处理" : dbStatus;
    };

    for (const SQL_AppealRecord &appeal : appeals) {
        QJsonObject appealObj;

        // 订单ID
        appealObj["orderId"] = appeal.orderId;

        // 验证结果
        QString verificationResult = mapStatusToResult(appeal.status, appeal.processingStatus);

        // 如果有具体的结果描述，使用结果描述
        if (!appeal.result.isEmpty()) {
            verificationResult = appeal.result;
        }

        appealObj["verificationResult"] = verificationResult;

        // 可选：添加其他信息
        // appealObj["appealTime"] = appeal.appealTime;
        // appealObj["appealType"] = appeal.appealType;
        // appealObj["status"] = appeal.status;
        // appealObj["processingStatus"] = appeal.processingStatus;
        // appealObj["priority"] = appeal.priority;

        dataArray.append(appealObj);
    }

    response["data"] = dataArray;

    // 发送响应
    sendJsonResponse(clientSocket, 200, response);

    qDebug() << "Order appeal response sent for user:" << username
             << "total appeals:" << appeals.size();
}

void HttpServer::handleGetOrderQuery(QTcpSocket *clientSocket, const QUrlQuery &query)
{
    qDebug() << "====== 处理订单查询请求 ======";

    // 1. 解析查询参数
    QString username = query.queryItemValue("username");
    QString orderStatus = query.queryItemValue("status"); // 可选参数：按状态筛选
    QString pageStr = query.queryItemValue("page");      // 可选参数：分页
    QString limitStr = query.queryItemValue("limit");    // 可选参数：每页数量

    // 验证必填字段
    if (username.isEmpty()) {
        qDebug() << "用户名为空";
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["success"] = false;
        errorResponse["message"] = "用户名不能为空";
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    qDebug() << "查询参数:";
    qDebug() << "  用户名:" << username;
    qDebug() << "  状态筛选:" << (orderStatus.isEmpty() ? "全部" : orderStatus);
    qDebug() << "  页码:" << (pageStr.isEmpty() ? "1" : pageStr);
    qDebug() << "  每页数量:" << (limitStr.isEmpty() ? "20" : limitStr);

    try {
        // 2. 查询数据库获取用户订单
        QList<SQL_Order> orders;

        if (orderStatus.isEmpty()) {
            // 查询所有订单
            orders = dbManager->getUserOrdersWithSnapshots(username);
        } else {
            // 按状态筛选
            orders = dbManager->getOrdersByUser(username);

            // 使用统一的状态转换函数进行过滤
            QList<SQL_Order> filteredOrders;
            for (const SQL_Order &order : orders) {
                QString chineseStatus = mapStatusToChinese(order.status);
                // 将筛选条件也转换为中文进行比较
                QString chineseFilterStatus = mapStatusToChinese(orderStatus);

                // 如果筛选条件已经是中文，直接比较
                if (orderStatus.contains("待支付") || orderStatus.contains("已支付") ||
                    orderStatus.contains("执行中") || orderStatus.contains("已完成") ||
                    orderStatus.contains("已取消")) {
                    // 筛选条件已经是中文
                    if (chineseStatus == orderStatus) {
                        filteredOrders.append(order);
                    }
                } else {
                    // 筛选条件是英文，需要转换后比较
                    if (chineseStatus == chineseFilterStatus ||
                        order.status.toLower() == orderStatus.toLower()) {
                        filteredOrders.append(order);
                    }
                }
            }

            // 为过滤后的订单获取截图信息
            for (int i = 0; i < filteredOrders.size(); ++i) {
                SQL_Order &order = filteredOrders[i];

                if (!order.commandId.isEmpty()) {
                    SQL_CommandHistory command = dbManager->getCommandById(order.commandId);
                    if (!command.completed_url.isEmpty()) {
                        order.snapshot = command.completed_url;
                    }
                }

                // 如果商品名称为空，使用默认名称
                if (order.productName.isEmpty() && !order.productId.isEmpty()) {
                    order.productName = QString("商品%1").arg(order.productId);
                }
            }
            orders = filteredOrders;
        }

        qDebug() << "查询到订单数量:" << orders.size();

        // 3. 分页处理
        int page = pageStr.toInt();
        int limit = limitStr.toInt();
        if (page <= 0) page = 1;
        if (limit <= 0 || limit > 100) limit = 20; // 默认每页20条，最大100条

        int total = orders.size();
        int startIndex = (page - 1) * limit;
        int endIndex = qMin(startIndex + limit, total);
        int returnedCount = qMax(0, endIndex - startIndex);

        // 打印分页信息
        qDebug() << "分页信息:";
        qDebug() << "  总订单数:" << total;
        qDebug() << "  当前页码:" << page;
        qDebug() << "  每页数量:" << limit;
        qDebug() << "  起始索引:" << startIndex;
        qDebug() << "  结束索引:" << endIndex;
        qDebug() << "  返回数量:" << returnedCount;

        // 4. 构建API响应
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        // 构建订单数据数组
        QJsonArray orderArray;

        for (int i = startIndex; i < endIndex && i < orders.size(); ++i) {
            const SQL_Order &order = orders[i];

            QJsonObject orderObj;

            // 格式化订单时间
            QString orderTime = formatOrderTime(order.createTime);
            orderObj["orderTime"] = orderTime;

            // 商品名称
            orderObj["productName"] = order.productName;

            // 数量
            orderObj["quantity"] = order.quantity;

            // 使用统一的状态转换函数转换状态为中文
            QString chineseStatus = mapStatusToChinese(order.status);
            orderObj["orderStatus"] = chineseStatus;

            // 订单ID
            orderObj["orderId"] = order.orderId;

            // 截图URL
            orderObj["snapshot"] = order.snapshot.isEmpty() ? "" : order.snapshot;

            // 可选：添加其他字段
            orderObj["totalPrice"] = order.totalPrice;
            orderObj["unitPrice"] = order.unitPrice;
            orderObj["note"] = order.note.isEmpty() ? "" : order.note;

            // 原始状态（用于调试）
            // orderObj["originalStatus"] = order.status;

            orderArray.append(orderObj);

            // 打印订单详情到调试台
            qDebug() << "订单详情:";
            qDebug() << "  订单ID:" << order.orderId;
            qDebug() << "  商品名称:" << order.productName;
            qDebug() << "  数量:" << order.quantity;
            qDebug() << "  总价:" << QString::number(order.totalPrice, 'f', 2) << "元";
            qDebug() << "  单价:" << QString::number(order.unitPrice, 'f', 2) << "元";
            qDebug() << "  原始状态:" << order.status;
            qDebug() << "  中文状态:" << chineseStatus;
            qDebug() << "  下单时间:" << orderTime;
            qDebug() << "  截图URL:" << (order.snapshot.isEmpty() ? "无" : "有");
            qDebug() << "  备注:" << (order.note.isEmpty() ? "无" : order.note);
            qDebug() << "-------------------------";
        }

        response["success"] = true;
        response["message"] = "查询成功";
        response["data"] = orderArray;

        // 添加分页信息
        QJsonObject pagination;
        pagination["page"] = page;
        pagination["limit"] = limit;
        pagination["total"] = total;
        pagination["returned"] = returnedCount;
        pagination["totalPages"] = (total + limit - 1) / limit;
        response["pagination"] = pagination;

        // 5. 发送响应
        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "订单查询完成";
        qDebug() << "总订单数:" << total;
        qDebug() << "返回订单数:" << orderArray.size();
        qDebug() << "====== 订单查询处理完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "订单查询异常:" << e.what();
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["success"] = false;
        errorResponse["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, errorResponse);
    } catch (...) {
        qDebug() << "订单查询未知异常";
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["success"] = false;
        errorResponse["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, errorResponse);
    }
}

// 辅助函数：映射订单状态到API状态
QString HttpServer::mapOrderStatusToApi(const QString &dbStatus)
{
    static QMap<QString, QString> statusMap = {
        {"pending", "未支付"},
        {"paid", "支付"},
        {"reviewing", "复核中"},
        {"completed", "结束"},
        {"finished", "结束"}, // 兼容不同的完成状态
        {"failed", "失败"},
        {"cancelled", "已取消"}
    };

    if (statusMap.contains(dbStatus)) {
        return statusMap[dbStatus];
    }

    return dbStatus; // 如果不在映射表中，返回原始状态
}

// 辅助函数：格式化订单时间
QString HttpServer::formatOrderTime(const QString &dbTime)
{
    // 尝试多种时间格式
    QDateTime dt;

    // 尝试标准SQL格式
    dt = QDateTime::fromString(dbTime, "yyyy-MM-dd HH:mm:ss");

    // 如果无效，尝试ISO格式
    if (!dt.isValid()) {
        dt = QDateTime::fromString(dbTime, Qt::ISODate);
    }

    // 如果无效，尝试其他常见格式
    if (!dt.isValid()) {
        dt = QDateTime::fromString(dbTime, "yyyy/MM/dd HH:mm:ss");
    }

    // 如果仍然无效，使用当前时间
    if (!dt.isValid()) {
        dt = QDateTime::currentDateTime();
    }

    // 格式化为 "yyyy.MM.dd HH:mm:ss"
    return dt.toString("yyyy.MM.dd HH:mm:ss");
}

// 辅助函数：发送JSON响应
void HttpServer::sendJsonResponse(QTcpSocket *clientSocket, int statusCode, const QJsonObject &json)
{
    QJsonDocument doc(json);
    QByteArray responseData = doc.toJson(QJsonDocument::Indented);

    QString response = QString("HTTP/1.1 %1 OK\r\n")
                           .arg(statusCode);
    response += "Content-Type: application/json; charset=utf-8\r\n";
    response += QString("Content-Length: %1\r\n")
                    .arg(responseData.size());
    response += "Connection: close\r\n";
    response += "Access-Control-Allow-Origin: *\r\n";
    response += "\r\n";
    response += QString::fromUtf8(responseData);

    clientSocket->write(response.toUtf8());
    clientSocket->flush();
    clientSocket->close();
}

void HttpServer::handleGetOrderList(QTcpSocket *clientSocket, const QUrlQuery &query) {
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

    // 计算实际返回的数量
    int returnedCount = qMax(0, endIdx - startIdx);

    QList<SQL_Order> pagedOrders;
    for (int i = startIdx; i < endIdx && i < totalCount; i++) {
        pagedOrders.append(orders[i]);
    }

    QString action = "action";        // $1
    QString subAction = "sub_action"; // $2
    QString startTime = "start_time"; // $3
    QString endTime = "end_time";     // $4

    // 构建JSON响应
    QJsonObject jsonResponse;
    jsonResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    // 创建data对象来包含订单数组和分页信息
    QJsonObject dataObj;

    QJsonArray dataArray;

    for (const SQL_Order &order : pagedOrders) {
        // 获取产品名称 - 直接从数据库获取
        QString productName = order.productName.isEmpty() ?
                                  QString("商品%1").arg(order.productId) :
                                  order.productName;

        // 将数据库状态映射为中文显示状态
        QString displayStatus = mapStatusToChinese(order.status);

        // 获取验证人 - 标记为待做接口
        QString verifier = "待做接口获取验证人";

        // 构建content文本（单行格式）
        QString content = QString("订单详情：商品名称：%1，下单单价：%2元，下单总数：%3，客户备注：%4")
                              .arg(productName) // 使用数据库中的productName
                              .arg(QString::number(order.unitPrice, 'f', 2))
                              .arg(order.quantity)
                              .arg(order.note.isEmpty() ? "无" : order.note);

        // 将createTime转换为ISO格式
        QString isoOrderTime = order.createTime;
        QDateTime createTime = QDateTime::fromString(order.createTime, "yyyy-MM-dd HH:mm:ss");
        if (createTime.isValid()) {
            isoOrderTime = createTime.toString(Qt::ISODate);
        } else {
            // 尝试其他常见格式
            createTime = QDateTime::fromString(order.createTime, "yyyy/MM/dd HH:mm:ss");
            if (createTime.isValid()) {
                isoOrderTime = createTime.toString(Qt::ISODate);
            }
            // 如果都不行，保持原样
        }

        // 构建订单对象
        QJsonObject orderObj;
        orderObj["orderId"] = order.orderId;
        // 新增：添加commandId字段，从数据库查询，如果为空则留空
        orderObj["commandId"] = order.commandId.isEmpty() ? "" : order.commandId;
        orderObj["content"] = content;
        orderObj["user"] = order.user;
        orderObj["orderTime"] = isoOrderTime;
        // 使用映射后的中文状态
        orderObj["status"] = displayStatus;

        // 只有当有验证人时才添加verifier字段（匹配D00005的示例）
        // 修改：总是添加verifier字段，值为"待做接口获取验证人"
        orderObj["verifier"] = verifier;

        // 构建defaultParams对象
        QJsonObject defaultParams;
        defaultParams["action"] = action;          // $1
        defaultParams["subAction"] = subAction;    // $2
        defaultParams["devices"] = order.quantity; // devices = quantity
        defaultParams["startTime"] = startTime;    // $3
        defaultParams["endTime"] = endTime;        // $4
        defaultParams["remark"] = order.note.isEmpty() ? "" : order.note; // remark = note，空则留空

        orderObj["defaultParams"] = defaultParams;

        dataArray.append(orderObj);
    }

    // 构建分页信息对象
    QJsonObject paginationObj;
    paginationObj["limit"] = limit;
    paginationObj["offset"] = offset;
    paginationObj["returned"] = returnedCount;
    paginationObj["total"] = totalCount;

    // 将订单数组和分页信息放入data对象
    dataObj["orders"] = dataArray;  // 注意：这里使用"orders"作为键名
    dataObj["pagination"] = paginationObj;

    // 将data对象放入根响应
    jsonResponse["data"] = dataObj;

    // 发送响应
    QJsonDocument jsonDoc(jsonResponse);
    QByteArray jsonData = jsonDoc.toJson(QJsonDocument::Indented);

    sendResponse(clientSocket, jsonData);
}
// 状态映射函数
QString HttpServer::mapStatusToChinese(const QString &status)
{
    // 状态映射表
    static QMap<QString, QString> statusMap = {
        {"pending", "待支付"},
        {"paid", "已支付"},
        {"execing", "执行中"},
        {"completed", "已完成"},
        {"cancelled", "已取消"},
        // 可以添加更多状态映射
        {"pending_payment", "待支付"},
        {"processing", "处理中"},
        {"finished", "已完成"},
        {"failed", "失败"}
    };

    if (status.isEmpty()) {
        return "待验证";
    }

    // 查找映射，如果找不到则返回原状态
    if (statusMap.contains(status)) {
        return statusMap[status];
    }

    // 如果状态包含这些关键词，也进行映射
    if (status.contains("pending", Qt::CaseInsensitive)) {
        return "待支付";
    } else if (status.contains("paid", Qt::CaseInsensitive)) {
        return "已支付";
    } else if (status.contains("execing", Qt::CaseInsensitive)) {
        return "执行中";
    } else if (status.contains("complete", Qt::CaseInsensitive) ||
               status.contains("finish", Qt::CaseInsensitive)) {
        return "已完成";
    } else if (status.contains("cancel", Qt::CaseInsensitive)) {
        return "已取消";
    }

    // 默认返回原状态
    return status;
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
        cmdObj["completeness"] = cmd.completeness;
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

void HttpServer::handleGetMallProducts(QTcpSocket *clientSocket)
{
    qDebug() << "处理获取商品分类列表请求";

    // 构建响应数据结构
    QJsonObject responseJson;
    responseJson["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    QJsonObject dataObj;
    QJsonArray categoriesArray;

    // 检查数据库是否有数据
    bool hasData = false;

    // 尝试从数据库获取商品
    QList<SQL_Product> allProducts;
    try {
            allProducts = dbManager->getAllProducts();
            hasData = !allProducts.isEmpty();
    } catch (...) {
        // 数据库错误，继续返回空数据
        qDebug() << "数据库查询失败，返回空数据";
    }

    // 如果没有数据，返回空数组
    if (!hasData) {
        qDebug() << "没有商品数据，返回空数组";
        dataObj["categories"] = categoriesArray; // 空数组
        responseJson["data"] = dataObj;
        sendJsonResponse(clientSocket, 200, responseJson);
        return;
    }

    // 如果有数据，按分类分组
    QMap<QString, QList<SQL_Product>> productsByCategory;
    for (const auto& product : allProducts) {
        productsByCategory[product.categoryId].append(product);
    }

    // 构建分类数组
    for (auto it = productsByCategory.begin(); it != productsByCategory.end(); ++it) {
        QJsonObject categoryObj;
        categoryObj["categoryId"] = it.key();
        categoryObj["categoryName"] = "未命名分类"; // 可以根据需要添加分类名

        QJsonArray productArray;
        for (const auto& product : it.value()) {
            QJsonObject productObj;
            productObj["productId"] = product.productId;
            productObj["productName"] = product.productName;
            productObj["unitPrice"] = product.unitPrice;
            productObj["stock"] = product.stock;
            productObj["minOrder"] = product.minOrder;
            productObj["maxOrder"] = product.maxOrder;
            productObj["status"] = product.status;
            productObj["imageUrl"] = product.imageUrl;
            productObj["description"] = product.description;
            productArray.append(productObj);
        }

        categoryObj["products"] = productArray;
        categoriesArray.append(categoryObj);
    }

    // 构建完整响应
    dataObj["categories"] = categoriesArray;
    responseJson["data"] = dataObj;

    // 发送响应
    sendJsonResponse(clientSocket, 200, responseJson);

    qDebug() << "返回数据，分类数量:" << categoriesArray.size();
}
void HttpServer::handlePostMallUserAppealtext(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query)
{
    qDebug() << "Handling POST user appeal text request";


    QString orderId = query.queryItemValue("orderId");
    QString username = query.queryItemValue("username");

    if (orderId.isEmpty() || username.isEmpty()) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Missing required parameters: orderId and username";
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    // 解析JSON body
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Invalid JSON format";
        errorResponse["message"] = parseError.errorString();
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    QJsonObject json = doc.object();
    QJsonObject dataObj = json.value("data").toObject();
    QString text = dataObj.value("text").toString();

    if (text.isEmpty()) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Text content is empty";
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    // 创建目录结构
    QString baseDir = QDir::currentPath() + "/UserAppeal";
    QString orderDir = baseDir + "/" + orderId;

    QDir dir;
    if (!dir.exists(baseDir)) {
        if (!dir.mkpath(baseDir)) {
            QJsonObject errorResponse;
            errorResponse["error"] = "Failed to create base directory";
            errorResponse["code"] = 500;
            sendJsonResponse(clientSocket, 500, errorResponse);
            return;
        }
    }

    if (!dir.exists(orderDir)) {
        if (!dir.mkpath(orderDir)) {
            QJsonObject errorResponse;
            errorResponse["error"] = "Failed to create order directory";
            errorResponse["code"] = 500;
            sendJsonResponse(clientSocket, 500, errorResponse);
            return;
        }
    }

    // 保存文本文件
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("appeal_%1_%2.txt").arg(username).arg(timestamp);
    QString filePath = orderDir + "/" + filename;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Failed to create text file";
        errorResponse["code"] = 500;
        sendJsonResponse(clientSocket, 500, errorResponse);
        return;
    }

    QTextStream stream(&file);
    stream.setEncoding(QStringConverter::Utf8); // Qt 6
    stream << "投诉时间: " << QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") << "\n";
    stream << "投诉用户: " << username << "\n";
    stream << "订单编号: " << orderId << "\n";
    stream << "投诉内容: \n" << text << "\n";
    file.close();

    // 保存到数据库

    dbManager->insertUserAppeal(username, orderId, "text", filePath, text);

    // 构建成功响应
    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    response["code"] = 200;

    QJsonObject data;
    data["success"] = true;
    data["message"] = "投诉文本已保存，已交由人工处理";
    data["filePath"] = filePath;
    response["data"] = data;

    sendJsonResponse(clientSocket, 200, response);

    qDebug() << "User appeal text saved successfully. Order:" << orderId
             << "User:" << username << "File:" << filePath;
}

void HttpServer::handlePostMallUserAppealPic(QTcpSocket *clientSocket, const QByteArray &body, const QUrlQuery &query)
{
    qDebug() << "Handling POST user appeal picture request";

    QString orderId = query.queryItemValue("orderId");
    QString username = query.queryItemValue("username");

    if (orderId.isEmpty() || username.isEmpty()) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Missing required parameters: orderId and username";
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    qDebug() << "Body size:" << body.size() << "bytes";

    // 1. 查找空行分隔符（HTTP头部的结束）
    QByteArray imageData;
    QString imageContentType = "image/jpeg"; // 默认值

    // 查找空行分隔符 \r\n\r\n
    int emptyLinePos = body.indexOf("\r\n\r\n");

    if (emptyLinePos != -1) {
        // 找到空行，之后的数据就是图片内容
        imageData = body.mid(emptyLinePos + 4); // 跳过 \r\n\r\n

        // 从头部提取Content-Type
        QByteArray headers = body.left(emptyLinePos);

        // 查找Content-Type行
        int contentTypeStart = headers.indexOf("Content-Type:");
        if (contentTypeStart != -1) {
            int lineEnd = headers.indexOf("\r\n", contentTypeStart);
            if (lineEnd != -1) {
                QByteArray contentTypeLine = headers.mid(contentTypeStart, lineEnd - contentTypeStart);
                QList<QByteArray> parts = contentTypeLine.split(':');
                if (parts.size() >= 2) {
                    imageContentType = parts[1].trimmed();
                    qDebug() << "Extracted Content-Type from headers:" << imageContentType;
                }
            }
        }

        qDebug() << "Found empty line at position:" << emptyLinePos;
        qDebug() << "Extracted image data size:" << imageData.size() << "bytes";

        // 检查提取的数据是否以常见的图片格式开头
        if (imageData.size() >= 4) {
            // 检查JPEG (FF D8 FF)
            if (imageData[0] == '\xFF' && imageData[1] == '\xD8' && imageData[2] == '\xFF') {
                qDebug() << "Data starts with JPEG signature";
            }
            // 检查PNG (\x89PNG)
            else if (imageData[0] == '\x89' && imageData[1] == 'P' && imageData[2] == 'N' && imageData[3] == 'G') {
                qDebug() << "Data starts with PNG signature";
            }
            // 检查其他格式...
        }
    } else {
        // 没有找到空行，可能整个body就是图片数据，或者格式有问题
        // 检查body是否以图片签名开头
        if (body.size() >= 4) {
            // 检查JPEG
            if (body[0] == '\xFF' && body[1] == '\xD8' && body[2] == '\xFF') {
                imageData = body;
                qDebug() << "Body starts with JPEG signature, using entire body as image data";
            }
            // 检查PNG
            else if (body[0] == '\x89' && body[1] == 'P' && body[2] == 'N' && body[3] == 'G') {
                imageData = body;
                qDebug() << "Body starts with PNG signature, using entire body as image data";
            } else {
                // 尝试查找常见图片格式的起始位置
                // JPEG的起始标记
                int jpegStart = body.indexOf("\xFF\xD8\xFF");
                if (jpegStart != -1) {
                    imageData = body.mid(jpegStart);
                    qDebug() << "Found JPEG start at position:" << jpegStart;
                } else {
                    // PNG的起始标记
                    int pngStart = body.indexOf("\x89PNG\r\n\x1a\n");
                    if (pngStart != -1) {
                        imageData = body.mid(pngStart);
                        qDebug() << "Found PNG start at position:" << pngStart;
                    } else {
                        // 如果没有找到图片签名，返回错误
                        QJsonObject errorResponse;
                        errorResponse["error"] = "Invalid image format or missing image data";
                        errorResponse["code"] = 400;
                        sendJsonResponse(clientSocket, 400, errorResponse);
                        return;
                    }
                }
            }
        } else {
            // Body太小，不可能是有效的图片
            QJsonObject errorResponse;
            errorResponse["error"] = "Invalid image data (too small)";
            errorResponse["code"] = 400;
            sendJsonResponse(clientSocket, 400, errorResponse);
            return;
        }
    }

    if (imageData.isEmpty()) {
        QJsonObject errorResponse;
        errorResponse["error"] = "No image data found in request";
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    // 检查图片大小（限制为5MB）
    if (imageData.size() > 5 * 1024 * 1024) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Image file too large, maximum size is 5MB";
        errorResponse["code"] = 400;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    // 创建目录结构
    QString baseDir = QDir::currentPath() + "/UserAppeal";
    QString orderDir = baseDir + "/" + orderId;

    QDir dir;
    if (!dir.exists(baseDir)) {
        if (!dir.mkpath(baseDir)) {
            QJsonObject errorResponse;
            errorResponse["error"] = "Failed to create base directory";
            errorResponse["code"] = 500;
            sendJsonResponse(clientSocket, 500, errorResponse);
            return;
        }
    }

    if (!dir.exists(orderDir)) {
        if (!dir.mkpath(orderDir)) {
            QJsonObject errorResponse;
            errorResponse["error"] = "Failed to create order directory";
            errorResponse["code"] = 500;
            sendJsonResponse(clientSocket, 500, errorResponse);
            return;
        }
    }

    // 确定图片扩展名
    QString extension = "jpg";
    if (imageContentType.contains("png")) {
        extension = "png";
    } else if (imageContentType.contains("gif")) {
        extension = "gif";
    } else if (imageContentType.contains("bmp")) {
        extension = "bmp";
    } else if (imageContentType.contains("jpeg")) {
        extension = "jpg";
    }

    // 保存图片文件
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_HHmmss");
    QString filename = QString("appeal_%1_%2.%3").arg(username).arg(timestamp).arg(extension);
    QString filePath = orderDir + "/" + filename;

    QFile file(filePath);
    if (!file.open(QIODevice::WriteOnly)) {
        QJsonObject errorResponse;
        errorResponse["error"] = "Failed to save image file";
        errorResponse["code"] = 500;
        sendJsonResponse(clientSocket, 500, errorResponse);
        return;
    }

    // 只写入提取的图片数据，不包含HTTP头部
    qint64 bytesWritten = file.write(imageData);
    file.close();

    qDebug() << "Wrote" << bytesWritten << "bytes to file:" << filePath;


   dbManager->insertUserAppeal(username, orderId, "picture", filePath, "", "normal", 1);

    // 构建成功响应
    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTime().toString(Qt::ISODate);
    response["code"] = 200;

    QJsonObject data;
    data["success"] = true;
    data["message"] = "投诉图片已保存，已交由人工处理";
    data["filePath"] = filePath;
    data["fileSize"] = QString::number(imageData.size() / 1024) + "KB";
    data["imageType"] = imageContentType;
    response["data"] = data;

    sendJsonResponse(clientSocket, 200, response);

    qDebug() << "User appeal picture saved successfully. Order:" << orderId
             << "User:" << username << "File:" << filePath
             << "Size:" << imageData.size() << "bytes";
}

// 辅助函数：解析multipart数据
QByteArray HttpServer::parseMultipartData(const QByteArray &body, const QByteArray &boundary)
{
    QByteArray boundaryWithPrefix = "--" + boundary;
    QByteArray result;

    // 查找boundary的位置
    int boundaryStart = body.indexOf(boundaryWithPrefix);
    if (boundaryStart == -1) return result;

    // 移动到boundary之后
    int dataStart = boundaryStart + boundaryWithPrefix.length();

    // 跳过可能存在的空格和换行
    while (dataStart < body.size() && (body[dataStart] == ' ' || body[dataStart] == '\r' || body[dataStart] == '\n')) {
        dataStart++;
    }

    // 查找下一个boundary
    int nextBoundaryPos = body.indexOf(boundaryWithPrefix, dataStart);
    if (nextBoundaryPos == -1) {
        // 尝试查找结束boundary
        nextBoundaryPos = body.indexOf(boundaryWithPrefix + "--", dataStart);
    }

    if (nextBoundaryPos == -1) {
        // 如果找不到下一个boundary，取到末尾
        nextBoundaryPos = body.size();
    }

    // 提取数据
    QByteArray partData = body.mid(dataStart, nextBoundaryPos - dataStart);

    // 查找空行分隔符
    int emptyLinePos = partData.indexOf("\r\n\r\n");
    if (emptyLinePos != -1) {
        // 空行之后才是真正的数据
        result = partData.mid(emptyLinePos + 4);

        // 移除末尾的换行符（如果下一个boundary前有换行）
        if (result.endsWith("\r\n")) {
            result = result.left(result.size() - 2);
        }
    } else {
        // 如果没有找到空行，假设整个部分都是数据
        result = partData;
    }

    return result;
}
void HttpServer::handlePostOrderVerify(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "====== 处理订单验证请求 ======";

    // 解析JSON body
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(body, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qDebug() << "JSON解析失败:" << parseError.errorString();
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["code"] = 400;
        errorResponse["success"] = false;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    QJsonObject json = doc.object();
    QJsonObject dataObj = json.value("data").toObject();

    // 提取参数
    QString orderId = dataObj.value("orderId").toString();
    QString user = dataObj.value("user").toString();
    QString verifier = dataObj.value("verifier").toString();
    QString action = dataObj.value("action").toString();
    QString subAction = dataObj.value("subAction").toString();
    int devices = dataObj.value("devices").toInt();
    QString startTimeStr = dataObj.value("startTime").toString();
    QString endTimeStr = dataObj.value("endTime").toString();
    QString remark = dataObj.value("remark").toString();

    // 参数验证
    if (orderId.isEmpty() || user.isEmpty() || verifier.isEmpty()) {
        qDebug() << "缺少必要参数: orderId, user, verifier";
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["code"] = 400;
        errorResponse["success"] = false;
        sendJsonResponse(clientSocket, 400, errorResponse);
        return;
    }

    if (devices <= 0) {
        devices = 1; // 默认1个设备
    }

    qDebug() << "订单ID:" << orderId << "需要设备:" << devices;

    try {
        // 1. 检查订单是否存在
        SQL_Order existingOrder = dbManager->getOrderById(orderId);
        if (existingOrder.orderId.isEmpty()) {
            qDebug() << "订单不存在:" << orderId;
            QJsonObject errorResponse;
            errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResponse["code"] = 404;
            errorResponse["success"] = false;
            sendJsonResponse(clientSocket, 404, errorResponse);
            return;
        }

        // 检查订单状态是否可以执行
        if (existingOrder.status != "paid" && existingOrder.status != "已支付") {
            qDebug() << "订单状态不允许执行，当前状态:" << existingOrder.status;
            QJsonObject errorResponse;
            errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResponse["code"] = 400;
            errorResponse["success"] = false;
            errorResponse["message"] = QString("订单状态[%1]不允许执行").arg(existingOrder.status);
            sendJsonResponse(clientSocket, 400, errorResponse);
            return;
        }

        // 2. 从内存容器中筛选空闲设备
        QList<QString> idleDevices;
        int totalDevices = deviceVector.size();
        qDebug() << "总设备数:" << totalDevices;

        for (const DeviceStatus &device : deviceVector) {
            QString currentAction = device.currentAction.trimmed().toLower();
            bool isIdle = currentAction.isEmpty() ||
                         currentAction == "idle" ||
                         currentAction == "空闲" ||
                         currentAction.contains("idle") ||
                         currentAction.contains("空闲");

            if (isIdle) {
                idleDevices.append(device.serialNumber);
                qDebug() << "找到空闲设备:" << device.serialNumber;
            }
        }

        qDebug() << "空闲设备数:" << idleDevices.size() << "需要设备数:" << devices;

        // 3. 检查空闲设备是否足够
        if (idleDevices.size() < devices) {
            qDebug() << "空闲设备不足";
            QJsonObject errorResponse;
            errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResponse["code"] = 400;
            errorResponse["success"] = false;
            errorResponse["message"] = QString("设备不足，需要%1台，空闲设备%2台").arg(devices).arg(idleDevices.size());
            sendJsonResponse(clientSocket, 400, errorResponse);
            return;
        }

        // 4. 取前devices个空闲设备
        QList<QString> serialNumbers;
        for (int i = 0; i < devices; i++) {
            serialNumbers.append(idleDevices[i]);
            qDebug() << "分配设备" << i+1 << ":" << idleDevices[i];
        }

        // 5. 生成指令ID
        QString commandId = QString("CMD_HTTP_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz"));
        qDebug() << "生成指令ID:" << commandId;

        // 6. 处理时间格式
        QString start_time = startTimeStr.isEmpty() ? QDateTime::currentDateTime().toString("HH:mm:ss") : startTimeStr;
        QString end_time = endTimeStr.isEmpty() ? QDateTime::currentDateTime().addSecs(3600).toString("HH:mm:ss") : endTimeStr;

        // 7. 创建指令并保存到数据库
        SQL_CommandHistory cmd;
        cmd.commandId = commandId;
        cmd.status = "execing";
        cmd.action = action.isEmpty() ? "default" : action;
        cmd.sub_action = subAction.isEmpty() ? "default" : subAction;
        cmd.start_time = start_time;
        cmd.end_time = end_time;
        cmd.remark = remark;
        cmd.completeness = "0%";
        cmd.completed_url = "";
        cmd.total_tasks = devices;
        cmd.completed_tasks = 0;

        if (!dbManager->insertCommandHistory(cmd)) {
            qDebug() << "插入指令失败";
            QJsonObject errorResponse;
            errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
            errorResponse["code"] = 500;
            errorResponse["success"] = false;
            sendJsonResponse(clientSocket, 500, errorResponse);
            return;
        }

        qDebug() << "指令插入成功";

        // 8. 更新订单状态和指令ID
        //dbManager->updateOrderStatus(orderId, "execing");
        SQL_Order updatedOrder = existingOrder;
        updatedOrder.commandId = commandId;
        updatedOrder.status = "execing";
        dbManager->updateOrder(updatedOrder);

        // 9. 更新数据库中的设备状态
        for (const QString &serial : serialNumbers) {
            SQL_Device device = dbManager->getDeviceBySerialNumber(serial);
            if (!device.serial_number.isEmpty()) {
                device.device_status = "占用中";
                device.bound_user = user;
                device.bound_time = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
                dbManager->updateDevice(device);
                qDebug() << "更新设备状态:" << serial << "-> 占用中";
            }
        }

        // 10. 构建要发送的指令JSON（给设备）
        QJsonObject commandJson;

        // 构建data部分
        QJsonObject commandData;
        commandData["command_id"] = commandId;
        commandData["action"] = action.isEmpty() ? "default" : action;
        commandData["sub_action"] = subAction.isEmpty() ? "default" : subAction;
        commandData["start_time"] = start_time;
        commandData["end_time"] = end_time;
        commandData["remark"] = remark;

        // 添加设备序列号数组
        QJsonArray serialArray;
        for (const QString &serial : serialNumbers) {
            serialArray.append(serial);
        }
        commandData["serial_numbers"] = serialArray;

        // 构建完整JSON
        commandJson["data"] = commandData;
        commandJson["messageType"] = "command";
        commandJson["password"] = "securePassword123";
        commandJson["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        commandJson["username"] = "user123";

        qDebug() << "生成的指令JSON:";
        qDebug() << QJsonDocument(commandJson).toJson(QJsonDocument::Indented);

        // 11. 发射信号发送指令
        emit devCommadSend(commandJson);
        qDebug() << "已发射指令信号";

        // 12. 构建HTTP响应
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["code"] = 200;
        response["success"] = true;

        QJsonObject responseData;
        responseData["orderId"] = orderId;
        responseData["commandId"] = commandId;
        responseData["devices"] = devices;
        responseData["serial_numbers"] = serialArray;

        response["data"] = responseData;

        // 13. 发送HTTP响应
        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "处理完成，指令已发送";

        SQL_Order info =  dbManager->getOrderById(orderId);

        qDebug() << info.orderId;
        qDebug() << info.commandId;
        qDebug() << info.status;


    } catch (const std::exception& e) {
        qDebug() << "异常:" << e.what();
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["code"] = 500;
        errorResponse["success"] = false;
        sendJsonResponse(clientSocket, 500, errorResponse);
    } catch (...) {
        qDebug() << "未知异常";
        QJsonObject errorResponse;
        errorResponse["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        errorResponse["code"] = 500;
        errorResponse["success"] = false;
        sendJsonResponse(clientSocket, 500, errorResponse);
    }
}

// 辅助函数：生成随机设备序列号（可以根据需要扩展）
QStringList HttpServer::generateRandomSerialNumbers(int count)
{
    QStringList serialNumbers;
    QRandomGenerator *generator = QRandomGenerator::global();

    // 首先添加一些固定的示例序列号
    QStringList fixedSNs = {
        "SN999321", "SN999322", "SN999323",
        "SN999324", "SN999325", "SN999326",
        "YDAT250701000007"
    };

    // 添加固定序列号（不超过请求的数量）
    int fixedCount = qMin(count, fixedSNs.size());
    for (int i = 0; i < fixedCount; i++) {
        serialNumbers.append(fixedSNs[i]);
    }

    // 如果还需要更多，生成随机序列号
    for (int i = fixedCount; i < count; i++) {
        QString prefix;
        int randomType = generator->bounded(3);

        switch (randomType) {
        case 0:
            prefix = "SN";
            break;
        case 1:
            prefix = "YDAT";
            break;
        case 2:
            prefix = "DEV";
            break;
        default:
            prefix = "SN";
        }

        // 生成随机数字
        QString serialNumber;
        if (prefix == "YDAT") {
            // YDAT格式：YDAT + 12位数字
            serialNumber = QString("YDAT%1")
                               .arg(generator->bounded(100000000000, 999999999999), 12, 10, QChar('0'));
        } else {
            // SN或DEV格式：前缀 + 6位数字
            serialNumber = QString("%1%2")
                               .arg(prefix)
                               .arg(generator->bounded(100000, 999999));
        }

        serialNumbers.append(serialNumber);
    }

    return serialNumbers;
}
void HttpServer::handlePostMallOrderCheckout(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "====== 处理订单结算请求 ======";

    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    try {
        // 解析JSON请求体
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(body, &parseError);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            qDebug() << "JSON解析失败:" << parseError.errorString();
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "无效的JSON格式";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        // 检查是否有data字段
        if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
            qDebug() << "请求缺少data字段";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "请求缺少data字段";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject dataObj = jsonObj["data"].toObject();

        // 提取字段
        QString productId = dataObj.value("productId").toString();
        int quantity = dataObj.value("quantity").toInt();
        QString note = dataObj.value("note").toString();
        QString user = dataObj.value("user").toString();
        QString contactInfo = dataObj.value("contactInfo").toString();

        qDebug() << "订单结算请求参数:";
        qDebug() << "  商品ID:" << productId;
        qDebug() << "  数量:" << quantity;
        qDebug() << "  用户:" << user;
        qDebug() << "  联系方式:" << contactInfo;
        qDebug() << "  备注:" << (note.isEmpty() ? "无" : note);

        // 验证必填字段
        if (productId.isEmpty() || user.isEmpty() || contactInfo.isEmpty()) {
            qDebug() << "缺少必要字段: productId, user, contactInfo";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "商品ID、用户名和联系方式不能为空";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 验证数量
        if (quantity <= 0) {
            qDebug() << "数量无效:" << quantity;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "数量必须大于0";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 1. 从数据库中查找商品信息
        SQL_Product product = dbManager->getProductById(productId);
        if (product.productId.isEmpty()) {
            qDebug() << "商品不存在:" << productId;
            response["code"] = 404;
            response["success"] = false;
            response["message"] = "商品不存在";
            sendJsonResponse(clientSocket, 404, response);
            return;
        }

        // 检查商品状态
        if (product.status != "active" && product.status != "在售") {
            qDebug() << "商品状态不可售:" << product.status;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "商品暂时不可购买";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查库存
        if (product.stock < quantity) {
            qDebug() << "库存不足: 库存" << product.stock << "需求" << quantity;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "商品库存不足";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查最小起订量
        if (quantity < product.minOrder) {
            qDebug() << "未达到最小起订量: 最小" << product.minOrder << "实际" << quantity;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = QString("未达到最小起订量（最少%1个）").arg(product.minOrder);
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查最大订购量
        if (quantity > product.maxOrder) {
            qDebug() << "超过最大订购量: 最大" << product.maxOrder << "实际" << quantity;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = QString("超过最大订购量（最多%1个）").arg(product.maxOrder);
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查用户是否存在
        if (!dbManager->checkMallUserExists(user)) {
            qDebug() << "用户不存在:" << user;
            response["code"] = 404;
            response["success"] = false;
            response["message"] = "用户不存在";
            sendJsonResponse(clientSocket, 404, response);
            return;
        }

        // 计算价格
        double unitPrice = product.unitPrice;
        double totalPrice = unitPrice * quantity;

        // 生成订单ID和支付码
        QString orderId = generateOrderId();
        QString paymentCode = generatePaymentCode();
        QString createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
        QString expireTime = QDateTime::currentDateTime().addSecs(ORDER_EXPIRY_SECONDS).toString("yyyy-MM-dd HH:mm:ss");

        // 创建订单对象
        SQL_Order order;
        order.orderId = orderId;
        order.productId = productId;
        order.productName = product.productName;  // 添加商品名称
        order.unitPrice = unitPrice;
        order.quantity = quantity;
        order.totalPrice = totalPrice;
        order.note = note;
        order.user = user;
        order.contactInfo = contactInfo;
        order.status = "pending";  // 待支付状态
        order.createTime = createTime;
        order.updateTime = createTime;

        // 临时存储在待支付容器中
        pendingOrders.insert(orderId, order);

        // 2. 打印重要信息到调试台
        qDebug() << "=== 订单结算详情 ===";
        qDebug() << "商品信息:";
        qDebug() << "  商品ID:" << product.productId;
        qDebug() << "  商品名称:" << product.productName;
        qDebug() << "  单价:" << QString::number(product.unitPrice, 'f', 2) << "元";
        qDebug() << "  库存:" << product.stock;
        qDebug() << "  最小起订:" << product.minOrder;
        qDebug() << "  最大订购:" << product.maxOrder;
        qDebug() << "  分类:" << product.categoryName << "(" << product.categoryId << ")";
        qDebug() << "";
        qDebug() << "订单信息:";
        qDebug() << "  订单ID:" << orderId;
        qDebug() << "  用户:" << user;
        qDebug() << "  数量:" << quantity;
        qDebug() << "  单价:" << QString::number(unitPrice, 'f', 2) << "元";
        qDebug() << "  总价:" << QString::number(totalPrice, 'f', 2) << "元";
        qDebug() << "  联系方式:" << contactInfo;
        qDebug() << "  备注:" << (note.isEmpty() ? "无" : note);
        qDebug() << "  状态: pending (待支付)";
        qDebug() << "  创建时间:" << createTime;
        qDebug() << "  过期时间:" << expireTime;
        qDebug() << "";
        qDebug() << "支付信息:";
        qDebug() << "  支付码:" << paymentCode;
        qDebug() << "  待支付订单数:" << pendingOrders.size();
        qDebug() << "=========================";

        // 3. 构建成功响应
        response["code"] = 200;
        response["success"] = true;

        response["pay_link"] = "url:12345679";

        response["message"] = "订单创建成功";

        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "====== 订单结算完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "订单结算异常:" << e.what();
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "订单结算未知异常";
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
}
// 生成订单ID的辅助函数
QString HttpServer::generateOrderId()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString random = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    return "ORD_" + timestamp + "_" + random;
}

// 生成支付码的辅助函数
QString HttpServer::generatePaymentCode()
{
    const QString chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString paymentCode;
    QRandomGenerator *generator = QRandomGenerator::global();

    // 生成12位支付码
    for (int i = 0; i < 12; i++) {
        int index = generator->bounded(chars.length());
        paymentCode.append(chars.at(index));
        // 每4位加一个横杠，便于阅读
        if ((i + 1) % 4 == 0 && i < 11) {
            paymentCode.append("-");
        }
    }

    return paymentCode;
}
// 初始化订单超时定时器
void HttpServer::initOrderTimer()
{
    orderTimeoutTimer = new QTimer(this);
    connect(orderTimeoutTimer, &QTimer::timeout, this, &HttpServer::cleanupExpiredOrders);
    // 每5分钟检查一次过期订单
    orderTimeoutTimer->start(5 * 60 * 1000);
    qDebug() << "Order timeout timer initialized";
}

// 清理过期订单
void HttpServer::cleanupExpiredOrders()
{
    QDateTime currentTime = QDateTime::currentDateTime();
    QStringList expiredOrders;

    // 查找过期订单
    for (auto it = pendingOrders.begin(); it != pendingOrders.end(); ++it) {
        const QString &orderId = it.key();
        const SQL_Order &order = it.value();

        QDateTime createTime = QDateTime::fromString(order.createTime, "yyyy-MM-dd HH:mm:ss");
        if (createTime.addSecs(ORDER_EXPIRY_SECONDS) < currentTime) {
            expiredOrders.append(orderId);
            qDebug() << "Order expired:" << orderId << "Created:" << order.createTime;
        }
    }

    // 删除过期订单
    for (const QString &orderId : expiredOrders) {
        pendingOrders.remove(orderId);
    }

    if (!expiredOrders.isEmpty()) {
        qDebug() << "Cleaned up" << expiredOrders.size() << "expired orders";
        qDebug() << "Remaining pending orders:" << pendingOrders.size();
    }
}

// 根据订单ID获取待支付订单
SQL_Order HttpServer::getPendingOrder(const QString &orderId)
{
    if (pendingOrders.contains(orderId)) {
        return pendingOrders.value(orderId);
    }
    return SQL_Order(); // 返回空订单
}

// 移除待支付订单（支付成功后调用）
bool HttpServer::removePendingOrder(const QString &orderId)
{
    if (pendingOrders.contains(orderId)) {
        pendingOrders.remove(orderId);
        qDebug() << "Pending order removed:" << orderId;
        qDebug() << "Remaining pending orders:" << pendingOrders.size();
        return true;
    }
    return false;
}

// 获取用户的待支付订单列表
QList<SQL_Order> HttpServer::getUserPendingOrders(const QString &username)
{
    QList<SQL_Order> userOrders;

    for (auto it = pendingOrders.begin(); it != pendingOrders.end(); ++it) {
        const SQL_Order &order = it.value();
        if (order.user == username) {
            userOrders.append(order);
        }
    }

    qDebug() << "Found" << userOrders.size() << "pending orders for user:" << username;
    return userOrders;
}

// 支付成功后处理订单（将待支付订单保存到数据库）
bool HttpServer::completeOrderPayment(const QString &orderId)
{
    if (!pendingOrders.contains(orderId)) {
        qDebug() << "Order not found in pending orders:" << orderId;
        return false;
    }

    SQL_Order order = pendingOrders.value(orderId);

    // 更新订单状态为已支付
    order.status = "paid";
    order.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

    // 将订单保存到数据库
    if (!dbManager || !dbManager->insertOrder(order)) {
        qDebug() << "Failed to save order to database:" << orderId;
        return false;
    }

    // 从待支付容器中移除
    pendingOrders.remove(orderId);

    qDebug() << "Order payment completed:";
    qDebug() << "  Order ID:" << orderId;
    qDebug() << "  User:" << order.user;
    qDebug() << "  Amount:" << order.totalPrice;
    qDebug() << "  Saved to database: yes";
    qDebug() << "  Remaining pending orders:" << pendingOrders.size();

    return true;
}
void HttpServer::handlePostMallSendwithdraw(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "====== 处理提现请求 ======";

    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    try {
        // 解析JSON请求体
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(body, &parseError);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            qDebug() << "JSON解析失败:" << parseError.errorString();
            response["success"] = false;
            response["message"] = "无效的JSON格式";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        // 检查是否有data字段
        if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
            qDebug() << "请求缺少data字段";
            response["success"] = false;
            response["message"] = "请求缺少data字段";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject dataObj = jsonObj["data"].toObject();

        // 提取字段
        QString username = dataObj.value("username").toString();
        QString password = dataObj.value("passwd").toString();
        QString alipay = dataObj.value("alipay").toString();
        double withdrawAmount = dataObj.value("amount").toDouble();

        qDebug() << "提现请求参数:";
        qDebug() << "  用户名:" << username;
        qDebug() << "  支付宝账号:" << alipay;
        qDebug() << "  提现金额:" << QString::number(withdrawAmount, 'f', 2) << "元";

        // 验证必填字段
        if (username.isEmpty() || password.isEmpty() || alipay.isEmpty()) {
            qDebug() << "缺少必要字段: username, password, alipay";
            response["success"] = false;
            response["message"] = "用户名、密码和支付宝账号不能为空";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 验证支付宝账号格式
        if (alipay.length() < 5) {
            qDebug() << "支付宝账号格式错误:" << alipay;
            response["success"] = false;
            response["message"] = "支付宝账号格式不正确";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查用户是否存在
        if (!dbManager->checkMallUserExists(username)) {
            qDebug() << "用户不存在:" << username;
            response["success"] = false;
            response["message"] = "用户不存在";
            sendJsonResponse(clientSocket, 404, response);
            return;
        }

        // 验证用户密码
        if (!dbManager->validateMallUserLogin(username, password)) {
            qDebug() << "密码错误:" << username;
            response["success"] = false;
            response["message"] = "密码错误";
            sendJsonResponse(clientSocket, 401, response);
            return;
        }

        // 获取用户信息
        SQL_MallUser user = dbManager->getMallUserByUsername(username);

        // 如果没有指定提现金额，默认提现全部余额
        if (withdrawAmount <= 0) {
            withdrawAmount = user.balance;
            qDebug() << "未指定金额，使用全部余额:" << withdrawAmount << "元";
        }

        // 验证提现金额
        if (withdrawAmount <= 0) {
            qDebug() << "提现金额必须大于0，当前:" << withdrawAmount;
            response["success"] = false;
            response["message"] = "提现金额必须大于0";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查余额是否足够
        if (user.balance < withdrawAmount) {
            qDebug() << "余额不足: 当前余额" << user.balance << "元，需提现" << withdrawAmount << "元";
            response["success"] = false;
            response["message"] = "余额不足";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查最小提现金额 (10元)
        double minWithdrawAmount = 10.0;
        if (withdrawAmount < minWithdrawAmount) {
            qDebug() << "未达到最小提现金额: 最少" << minWithdrawAmount << "元，实际" << withdrawAmount << "元";
            response["success"] = false;
            response["message"] = QString("提现金额不能低于%1元").arg(minWithdrawAmount);
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查最大提现金额 (50000元)
        double maxWithdrawAmount = 50000.0;
        if (withdrawAmount > maxWithdrawAmount) {
            qDebug() << "超过最大提现金额: 最多" << maxWithdrawAmount << "元，实际" << withdrawAmount << "元";
            response["success"] = false;
            response["message"] = QString("单次提现金额不能超过%1元").arg(maxWithdrawAmount);
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 打印提现详情
        qDebug() << "=== 提现详情 ===";
        qDebug() << "用户名:" << username;
        qDebug() << "用户等级:" << user.userLevel;
        qDebug() << "当前余额:" << QString::number(user.balance, 'f', 2) << "元";
        qDebug() << "提现金额:" << QString::number(withdrawAmount, 'f', 2) << "元";
        qDebug() << "支付宝账号:" << alipay;
        qDebug() << "提现后余额:" << QString::number(user.balance - withdrawAmount, 'f', 2) << "元";

        // 扣除余额
        if (!dbManager->updateMallUserBalance(username, -withdrawAmount)) {
            qDebug() << "扣除余额失败";
            response["success"] = false;
            response["message"] = "扣除余额失败";
            sendJsonResponse(clientSocket, 500, response);
            return;
        }

        // 创建提现记录ID
        QString withdrawId = generateWithdrawId();
        QString withdrawTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // 创建提现记录
        bool withdrawRecordCreated = dbManager->createWithdrawRecord(withdrawId, username, withdrawAmount, alipay, "提现申请");

        if (!withdrawRecordCreated) {
            // 如果创建记录失败，回滚余额扣除
            dbManager->updateMallUserBalance(username, withdrawAmount);
            qDebug() << "创建提现记录失败，已回滚余额";
            response["success"] = false;
            response["message"] = "创建提现记录失败";
            sendJsonResponse(clientSocket, 500, response);
            return;
        }

        // 获取更新后的用户信息
        SQL_MallUser updatedUser = dbManager->getMallUserByUsername(username);

        // 打印成功信息
        qDebug() << "=== 提现成功 ===";
        qDebug() << "提现ID:" << withdrawId;
        qDebug() << "提现时间:" << withdrawTime;
        qDebug() << "原余额:" << QString::number(user.balance, 'f', 2) << "元";
        qDebug() << "新余额:" << QString::number(updatedUser.balance, 'f', 2) << "元";
        qDebug() << "提现状态: 待处理";
        qDebug() << "到账时间: 预计1-3个工作日";
        qDebug() << "=========================";

        // 构建成功响应
        response["success"] = true;
        response["message"] = "提现申请提交成功";

        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "====== 提现处理完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "提现处理异常:" << e.what();
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "提现处理未知异常";
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
}
// 生成提现ID的辅助函数
QString HttpServer::generateWithdrawId()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString random = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    return "WD_" + timestamp + "_" + random;
}

void HttpServer::handlePostMallPasswdReset(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "====== 处理重置密码请求 ======";

    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    try {
        // 解析JSON请求体
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(body, &parseError);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            qDebug() << "JSON解析失败:" << parseError.errorString();
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "无效的JSON格式";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        // 检查是否有data字段
        if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
            qDebug() << "请求缺少data字段";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "请求缺少data字段";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject dataObj = jsonObj["data"].toObject();

        // 提取字段
        QString username = dataObj.value("username").toString();
        QString password = dataObj.value("password").toString();
        QString email = dataObj.value("email").toString();
        QString verifyCode = dataObj.value("Verifycode").toString();

        qDebug() << "重置密码请求参数:";
        qDebug() << "  用户名:" << username;
        qDebug() << "  新密码:" << password;
        qDebug() << "  邮箱:" << email;
        qDebug() << "  验证码:" << verifyCode;

        // 验证必填字段
        if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
            qDebug() << "缺少必要字段: username, password, email";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "用户名、密码和邮箱不能为空";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 密码强度验证：至少6位
        if (password.length() < 6) {
            qDebug() << "密码太短:" << password.length() << "位";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "密码至少需要6位";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查用户是否存在
        if (!dbManager->checkMallUserExists(username)) {
            qDebug() << "用户不存在:" << username;
            response["code"] = 404;
            response["success"] = false;
            response["message"] = "用户不存在";
            sendJsonResponse(clientSocket, 404, response);
            return;
        }

        // 获取用户信息，验证邮箱是否匹配
        SQL_MallUser user = dbManager->getMallUserByUsername(username);

        if (user.email != email) {
            qDebug() << "邮箱不匹配:";
            qDebug() << "  提供的邮箱:" << email;
            qDebug() << "  注册的邮箱:" << user.email;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "邮箱和账号不匹配";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 打印当前容器内所有验证码（调试用）
        qDebug() << "=== 当前容器内所有验证码 ===";
        {
            QMutexLocker locker(&codeMutex);
            int count = 0;
            for (const auto &vc : verificationCodes) {
                qDebug() << QString("  [%1] 用户名: %2, 验证码: %3, 邮箱: %4, 有效: %5, 过期时间: %6")
                    .arg(++count)
                    .arg(vc.username)
                    .arg(vc.code)
                    .arg(vc.email)
                    .arg(vc.isValid() ? "是" : "否")
                    .arg(vc.expireTime.toString("HH:mm:ss"));
            }
            qDebug() << "=== 总共" << count << "个验证码 ===";
        }

        // 验证码验证 - 从容器中查找并验证
        bool verifySuccess = false;
        {
            QMutexLocker locker(&codeMutex);

            for (int i = 0; i < verificationCodes.size(); i++) {
                VerificationCode &vc = verificationCodes[i];

                if (vc.username == username && vc.code == verifyCode) {
                    if (vc.isValid()) {
                        qDebug() << "验证码验证成功:";
                        qDebug() << "  用户名:" << vc.username;
                        qDebug() << "  验证码:" << vc.code;
                        qDebug() << "  邮箱:" << vc.email;
                        qDebug() << "  创建时间:" << vc.createTime.toString("HH:mm:ss");
                        qDebug() << "  过期时间:" << vc.expireTime.toString("HH:mm:ss");

                        // 标记为已验证
                        vc.verified = true;
                        verifySuccess = true;

                        // 清理该用户的验证码（可以移除或保留已验证状态）
                        verificationCodes.remove(i);
                        qDebug() << "已从容器中移除该验证码";

                        break;
                    } else {
                        qDebug() << "验证码无效或已过期:";
                        qDebug() << "  验证码:" << vc.code;
                        qDebug() << "  是否已验证:" << vc.verified;
                        qDebug() << "  是否过期:" << vc.isExpired();
                    }
                }
            }
        }

        if (!verifySuccess) {
            qDebug() << "验证码错误或不存在:";
            qDebug() << "  用户名:" << username;
            qDebug() << "  验证码:" << verifyCode;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "验证码错误或已过期";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 检查新密码是否与旧密码相同
        if (user.password == password) {
            qDebug() << "新密码与旧密码相同";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "新密码不能与旧密码相同";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 打印详细信息
        qDebug() << "=== 重置密码详情 ===";
        qDebug() << "用户名:" << username;
        qDebug() << "邮箱:" << email;
        qDebug() << "验证码:" << verifyCode;
        qDebug() << "旧密码:" << user.password;
        qDebug() << "新密码:" << password;
        qDebug() << "用户等级:" << user.userLevel;
        qDebug() << "余额:" << user.balance;
        qDebug() << "积分:" << user.points;
        qDebug() << "邀请码:" << user.inviteCode;
        qDebug() << "创建时间:" << user.createTime;

        // 更新密码
        if (!dbManager->updateMallUserPassword(username, password)) {
            qDebug() << "更新密码失败";
            response["code"] = 500;
            response["success"] = false;
            response["message"] = "重置密码失败";
            sendJsonResponse(clientSocket, 500, response);
            return;
        }

        // 获取更新后的用户信息
        SQL_MallUser updatedUser = dbManager->getMallUserByUsername(username);

        qDebug() << "=== 密码更新成功 ===";
        qDebug() << "更新前密码:" << user.password;
        qDebug() << "更新后密码:" << updatedUser.password;
        qDebug() << "用户其他信息:";
        qDebug() << "  用户等级:" << updatedUser.userLevel;
        qDebug() << "  余额:" << updatedUser.balance;
        qDebug() << "  积分:" << updatedUser.points;
        qDebug() << "  邀请码:" << updatedUser.inviteCode;
        qDebug() << "=========================";

        // 构建成功响应
        response["code"] = 200;
        response["success"] = true;
        response["message"] = "密码重置成功";

        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "====== 密码重置完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "重置密码异常:" << e.what();
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "重置密码未知异常";
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
}


void HttpServer::printVerificationCodes()
{
    QMutexLocker locker(&codeMutex);

    qDebug() << "=== 验证码容器状态 ===";
    qDebug() << "总数量:" << verificationCodes.size();

    if (verificationCodes.isEmpty()) {
        qDebug() << "容器为空";
        return;
    }

    int validCount = 0;
    int expiredCount = 0;
    int verifiedCount = 0;

    for (int i = 0; i < verificationCodes.size(); i++) {
        const VerificationCode &vc = verificationCodes[i];

        QString status;
        if (vc.verified) {
            status = "已验证";
            verifiedCount++;
        } else if (vc.isExpired()) {
            status = "已过期";
            expiredCount++;
        } else {
            status = "有效";
            validCount++;
        }

        qDebug() << QString("  [%1] 用户名: %2, 验证码: %3, 邮箱: %4, 状态: %5, 过期时间: %6")
                        .arg(i + 1)
                        .arg(vc.username)
                        .arg(vc.code)
                        .arg(vc.email)
                        .arg(status)
                        .arg(vc.expireTime.toString("HH:mm:ss"));
    }

    qDebug() << "=== 统计 ===";
    qDebug() << "有效验证码:" << validCount;
    qDebug() << "已验证验证码:" << verifiedCount;
    qDebug() << "已过期验证码:" << expiredCount;
    qDebug() << "==================";
}
void HttpServer::handlePostMallRegist(QTcpSocket *clientSocket, const QByteArray &body)
{
    qDebug() << "====== 处理商城注册请求 ======";

    QJsonObject response;
    response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

    try {
        // 解析JSON请求体
        QJsonParseError parseError;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(body, &parseError);
        if (jsonDoc.isNull() || !jsonDoc.isObject()) {
            qDebug() << "JSON解析失败:" << parseError.errorString();
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "无效的JSON格式";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        QJsonObject jsonObj = jsonDoc.object();

        // 检查是否有data字段
        if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
            qDebug() << "请求缺少data字段";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "请求缺少data字段";
            sendJsonResponse(clientSocket, 400, response);
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

        // 打印接收到的参数
        qDebug() << "注册参数:";
        qDebug() << "  用户名:" << username;
        qDebug() << "  邮箱:" << email;
        qDebug() << "  邀请码:" << (inviteCode.isEmpty() ? "无" : inviteCode);
        qDebug() << "  手机号:" << (phone.isEmpty() ? "未提供" : phone);

        // 验证必填字段
        if (username.isEmpty() || password.isEmpty() || email.isEmpty()) {
            qDebug() << "缺少必要字段: username, password, email";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "用户名、密码和邮箱不能为空";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 用户名验证：3-20位，只允许字母、数字、下划线
        QRegularExpression usernameRegex("^[a-zA-Z0-9_]{3,20}$");
        if (!usernameRegex.match(username).hasMatch()) {
            qDebug() << "用户名格式错误:" << username;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "用户名必须为3-20位字母、数字或下划线";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 密码强度验证：至少6位
        if (password.length() < 6) {
            qDebug() << "密码太短:" << password.length() << "位";
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "密码至少需要6位";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 邮箱格式验证
        QRegularExpression emailRegex(R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)");
        if (!emailRegex.match(email).hasMatch()) {
            qDebug() << "邮箱格式错误:" << email;
            response["code"] = 400;
            response["success"] = false;
            response["message"] = "邮箱格式不正确";
            sendJsonResponse(clientSocket, 400, response);
            return;
        }

        // 如果提供了手机号，验证格式
        if (!phone.isEmpty()) {
            QRegularExpression phoneRegex(R"(^1[3-9]\d{9}$)");
            if (!phoneRegex.match(phone).hasMatch()) {
                qDebug() << "手机号格式错误:" << phone;
                response["code"] = 400;
                response["success"] = false;
                response["message"] = "手机号格式不正确";
                sendJsonResponse(clientSocket, 400, response);
                return;
            }
        }

        // 检查用户名是否已存在
        if (dbManager->checkMallUserExists(username)) {
            qDebug() << "用户名已存在:" << username;
            response["code"] = 409;
            response["success"] = false;
            response["message"] = "用户名已被注册";
            sendJsonResponse(clientSocket, 409, response);
            return;
        }

        // 检查邮箱是否已存在
        if (dbManager->checkEmailExists(email)) {
            qDebug() << "邮箱已存在:" << email;
            response["code"] = 409;
            response["success"] = false;
            response["message"] = "邮箱已被注册";
            sendJsonResponse(clientSocket, 409, response);
            return;
        }

        // 如果提供了手机号，检查手机号是否已存在
        if (!phone.isEmpty() && dbManager->checkPhoneExists(phone)) {
            qDebug() << "手机号已存在:" << phone;
            response["code"] = 409;
            response["success"] = false;
            response["message"] = "手机号已被注册";
            sendJsonResponse(clientSocket, 409, response);
            return;
        }

        // 邀请码相关逻辑
        QString inviterUsername = "";

        if (!inviteCode.isEmpty()) {
            if (!dbManager->checkInviteCodeExists(inviteCode)) {
                qDebug() << "邀请码无效:" << inviteCode;
                response["code"] = 400;
                response["success"] = false;
                response["message"] = "邀请码无效";
                sendJsonResponse(clientSocket, 400, response);
                return;
            }

            // 查找拥有该邀请码的用户
            SQL_MallUser inviterUser = dbManager->getMallUserByInviteCode(inviteCode);
            if (inviterUser.username.isEmpty()) {
                qDebug() << "找不到邀请码对应的用户:" << inviteCode;
                response["code"] = 400;
                response["success"] = false;
                response["message"] = "邀请码无效";
                sendJsonResponse(clientSocket, 400, response);
                return;
            }

            inviterUsername = inviterUser.username;
            qDebug() << "找到邀请人:" << inviterUsername << "邀请码:" << inviteCode;
        }

        // 为新用户生成邀请码
        QString userInviteCode = generateInviteCode();

        // 确保邀请码唯一
        int retryCount = 0;
        while (dbManager->checkInviteCodeExists(userInviteCode) && retryCount < 10) {
            userInviteCode = generateInviteCode();
            retryCount++;
        }

        if (retryCount >= 10) {
            qDebug() << "生成邀请码失败，重试次数过多";
            response["code"] = 500;
            response["success"] = false;
            response["message"] = "系统错误，请稍后重试";
            sendJsonResponse(clientSocket, 500, response);
            return;
        }

        // 创建商城用户对象
        SQL_MallUser newUser;
        newUser.username = username;
        newUser.password = password;  // 明文保存
        newUser.email = email;
        newUser.phone = phone;
        newUser.inviteCode = userInviteCode;
        newUser.inviterUsername = inviterUsername;
        newUser.userLevel = 1;
        newUser.balance = 0.0;
        newUser.points = 0;
        newUser.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");

        // 插入用户到数据库
        if (!dbManager->insertMallUser(newUser)) {
            qDebug() << "插入用户到数据库失败";
            response["code"] = 500;
            response["success"] = false;
            response["message"] = "注册失败，请稍后重试";
            sendJsonResponse(clientSocket, 500, response);
            return;
        }

        // 注册成功，打印详细信息
        qDebug() << "=== 注册成功 ===";
        qDebug() << "用户名:" << username;
        qDebug() << "邮箱:" << email;
        qDebug() << "手机号:" << (phone.isEmpty() ? "未设置" : phone);
        qDebug() << "用户等级:" << newUser.userLevel;
        qDebug() << "余额:" << newUser.balance;
        qDebug() << "积分:" << newUser.points;
        qDebug() << "生成的邀请码:" << userInviteCode;
        qDebug() << "使用的邀请码:" << (inviteCode.isEmpty() ? "无" : inviteCode);
        qDebug() << "邀请人:" << (inviterUsername.isEmpty() ? "无" : inviterUsername);
        qDebug() << "注册时间:" << newUser.createTime;
        qDebug() << "密码:" << password;
        qDebug() << "=========================";

        // 构建成功响应
        response["code"] = 200;
        response["success"] = true;
        response["message"] = "通过";

        sendJsonResponse(clientSocket, 200, response);

        qDebug() << "====== 注册处理完成 ======";

    } catch (const std::exception& e) {
        qDebug() << "注册处理异常:" << e.what();
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "注册处理未知异常";
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
}
void HttpServer::handlePostDeviceCommand(QTcpSocket *clientSocket, const QByteArray &body) {
    qDebug() << "[POST /device/command] body =" << body;
    QJsonDocument doc = QJsonDocument::fromJson(body);
    if (!doc.isNull()) {
        QJsonObject jsonObj = doc.object();

        // 生成指令ID
        QString commandId = QString("CMD_HTTP_%1").arg(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_zzz"));

        // 确保data字段存在
        QJsonObject dataObj = jsonObj["data"].toObject();

        // 添加command_id到data中
        dataObj["command_id"] = commandId;

        // 计算总任务数
        int totalTasks = 0;
        if (dataObj.contains("serial_numbers")) {
            QJsonArray serialArray = dataObj["serial_numbers"].toArray();
            totalTasks = serialArray.size();
        }

        // 插入数据库
        SQL_CommandHistory cmd;
        cmd.commandId = commandId;
        cmd.status = "execing";
        cmd.action = dataObj["action"].toString();
        cmd.sub_action = dataObj["sub_action"].toString();
        cmd.start_time = dataObj["start_time"].toString();
        cmd.end_time = dataObj["end_time"].toString();
        cmd.remark = dataObj["remark"].toString();
        cmd.completeness = "0%";
        cmd.completed_url = "";
        cmd.total_tasks = totalTasks;
        cmd.completed_tasks = 0;

        // 这里需要数据库管理器的实例，你可能需要传入或全局获取
        if (dbManager) {
            if (dbManager->insertCommandHistory(cmd)) {
                qDebug() << "HTTP指令已插入数据库，ID:" << commandId;
            } else {
                qDebug() << "HTTP指令插入数据库失败";
            }
        }

        // 更新外层JSON
        jsonObj["data"] = dataObj;

        qDebug() << "添加command_id后的JSON:" << jsonObj;

        emit devCommadSend(jsonObj);
    }
    sendResponse(clientSocket, "{\"code\":200,\"msg\":\"POST /device/command success\"}");
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
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["code"] = 400;
        response["success"] = false;
        response["message"] = "Invalid JSON format";
        sendJsonResponse(clientSocket, 400, response);
        return;
    }

    QJsonObject jsonObj = doc.object();

    // 检查是否有data字段
    if (!jsonObj.contains("data") || !jsonObj["data"].isObject()) {
        qDebug() << "Missing data field";
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["code"] = 400;
        response["success"] = false;
        response["message"] = "Missing data field";
        sendJsonResponse(clientSocket, 400, response);
        return;
    }

    QJsonObject dataObj = jsonObj["data"].toObject();

    // 获取用户名和密码
    QString username = dataObj["username"].toString();
    QString password = dataObj["password"].toString();

    qDebug() << "=== 登录尝试 ===";
    qDebug() << "用户名:" << username;
    qDebug() << "密码:" << password;

    // 检查是否为空
    if (username.isEmpty() || password.isEmpty()) {
        qDebug() << "用户名或密码为空";
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["code"] = 400;
        response["success"] = false;
        response["message"] = "用户名和密码不能为空";
        sendJsonResponse(clientSocket, 400, response);
        return;
    }

    try {
        // 验证用户登录
        bool loginSuccess = dbManager->validateMallUserLogin(username, password);

        // 构建响应
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);

        if (loginSuccess) {
            // 获取用户详细信息（用于调试输出）
            SQL_MallUser user = dbManager->getMallUserByUsername(username);

            // 生成token
            QString token = generateToken(username);
            dbManager->saveUserToken(username, token);
            // 更新最后登录时间
            dbManager->updateMallUserLastLogin(username);

            // 调试输出用户详细信息
            qDebug() << "=== 登录成功 ===";
            qDebug() << "用户:" << user.username;
            qDebug() << "邮箱:" << user.email;
            qDebug() << "手机:" << user.phone;
            qDebug() << "余额:" << user.balance;
            qDebug() << "积分:" << user.points;
            qDebug() << "等级:" << user.userLevel;
            qDebug() << "邀请码:" << user.inviteCode;
            qDebug() << "邀请人:" << user.inviterUsername;
            qDebug() << "创建时间:" << user.createTime;
            qDebug() << "最后登录:" << user.lastLoginTime;
            qDebug() << "生成的Token:" << token;
            qDebug() << "密码:" << user.password;
            qDebug() << "=========================";

            // 构建成功响应
            response["code"] = 200;
            response["success"] = true;
            response["message"] = "通过";
            response["token"] = token;

        } else {
            // 登录失败时也获取一些调试信息
            bool userExists = dbManager->checkMallUserExists(username);

            qDebug() << "=== 登录失败 ===";
            qDebug() << "用户是否存在:" << (userExists ? "是" : "否");
            qDebug() << "尝试的用户名:" << username;
            qDebug() << "尝试的密码:" << password;

            if (userExists) {
                // 获取用户信息用于调试
                SQL_MallUser user = dbManager->getMallUserByUsername(username);
                qDebug() << "数据库中的密码:" << user.password;
              //  qDebug() << "用户状态:" << user.status;
                qDebug() << "=========================";
            }

            // 构建失败响应
            response["code"] = 401;
            response["success"] = false;
            response["message"] = "账号或者密码失败";
        }

        // 发送响应
        sendJsonResponse(clientSocket, response["code"].toInt(), response);

    } catch (const std::exception& e) {
        qDebug() << "登录处理异常:" << e.what();
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    } catch (...) {
        qDebug() << "登录处理未知异常";
        QJsonObject response;
        response["timestamp"] = QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
        response["code"] = 500;
        response["success"] = false;
        response["message"] = "未知服务器错误";
        sendJsonResponse(clientSocket, 500, response);
    }
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
        QStringList statusOptions = {"pending", "paid","execing", "completed", "cancelled"};
        order.status = statusOptions[QRandomGenerator::global()->bounded(statusOptions.size())];

        order.note = QString("测试订单%1，备注信息").arg(i);
        order.createTime = now.addDays(-QRandomGenerator::global()->bounded(30)).toString("yyyy-MM-dd HH:mm:ss");

        orders.append(order);

        // 插入数据库
        dbManager->insertOrder(order);
    }


}
void HttpServer::handleCreateProductDebug()
{
    qDebug() << "====== 创建简单测试商品 ======";

    // 获取当前工作目录
    QDir currentDir = QDir::current();

    // 创建Products目录
    QString productsDirPath = currentDir.filePath("Products");
    QDir productsDir(productsDirPath);

    if (!productsDir.exists()) {
        if (productsDir.mkpath(".")) {
            qDebug() << "✅ 创建Products目录: " << productsDirPath;
        } else {
            qDebug() << "❌ 无法创建Products目录";
            return;
        }
    }

    // 创建3个简单的测试商品
    QList<SQL_Product> testProducts;

    // 商品1：手机
    {
        SQL_Product p;
        p.productId = "PHONE001";
        p.productName = "华为手机";
        p.categoryId = "ELECTRONICS";
        p.categoryName = "电子产品";
        p.unitPrice = 5000.00;  // 修正价格
        p.stock = 50;
        p.minOrder = 1;
        p.maxOrder = 5;
        p.status = "active";
        p.action = "sale";
        p.subaction = "new";
        p.description = "遥遥领先 全球第一";

        // 创建商品目录并设置图片路径
        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            // 保存相对路径到imageUrl
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    {
        SQL_Product p;
        p.productId = "PHONE002";
        p.productName = "小米手机";
        p.categoryId = "ELECTRONICS";
        p.categoryName = "电子产品";
        p.unitPrice = 3000.00;  // 修正价格
        p.stock = 50;
        p.minOrder = 1;
        p.maxOrder = 5;
        p.status = "active";
        p.action = "sale";
        p.subaction = "new";
        p.description = "全银河第一";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    {
        SQL_Product p;
        p.productId = "PHONE003";
        p.productName = "vivo手机";
        p.categoryId = "ELECTRONICS";
        p.categoryName = "电子产品";
        p.unitPrice = 2999.00;
        p.stock = 50;
        p.minOrder = 1;
        p.maxOrder = 5;
        p.status = "active";
        p.action = "sale";
        p.subaction = "new";
        p.description = "全宇宙第一";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    // 商品2：衣服
    {
        SQL_Product p;
        p.productId = "CLOTH001";
        p.productName = "美特斯邦威T恤";
        p.categoryId = "CLOTHING";
        p.categoryName = "服装";
        p.unitPrice = 199.00;
        p.stock = 100;
        p.minOrder = 2;
        p.maxOrder = 20;
        p.status = "active";
        p.action = "sale";
        p.subaction = "hot";
        p.description = "这是一件测试T恤";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    {
        SQL_Product p;
        p.productId = "CLOTH002";
        p.productName = "李宁运动服";
        p.categoryId = "CLOTHING";
        p.categoryName = "服装";
        p.unitPrice = 499.00;  // 修正价格
        p.stock = 100;
        p.minOrder = 2;
        p.maxOrder = 20;
        p.status = "active";
        p.action = "sale";
        p.subaction = "hot";
        p.description = "国货之光运动服";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    {
        SQL_Product p;
        p.productId = "CLOTH003";
        p.productName = "耐克运动鞋";
        p.categoryId = "CLOTHING";
        p.categoryName = "服装";
        p.unitPrice = 899.00;  // 修正价格
        p.stock = 100;
        p.minOrder = 2;
        p.maxOrder = 20;
        p.status = "active";
        p.action = "sale";
        p.subaction = "hot";
        p.description = "经典运动鞋款";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    // 商品3：食品
    {
        SQL_Product p;
        p.productId = "FOOD001";
        p.productName = "三只松鼠坚果";
        p.categoryId = "FOOD";
        p.categoryName = "食品";
        p.unitPrice = 39.90;
        p.stock = 200;
        p.minOrder = 1;
        p.maxOrder = 100;
        p.status = "active";
        p.action = "sale";
        p.subaction = "normal";
        p.description = "这是一包测试零食";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    {
        SQL_Product p;
        p.productId = "FOOD002";
        p.productName = "九只松鼠坚果";
        p.categoryId = "FOOD";
        p.categoryName = "食品";
        p.unitPrice = 49.90;
        p.stock = 20;
        p.minOrder = 1;
        p.maxOrder = 100;
        p.status = "active";
        p.action = "sale";
        p.subaction = "normal";
        p.description = "这是另一包测试零食";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }
    {
        SQL_Product p;
        p.productId = "FOOD003";
        p.productName = "十只松鼠坚果";
        p.categoryId = "FOOD";
        p.categoryName = "食品";
        p.unitPrice = 59.90;
        p.stock = 200;
        p.minOrder = 1;
        p.maxOrder = 100;
        p.status = "active";
        p.action = "sale";
        p.subaction = "normal";
        p.description = "这是高级版测试零食";

        QString categoryProductDir = QString("%1_%2").arg(p.categoryId).arg(p.productId);
        QString productDirPath = productsDir.filePath(categoryProductDir);
        QDir productDir(productDirPath);
        if (productDir.mkpath(".")) {
            p.imageUrl = QString("Products/%1/").arg(categoryProductDir);
            qDebug() << "✅ 创建商品目录: " << productDirPath;
        } else {
            p.imageUrl = "";
            qDebug() << "❌ 无法创建商品目录: " << productDirPath;
        }

        testProducts.append(p);
    }

    // 插入商品
    int successCount = 0;
    for (const SQL_Product &product : testProducts) {
        qDebug() << "插入商品: " << product.productName
                 << " (" << product.productId << ")"
                 << " 价格: " << product.unitPrice << "元"
                 << " 图片路径: " << product.imageUrl;

        if (dbManager->insertProduct(product)) {
            qDebug() << "✅ 成功";
            successCount++;
        } else {
            qDebug() << "❌ 失败";
        }
    }

    qDebug() << "====== 完成 ======";
    qDebug() << "成功插入: " << successCount << " 个商品";
    qDebug() << "失败: " << (testProducts.size() - successCount) << " 个商品";

    // 简单验证
    QList<SQL_Product> allProducts = dbManager->getAllProducts();
    qDebug() << "数据库中现有商品总数: " << allProducts.size();

    // 显示目录结构
    qDebug() << "目录结构:";
    qDebug() << "当前目录: " << currentDir.absolutePath();
    qDebug() << "Products目录: " << productsDirPath;

    // 列出所有创建的商品目录
    QStringList productDirs = productsDir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug() << "创建的商品目录数量: " << productDirs.size();
    for (const QString &dir : productDirs) {
        qDebug() << "  - " << dir;
    }
}
