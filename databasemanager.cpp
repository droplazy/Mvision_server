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
    return createTable1() && createTable2() && createTable3()&& createTable4()&& createTable5();
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

bool DatabaseManager:: insertProcessSteps(const Machine_Process_Total &process)
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
bool DatabaseManager::createTable4()
{
    QSqlQuery query;

    // 1. 先创建表
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS CommandHistory (
            command_id TEXT PRIMARY KEY,
            status TEXT NOT NULL,
            action TEXT,
            sub_action TEXT,
            start_time TEXT,
            end_time TEXT,
            remark TEXT,
            completeness TEXT,
            completed_url TEXT,
            created_at TEXT DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT DEFAULT (datetime('now', 'localtime'))
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table CommandHistory: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table CommandHistory created successfully.";

    // 2. 创建索引（分开执行）
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_command_status ON CommandHistory(status)",
        "CREATE INDEX IF NOT EXISTS idx_command_action ON CommandHistory(action)",
        "CREATE INDEX IF NOT EXISTS idx_command_time ON CommandHistory(start_time)"
    };

    for (const QString &indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qDebug() << "Error creating index: " << query.lastError().text();
            // 注意：索引创建失败不应该阻止表创建成功
        }
    }

    return true;
}

bool DatabaseManager::createTable5()
{
    QSqlQuery query;

    // 创建订单表
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS Orders (
            order_id TEXT PRIMARY KEY,
            product_id TEXT NOT NULL,
            unit_price REAL NOT NULL,
            quantity INTEGER NOT NULL,
            total_price REAL NOT NULL,
            note TEXT,
            user TEXT NOT NULL,
            contact_info TEXT,
            status TEXT DEFAULT 'pending',
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            update_time TEXT DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (product_id) REFERENCES Products(product_id) ON DELETE SET NULL
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table Orders: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table Orders created successfully.";

    // 创建索引
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_order_status ON Orders(status)",
        "CREATE INDEX IF NOT EXISTS idx_order_user ON Orders(user)",
        "CREATE INDEX IF NOT EXISTS idx_order_product ON Orders(product_id)",
        "CREATE INDEX IF NOT EXISTS idx_order_create_time ON Orders(create_time)"
    };

    for (const QString &indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qDebug() << "Error creating index: " << query.lastError().text();
        }
    }

    return true;
}
// 插入订单
bool DatabaseManager::insertOrder(const SQL_Order &order)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO Orders
        (order_id, product_id, unit_price, quantity, total_price,
         note, user, contact_info, status)
        VALUES (:order_id, :product_id, :unit_price, :quantity, :total_price,
                :note, :user, :contact_info, :status)
    )");

    query.bindValue(":order_id", order.orderId);
    query.bindValue(":product_id", order.productId);
    query.bindValue(":unit_price", order.unitPrice);
    query.bindValue(":quantity", order.quantity);
    query.bindValue(":total_price", order.totalPrice);
    query.bindValue(":note", order.note);
    query.bindValue(":user", order.user);
    query.bindValue(":contact_info", order.contactInfo);
    query.bindValue(":status", order.status.isEmpty() ? "pending" : order.status);

    if (!query.exec()) {
        qDebug() << "Error inserting order: " << query.lastError().text();
        qDebug() << "Last query:" << query.lastQuery();
        return false;
    }

    qDebug() << "Order inserted successfully. ID:" << order.orderId;
    return true;
}

// 更新订单
bool DatabaseManager::updateOrder(const SQL_Order &order)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE Orders
        SET product_id = :product_id,
            unit_price = :unit_price,
            quantity = :quantity,
            total_price = :total_price,
            note = :note,
            user = :user,
            contact_info = :contact_info,
            status = :status,
            update_time = datetime('now', 'localtime')
        WHERE order_id = :order_id
    )");

    query.bindValue(":order_id", order.orderId);
    query.bindValue(":product_id", order.productId);
    query.bindValue(":unit_price", order.unitPrice);
    query.bindValue(":quantity", order.quantity);
    query.bindValue(":total_price", order.totalPrice);
    query.bindValue(":note", order.note);
    query.bindValue(":user", order.user);
    query.bindValue(":contact_info", order.contactInfo);
    query.bindValue(":status", order.status);

    if (!query.exec()) {
        qDebug() << "Error updating order: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No order found with ID:" << order.orderId;
        return false;
    }

    qDebug() << "Order updated successfully. ID:" << order.orderId;
    return true;
}

// 更新订单状态
bool DatabaseManager::updateOrderStatus(const QString &orderId, const QString &status)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE Orders
        SET status = :status,
            update_time = datetime('now', 'localtime')
        WHERE order_id = :order_id
    )");

    query.bindValue(":order_id", orderId);
    query.bindValue(":status", status);

    if (!query.exec()) {
        qDebug() << "Error updating order status: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No order found with ID:" << orderId;
        return false;
    }

    qDebug() << "Order status updated successfully. ID:" << orderId << "Status:" << status;
    return true;
}

// 删除订单
bool DatabaseManager::deleteOrder(const QString &orderId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM Orders WHERE order_id = :order_id");
    query.bindValue(":order_id", orderId);

    if (!query.exec()) {
        qDebug() << "Error deleting order: " << query.lastError().text();
        return false;
    }

    qDebug() << "Order deleted successfully. ID:" << orderId;
    return true;
}

// 获取所有订单
QList<SQL_Order> DatabaseManager::getAllOrders()
{
    QList<SQL_Order> orders;
    QSqlQuery query("SELECT * FROM Orders ORDER BY create_time DESC");

    while (query.next()) {
        SQL_Order order = extractOrderFromQuery(query);
        orders.append(order);
    }

    qDebug() << "Retrieved" << orders.size() << "orders";
    return orders;
}

// 根据ID获取订单
SQL_Order DatabaseManager::getOrderById(const QString &orderId)
{
    SQL_Order order;
    QSqlQuery query;
    query.prepare("SELECT * FROM Orders WHERE order_id = :order_id");
    query.bindValue(":order_id", orderId);

    if (query.exec() && query.next()) {
        order = extractOrderFromQuery(query);
    } else {
        qDebug() << "Error fetching order by ID:" << orderId
                 << "Error:" << query.lastError().text();
    }

    return order;
}

// 根据用户获取订单
QList<SQL_Order> DatabaseManager::getOrdersByUser(const QString &user)
{
    QList<SQL_Order> orders;
    QSqlQuery query;
    query.prepare("SELECT * FROM Orders WHERE user = :user ORDER BY create_time DESC");
    query.bindValue(":user", user);

    if (query.exec()) {
        while (query.next()) {
            SQL_Order order = extractOrderFromQuery(query);
            orders.append(order);
        }
    } else {
        qDebug() << "Error fetching orders by user:" << query.lastError().text();
    }

    return orders;
}

// 根据状态获取订单
QList<SQL_Order> DatabaseManager::getOrdersByStatus(const QString &status)
{
    QList<SQL_Order> orders;
    QSqlQuery query;
    query.prepare("SELECT * FROM Orders WHERE status = :status ORDER BY create_time DESC");
    query.bindValue(":status", status);

    if (query.exec()) {
        while (query.next()) {
            SQL_Order order = extractOrderFromQuery(query);
            orders.append(order);
        }
    } else {
        qDebug() << "Error fetching orders by status:" << query.lastError().text();
    }

    return orders;
}

// 根据产品获取订单
QList<SQL_Order> DatabaseManager::getOrdersByProduct(const QString &productId)
{
    QList<SQL_Order> orders;
    QSqlQuery query;
    query.prepare("SELECT * FROM Orders WHERE product_id = :product_id ORDER BY create_time DESC");
    query.bindValue(":product_id", productId);

    if (query.exec()) {
        while (query.next()) {
            SQL_Order order = extractOrderFromQuery(query);
            orders.append(order);
        }
    } else {
        qDebug() << "Error fetching orders by product:" << query.lastError().text();
    }

    return orders;
}

// 根据时间范围获取订单
QList<SQL_Order> DatabaseManager::getOrdersByTimeRange(const QString &startTime, const QString &endTime)
{
    QList<SQL_Order> orders;
    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM Orders
        WHERE create_time >= :start_time AND create_time <= :end_time
        ORDER BY create_time DESC
    )");
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);

    if (query.exec()) {
        while (query.next()) {
            SQL_Order order = extractOrderFromQuery(query);
            orders.append(order);
        }
    } else {
        qDebug() << "Error fetching orders by time range:" << query.lastError().text();
    }

    return orders;
}
bool DatabaseManager::insertCommandHistory(const SQL_CommandHistory &command)
{
    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO CommandHistory
        (command_id, status, action, sub_action, start_time, end_time,
         remark, completeness, completed_url)
        VALUES (:command_id, :status, :action, :sub_action, :start_time,
                :end_time, :remark, :completeness, :completed_url)
    )");

    query.bindValue(":command_id", command.commandId);
    query.bindValue(":status", command.status);
    query.bindValue(":action", command.action);
    query.bindValue(":sub_action", command.sub_action);
    query.bindValue(":start_time", command.start_time);
    query.bindValue(":end_time", command.end_time);
    query.bindValue(":remark", command.remark);
    query.bindValue(":completeness", command.Completeness);  // 注意：小写c
    query.bindValue(":completed_url", command.completed_url);

    if (!query.exec()) {
        qDebug() << "Error inserting command history: " << query.lastError().text();
        qDebug() << "Last query:" << query.lastQuery();
        qDebug() << "Bound values:" << query.boundValues();
        return false;
    }

    qDebug() << "Command history inserted successfully. ID:" << command.commandId;
    return true;
}
bool DatabaseManager::updateCommandHistory(const SQL_CommandHistory &command)
{
    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET status = :status,
            action = :action,
            sub_action = :sub_action,
            start_time = :start_time,
            end_time = :end_time,
            remark = :remark,
            completeness = :completeness,
            completed_url = :completed_url,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");

    query.bindValue(":command_id", command.commandId);
    query.bindValue(":status", command.status);
    query.bindValue(":action", command.action);
    query.bindValue(":sub_action", command.sub_action);
    query.bindValue(":start_time", command.start_time);
    query.bindValue(":end_time", command.end_time);
    query.bindValue(":remark", command.remark);
    query.bindValue(":completeness", command.Completeness);
    query.bindValue(":completed_url", command.completed_url);

    if (!query.exec()) {
        qDebug() << "Error updating command history: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No command found with ID:" << command.commandId;
        return false;
    }

    qDebug() << "Command history updated successfully. ID:" << command.commandId;
    return true;
}
bool DatabaseManager::deleteCommandHistory(const QString &commandId)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CommandHistory WHERE command_id = :command_id");
    query.bindValue(":command_id", commandId);

    if (!query.exec()) {
        qDebug() << "Error deleting command history: " << query.lastError().text();
        return false;
    }

    qDebug() << "Command history deleted successfully. ID:" << commandId;
    return true;
}

bool DatabaseManager::deleteCommandHistoryByStatus(const QString &status)
{
    QSqlQuery query;
    query.prepare("DELETE FROM CommandHistory WHERE status = :status");
    query.bindValue(":status", status);

    if (!query.exec()) {
        qDebug() << "Error deleting command history by status: " << query.lastError().text();
        return false;
    }

    qDebug() << "Deleted command history with status:" << status
             << ", count:" << query.numRowsAffected();
    return true;
}

bool DatabaseManager::deleteCommandHistoryByTimeRange(const QString &startTime, const QString &endTime)
{
    QSqlQuery query;
    query.prepare(R"(
        DELETE FROM CommandHistory
        WHERE start_time >= :start_time AND start_time <= :end_time
    )");
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);

    if (!query.exec()) {
        qDebug() << "Error deleting command history by time range: " << query.lastError().text();
        return false;
    }

    qDebug() << "Deleted command history between" << startTime << "and" << endTime
             << ", count:" << query.numRowsAffected();
    return true;
}
// 获取所有指令
QList<SQL_CommandHistory> DatabaseManager::getAllCommands()
{
    QList<SQL_CommandHistory> commands;
    QSqlQuery query("SELECT * FROM CommandHistory ORDER BY start_time DESC");

    while (query.next()) {
        SQL_CommandHistory command = extractCommandFromQuery(query);
        commands.append(command);
    }

    qDebug() << "Retrieved" << commands.size() << "command records";
    return commands;
}

// 根据ID获取指令
SQL_CommandHistory DatabaseManager::getCommandById(const QString &commandId)
{
    SQL_CommandHistory command;
    QSqlQuery query;
    query.prepare("SELECT * FROM CommandHistory WHERE command_id = :command_id");
    query.bindValue(":command_id", commandId);

    if (query.exec() && query.next()) {
        command = extractCommandFromQuery(query);
    } else {
        qDebug() << "Error fetching command by ID:" << commandId
                 << "Error:" << query.lastError().text();
    }

    return command;
}

// 根据状态获取指令
QList<SQL_CommandHistory> DatabaseManager::getCommandsByStatus(const QString &status)
{
    QList<SQL_CommandHistory> commands;
    QSqlQuery query;
    query.prepare("SELECT * FROM CommandHistory WHERE status = :status ORDER BY start_time DESC");
    query.bindValue(":status", status);

    if (query.exec()) {
        while (query.next()) {
            SQL_CommandHistory command = extractCommandFromQuery(query);
            commands.append(command);
        }
    } else {
        qDebug() << "Error fetching commands by status:" << query.lastError().text();
    }

    return commands;
}

// 根据动作获取指令
QList<SQL_CommandHistory> DatabaseManager::getCommandsByAction(const QString &action)
{
    QList<SQL_CommandHistory> commands;
    QSqlQuery query;
    query.prepare("SELECT * FROM CommandHistory WHERE action = :action ORDER BY start_time DESC");
    query.bindValue(":action", action);

    if (query.exec()) {
        while (query.next()) {
            SQL_CommandHistory command = extractCommandFromQuery(query);
            commands.append(command);
        }
    } else {
        qDebug() << "Error fetching commands by action:" << query.lastError().text();
    }

    return commands;
}

// 根据时间范围获取指令
QList<SQL_CommandHistory> DatabaseManager::getCommandsByTimeRange(const QString &startTime, const QString &endTime)
{
    QList<SQL_CommandHistory> commands;
    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM CommandHistory
        WHERE start_time >= :start_time AND start_time <= :end_time
        ORDER BY start_time DESC
    )");
    query.bindValue(":start_time", startTime);
    query.bindValue(":end_time", endTime);

    if (query.exec()) {
        while (query.next()) {
            SQL_CommandHistory command = extractCommandFromQuery(query);
            commands.append(command);
        }
    } else {
        qDebug() << "Error fetching commands by time range:" << query.lastError().text();
    }

    return commands;
}
// 从查询结果中提取指令数据（辅助函数）
SQL_CommandHistory DatabaseManager::extractCommandFromQuery(const QSqlQuery &query)
{
    SQL_CommandHistory command;
    command.commandId = query.value("command_id").toString();
    command.status = query.value("status").toString();
    command.action = query.value("action").toString();
    command.sub_action = query.value("sub_action").toString();
    command.start_time = query.value("start_time").toString();
    command.end_time = query.value("end_time").toString();
    command.remark = query.value("remark").toString();
    command.Completeness = query.value("completeness").toString();
    command.completed_url = query.value("completed_url").toString();
    return command;
}

// 统计不同状态的指令数量
int DatabaseManager::getCommandCountByStatus(const QString &status)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM CommandHistory WHERE status = :status");
    query.bindValue(":status", status);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 获取所有不同的动作类型
QList<QString> DatabaseManager::getDistinctActions()
{
    QList<QString> actions;
    QSqlQuery query("SELECT DISTINCT action FROM CommandHistory WHERE action IS NOT NULL");

    while (query.next()) {
        QString action = query.value(0).toString();
        if (!action.isEmpty()) {
            actions.append(action);
        }
    }

    return actions;
}
// 批量插入指令
bool DatabaseManager::batchInsertCommands(const QList<SQL_CommandHistory> &commands)
{
    if (commands.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_CommandHistory &command : commands) {
        if (!insertCommandHistory(command)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch inserted" << commands.size() << "commands successfully";
    return true;
}

// 批量更新指令
bool DatabaseManager::batchUpdateCommands(const QList<SQL_CommandHistory> &commands)
{
    if (commands.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_CommandHistory &command : commands) {
        if (!updateCommandHistory(command)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch updated" << commands.size() << "commands successfully";
    return true;
}

// 根据状态统计订单数量
int DatabaseManager::getOrderCountByStatus(const QString &status)
{
    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Orders WHERE status = :status");
    query.bindValue(":status", status);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 获取总销售额
double DatabaseManager::getTotalSales()
{
    QSqlQuery query("SELECT SUM(total_price) FROM Orders WHERE status = 'completed'");

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

// 获取指定产品的销售额
double DatabaseManager::getTotalSalesByProduct(const QString &productId)
{
    QSqlQuery query;
    query.prepare("SELECT SUM(total_price) FROM Orders WHERE product_id = :product_id AND status = 'completed'");
    query.bindValue(":product_id", productId);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

// 按状态统计订单
QMap<QString, int> DatabaseManager::getOrderStatistics()
{
    QMap<QString, int> statistics;
    QSqlQuery query("SELECT status, COUNT(*) FROM Orders GROUP BY status");

    while (query.next()) {
        QString status = query.value(0).toString();
        int count = query.value(1).toInt();
        statistics[status] = count;
    }

    return statistics;
}
// 批量插入订单
bool DatabaseManager::batchInsertOrders(const QList<SQL_Order> &orders)
{
    if (orders.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_Order &order : orders) {
        if (!insertOrder(order)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch inserted" << orders.size() << "orders successfully";
    return true;
}

// 批量更新订单
bool DatabaseManager::batchUpdateOrders(const QList<SQL_Order> &orders)
{
    if (orders.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_Order &order : orders) {
        if (!updateOrder(order)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch updated" << orders.size() << "orders successfully";
    return true;
}
// 从查询结果中提取订单数据
SQL_Order DatabaseManager::extractOrderFromQuery(const QSqlQuery &query)
{
    SQL_Order order;
    order.orderId = query.value("order_id").toString();
    order.productId = query.value("product_id").toString();
    order.unitPrice = query.value("unit_price").toDouble();
    order.quantity = query.value("quantity").toInt();
    order.totalPrice = query.value("total_price").toDouble();
    order.note = query.value("note").toString();
    order.user = query.value("user").toString();
    order.contactInfo = query.value("contact_info").toString();
    order.status = query.value("status").toString();
    order.createTime = query.value("create_time").toString();
    order.updateTime = query.value("update_time").toString();

    return order;
}
#if 0
// 创建订单
void createOrderExample()
{
    DatabaseManager dbManager;
    if (!dbManager.openDatabase("your_database.db")) {
        return;
    }

    // 确保表存在
    dbManager.createTable5();

    // 创建订单
    SQL_Order order;
    order.orderId = "ORD" + QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    order.productId = "PROD001";
    order.unitPrice = 99.99;
    order.quantity = 2;
    order.calculateTotal(); // 计算总价
    order.note = "请尽快发货";
    order.user = "张三";
    order.contactInfo = "13800138000";
    order.status = "pending";

    // 插入订单
    if (dbManager.insertOrder(order)) {
        qDebug() << "Order created successfully";
    }

    // 查询用户的所有订单
    QList<SQL_Order> userOrders = dbManager.getOrdersByUser("张三");
    qDebug() << "User has" << userOrders.size() << "orders";

    // 更新订单状态
    dbManager.updateOrderStatus(order.orderId, "paid");

    // 获取统计信息
    int pendingCount = dbManager.getOrderCountByStatus("pending");
    double totalSales = dbManager.getTotalSales();
    qDebug() << "Pending orders:" << pendingCount << "Total sales:" << totalSales;
}

// 生成订单ID的辅助函数
QString generateOrderId()
{
    QString timestamp = QDateTime::currentDateTime().toString("yyyyMMddHHmmss");
    QString random = QString::number(QRandomGenerator::global()->bounded(1000, 9999));
    return "ORD_" + timestamp + "_" + random;
}
static SQL_CommandHistory extractCommandFromQuery(const QSqlQuery &query)
{
    SQL_CommandHistory command;
    command.commandId = query.value("command_id").toString();
    command.status = query.value("status").toString();
    command.action = query.value("action").toString();
    command.sub_action = query.value("sub_action").toString();
    command.start_time = query.value("start_time").toString();
    command.end_time = query.value("end_time").toString();
    command.remark = query.value("remark").toString();
    command.Completeness = query.value("completeness").toString();
    command.completed_url = query.value("completed_url").toString();
    return command;
}
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
