#ifndef PUBLICHEADER_H
#define PUBLICHEADER_H
#include "qjsonarray.h"
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QVector>
struct DeviceStatus {
    QString serialNumber;          // 设备序列号
    QString checksum;
    QString status;                // 设备状态
    QString location;              // 设备所在位置
    QString currentAction;         // 当前动作
    QString trafficStatistics;     // 已用流量
    QString lastHeartbeat;         // 最后心跳时间


    QString ip;
    QString current_start;
    QString current_end;
    QString next_action;
    QString next_action_start;
    QString next_action_end;
    QString usedProcess;         // 正在使用的流程TODO
    QString usedProcessID;         // 正在使用的流程TODO

    // 构造函数，用于初始化
    DeviceStatus(const QString& sn, const QString& st, const QString& loc,
                 const QString& action, const QString& traffic, const QString& heartbeat,
                const QString& ip, const QString& cs, const QString& ce, const QString& na,
const QString& nas ,const QString& nae , const QString& process ,const QString& processId)
        : serialNumber(sn), status(st), location(loc), currentAction(action),
        trafficStatistics(traffic), lastHeartbeat(heartbeat) ,ip(ip) ,current_start(cs), current_end(ce),
        next_action(na),next_action_start(nas),next_action_end(nae) ,usedProcess(process) ,usedProcessID(processId) {}

    // 将结构体转化为 JSON 对象
    QJsonObject toJsonAll() const {
        QJsonObject jsonObj;
        jsonObj["serial_number"] = serialNumber;
        jsonObj["status"] = status;
        jsonObj["location"] = location;
        jsonObj["current_action"] = currentAction;
        jsonObj["traffic_statistics"] = trafficStatistics;
        jsonObj["last_heartbeat"] = lastHeartbeat;
        jsonObj["process_name"] = usedProcess;
        jsonObj["process_id"] = usedProcessID;

        return jsonObj;
    }

    // 将结构体转化为 JSON 对象
    QJsonObject toJsonSingle() const {
        QJsonObject jsonObj;
        QJsonObject obj_c;
        QJsonObject obj_n;

        obj_c["name"] = currentAction;
        obj_c["start_time"] = current_start;
        obj_c["end_time"] = current_end;

        obj_n["name"] = next_action;
        obj_n["start_time"] = next_action_start;
        obj_n["end_time"] = next_action_end;

        jsonObj["ip"] = ip;
        jsonObj["current_action"] = obj_c;
        jsonObj["next_action"] = obj_n;
        jsonObj["traffic_statistics"] = trafficStatistics;
        jsonObj["serial_number"] = serialNumber;
        jsonObj["status"] = status;


        return jsonObj;
    }

    // 可以考虑添加一个输出函数，用于打印该结构体的内容
    void printInfo() const {
        qDebug() << "Serial Number: " << serialNumber;
        qDebug() << "Status: " << status;
        qDebug() << "Location: " << location;
        qDebug() << "Current Action: " << currentAction;
        qDebug() << "Traffic Statistics: " << trafficStatistics;
        qDebug() << "Last Heartbeat: " << lastHeartbeat;
    }
};
struct Machine_Process_Single {
    QString action;
    QString sub_action;
    QString start_time;
    QString end_time;
    QString remark;
   // int step;
    // 将结构体转化为 JSON 对象
    QJsonObject toJsonAll() const {
        QJsonObject jsonObj;
        jsonObj["action"] = action;
        jsonObj["sub_action"] = sub_action;
        jsonObj["start_time"] = start_time;
        jsonObj["end_time"] = end_time;
        jsonObj["remark"] = remark;
        return jsonObj;
    }
};


struct Machine_Process_Total {
    QString process_id;
    QString process_name;
    QString creation_time;
    QString remark;
    QVector<Machine_Process_Single> Processes;
    // 将结构体转化为 JSON 对象
    QJsonObject toJsonAll() const {
        QJsonObject jsonObj;
        jsonObj["process_id"] = process_id;
        jsonObj["process_name"] = process_name;
        jsonObj["creation_time"] = creation_time;
        jsonObj["remark"] = remark;
        return jsonObj;
    }


    QJsonObject alljson() const {
        QJsonObject jsonObj;
        jsonObj["process_id"] = process_id;
        jsonObj["process_name"] = process_name;
        jsonObj["creation_time"] = creation_time;
        jsonObj["remark"] = remark;

        // 将 Processes 转为 JSON 数组
        QJsonArray processArray;
        for (const auto& process : Processes) {
            processArray.append(process.toJsonAll());
        }
        jsonObj["Processes"] = processArray;

        return jsonObj;
    }

    // 可以考虑添加一个输出函数，用于打印该结构体的内容
    void printInfo() const {
        qDebug() << "process_id: " << process_id;
        qDebug() << "process_name: " << process_name;
        qDebug() << "creation_time: " << creation_time;
        qDebug() << "remark : " << remark;
        qDebug() << "this process has :" <<Processes.count();
    }
};

struct SQL_Device {
    QString serial_number;
    QString checksum;
    QString total_flow;
    QString ip_address;
    QString device_status;
    QString bound_user;
    QString bound_time;

};
struct SQL_User {
    QString username;
    QString password;
    QString phone_number;
    QString email;
};
#endif // PUBLICHEADER_H
