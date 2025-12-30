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
    QString warningmsg;
    float Temperature = 0;           // 温度，默认0
    bool newdev = true;
    bool warining_ignore=false;
    QString hardversion;

    /*准备丢弃*/
    QString next_action;
    QString next_action_start;
    QString next_action_end;
    /**/
    QString usedProcess;           // 正在使用的流程
    QString usedProcessID;         // 正在使用的流程ID

    // 修改后的构造函数，添加了 Temperature 参数
    DeviceStatus(const QString& sn, const QString& st, const QString& loc,
                 const QString& action, const QString& traffic, const QString& heartbeat,
                 const QString& ip, const QString& cs, const QString& ce,
                 const QString& na, const QString& nas, const QString& nae,
                 const QString& process, const QString& processId,
                 float temperature = 0)  // 新增温度参数，默认为0
        : serialNumber(sn), status(st), location(loc),
        currentAction(action), trafficStatistics(traffic),
        lastHeartbeat(heartbeat), ip(ip),
        current_start(cs), current_end(ce),
        Temperature(temperature), next_action(na), next_action_start(nas),
        next_action_end(nae), usedProcess(process),
        usedProcessID(processId)  // 初始化温度
    {
        // 可以根据温度自动设置警告信息
        if (temperature > 60) {  // 假设60度是高温阈值
            warningmsg = QString("设备高温，当前温度：%1").arg(temperature);
        }
    }
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
    QJsonObject toJsonWar() const {
        QJsonObject jsonObj;
        jsonObj["serial_number"] = serialNumber;
        jsonObj["warning"] = warningmsg;
        jsonObj["last_heartbeat"] = lastHeartbeat;

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
        jsonObj["warning"] = warningmsg;
        jsonObj["hard-version"] = hardversion;


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

// 指令历史记录结构体
struct SQL_CommandHistory {
    QString commandId;          // 指令ID（主键）
    QString status;             // 状态（如：pending, executing, completed, failed）
    QString action;             // 动作
    QString sub_action;         // 子动作
    QString start_time;         // 开始时间
    QString end_time;           // 结束时间
    QString remark;             // 备注
    QString Completeness;       // 完成度（如：0%, 50%, 100%）
    QString completed_url;      // 完成后的URL或资源路径

    // 构造函数
    SQL_CommandHistory() {}

    SQL_CommandHistory(const QString& id, const QString& stat, const QString& act,
                       const QString& subAct, const QString& start, const QString& end,
                       const QString& rem, const QString& comp, const QString& url)
        : commandId(id), status(stat), action(act), sub_action(subAct),
        start_time(start), end_time(end), remark(rem),
        Completeness(comp), completed_url(url) {}
};
#endif // PUBLICHEADER_H
