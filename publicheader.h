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

// SQL_Order.h 或直接在 DatabaseManager.h 中添加
struct SQL_Order {
    QString orderId;           // 订单ID（主键）
    QString productId;         // 产品ID
    double unitPrice;          // 单价
    int quantity;              // 数量
    double totalPrice;         // 总价
    QString note;              // 备注
    QString user;              // 用户
    QString contactInfo;       // 联系方式
    QString status;            // 订单状态（pending, paid, shipped, completed, cancelled）
    QString createTime;        // 创建时间
    QString updateTime;        // 更新时间

    // 构造函数
    SQL_Order()
        : unitPrice(0.0)
        , quantity(0)
        , totalPrice(0.0)
    {}

    SQL_Order(const QString& oid, const QString& pid, double uprice, int qty,
              double tprice, const QString& n, const QString& u,
              const QString& contact, const QString& stat = "pending")
        : orderId(oid), productId(pid), unitPrice(uprice), quantity(qty),
        totalPrice(tprice), note(n), user(u), contactInfo(contact),
        status(stat)
    {}

    // 计算总价
    void calculateTotal() {
        totalPrice = unitPrice * quantity;
    }
};
// 在struct SQL_MallUser中添加inviterUsername字段
struct SQL_MallUser {
    QString username;      // 用户名（主键）
    QString password;      // 密码
    QString email;         // 邮箱
    QString inviteCode;    // 邀请码
    QString inviterUsername; // 邀请人账号（新增）
    QString createTime;    // 创建时间
    QString lastLoginTime; // 最后登录时间
    QString phone;         // 手机号（可选）
    int userLevel;         // 用户等级
    double balance;        // 余额
    int points;            // 积分

    // 构造函数
    SQL_MallUser()
        : userLevel(1)
        , balance(0.0)
        , points(0)
    {}

    SQL_MallUser(const QString& uname, const QString& pwd, const QString& mail,
                 const QString& invite = "", const QString& inviter = "",
                 const QString& phoneNum = "")
        : username(uname), password(pwd), email(mail), inviteCode(invite),
        inviterUsername(inviter), phone(phoneNum), userLevel(1),
        balance(0.0), points(0)
    {
        createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    }
};
struct SQL_WithdrawRecord {
    QString withdrawId;      // 提现记录ID
    QString username;        // 用户名
    double amount;           // 提现金额
    QString alipayAccount;   // 支付宝账号
    QString status;          // 状态：pending, processing, completed, failed
    QString createTime;      // 创建时间
    QString updateTime;      // 更新时间
    QString remark;          // 备注
};

// // 产品ID到名称的映射（这里需要你根据实际情况完善）
// QMap<QString, QString> productNameMap = {
//     {"PROD001", "iPhone 15 Pro"}, {"PROD002", "MacBook Pro"},
//     {"PROD003", "iPad Air"},      {"PROD004", "Apple Watch"},
//     {"PROD005", "AirPods Pro"},   {"PROD006", "iMac"},
//     {"PROD007", "Mac mini"} // 在这里添加更多产品映射...
// };
#define LOGIN_BACKGROUND_PIC    "192.168.10.103:/images/login-bg.jpg"
#define LOGIN_GUIDE_TEXT        "欢迎登录设备管理系统"
#define LOGIN_SLOGAN1           "安全可靠，智能管理"
#define LOGIN_SLOGAN2           "让设备管理更简单"
#endif // PUBLICHEADER_H
