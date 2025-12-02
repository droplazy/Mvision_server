#include "DatabaseManager.h"

DatabaseManager::DatabaseManager(QObject *parent)
    : QObject(parent), db(QSqlDatabase::addDatabase("QSQLITE"))
{
}

DatabaseManager::~DatabaseManager()
{
    if (db.isOpen()) {
        db.close();
    }
}

bool DatabaseManager::openDatabase(const QString &dbName)
{
    db.setDatabaseName(dbName);
    if (!db.open()) {
        qDebug() << "Failed to open database: " << db.lastError().text();
        return false;
    }
    return true;
}

bool DatabaseManager::createDatabase(const QString &dbName)
{
    if (openDatabase(dbName)) {
        qDebug() << "Database opened successfully.";
        return createTables();  // Create tables after opening database
    }
    return false;
}

bool DatabaseManager::createTables()
{
    return createTable1() && createTable2() && createTable3();
}

bool DatabaseManager::createTable1()
{
    QSqlQuery query;
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS Devices (
            serial_number TEXT PRIMARY KEY,
            checksum TEXT,
            total_flow INTEGER,
            ip_address TEXT,
            device_status TEXT,
            bound_user TEXT,
            bound_time TEXT
        );
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table Devices: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table Devices created successfully.";
    return true;
}


bool DatabaseManager::createTable2()
{
    QSqlQuery query;
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS Users (
            username TEXT PRIMARY KEY,
            password TEXT,
            phone_number TEXT,
            email TEXT
        );
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table Users: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table Users created successfully.";
    return true;
}

bool DatabaseManager::createTable3()
{
    QSqlQuery query;
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS ProcessSteps (
            process_id TEXT PRIMARY KEY,
            process_name TEXT,
            creation_time TEXT,
            remark TEXT,
            step1_action TEXT, step1_subaction TEXT, step1_start_time TEXT, step1_end_time TEXT, step1_remark TEXT,
            step2_action TEXT, step2_subaction TEXT, step2_start_time TEXT, step2_end_time TEXT, step2_remark TEXT,
            step3_action TEXT, step3_subaction TEXT, step3_start_time TEXT, step3_end_time TEXT, step3_remark TEXT,
            step4_action TEXT, step4_subaction TEXT, step4_start_time TEXT, step4_end_time TEXT, step4_remark TEXT,
            step5_action TEXT, step5_subaction TEXT, step5_start_time TEXT, step5_end_time TEXT, step5_remark TEXT,
            step6_action TEXT, step6_subaction TEXT, step6_start_time TEXT, step6_end_time TEXT, step6_remark TEXT,
            step7_action TEXT, step7_subaction TEXT, step7_start_time TEXT, step7_end_time TEXT, step7_remark TEXT,
            step8_action TEXT, step8_subaction TEXT, step8_start_time TEXT, step8_end_time TEXT, step8_remark TEXT,
            step9_action TEXT, step9_subaction TEXT, step9_start_time TEXT, step9_end_time TEXT, step9_remark TEXT,
            step10_action TEXT, step10_subaction TEXT, step10_start_time TEXT, step10_end_time TEXT, step10_remark TEXT,
            step11_action TEXT, step11_subaction TEXT, step11_start_time TEXT, step11_end_time TEXT, step11_remark TEXT,
            step12_action TEXT, step12_subaction TEXT, step12_start_time TEXT, step12_end_time TEXT, step12_remark TEXT
        );
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table ProcessSteps: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table ProcessSteps created successfully.";
    return true;
}

bool DatabaseManager::insertDevice(const SQL_Device &device)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO Devices (serial_number, checksum, total_flow, ip_address, device_status, bound_user ,bound_time)
        VALUES (:serial_number, :checksum, :total_flow, :ip_address, :device_status, :bound_user, :bound_time)
    )");

    query.bindValue(":serial_number", device.serial_number);
    query.bindValue(":checksum", device.checksum);
    query.bindValue(":total_flow", device.total_flow);
    query.bindValue(":ip_address", device.ip_address);
    query.bindValue(":device_status", device.device_status);
    query.bindValue(":bound_user", device.bound_user);
    query.bindValue(":bound_time", device.bound_time);

    if (!query.exec()) {
        qDebug() << "Error inserting device: " << query.lastError().text();
        return false;
    }

    qDebug() << "Device inserted successfully.";
    return true;
}

bool DatabaseManager::updateDevice(const SQL_Device &device)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE Devices
        SET checksum = :checksum,
            total_flow = :total_flow,
            ip_address = :ip_address,
            device_status = :device_status,
            bound_user = :bound_user,
            bound_time = :bound_time
        WHERE serial_number = :serial_number
    )");

    query.bindValue(":serial_number", device.serial_number);
    query.bindValue(":checksum", device.checksum);
    query.bindValue(":total_flow", device.total_flow);
    query.bindValue(":ip_address", device.ip_address);
    query.bindValue(":device_status", device.device_status);
    query.bindValue(":bound_user", device.bound_user);
    query.bindValue(":bound_time", device.bound_time);

    if (!query.exec()) {
        qDebug() << "Error updating device: " << query.lastError().text();
        return false;
    }

    qDebug() << "Device updated successfully.";
    return true;
}

bool DatabaseManager::deleteDevice(const QString &serial_number)
{
    QSqlQuery query;
    query.prepare(R"(
        DELETE FROM Devices WHERE serial_number = :serial_number
    )");

    query.bindValue(":serial_number", serial_number);

    if (!query.exec()) {
        qDebug() << "Error deleting device: " << query.lastError().text();
        return false;
    }

    qDebug() << "Device deleted successfully.";
    return true;
}

QList<SQL_Device> DatabaseManager::getAllDevices()
{
    QList<SQL_Device> devices;
    QSqlQuery query("SELECT * FROM Devices");

    while (query.next()) {
        SQL_Device device;
        device.serial_number = query.value("serial_number").toString();
        device.checksum = query.value("checksum").toString();
        device.total_flow = query.value("total_flow").toString();
        device.ip_address = query.value("ip_address").toString();
        device.device_status = query.value("device_status").toString();
        device.bound_user = query.value("bound_user").toString();
        device.bound_time = query.value("bound_time").toString();

        devices.append(device);
    }

    return devices;
}

SQL_Device DatabaseManager::getDeviceBySerialNumber(const QString &serial_number)
{
    SQL_Device device;
    QSqlQuery query;
    query.prepare("SELECT * FROM Devices WHERE serial_number = :serial_number");
    query.bindValue(":serial_number", serial_number);

    if (query.exec() && query.next()) {
        device.serial_number = query.value("serial_number").toString();
        device.checksum = query.value("checksum").toString();
        device.total_flow = query.value("total_flow").toString();
        device.ip_address = query.value("ip_address").toString();
        device.device_status = query.value("device_status").toString();
        device.bound_user = query.value("bound_user").toString();
    } else {
        qDebug() << "Error fetching device: " << query.lastError().text();
    }

    return device;
}

bool DatabaseManager::insertUser(const SQL_User &user)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO Users (username, password, phone_number, email)
        VALUES (:username, :password, :phone_number, :email)
    )");

    query.bindValue(":username", user.username);
    query.bindValue(":password", user.password);
    query.bindValue(":phone_number", user.phone_number);
    query.bindValue(":email", user.email);

    if (!query.exec()) {
        qDebug() << "Error inserting user: " << query.lastError().text();
        return false;
    }

    qDebug() << "User inserted successfully.";
    return true;
}

bool DatabaseManager::updateUser(const SQL_User &user)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE Users
        SET password = :password,
            phone_number = :phone_number,
            email = :email
        WHERE username = :username
    )");

    query.bindValue(":username", user.username);
    query.bindValue(":password", user.password);
    query.bindValue(":phone_number", user.phone_number);
    query.bindValue(":email", user.email);

    if (!query.exec()) {
        qDebug() << "Error updating user: " << query.lastError().text();
        return false;
    }

    qDebug() << "User updated successfully.";
    return true;
}

bool DatabaseManager::deleteUser(const QString &username)
{
    QSqlQuery query;
    query.prepare(R"(
        DELETE FROM Users WHERE username = :username
    )");

    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error deleting user: " << query.lastError().text();
        return false;
    }

    qDebug() << "User deleted successfully.";
    return true;
}

QList<SQL_User> DatabaseManager::getAllUsers()
{
    QList<SQL_User> users;
    QSqlQuery query("SELECT * FROM Users");

    while (query.next()) {
        SQL_User user;
        user.username = query.value("username").toString();
        user.password = query.value("password").toString();
        user.phone_number = query.value("phone_number").toString();
        user.email = query.value("email").toString();

        users.append(user);
    }

    return users;
}

SQL_User DatabaseManager::getUserByUsername(const QString &username)
{
    SQL_User user;
    QSqlQuery query;
    query.prepare("SELECT * FROM Users WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        user.username = query.value("username").toString();
        user.password = query.value("password").toString();
        user.phone_number = query.value("phone_number").toString();
        user.email = query.value("email").toString();
    } else {
        qDebug() << "Error fetching user: " << query.lastError().text();
    }

    return user;
}

bool DatabaseManager::insertProcessSteps(const Machine_Process_Total &process)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO ProcessSteps (process_id, process_name, creation_time, remark,
            step1_action, step1_subaction, step1_start_time, step1_end_time, step1_remark,
            step2_action, step2_subaction, step2_start_time, step2_end_time, step2_remark,
            step3_action, step3_subaction, step3_start_time, step3_end_time, step3_remark,
            step4_action, step4_subaction, step4_start_time, step4_end_time, step4_remark,
            step5_action, step5_subaction, step5_start_time, step5_end_time, step5_remark,
            step6_action, step6_subaction, step6_start_time, step6_end_time, step6_remark,
            step7_action, step7_subaction, step7_start_time, step7_end_time, step7_remark,
            step8_action, step8_subaction, step8_start_time, step8_end_time, step8_remark,
            step9_action, step9_subaction, step9_start_time, step9_end_time, step9_remark,
            step10_action, step10_subaction, step10_start_time, step10_end_time, step10_remark,
            step11_action, step11_subaction, step11_start_time, step11_end_time, step11_remark,
            step12_action, step12_subaction, step12_start_time, step12_end_time, step12_remark
        ) VALUES (
            :process_id, :process_name, :creation_time, :remark,
            :step1_action, :step1_subaction, :step1_start_time, :step1_end_time, :step1_remark,
            :step2_action, :step2_subaction, :step2_start_time, :step2_end_time, :step2_remark,
            :step3_action, :step3_subaction, :step3_start_time, :step3_end_time, :step3_remark,
            :step4_action, :step4_subaction, :step4_start_time, :step4_end_time, :step4_remark,
            :step5_action, :step5_subaction, :step5_start_time, :step5_end_time, :step5_remark,
            :step6_action, :step6_subaction, :step6_start_time, :step6_end_time, :step6_remark,
            :step7_action, :step7_subaction, :step7_start_time, :step7_end_time, :step7_remark,
            :step8_action, :step8_subaction, :step8_start_time, :step8_end_time, :step8_remark,
            :step9_action, :step9_subaction, :step9_start_time, :step9_end_time, :step9_remark,
            :step10_action, :step10_subaction, :step10_start_time, :step10_end_time, :step10_remark,
            :step11_action, :step11_subaction, :step11_start_time, :step11_end_time, :step11_remark,
            :step12_action, :step12_subaction, :step12_start_time, :step12_end_time, :step12_remark
        )
    )");

    query.bindValue(":process_id", process.process_id);
    query.bindValue(":process_name", process.process_name);
    query.bindValue(":creation_time", process.creation_time);
    query.bindValue(":remark", process.remark);

    // Loop through the `Processes` vector and bind each step dynamically
    int stepCount = process.Processes.size();
    for (int i = 0; i < 12; ++i) {
        if (i < stepCount) {
            const Machine_Process_Single &step = process.Processes[i];
            query.bindValue(":step" + QString::number(i + 1) + "_action", step.action);
            query.bindValue(":step" + QString::number(i + 1) + "_subaction", step.sub_action);
            query.bindValue(":step" + QString::number(i + 1) + "_start_time", step.start_time);
            query.bindValue(":step" + QString::number(i + 1) + "_end_time", step.end_time);
            query.bindValue(":step" + QString::number(i + 1) + "_remark", step.remark); // Add the remark for each step
        } else {
            query.bindValue(":step" + QString::number(i + 1) + "_action", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_subaction", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_start_time", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_end_time", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_remark", QVariant());
        }
    }

    if (!query.exec()) {
        qDebug() << "Error inserting process steps: " << query.lastError().text();
        return false;
    }

    qDebug() << "Process steps inserted successfully.";
    return true;
}

bool DatabaseManager::deleteProcessSteps(const QString &process_id)
{
    QSqlQuery query;
    query.prepare("DELETE FROM ProcessSteps WHERE process_id = :process_id");
    query.bindValue(":process_id", process_id);

    if (!query.exec()) {
        qDebug() << "Error deleting process steps: " << query.lastError().text();
        return false;
    }

    qDebug() << "Process steps deleted successfully.";
    return true;
}
bool DatabaseManager::updateProcessSteps(const Machine_Process_Total &process)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE ProcessSteps SET
            process_name = :process_name,
            creation_time = :creation_time,
            remark = :remark,
            step1_action = :step1_action, step1_subaction = :step1_subaction, step1_start_time = :step1_start_time, step1_end_time = :step1_end_time, step1_remark = :step1_remark,
            step2_action = :step2_action, step2_subaction = :step2_subaction, step2_start_time = :step2_start_time, step2_end_time = :step2_end_time, step3_remark = :step2_remark,
            step3_action = :step3_action, step3_subaction = :step3_subaction, step3_start_time = :step3_start_time, step3_end_time = :step3_end_time, step3_remark = :step3_remark,
            step4_action = :step4_action, step4_subaction = :step4_subaction, step4_start_time = :step4_start_time, step4_end_time = :step4_end_time, step4_remark = :step4_remark,
            step5_action = :step5_action, step5_subaction = :step5_subaction, step5_start_time = :step5_start_time, step5_end_time = :step5_end_time, step5_remark = :step5_remark,
            step6_action = :step6_action, step6_subaction = :step6_subaction, step6_start_time = :step6_start_time, step6_end_time = :step6_end_time, step6_remark = :step6_remark,
            step7_action = :step7_action, step7_subaction = :step7_subaction, step7_start_time = :step7_start_time, step7_end_time = :step7_end_time, step7_remark = :step7_remark,
            step8_action = :step8_action, step8_subaction = :step8_subaction, step8_start_time = :step8_start_time, step8_end_time = :step8_end_time, step8_remark = :step8_remark,
            step9_action = :step9_action, step9_subaction = :step9_subaction, step9_start_time = :step9_start_time, step9_end_time = :step9_end_time, step9_remark = :step9_remark,
            step10_action = :step10_action, step10_subaction = :step10_subaction, step10_start_time = :step10_start_time, step10_end_time = :step10_end_time, step10_remark = :step10_remark,
            step11_action = :step11_action, step11_subaction = :step11_subaction, step11_start_time = :step11_start_time, step11_end_time = :step11_end_time, step11_remark = :step11_remark,
            step12_action = :step12_action, step12_subaction = :step12_subaction, step12_start_time = :step12_start_time, step12_end_time = :step12_end_time, step12_remark = :step12_remark
        WHERE process_id = :process_id
    )");

    query.bindValue(":process_id", process.process_id);
    query.bindValue(":process_name", process.process_name);
    query.bindValue(":creation_time", process.creation_time);
    query.bindValue(":remark", process.remark);

    // 绑定步骤数据
    int stepCount = process.Processes.size();
    for (int i = 0; i < 12; ++i) {
        if (i < stepCount) {
            const Machine_Process_Single &step = process.Processes[i];
            query.bindValue(":step" + QString::number(i + 1) + "_action", step.action);
            query.bindValue(":step" + QString::number(i + 1) + "_subaction", step.sub_action);
            query.bindValue(":step" + QString::number(i + 1) + "_start_time", step.start_time);
            query.bindValue(":step" + QString::number(i + 1) + "_end_time", step.end_time);
            query.bindValue(":step" + QString::number(i + 1) + "_remark", step.end_time);
        } else {
            query.bindValue(":step" + QString::number(i + 1) + "_action", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_subaction", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_start_time", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_end_time", QVariant());
            query.bindValue(":step" + QString::number(i + 1) + "_remark", QVariant());

        }
    }

    if (!query.exec()) {
        qDebug() << "Error updating process steps: " << query.lastError().text();
        return false;
    }

    qDebug() << "Process steps updated successfully.";
    return true;
}

QList<Machine_Process_Total> DatabaseManager::getAllProcessSteps()
{
    QList<Machine_Process_Total> processes;
    QSqlQuery query("SELECT * FROM ProcessSteps");

    while (query.next()) {
        Machine_Process_Total process;
        process.process_id = query.value("process_id").toString();
        process.process_name = query.value("process_name").toString();
        process.creation_time = query.value("creation_time").toString();
        process.remark = query.value("remark").toString();

        // 逐步加载步骤数据
        for (int i = 0; i < 12; ++i) {
            Machine_Process_Single step;
            step.action = query.value("step" + QString::number(i+1) + "_action").toString();
            step.sub_action = query.value("step" + QString::number(i+1) + "_subaction").toString();
            step.start_time = query.value("step" + QString::number(i+1) + "_start_time").toString();
            step.end_time = query.value("step" + QString::number(i+1) + "_end_time").toString();
            step.remark = query.value("step" + QString::number(i+1) + "_remark").toString();

            if (!step.action.isEmpty()) { // 如果存在该步骤数据，则添加到列表
                process.Processes.append(step);
            }
        }

        processes.append(process);
    }

    return processes;
}

#if 0
// 创建 User 结构体实例
User newUser = { "john_doe", "password123", "1234567890", "john@example.com" };

// 插入用户
dbManager.insertUser(newUser);

// 更新用户信息
newUser.password = "new_password";
dbManager.updateUser(newUser);

// 获取所有用户
QList<User> users = dbManager.getAllUsers();

// 获取指定用户
User user = dbManager.getUserByUsername("john_doe");

// 删除用户
dbManager.deleteUser("john_doe");

Device newDevice = { "12345", "abcd", 100, "192.168.1.1", "active", "user1" };

// 插入设备
dbManager.insertDevice(newDevice);

// 更新设备
newDevice.total_flow = 200;
dbManager.updateDevice(newDevice);

// 获取所有设备
QList<Device> devices = dbManager.getAllDevices();

// 获取指定设备
Device device = dbManager.getDeviceBySerialNumber("12345");

// 删除设备
dbManager.deleteDevice("12345");
#endif
