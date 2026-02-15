#include "ai_bragger.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>

AI_bragger::AI_bragger()
{
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &AI_bragger::checkProgramList);
    // aiCooldownTimer = new QTimer(this);
    // aiCooldownTimer->setSingleShot(true);
    // connect(aiCooldownTimer, &QTimer::timeout, this, &AI_bragger::resetAIState);
}

AI_bragger::~AI_bragger()
{
    // 清理所有语音识别器相关代码已移除
}

void AI_bragger::sethostpath(QString ip, QString port)
{
    host_ip = ip;
    host_port = port;
}

void AI_bragger::run()
{
    qDebug() << "AI_bragger线程启动";

    while(!isInterruptionRequested()) {
        checkProgramList();
        checkAndDistributeBraggers();
        checkCoolDown();  // 冷却检查
        checkDeviceStatusForPrograms();// 设备状态检查

        QThread::sleep(1);
    }

    qDebug() << "AI_bragger线程结束";
}

QString AI_bragger::generateRandomSuffix()
{
    const QString chars = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    QString suffix;
    for(int i = 0; i < 6; ++i) {
        int index = QRandomGenerator::global()->bounded(chars.length());
        suffix.append(chars.at(index));
    }
    return suffix;
}

QString AI_bragger::getCurrentTimestamp()
{
    return QDateTime::currentDateTimeUtc().toString(Qt::ISODate);
}

void AI_bragger::onProgramInfoGenerated(const ProgramInfo &programInfo)
{
    qDebug() << "====== 开始处理节目信息 ======";
    qDebug() << "收到节目单信息:";
    qDebug() << "  commandId:" << programInfo.commandId;
    qDebug() << "  programName:" << programInfo.programName;
    qDebug() << "  rtspurl:" << programInfo.rtspurl;
    qDebug() << "  keepTime:" << programInfo.keepTime;
    qDebug() << "  startTime是否有效:" << programInfo.startTime.isValid();
    qDebug() << "  isStreaming:" << programInfo.isStreaming;

    qDebug() << "  deviceList内容:";
    for(int i = 0; i < programInfo.deviceList.size(); ++i) {
        qDebug() << "    [" << i << "]" << programInfo.deviceList[i];
    }
    qDebug() << "  deviceList大小:" << programInfo.deviceList.size();

    // 创建新的节目信息，初始化状态
    ProgramInfo newProgram = programInfo;
    newProgram.startTime = QDateTime::currentDateTime();
    newProgram.isStreaming = false;
    newProgram.isListen = false;
    newProgram.isGenerating = false;
    newProgram.isCoolingDown = false;  // 初始不在冷却中
    newProgram.cooldownEndTime = QDateTime();  // 初始化为无效时间
    newProgram.checkTime =  QDateTime::currentDateTime();;
    qDebug() << "设置后startTime:" << newProgram.startTime.toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "设置后isStreaming:" << newProgram.isStreaming;
    qDebug() << "命令原文:" << newProgram.cmdtext;

    // 添加到容器
    ProgramList.append(newProgram);

    qDebug() << "当前容器大小:" << ProgramList.size();
    qDebug() << "====== 处理完成 ======";
}


// 定时检查节目列表
void AI_bragger::checkProgramList()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    // 检查每个节目
    for(int i = 0; i < ProgramList.size(); ++i) {
        ProgramInfo &program = ProgramList[i];

        /*  qDebug() << QString("节目[%1]: ID=%2, rtspurl='%3', isStreaming=%4")
                        .arg(i)
                        .arg(program.commandId)
                        .arg(program.rtspurl)
                        .arg(program.isStreaming ? "是" : "否");*/

        // ========== 原有的推流逻辑 ==========
        if(program.rtspurl.isEmpty() && !program.isStreaming) {
            qint64 elapsedSeconds = program.startTime.secsTo(currentTime);

            qDebug() << QString("  空跑中，已等待: %1秒").arg(elapsedSeconds);

            if(elapsedSeconds >= 120) {  // 已空跑30秒
                qDebug() << "  已满120秒，准备推流...";

                if(!program.deviceList.isEmpty()) {
                    int randomIndex = QRandomGenerator::global()->bounded(program.deviceList.size());
                    QString selectedDevice = program.deviceList.at(randomIndex);

                    // 生成rtsp URL
                    QString suffix = generateRandomSuffix();
                    QString rtspUrl = QString("rtsp://%1:%2/%3")
                                          .arg(host_ip)
                                          .arg(host_port).arg(suffix);


                    qDebug() << QString("  生成的RTSP URL: %1").arg(rtspUrl);

                    // 更新节目信息
                    program.rtspurl = rtspUrl;
                    program.isStreaming = true;

                    // 创建JSON payload
                    QJsonObject payloadObj;
                    QJsonObject dataObj;
                    dataObj["url"] = rtspUrl;
                    dataObj["switch"] = "on";
                    payloadObj["data"] = dataObj;
                    payloadObj["messageType"] = "stream";
                    payloadObj["password"] = "securePassword123";
                    payloadObj["timestamp"] = getCurrentTimestamp();
                    payloadObj["username"] = "user123";

                    QJsonDocument doc(payloadObj);
                    QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
                    QString topic = QString("Device/Dispatch/%1").arg(selectedDevice);

                    // 发送信号
                    emit sCommadSend(topic, payload);
                    program.streamDev = selectedDevice;

                    qDebug() << "  >>> 发送推流指令";
                    qDebug() << "  主题:" << topic;
                    qDebug() << "  RTSP URL:" << rtspUrl;
                }
            }
        }
    }
}
void AI_bragger::checkAndDistributeBraggers()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    for (ProgramInfo &program : ProgramList) {
        // 检查分发条件（增加冷却检查）
        if (!program.voicetotext.isEmpty() &&      // 有语音文本
            !program.bragger.isEmpty() &&          // 有AI评论
            !program.isListen &&                    // 不在识别中
            !program.isGenerating &&               // 不在AI生成中
            !program.isCoolingDown &&              // 不在冷却中
            !program.deviceList.isEmpty() &&
            !isProgramTimeout(program.p_endtime)) {       // 有设备

            qDebug() << "🎯 满足分发条件，节目:" << program.commandId;

            // 计算结束时间（当前时间+10分钟）  执行时间




            int randomValue = QRandomGenerator::global()->bounded(90, 301); // 注意：301 是上限（不包含）
       //     QString endTime = currentTime.addSecs(randomValue).toString("hh:mm:ss");
       //     QString startTime = currentTime.toString("hh:mm:ss");

            // 将bragger按设备数量切片
            QStringList braggerSlices = splitBraggerByDevices(program.bragger, program.deviceList.size());
            if(!braggerSlices.isEmpty())
            {

                // 为每个设备发送一条评论
                for (int i = 0; i < program.deviceList.size(); ++i) {
                    if (i < braggerSlices.size()) {
                        QString deviceSerial = program.deviceList[i];
                        QString braggerSlice = braggerSlices[i];

                        // 构建JSON
                        QJsonObject payloadObj;
                        QJsonObject dataObj;
                        dataObj["action"] = program.action;
                        dataObj["sub_action"] = "弹幕";
                        dataObj["start_time"] = program.p_startime;
                        dataObj["end_time"] = program.p_endtime;
                        dataObj["commandid"] = program.commandId;


                        QString cmdtext = program.cmdtext;

                        // 1. 直接提取remark（假设JSON格式固定）
                        int start = cmdtext.indexOf("\"remark\": \"") + 11;
                        int end = cmdtext.indexOf("\",", start);
                        QString remark = cmdtext.mid(start, end - start);

                        // 2. 替换MSG::MSG
                        QString finalRemark = remark.replace("MSG::MSG", QString(" MSG:%1:MSG").arg(braggerSlice));


                        dataObj["remark"] =finalRemark;

                        payloadObj["data"] = dataObj;
                        payloadObj["messageType"] = "command";
                        payloadObj["password"] = "securePassword123";
                        payloadObj["timestamp"] = getCurrentTimestamp();
                        payloadObj["username"] = "user123";

                        QJsonDocument doc(payloadObj);
                        QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
                        QString topic = QString("Device/Dispatch/%1").arg(deviceSerial);
                        program.sentbrager.append(braggerSlice);
                        // 发送信号
                        emit sCommadSend(topic, payload);

                        qDebug() << "  📤 发送评论到设备" << deviceSerial;
                        qDebug() << "    评论切片:" << braggerSlice.left(50) << "...";
                        // qDebug() << "    检查弹幕容器:" <<program.otherbragger << "...";

                    }
                }
            }



            // 设置冷却状态
            program.isCoolingDown = true;
            program.cooldownEndTime = currentTime.addSecs(cooldownTimer); // 1分钟后结束冷却

            // 设置其他状态
            program.isGenerating = true;
            program.isListen = true;

            // qDebug() << "✅ 分发完成，启动1分钟冷却";
              qDebug() << "  冷却结束时间:" << program.cooldownEndTime.toString("hh:mm:ss");
        }
    }
}

void AI_bragger::checkCoolDown()
{
    QDateTime currentTime = QDateTime::currentDateTime();

    for (ProgramInfo &program : ProgramList) {
        // 如果正在冷却且时间已到
        if (program.isCoolingDown && currentTime >= program.cooldownEndTime) {
            // 重置冷却状态
            program.isCoolingDown = false;
            program.isGenerating = false;
            program.isListen = false;
            program.voicetotext.clear();
            program.bragger.clear();
            qDebug() << "🔄 节目" << program.commandId << "冷却结束，重置状态";
        }
    }
}


QStringList AI_bragger::splitBraggerByDevices(const QString &bragger, int deviceCount)
{
    QStringList result;

    // 提取方括号内的所有内容
    QRegularExpression regex("\\[(.*?)\\]");
    QRegularExpressionMatchIterator it = regex.globalMatch(bragger);

    QStringList comments;
    while (it.hasNext()) {
        QRegularExpressionMatch match = it.next();
        QString comment = match.captured(1).trimmed();
        if (!comment.isEmpty()) {
            comments << comment;
        }
    }

    // 如果没有找到方括号内容，使用换行分割
    if (comments.isEmpty()) {
        //  comments = bragger.split("\n", Qt::SkipEmptyParts);
        return comments;
    }

    // 分配评论给设备
    for (int i = 0; i < deviceCount; i++) {
        if (i < comments.size()) {
            result << comments[i].trimmed();
        } else if (!comments.isEmpty()) {
            result << comments[i % comments.size()].trimmed();
        } else {
            result << "评论内容";
        }
    }

    return result;
}
void AI_bragger::resetAIState()
{
    qDebug() << "🔄 AI冷却时间结束，重置所有节目的AI状态";

    for (ProgramInfo &program : ProgramList) {
        // 将isGenerating置为false，允许下一轮处理
        if (program.isGenerating) {
            program.isGenerating = false;
            qDebug() << "  节目" << program.commandId << "AI状态已重置，可以继续下一轮";
            // 分发完成后，清空语音文本和AI评论

        }
    }
}
void AI_bragger::onProgramEnded(const QString &commandId)
{
    qDebug() << "🛑 收到节目结束请求，commandId:" << commandId;

    // 查找要结束的节目
    for (int i = 0; i < ProgramList.size(); ++i) {
        if (ProgramList[i].commandId == commandId) {
            ProgramInfo &program = ProgramList[i];

            qDebug() << "  找到节目:" << program.programName;
            qDebug() << "  当前状态:";
            qDebug() << "    isStreaming:" << program.isStreaming;
            qDebug() << "    rtspurl:" << program.rtspurl;
            qDebug() << "    设备数量:" << program.deviceList.size();

            // 如果正在推流，发送停止推流的指令
            if (!program.rtspurl.isEmpty() && program.isStreaming) {
                qDebug() << "  正在停止推流...";

                // 为每个设备发送停止推流指令
                for (const QString &device : program.deviceList) {
                    if (!device.isEmpty()) {
                        QJsonObject payloadObj;
                        QJsonObject dataObj;
                        dataObj["url"] = program.rtspurl;
                        dataObj["switch"] = "off";  // 关闭推流
                        payloadObj["data"] = dataObj;
                        payloadObj["messageType"] = "stream";
                        payloadObj["password"] = "securePassword123";
                        payloadObj["timestamp"] = getCurrentTimestamp();
                        payloadObj["username"] = "user123";

                        QJsonDocument doc(payloadObj);
                        QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
                        QString topic = QString("Device/Dispatch/%1").arg(device);

                        // 发送停止推流信号
                        emit sCommadSend(topic, payload);
                        qDebug() << "    发送停止推流到设备:" << device;
                    }
                }
            }

            // 清理评论相关的指令（如果需要）
            if (!program.bragger.isEmpty()) {
                qDebug() << "  清理评论相关数据...";
                // 这里可以发送清除评论的指令，如果需要的话
            }

            // 从容器的中移除该节目
            ProgramList.removeAt(i);

            qDebug() << "✅ 节目" << commandId << "已成功结束并移除";
            qDebug() << "  剩余节目数量:" << ProgramList.size();

            return;
        }
    }

    qDebug() << "⚠️ 未找到commandId为" << commandId << "的节目";
}

void AI_bragger::updateOtherbragger(const QString cmdid, const QStringList &textList)
{
    for (int i = 0; i < ProgramList.size(); ++i) {
        if (ProgramList[i].commandId == cmdid) {
            // 遍历QStringList中的每个字符串
            for (const QString &text : textList) {
                // 检查是否已存在相同的文本
                if (!ProgramList[i].otherbragger.contains(text)) {
                    ProgramList[i].otherbragger.append(text);
                   // qDebug() << "已为节目单" << cmdid << "添加otherbragger:" << text;
                } else {
                  //  qDebug() << "otherbragger中已存在相同文本:" << text;
                }
            }
            return;
        }
    }
    qWarning() << "未找到commandId为" << cmdid << "的节目单";
}
void AI_bragger::setDeviceVector(QVector<DeviceStatus>* vector) {
    deviceVector = vector;
    if (deviceVector) {
        updateDeviceIndexMap();
    }
}
void AI_bragger::updateDeviceIndexMap()
{
    deviceIndexMap.clear();
    if (!deviceVector) return;

    for (int i = 0; i < deviceVector->size(); ++i) {
        const auto& device = deviceVector->at(i);
        deviceIndexMap[device.serialNumber] = i;
    }

    qDebug() << "设备索引映射已更新，共" << deviceIndexMap.size() << "个设备";
}
void AI_bragger::checkDeviceStatusForPrograms()
{
    if (!deviceVector || ProgramList.isEmpty()) {
        return;
    }

    QDateTime now = QDateTime::currentDateTime();

    for (auto& program : ProgramList) {
        // 跳过没有设备列表的节目
        if (program.deviceList.isEmpty()) continue;

        // 检查该节目是否需要检查（默认60秒检查一次）
        if (program.checkTime.isValid() && program.checkTime.secsTo(now) < 300 ) {
            continue; // 距离上次检查不到60秒，跳过
        }
        if(isProgramTimeout(program.p_endtime))
        {
            // 获取当前时间
            QTime currentTime = QTime::currentTime();

            // 将字符串转换为QTime
            QTime endTime = QTime::fromString(program.p_endtime, "HH:mm:ss");
            // 计算时间差（毫秒）
            int diffInMilliseconds = currentTime.msecsTo(endTime);

            // 计算时间差（秒）
            int diffInSeconds = diffInMilliseconds / 1000;
            qDebug() << "已经超时 " << diffInSeconds;
             continue; // 距离上次检查不到60秒，跳过
        }
        // 更新该节目的检查时间
        program.checkTime = now;

        qDebug() << "检查节目:" << program.programName
                 << "命令ID:" << program.commandId
                 << "时间:" << now.toString("hh:mm:ss");

        for (const QString& deviceId : program.deviceList) {
            if (!deviceIndexMap.contains(deviceId)) {
                QString warning = QString("节目[%1] 设备[%2]未在设备列表中找到")
                                      .arg(program.programName)
                                      .arg(deviceId);
                qDebug() << warning;
                continue;
            }

            int index = deviceIndexMap[deviceId];
            const auto& device = deviceVector->at(index);

            // 检查离线状态
            if (device.status == "离线") {
                QString warning = QString("节目[%1] 设备[%2]离线")
                                      .arg(program.programName)
                                      .arg(deviceId);
                qDebug() << warning;
            }
            // 检查异常动作
            else if (device.currentAction == "未知" || device.currentAction == "空闲") {
                QString warning = QString("节目[%1] 设备[%2]异常动作: %3")
                                      .arg(program.programName)
                                      .arg(deviceId)
                                      .arg(device.currentAction);
                qDebug() << warning;

                // 补发命令
                QString payload = program.cmdtext;
                QString topic = QString("Device/Dispatch/%1").arg(deviceId);
                emit sCommadSend(topic, payload);
                qDebug() << "补发命令:" << topic << "\n" << payload;
                qDebug() << "检查推流设备:" << deviceId << "<------>" << program.streamDev;

                // 如果是推流设备，补发推流命令
                if(deviceId == program.streamDev)
                {
                    qDebug() << "检测到异常设备为推流设备" << deviceId;
                    qDebug() << "需要补发一条推流命令";
                     QThread::sleep(1);
                    // 创建JSON payload
                    QJsonObject payloadObj;
                    QJsonObject dataObj;
                    dataObj["url"] = program.rtspurl;
                    dataObj["switch"] = "on";
                    payloadObj["data"] = dataObj;
                    payloadObj["messageType"] = "stream";
                    payloadObj["password"] = "securePassword123";
                    payloadObj["timestamp"] = getCurrentTimestamp();
                    payloadObj["username"] = "user123";

                    QJsonDocument doc(payloadObj);
                    QString streamPayload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
                    QString streamTopic = QString("Device/Dispatch/%1").arg(deviceId);

                    // 发送信号
                    emit sCommadSend(streamTopic, streamPayload);

                    qDebug() << ">>> 发送推流指令";
                    qDebug() << "主题:" << streamTopic;
                    qDebug() << "RTSP URL:" << program.rtspurl;
                }
            }
        }
    }
}
bool AI_bragger::isProgramTimeout(const QString& p_endtime)
{
    // 获取当前时间
    QTime currentTime = QTime::currentTime();

    // 将字符串转换为QTime
    QTime endTime = QTime::fromString(p_endtime, "HH:mm:ss");

    // 比较时间，如果当前时间大于结束时间则返回true
    return currentTime > endTime;
}
