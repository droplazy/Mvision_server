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
        checkCoolDown();  // 添加冷却检查
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

    qDebug() << "设置后startTime:" << newProgram.startTime.toString("yyyy-MM-dd hh:mm:ss");
    qDebug() << "设置后isStreaming:" << newProgram.isStreaming;

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

            if(elapsedSeconds >= 5) {  // 已空跑30秒
                qDebug() << "  已满30秒，准备推流...";

                if(!program.deviceList.isEmpty()) {
                    int randomIndex = QRandomGenerator::global()->bounded(program.deviceList.size());
                    QString selectedDevice = program.deviceList.at(randomIndex);

                    // 生成rtsp URL
                    QString suffix = generateRandomSuffix();
                    QString rtspUrl = QString("rtsp://%1:%2/audio")
                                          .arg(host_ip)
                                          .arg(host_port);//.arg(suffix);


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
            !program.deviceList.isEmpty()) {       // 有设备

            qDebug() << "🎯 满足分发条件，节目:" << program.commandId;

            // 计算结束时间（当前时间+10分钟）
            QString endTime = currentTime.addSecs(600).toString("hh:mm:ss");
            QString startTime = currentTime.toString("hh:mm:ss");

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
                        dataObj["action"] = "XXXAAAA";
                        dataObj["sub_action"] = "弹幕";
                        dataObj["start_time"] = startTime;
                        dataObj["end_time"] = endTime;
                        dataObj["commandid"] = program.commandId;
                        dataObj["remark"] = QString("MSG:%1:MSG").arg(braggerSlice);

                        payloadObj["data"] = dataObj;
                        payloadObj["messageType"] = "command";
                        payloadObj["password"] = "securePassword123";
                        payloadObj["timestamp"] = getCurrentTimestamp();
                        payloadObj["username"] = "user123";

                        QJsonDocument doc(payloadObj);
                        QString payload = QString::fromUtf8(doc.toJson(QJsonDocument::Compact));
                        QString topic = QString("Device/Dispatch/%1").arg(deviceSerial);

                        // 发送信号
                        emit sCommadSend(topic, payload);

                        qDebug() << "  📤 发送评论到设备" << deviceSerial;
                        qDebug() << "    评论切片:" << braggerSlice.left(50) << "...";
                    }
                }
            }



            // 设置冷却状态
            program.isCoolingDown = true;
            program.cooldownEndTime = currentTime.addSecs(10); // 1分钟后结束冷却

            // 设置其他状态
            program.isGenerating = true;
            program.isListen = true;

            qDebug() << "✅ 分发完成，启动1分钟冷却";
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
