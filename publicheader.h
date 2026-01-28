#ifndef PUBLICHEADER_H
#define PUBLICHEADER_H
#include "qjsonarray.h"
#include <QString>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonValue>
#include <QDebug>
#include <QVector>

struct VerificationCode {
    QString code;           // 验证码（4位数字）
    QString username;       // 用户名
    QString email;          // 邮箱
    bool verified;          // 是否已验证
    QDateTime createTime;   // 创建时间
    QDateTime expireTime;   // 过期时间（5分钟后）

    // 构造函数
    VerificationCode()
        : verified(false) {}

    VerificationCode(const QString &c, const QString &u, const QString &e)
        : code(c), username(u), email(e), verified(false)
    {
        createTime = QDateTime::currentDateTime();
        expireTime = createTime.addSecs(300); // 5分钟 = 300秒
    }

    // 检查是否过期
    bool isExpired() const {
        return QDateTime::currentDateTime() > expireTime;
    }

    // 检查是否有效（未验证且未过期）
    bool isValid() const {
        return !verified && !isExpired();
    }
};


// 邮件信息结构体
struct EmailInfo {
    QString toEmail;      // 收件人邮箱
    QString message;      // 邮件内容
    QString subject;      // 邮件主题（可选）

    EmailInfo() = default;
    EmailInfo(const QString &to, const QString &msg, const QString &sub = "验证码通知")
        : toEmail(to), message(msg), subject(sub) {}
};

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
        jsonObj["firmware_version"] = hardversion;


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
        qDebug() << "HardVerison: " << hardversion;

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

// 设备结构体
struct SQL_Device {
    QString serial_number;
    QString checksum;
    QString total_flow;
    QString ip_address;
    QString device_status;
    QString bound_user;
    QString bound_time;
    QString tiktok;      // 新增：TikTok开关 (0=关闭, 1=开启)
    QString bilibili;    // 新增：Bilibili开关
    QString xhs;         // 新增：小红书开关
    QString weibo;       // 新增：微博开关
    QString kuaishou;    // 新增：快手开关

    // 构造函数
    SQL_Device() : tiktok("未登录"), bilibili("未登录"), xhs("未登录"), weibo("未登录"), kuaishou("未登录") {}
};
struct SQL_User {
    QString username;
    QString password;
    QString phone_number;
    QString email;
};

// 指令历史记录结构体
// 指令历史记录结构体
struct SQL_CommandHistory {
    QString commandId;          // 指令ID（主键）
    QString status;             // 状态（如：pending, executing, completed, failed）
    QString action;             // 动作
    QString sub_action;         // 子动作
    QString start_time;         // 开始时间
    QString end_time;           // 结束时间
    QString remark;             // 备注
    QString completeness  ;       // 完成度（如：0%, 50%, 100%）
    QString completed_url ;      // 完成后的URL或资源路径
    int total_tasks ;             // 新增：总任务数
    int completed_tasks ;         // 新增：已完成任务数
   int failed_tasks = 0;  // 新增失败任务数
    // 构造函数
    SQL_CommandHistory() : total_tasks (0), completed_tasks (0) {}

    SQL_CommandHistory(const QString& id, const QString& stat, const QString& act,
                       const QString& subAct, const QString& start, const QString& end,
                       const QString& rem, const QString& comp, const QString& url,
                       int total = 0, int completed = 0)
        : commandId(id), status(stat), action(act), sub_action(subAct),
        start_time(start), end_time(end), remark(rem),
        completeness(comp), completed_url(url),
        total_tasks(total), completed_tasks(completed) {}
};
// SQL_Order.h 或直接在 DatabaseManager.h 中添加
struct SQL_Order {
    QString orderId;
    QString commandId;      // 新增：关联的指令ID
    QString productId;
    double unitPrice;
    int quantity;
    double totalPrice;
    QString note;
    QString user;
    QString contactInfo;
    QString status;
    QString createTime;
    QString updateTime;
    QString verifier;

 QString snapshot;
       QString productName;
    // 计算总价的方法
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

// 在头文件中定义
struct SQL_AppealRecord {
    int id;
    QString username;
    QString orderId;
    QString appealType;  // text 或 picture
    QString contentPath;
    QString textContent;
    QString appealTime;
    QString status;      // 总体状态
    QString operatorName;
    QString result;
    QString resultTime;
    QString processingStatus;  // 处理状态
    int appealLevel;     // 投诉级别
    QString priority;    // 优先级

    // // 从查询结果初始化
    // void fromQuery(const QSqlQuery &query) {
    //     id = query.value("id").toInt();
    //     username = query.value("username").toString();
    //     orderId = query.value("order_id").toString();
    //     appealType = query.value("appeal_type").toString();
    //     contentPath = query.value("content_path").toString();
    //     textContent = query.value("text_content").toString();
    //     appealTime = query.value("appeal_time").toString();
    //     status = query.value("status").toString();
    //     operatorName = query.value("operator").toString();
    //     result = query.value("result").toString();
    //     resultTime = query.value("result_time").toString();
    //     processingStatus = query.value("processing_status").toString();
    //     appealLevel = query.value("appeal_level").toInt();
    //     priority = query.value("priority").toString();
    // }

    // 转换为JSON
    QJsonObject toJson() const {
        QJsonObject obj;
        obj["id"] = id;
        obj["username"] = username;
        obj["orderId"] = orderId;
        obj["appealType"] = appealType;
        obj["contentPath"] = contentPath;
        obj["appealTime"] = appealTime;
        obj["status"] = status;
        obj["processingStatus"] = processingStatus;
        obj["appealLevel"] = appealLevel;
        obj["priority"] = priority;

        if (!operatorName.isEmpty()) obj["operator"] = operatorName;
        if (!result.isEmpty()) obj["result"] = result;
        if (!resultTime.isEmpty()) obj["resultTime"] = resultTime;

        return obj;
    }
};

// 商品结构体
struct SQL_Product {
    QString productId;           // 商品ID
    QString productName;         // 商品名称
    QString categoryId;          // 分类ID
    QString categoryName;        // 分类名称（可选）
    double unitPrice;           // 单价
    int stock;                 // 库存
    int minOrder;              // 最小起订量
    int maxOrder;              // 最大订购量
    QString status;            // 状态（例如："active", "inactive", "out_of_stock"）
    QString action;            // 主要动作/操作
    QString subaction;         // 子动作/详细操作
    QString description;       // 商品描述
    QString imageUrl;         // 商品图片URL
    QString createTime;       // 创建时间
    QString updateTime;       // 更新时间
    QString tags;             // 商品标签（逗号分隔）
    QString specifications;   // 规格参数（JSON格式）
    int salesCount;          // 销量
    double rating;           // 评分
    int ratingCount;         // 评分人数

    SQL_Product() : unitPrice(0.0), stock(0), minOrder(1), maxOrder(9999),
        salesCount(0), rating(0.0), ratingCount(0) {}
};
// 在DatabaseManager.h中添加结构体定义和函数声明：
struct SQL_AppAccount {
    int id;              // 主键，自增
    QString accountName; // 账号名（唯一标识）
    QString platform;    // 平台：抖音、B站、小红书、微博、快手等
    QString username;    // 用户名
    QString createTime;  // 创建时间
    QString status;      // 状态：active, inactive, locked
    QString remark;      // 备注
    QString devserial;
};

struct ProgramInfo
{
    QString commandId;
    QString programName;
    QList<QString> deviceList;
    QString rtspurl;
    int keepTime;
    QDateTime startTime;  // 添加开始时间
    bool isStreaming;     // 是否正在推流
    bool isListen;     // 是否正在推流
    bool isGenerating;     //AI

    QString voicetotext;
    QString bragger;

};
struct AIpost
{
    QString text;
    QString theme;
    QString motion;
    QString commandid;
    QString scene;
    int num;
};
// // 产品ID到名称的映射（这里需要你根据实际情况完善）
// QMap<QString, QString> productNameMap = {
//     {"PROD001", "iPhone 15 Pro"}, {"PROD002", "MacBook Pro"},
//     {"PROD003", "iPad Air"},      {"PROD004", "Apple Watch"},
//     {"PROD005", "AirPods Pro"},   {"PROD006", "iMac"},
//     {"PROD007", "Mac mini"} // 在这里添加更多产品映射...
// };
#define LOGIN_BACKGROUND_PIC    "/images/login-bg.jpg"
// #define LOGIN_GUIDE_TEXT        "欢迎登录设备管理系统"
// #define LOGIN_SLOGAN1           "安全可靠，智能管理"
// #define LOGIN_SLOGAN2           "让设备管理更简单"
#endif // PUBLICHEADER_H
