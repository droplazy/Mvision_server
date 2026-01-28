#include "ai_bragger.h"
#include <QDebug>
#include <QJsonObject>
#include <QJsonDocument>
#include <QRandomGenerator>

AI_bragger::AI_bragger()
{
    checkTimer = new QTimer(this);
    connect(checkTimer, &QTimer::timeout, this, &AI_bragger::checkProgramList);
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
                                          .arg(host_port);

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

        // ========== 节目超时检查 ==========
        if(program.keepTime > 0) {
            qint64 elapsedSeconds = program.startTime.secsTo(currentTime);
            if(elapsedSeconds >= program.keepTime) {
                qDebug() << QString("  >>> 节目%1已超时，持续%2秒，准备移除")
                                .arg(program.commandId)
                                .arg(program.keepTime);
                ProgramList.removeAt(i);
                i--;
            }
        }
    }
}
