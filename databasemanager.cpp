#include "DatabaseManager.h"

#include <QVariantList>
#include <QVariant>
#include <QSqlRecord>
#include <QMap>

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
// 验证token是否有效
bool DatabaseManager::validateToken(const QString &token)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare(R"(
        SELECT username FROM UserTokens
        WHERE token = :token
        AND expire_time > datetime('now', 'localtime')
    )");

    query.bindValue(":token", token);

    if (query.exec() && query.next()) {
        QString username = query.value("username").toString();
        qDebug() << "Token valid for user:" << username;
        return true;
    }

    qDebug() << "Token invalid or expired";
    return false;
}

// 根据token获取用户名
QString DatabaseManager::getUsernameByToken(const QString &token)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare(R"(
        SELECT username FROM UserTokens
        WHERE token = :token
        AND expire_time > datetime('now', 'localtime')
    )");

    query.bindValue(":token", token);

    if (query.exec() && query.next()) {
        return query.value("username").toString();
    }

    return "";
}

// 删除用户token（退出登录时使用）
bool DatabaseManager::deleteUserToken(const QString &username)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare("DELETE FROM UserTokens WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error deleting user token: " << query.lastError().text();
        return false;
    }

    qDebug() << "User token deleted for:" << username;
    return true;
}

// 清理过期token
bool DatabaseManager::cleanExpiredTokens()
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare("DELETE FROM UserTokens WHERE expire_time <= datetime('now', 'localtime')");

    if (!query.exec()) {
        qDebug() << "Error cleaning expired tokens: " << query.lastError().text();
        return false;
    }

    qDebug() << "Expired tokens cleaned";
    return true;
}
bool DatabaseManager::createTables()
{
    return createTable1() && createTable2() && createTable3() && createAppealTable()&&
           createTable4() && createTable5() && createTable6() && createTable7() &&  // 添加商品表
           createWithdrawTable()&& createTokenTable()&& createAppAccountTable();
}
// 保存token
bool DatabaseManager::saveUserToken(const QString &username, const QString &token)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    // 设置token过期时间（24小时后）
    QDateTime expireTime = QDateTime::currentDateTime().addSecs(24 * 3600);

    query.prepare(R"(
        INSERT OR REPLACE INTO UserTokens
        (username, token, expire_time)
        VALUES (:username, :token, :expire_time)
    )");

    query.bindValue(":username", username);
    query.bindValue(":token", token);
    query.bindValue(":expire_time", expireTime.toString("yyyy-MM-dd HH:mm:ss"));

    if (!query.exec()) {
        qDebug() << "Error saving user token: " << query.lastError().text();
        return false;
    }

    qDebug() << "User token saved for:" << username;
    return true;
}

bool DatabaseManager::createTokenTable()
{
    QSqlQuery query;

    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS UserTokens (
            username TEXT PRIMARY KEY,
            token TEXT NOT NULL,
            expire_time TEXT NOT NULL,
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (username) REFERENCES Users(username) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table UserTokens: " << query.lastError().text();
        return false;
    }

    // 创建索引
    query.exec("CREATE INDEX IF NOT EXISTS idx_token ON UserTokens(token)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_token_expire ON UserTokens(expire_time)");

    qDebug() << "Table UserTokens created successfully.";
    return true;
}
bool DatabaseManager::createTable7()
{
    QSqlQuery query;

    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS Products (
            product_id TEXT PRIMARY KEY,
            product_name TEXT NOT NULL,
            category_id TEXT NOT NULL,
            category_name TEXT,
            unit_price REAL NOT NULL DEFAULT 0.0,
            stock INTEGER NOT NULL DEFAULT 0,
            min_order INTEGER NOT NULL DEFAULT 1,
            max_order INTEGER NOT NULL DEFAULT 9999,
            status TEXT NOT NULL DEFAULT 'active',
            action TEXT,
            subaction TEXT,
            description TEXT,
            image_url TEXT,
            tags TEXT,
            specifications TEXT,
            sales_count INTEGER DEFAULT 0,
            rating REAL DEFAULT 0.0,
            rating_count INTEGER DEFAULT 0,
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            update_time TEXT DEFAULT (datetime('now', 'localtime')),

            -- 添加外键约束（如果存在Categories表）
            -- FOREIGN KEY (category_id) REFERENCES Categories(category_id)

            -- 添加检查约束
            CHECK (unit_price >= 0),
            CHECK (stock >= 0),
            CHECK (min_order >= 1),
            CHECK (max_order >= min_order)
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table Products: " << query.lastError().text();
        return false;
    }

    qDebug() << "Table Products created successfully.";

    // 创建索引以提高查询性能
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_product_category ON Products(category_id)",
        "CREATE INDEX IF NOT EXISTS idx_product_status ON Products(status)",
        "CREATE INDEX IF NOT EXISTS idx_product_price ON Products(unit_price)",
        "CREATE INDEX IF NOT EXISTS idx_product_stock ON Products(stock)",
        "CREATE INDEX IF NOT EXISTS idx_product_create_time ON Products(create_time)",
        "CREATE INDEX IF NOT EXISTS idx_product_sales ON Products(sales_count)",
        "CREATE INDEX IF NOT EXISTS idx_product_rating ON Products(rating)"
    };

    for (const QString &indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qDebug() << "Error creating index: " << query.lastError().text();
        }
    }

    return true;
}

bool DatabaseManager::insertProduct(const SQL_Product &product)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO Products
        (product_id, product_name, category_id, category_name, unit_price,
         stock, min_order, max_order, status, action, subaction, description,
         image_url, tags, specifications, sales_count, rating, rating_count)
        VALUES
        (:product_id, :product_name, :category_id, :category_name, :unit_price,
         :stock, :min_order, :max_order, :status, :action, :subaction, :description,
         :image_url, :tags, :specifications, :sales_count, :rating, :rating_count)
    )");

    query.bindValue(":product_id", product.productId);
    query.bindValue(":product_name", product.productName);
    query.bindValue(":category_id", product.categoryId);
    query.bindValue(":category_name", product.categoryName);
    query.bindValue(":unit_price", product.unitPrice);
    query.bindValue(":stock", product.stock);
    query.bindValue(":min_order", product.minOrder);
    query.bindValue(":max_order", product.maxOrder);
    query.bindValue(":status", product.status.isEmpty() ? "active" : product.status);
    query.bindValue(":action", product.action);
    query.bindValue(":subaction", product.subaction);
    query.bindValue(":description", product.description);
    query.bindValue(":image_url", product.imageUrl);
    query.bindValue(":tags", product.tags);
    query.bindValue(":specifications", product.specifications);
    query.bindValue(":sales_count", product.salesCount);
    query.bindValue(":rating", product.rating);
    query.bindValue(":rating_count", product.ratingCount);

    if (!query.exec()) {
        qDebug() << "Error inserting product: " << query.lastError().text();
        qDebug() << "Last query:" << query.lastQuery();
        qDebug() << "Bound values:" << query.boundValues();
        return false;
    }

    qDebug() << "Product inserted successfully. ID:" << product.productId;
    return true;
}

bool DatabaseManager::updateProduct(const SQL_Product &product)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare(R"(
        UPDATE Products
        SET product_name = :product_name,
            category_id = :category_id,
            category_name = :category_name,
            unit_price = :unit_price,
            stock = :stock,
            min_order = :min_order,
            max_order = :max_order,
            status = :status,
            action = :action,
            subaction = :subaction,
            description = :description,
            image_url = :image_url,
            tags = :tags,
            specifications = :specifications,
            update_time = datetime('now', 'localtime')
        WHERE product_id = :product_id
    )");

    query.bindValue(":product_id", product.productId);
    query.bindValue(":product_name", product.productName);
    query.bindValue(":category_id", product.categoryId);
    query.bindValue(":category_name", product.categoryName);
    query.bindValue(":unit_price", product.unitPrice);
    query.bindValue(":stock", product.stock);
    query.bindValue(":min_order", product.minOrder);
    query.bindValue(":max_order", product.maxOrder);
    query.bindValue(":status", product.status);
    query.bindValue(":action", product.action);
    query.bindValue(":subaction", product.subaction);
    query.bindValue(":description", product.description);
    query.bindValue(":image_url", product.imageUrl);
    query.bindValue(":tags", product.tags);
    query.bindValue(":specifications", product.specifications);

    if (!query.exec()) {
        qDebug() << "Error updating product: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No product found with ID:" << product.productId;
        return false;
    }

    qDebug() << "Product updated successfully. ID:" << product.productId;
    return true;
}

bool DatabaseManager::deleteProduct(const QString &productId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("DELETE FROM Products WHERE product_id = :product_id");
    query.bindValue(":product_id", productId);

    if (!query.exec()) {
        qDebug() << "Error deleting product: " << query.lastError().text();
        return false;
    }

    qDebug() << "Product deleted successfully. ID:" << productId;
    return true;
}

SQL_Product DatabaseManager::getProductById(const QString &productId)
{

    SQL_Product product;
    QSqlQuery query;
    query.prepare("SELECT * FROM Products WHERE product_id = :product_id");
    query.bindValue(":product_id", productId);

    if (query.exec() && query.next()) {
        product = extractProductFromQuery(query);
    } else {
        qDebug() << "Error fetching product by ID:" << productId
                 << "Error:" << query.lastError().text();
    }

    return product;
}

QList<SQL_Product> DatabaseManager::getAllProducts()
{

    QMutexLocker locker(&m_mutex);

    QList<SQL_Product> products;
    qDebug() << "1231231231323products";

    QSqlQuery query("SELECT * FROM Products ORDER BY create_time DESC");

    while (query.next()) {
        SQL_Product product = extractProductFromQuery(query);
        products.append(product);
    }

    qDebug() << "Retrieved" << products.size() << "products";
    return products;
}
QList<SQL_Product> DatabaseManager::getProductsByCategory(const QString &categoryId)
{

    QMutexLocker locker(&m_mutex);

    QList<SQL_Product> products;
    QSqlQuery query;
    query.prepare("SELECT * FROM Products WHERE category_id = :category_id ORDER BY create_time DESC");
    query.bindValue(":category_id", categoryId);

    if (query.exec()) {
        while (query.next()) {
            SQL_Product product = extractProductFromQuery(query);
            products.append(product);
        }
    } else {
        qDebug() << "Error fetching products by category:" << query.lastError().text();
    }

    return products;
}

QList<SQL_Product> DatabaseManager::searchProducts(const QString &keyword)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_Product> products;
    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM Products
        WHERE product_name LIKE :keyword
           OR description LIKE :keyword
           OR tags LIKE :keyword
        ORDER BY create_time DESC
    )");
    query.bindValue(":keyword", "%" + keyword + "%");

    if (query.exec()) {
        while (query.next()) {
            SQL_Product product = extractProductFromQuery(query);
            products.append(product);
        }
    } else {
        qDebug() << "Error searching products:" << query.lastError().text();
    }

    return products;
}


bool DatabaseManager::updateProductStock(const QString &productId, int quantity, bool increment = true)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    if (increment) {
        query.prepare("UPDATE Products SET stock = stock + :quantity, update_time = datetime('now', 'localtime') WHERE product_id = :product_id");
    } else {
        query.prepare("UPDATE Products SET stock = stock - :quantity, update_time = datetime('now', 'localtime') WHERE product_id = :product_id");
    }

    query.bindValue(":product_id", productId);
    query.bindValue(":quantity", quantity);

    if (!query.exec()) {
        qDebug() << "Error updating product stock: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No product found with ID:" << productId;
        return false;
    }

    qDebug() << "Product stock updated. ID:" << productId << "Quantity:" << quantity;
    return true;
}

bool DatabaseManager::updateProductSales(const QString &productId, int quantity)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE Products
        SET sales_count = sales_count + :quantity,
            stock = stock - :quantity,
            update_time = datetime('now', 'localtime')
        WHERE product_id = :product_id
    )");

    query.bindValue(":product_id", productId);
    query.bindValue(":quantity", quantity);

    if (!query.exec()) {
        qDebug() << "Error updating product sales: " << query.lastError().text();
        return false;
    }

    qDebug() << "Product sales updated. ID:" << productId << "Quantity:" << quantity;
    return true;
}



bool DatabaseManager::updateProductRating(const QString &productId, double newRating)
{
    QMutexLocker locker(&m_mutex);

    // 先获取当前评分信息
    SQL_Product product = getProductById(productId);
    if (product.productId.isEmpty()) {
        return false;
    }

    // 计算新的平均评分
    double totalRating = product.rating * product.ratingCount + newRating;
    int newRatingCount = product.ratingCount + 1;
    double newAverageRating = totalRating / newRatingCount;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE Products
        SET rating = :rating,
            rating_count = :rating_count,
            update_time = datetime('now', 'localtime')
        WHERE product_id = :product_id
    )");

    query.bindValue(":product_id", productId);
    query.bindValue(":rating", newAverageRating);
    query.bindValue(":rating_count", newRatingCount);

    if (!query.exec()) {
        qDebug() << "Error updating product rating: " << query.lastError().text();
        return false;
    }

    qDebug() << "Product rating updated. ID:" << productId << "New rating:" << newAverageRating;
    return true;
}

bool DatabaseManager::batchInsertProducts(const QList<SQL_Product> &products)
{
    QMutexLocker locker(&m_mutex);

    if (products.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_Product &product : products) {
        if (!insertProduct(product)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch inserted" << products.size() << "products successfully";
    return true;
}

bool DatabaseManager::batchUpdateProducts(const QList<SQL_Product> &products)
{
    QMutexLocker locker(&m_mutex);

    if (products.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_Product &product : products) {
        if (!updateProduct(product)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch updated" << products.size() << "products successfully";
    return true;
}

QList<SQL_Product> DatabaseManager::getNewProducts(int limit)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_Product> products;
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM Products ORDER BY create_time DESC LIMIT %1").arg(limit));

    if (query.exec()) {
        while (query.next()) {
            SQL_Product product = extractProductFromQuery(query);
            products.append(product);
        }
    } else {
        qDebug() << "Error fetching new products:" << query.lastError().text();
    }

    return products;
}

SQL_Product DatabaseManager::extractProductFromQuery(const QSqlQuery &query)
{
    SQL_Product product;

    product.productId = query.value("product_id").toString();
    product.productName = query.value("product_name").toString();
    product.categoryId = query.value("category_id").toString();
    product.categoryName = query.value("category_name").toString();
    product.unitPrice = query.value("unit_price").toDouble();
    product.stock = query.value("stock").toInt();
    product.minOrder = query.value("min_order").toInt();
    product.maxOrder = query.value("max_order").toInt();
    product.status = query.value("status").toString();
    product.action = query.value("action").toString();
    product.subaction = query.value("subaction").toString();
    product.description = query.value("description").toString();
    product.imageUrl = query.value("image_url").toString();
    product.createTime = query.value("create_time").toString();
    product.updateTime = query.value("update_time").toString();
    product.tags = query.value("tags").toString();
    product.specifications = query.value("specifications").toString();
    product.salesCount = query.value("sales_count").toInt();
    product.rating = query.value("rating").toDouble();
    product.ratingCount = query.value("rating_count").toInt();

    return product;
}

QList<SQL_Product> DatabaseManager::getHotProducts(int limit)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_Product> products;
    QSqlQuery query;
    query.prepare(QString("SELECT * FROM Products ORDER BY sales_count DESC LIMIT %1").arg(limit));

    if (query.exec()) {
        while (query.next()) {
            SQL_Product product = extractProductFromQuery(query);
            products.append(product);
        }
    } else {
        qDebug() << "Error fetching hot products:" << query.lastError().text();
    }

    return products;
}

QList<SQL_Product> DatabaseManager::getProductsByStatus(const QString &status)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_Product> products;
    QSqlQuery query;
    query.prepare("SELECT * FROM Products WHERE status = :status ORDER BY create_time DESC");
    query.bindValue(":status", status);

    if (query.exec()) {
        while (query.next()) {
            SQL_Product product = extractProductFromQuery(query);
            products.append(product);
        }
    } else {
        qDebug() << "Error fetching products by status:" << query.lastError().text();
    }

    return products;
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
            bound_time TEXT,
            tiktok TEXT DEFAULT '未登录',     -- 改为文本类型，存储状态描述
            bilibili TEXT DEFAULT '未登录',   -- 改为文本类型，存储状态描述
            xhs TEXT DEFAULT '未登录',        -- 改为文本类型，存储状态描述
            weibo TEXT DEFAULT '未登录',      -- 改为文本类型，存储状态描述
            kuaishou TEXT DEFAULT '未登录'    -- 改为文本类型，存储状态描述
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
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO Devices (serial_number, checksum, total_flow, ip_address,
                           device_status, bound_user, bound_time,
                           tiktok, bilibili, xhs, weibo, kuaishou)
        VALUES (:serial_number, :checksum, :total_flow, :ip_address,
                :device_status, :bound_user, :bound_time,
                :tiktok, :bilibili, :xhs, :weibo, :kuaishou)
    )");

    query.bindValue(":serial_number", device.serial_number);
    query.bindValue(":checksum", device.checksum);
    query.bindValue(":total_flow", device.total_flow);
    query.bindValue(":ip_address", device.ip_address);
    query.bindValue(":device_status", device.device_status);
    query.bindValue(":bound_user", device.bound_user);
    query.bindValue(":bound_time", device.bound_time);

    // 文本类型的社交媒体状态
    query.bindValue(":tiktok", device.tiktok.isEmpty() ? "未登录" : device.tiktok);
    query.bindValue(":bilibili", device.bilibili.isEmpty() ? "未登录" : device.bilibili);
    query.bindValue(":xhs", device.xhs.isEmpty() ? "未登录" : device.xhs);
    query.bindValue(":weibo", device.weibo.isEmpty() ? "未登录" : device.weibo);
    query.bindValue(":kuaishou", device.kuaishou.isEmpty() ? "未登录" : device.kuaishou);

    if (!query.exec()) {
        qDebug() << "Error inserting device: " << query.lastError().text();
        return false;
    }

    qDebug() << "Device inserted successfully.";
    return true;
}

bool DatabaseManager::updateDevice(const SQL_Device &device)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE Devices
        SET checksum = :checksum,
            total_flow = :total_flow,
            ip_address = :ip_address,
            device_status = :device_status,
            bound_user = :bound_user,
            bound_time = :bound_time,
            tiktok = :tiktok,
            bilibili = :bilibili,
            xhs = :xhs,
            weibo = :weibo,
            kuaishou = :kuaishou
        WHERE serial_number = :serial_number
    )");

    query.bindValue(":serial_number", device.serial_number);
    query.bindValue(":checksum", device.checksum);
    query.bindValue(":total_flow", device.total_flow);
    query.bindValue(":ip_address", device.ip_address);
    query.bindValue(":device_status", device.device_status);
    query.bindValue(":bound_user", device.bound_user);
    query.bindValue(":bound_time", device.bound_time);

    // 文本类型的社交媒体状态
    query.bindValue(":tiktok", device.tiktok.isEmpty() ? "未登录" : device.tiktok);
    query.bindValue(":bilibili", device.bilibili.isEmpty() ? "未登录" : device.bilibili);
    query.bindValue(":xhs", device.xhs.isEmpty() ? "未登录" : device.xhs);
    query.bindValue(":weibo", device.weibo.isEmpty() ? "未登录" : device.weibo);
    query.bindValue(":kuaishou", device.kuaishou.isEmpty() ? "未登录" : device.kuaishou);

    if (!query.exec()) {
        qDebug() << "Error updating device: " << query.lastError().text();
        return false;
    }

    return true;
}
bool DatabaseManager::deleteDevice(const QString &serial_number)
{
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

    qDebug() << "[DB] 开始查询所有设备...";

    QList<SQL_Device> devices;
    QSqlQuery query("SELECT * FROM Devices");

    if (!query.exec()) {
        qCritical() << "[DB] SQL查询失败：" << query.lastError().text();
        return devices;
    }

    //int count = 0;
    while (query.next()) {
        SQL_Device device;
        device.serial_number = query.value("serial_number").toString();
        device.checksum = query.value("checksum").toString();
        device.total_flow = query.value("total_flow").toString();
        device.ip_address = query.value("ip_address").toString();
        device.device_status = query.value("device_status").toString();
        device.bound_user = query.value("bound_user").toString();
        device.bound_time = query.value("bound_time").toString();

        // 修改为文本类型的社交媒体状态
        device.tiktok = query.value("tiktok").toString();
        device.bilibili = query.value("bilibili").toString();
        device.xhs = query.value("xhs").toString();
        device.weibo = query.value("weibo").toString();
        device.kuaishou = query.value("kuaishou").toString();

        devices.append(device);
     //   count++;
    }

    qDebug() << "[DB] 查询完成，共获取" << devices.size() << "个设备";

    // 简单打印一个样本设备（如果有设备的话）
    if (!devices.isEmpty()) {
        const SQL_Device &firstDevice = devices.first();
        qDebug() << "[DB] 样本设备 - SN:" << firstDevice.serial_number
                 << "状态:" << firstDevice.device_status
                 << "TikTok:" << firstDevice.tiktok
                 << "Bilibili:" << firstDevice.bilibili;
    }

    return devices;
}
SQL_Device DatabaseManager::getDeviceBySerialNumber(const QString &serial_number)
{
    QMutexLocker locker(&m_mutex);

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
        device.bound_time = query.value("bound_time").toString();
        device.tiktok = query.value("tiktok").toString();
        device.bilibili = query.value("bilibili").toString();
        device.xhs = query.value("xhs").toString();
        device.weibo = query.value("weibo").toString();
        device.kuaishou = query.value("kuaishou").toString();
    } else {
        qDebug() << "Error fetching device: " << query.lastError().text();
    }

    return device;
}

// 更新设备社交媒体开关
bool DatabaseManager::updateDeviceSocialMedia(const QString &serialNumber,
                                              const QString &tiktok, const QString &bilibili,
                                              const QString &xhs, const QString &weibo, const QString &kuaishou)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE Devices
        SET tiktok = :tiktok,
            bilibili = :bilibili,
            xhs = :xhs,
            weibo = :weibo,
            kuaishou = :kuaishou
        WHERE serial_number = :serial_number
    )");

    query.bindValue(":serial_number", serialNumber);
    query.bindValue(":tiktok", tiktok.isEmpty() ? "未登录" : tiktok);
    query.bindValue(":bilibili", bilibili.isEmpty() ? "未登录" : bilibili);
    query.bindValue(":xhs", xhs.isEmpty() ? "未登录" : xhs);
    query.bindValue(":weibo", weibo.isEmpty() ? "未登录" : weibo);
    query.bindValue(":kuaishou", kuaishou.isEmpty() ? "未登录" : kuaishou);

    if (!query.exec()) {
        qDebug() << "Error updating device social media: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No device found with serial number:" << serialNumber;
        return false;
    }

    qDebug() << "Device social media updated. SN:" << serialNumber
             << " TikTok:" << tiktok << " Bilibili:" << bilibili
             << " XHS:" << xhs << " Weibo:" << weibo << " Kuaishou:" << kuaishou;
    return true;
}


QMap<QString, QString> DatabaseManager::getDeviceSocialMedia(const QString &serialNumber)
{
    QMutexLocker locker(&m_mutex);

    QMap<QString, QString> socialMedia;

    QSqlQuery query;
    query.prepare("SELECT tiktok, bilibili, xhs, weibo, kuaishou FROM Devices WHERE serial_number = :serial_number");
    query.bindValue(":serial_number", serialNumber);

    if (query.exec() && query.next()) {
        socialMedia["tiktok"] = query.value("tiktok").toString();
        socialMedia["bilibili"] = query.value("bilibili").toString();
        socialMedia["xhs"] = query.value("xhs").toString();
        socialMedia["weibo"] = query.value("weibo").toString();
        socialMedia["kuaishou"] = query.value("kuaishou").toString();
    } else {
        qDebug() << "Error fetching device social media: " << query.lastError().text();
    }

    return socialMedia;
}

bool DatabaseManager::insertUser(const SQL_User &user)
{
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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

    // 修改创建表的SQL，添加failed_tasks字段
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
            total_tasks INTEGER DEFAULT 0,      -- 总任务数
            completed_tasks INTEGER DEFAULT 0,  -- 已完成任务数
            failed_tasks INTEGER DEFAULT 0,     -- 新增：失败任务数
            created_at TEXT DEFAULT (datetime('now', 'localtime')),
            updated_at TEXT DEFAULT (datetime('now', 'localtime'))
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table CommandHistory: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table CommandHistory created successfully.";

    // 创建索引（分开执行）
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

    // 创建订单表 - 添加必要的字段
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS Orders (
            order_id TEXT PRIMARY KEY,
            command_id TEXT,                    -- 关联指令表的指令ID
            product_id TEXT NOT NULL,
            product_name TEXT,                  -- 新增：商品名称
            unit_price REAL NOT NULL,
            quantity INTEGER NOT NULL,
            total_price REAL NOT NULL,
            note TEXT,
            user TEXT NOT NULL,
            contact_info TEXT,
            snapshot TEXT,                      -- 新增：截图URL（可从指令表获取）
            status TEXT DEFAULT 'pending',
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            update_time TEXT DEFAULT (datetime('now', 'localtime')),
            FOREIGN KEY (command_id) REFERENCES CommandHistory(command_id) ON DELETE SET NULL
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table Orders: " << query.lastError().text();
        return false;
    }

    // 更新插入订单函数，支持新字段
    return true;
}

bool DatabaseManager::insertOrder(const SQL_Order &order)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    // 调试输出查看字段值
    qDebug() << "Inserting order with values:";
    qDebug() << "  order_id:" << order.orderId;
    qDebug() << "  command_id:" << order.commandId;
    qDebug() << "  product_id:" << order.productId;
    qDebug() << "  product_name:" << order.productName;
    qDebug() << "  unit_price:" << order.unitPrice;
    qDebug() << "  quantity:" << order.quantity;
    qDebug() << "  total_price:" << order.totalPrice;
    qDebug() << "  note:" << order.note;
    qDebug() << "  user:" << order.user;
    qDebug() << "  contact_info:" << order.contactInfo;
    qDebug() << "  snapshot:" << order.snapshot;
    qDebug() << "  status:" << order.status;

    // 使用更简洁的SQL语句，明确指定所有字段
    QString sql = R"(
        INSERT INTO Orders
        (order_id, command_id, product_id, product_name, unit_price,
         quantity, total_price, note, user, contact_info, snapshot, status)
        VALUES
        (:order_id, :command_id, :product_id, :product_name, :unit_price,
         :quantity, :total_price, :note, :user, :contact_info, :snapshot, :status)
    )";

    query.prepare(sql);

    // 绑定所有参数
    query.bindValue(":order_id", order.orderId);
    query.bindValue(":command_id", order.commandId);
    query.bindValue(":product_id", order.productId);
    query.bindValue(":product_name", order.productName);
    query.bindValue(":unit_price", order.unitPrice);
    query.bindValue(":quantity", order.quantity);
    query.bindValue(":total_price", order.totalPrice);
    query.bindValue(":note", order.note);
    query.bindValue(":user", order.user);
    query.bindValue(":contact_info", order.contactInfo);
    query.bindValue(":snapshot", order.snapshot);
    query.bindValue(":status", order.status.isEmpty() ? "pending" : order.status);

    if (!query.exec()) {
        qDebug() << "Error inserting order: " << query.lastError().text();
        qDebug() << "SQL error:" << query.lastError().databaseText();
        qDebug() << "SQL driver error:" << query.lastError().driverText();
        qDebug() << "Last query:" << query.lastQuery();
        qDebug() << "Bound values:" << query.boundValues();
        return false;
    }

    qDebug() << "Order inserted successfully. ID:" << order.orderId;
    return true;
}
// 更新订单
bool DatabaseManager::updateOrder(const SQL_Order &order)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE Orders
        SET command_id = :command_id,           -- 新增
            product_id = :product_id,
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
    query.bindValue(":command_id", order.commandId);  // 新增绑定
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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    qDebug() << "=== 开始插入指令历史记录 ===";
    qDebug() << "指令ID:" << command.commandId;
    qDebug() << "状态:" << command.status;

    QString completedUrl = QString("Upload/%1/").arg(command.commandId);

    // 使用简单的SQL语句，添加failed_tasks字段
    QString sql = QString("INSERT INTO CommandHistory (command_id, status, action, sub_action, start_time, end_time, "
                          "remark, completeness, completed_url, total_tasks, completed_tasks, failed_tasks) "
                          "VALUES ('%1', '%2', '%3', '%4', '%5', '%6', '%7', '%8', '%9', %10, %11, %12)")
                      .arg(command.commandId)
                      .arg(command.status)
                      .arg(command.action)
                      .arg(command.sub_action)
                      .arg(command.start_time)
                      .arg(command.end_time)
                      .arg(command.remark)
                      .arg(command.completeness)
                      .arg(completedUrl)  // 使用固定的路径格式
                      .arg(command.total_tasks)
                      .arg(command.completed_tasks)
                      .arg(command.failed_tasks);  // 新增failed_tasks

    qDebug() << "生成的SQL:" << sql;

    if (!query.exec(sql)) {
        qDebug() << "插入失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "插入成功";
    qDebug() << "completed_url 已设置为:" << completedUrl;

    // 立即查询验证
    QSqlQuery verifyQuery;
    verifyQuery.prepare("SELECT command_id, completed_url FROM CommandHistory WHERE command_id = ?");
    verifyQuery.addBindValue(command.commandId);

    if (verifyQuery.exec() && verifyQuery.next()) {
        QString storedCommandId = verifyQuery.value("command_id").toString();
        QString storedUrl = verifyQuery.value("completed_url").toString();
        qDebug() << "验证成功:";
        qDebug() << "  - 指令ID:" << storedCommandId;
        qDebug() << "  - 存储的URL:" << storedUrl;

        // 验证格式是否正确
        QString expectedUrl = QString("Upload/%1/").arg(command.commandId);
        if (storedUrl == expectedUrl) {
            qDebug() << "  - URL格式验证: 正确";
        } else {
            qDebug() << "  - URL格式验证: 错误 (期望:" << expectedUrl << ")";
        }
    } else {
        qDebug() << "验证失败，未找到记录";
    }

    return true;
}

bool DatabaseManager::clearAllCommandHistory()
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    QString sql = "DELETE FROM CommandHistory";

    if (!query.exec(sql)) {
        qDebug() << "清空指令历史失败:" << query.lastError().text();
        return false;
    }

    // 重置自增ID（如果使用SQLite）
    query.exec("DELETE FROM sqlite_sequence WHERE name='CommandHistory'");

    qDebug() << "已清空所有指令历史记录";
    return true;
}

bool DatabaseManager::updateCommandHistory(const SQL_CommandHistory &command)
{
    QMutexLocker locker(&m_mutex);

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
            total_tasks = :total_tasks,
            completed_tasks = :completed_tasks,
            failed_tasks = :failed_tasks,  -- 新增
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
    query.bindValue(":completeness", command.completeness);
    query.bindValue(":completed_url", command.completed_url);
    query.bindValue(":total_tasks", command.total_tasks);
    query.bindValue(":completed_tasks", command.completed_tasks);
    query.bindValue(":failed_tasks", command.failed_tasks);  // 新增绑定

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

    QList<SQL_CommandHistory> commands;
    QSqlQuery query("SELECT * FROM CommandHistory ORDER BY start_time DESC");

    qDebug() << "=== 查询指令历史 ===";
    qDebug() << "SQL:" << query.lastQuery();

    if (!query.exec()) {
        qDebug() << "查询失败:" << query.lastError().text();
        return commands;
    }

    int count = 0;
    while (query.next()) {
        SQL_CommandHistory command = extractCommandFromQuery(query);

        // 调试输出每个指令的详细信息
        qDebug() << "指令" << count + 1 << ":";
        qDebug() << "  ID:" << command.commandId;
        qDebug() << "  状态:" << command.status;
        qDebug() << "  动作:" << command.action;
        qDebug() << "  完成度:" << command.completeness;
        qDebug() << "  总任务:" << command.total_tasks;

        commands.append(command);
        count++;
    }

    qDebug() << "总共检索到" << commands.size() << "条指令记录";
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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    command.completeness = query.value("completeness").toString();
    command.completed_url = query.value("completed_url").toString();
    command.total_tasks = query.value("total_tasks").toInt();
    command.completed_tasks = query.value("completed_tasks").toInt();
    command.failed_tasks = query.value("failed_tasks").toInt();  // 新增
    return command;
}
bool DatabaseManager::incrementCommandFailedTasks(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "=== 增加指令失败任务数 ===";
    qDebug() << "指令ID:" << commandId;

    // 先获取当前的任务信息
    SQL_CommandHistory command = getCommandById(commandId);
    if (command.commandId.isEmpty()) {
        qDebug() << "指令不存在:" << commandId;
        return false;
    }

    // 检查是否已经完成所有任务（成功+失败）
    int totalFinished = command.completed_tasks + command.failed_tasks;
    if (totalFinished >= command.total_tasks) {
        qDebug() << "指令已完成所有任务，无需增加失败数";
        qDebug() << "总任务:" << command.total_tasks;
        qDebug() << "已完成（成功+失败）:" << totalFinished;
        return false;
    }

    // 增加失败任务数
    int newFailedTasks = command.failed_tasks + 1;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET failed_tasks = :failed_tasks,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");
    query.bindValue(":failed_tasks", newFailedTasks);
    query.bindValue(":command_id", commandId);

    if (!query.exec()) {
        qDebug() << "增加指令失败任务数失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "指令失败任务数更新成功";
    qDebug() << "原失败数:" << command.failed_tasks << "-> 新失败数:" << newFailedTasks;
    qDebug() << "总任务数:" << command.total_tasks;
    qDebug() << "成功任务数:" << command.completed_tasks;

    // 更新指令状态（这会自动更新订单状态）
    updateCommandStatus(commandId);

    return true;
}

// 更新指令任务统计信息
bool DatabaseManager::updateCommandTaskStatistics(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    SQL_CommandHistory command = getCommandById(commandId);
    if (command.commandId.isEmpty()) {
        return false;
    }

    // 计算成功任务数（已完成任务数减去失败任务数）
    int successfulTasks = qMax(0, command.completed_tasks - command.failed_tasks);

    // 计算完成率（基于成功任务数）
    int completionRate = 0;
    if (command.total_tasks > 0) {
        completionRate = (successfulTasks * 100) / command.total_tasks;
    }

    // 根据任务状态更新指令状态
    QString newStatus = command.status;
    if (command.completed_tasks >= command.total_tasks) {
        if (command.failed_tasks == 0) {
            newStatus = "completed";
        } else if (command.failed_tasks == command.total_tasks) {
            newStatus = "failed";
        } else {
            newStatus = "partial";
        }
    }

    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET completeness = :completeness,
            status = :status,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");
    query.bindValue(":completeness", QString("%1%").arg(completionRate));
    query.bindValue(":status", newStatus);
    query.bindValue(":command_id", commandId);

    if (!query.exec()) {
        qDebug() << "更新指令任务统计失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "指令任务统计更新成功";
    qDebug() << "指令ID:" << commandId;
    qDebug() << "总任务:" << command.total_tasks;
    qDebug() << "完成:" << command.completed_tasks;
    qDebug() << "失败:" << command.failed_tasks;
    qDebug() << "成功:" << successfulTasks;
    qDebug() << "完成率:" << completionRate << "%";
    qDebug() << "状态:" << newStatus;

    return true;
}
// 更新指令的任务数量
bool DatabaseManager::updateCommandTasks(const QString &commandId, int total_tasks, int completed_tasks)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET total_tasks = :total_tasks,
            completed_tasks = :completed_tasks,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");

    query.bindValue(":command_id", commandId);
    query.bindValue(":total_tasks", total_tasks);
    query.bindValue(":completed_tasks", completed_tasks);

    if (!query.exec()) {
        qDebug() << "Error updating command tasks: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No command found with ID:" << commandId;
        return false;
    }

    qDebug() << "Command tasks updated. ID:" << commandId
             << "Total:" << total_tasks << "Completed:" << completed_tasks;
    return true;
}
// 统计不同状态的指令数量
int DatabaseManager::getCommandCountByStatus(const QString &status)
{
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

    QSqlQuery query("SELECT SUM(total_price) FROM Orders WHERE status = 'completed'");

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

// 获取指定产品的销售额
double DatabaseManager::getTotalSalesByProduct(const QString &productId)
{
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
    QMutexLocker locker(&m_mutex);

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
// 修改从查询结果中提取订单数据的函数
SQL_Order DatabaseManager::extractOrderFromQuery(const QSqlQuery &query)
{
    SQL_Order order;
    order.orderId = query.value("order_id").toString();
    order.commandId = query.value("command_id").toString();  // 新增
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
// 从查询结果中提取投诉记录数据
SQL_AppealRecord DatabaseManager::extractAppealFromQuery(const QSqlQuery &query)
{
    SQL_AppealRecord appeal;

    appeal.id = query.value("id").toInt();
    appeal.username = query.value("username").toString();
    appeal.orderId = query.value("order_id").toString();
    appeal.appealType = query.value("appeal_type").toString();
    appeal.contentPath = query.value("content_path").toString();
    appeal.textContent = query.value("text_content").toString();
    appeal.appealTime = query.value("appeal_time").toString();
    appeal.status = query.value("status").toString();
    appeal.operatorName = query.value("operator").toString();
    appeal.result = query.value("result").toString();
    appeal.resultTime = query.value("result_time").toString();
    appeal.processingStatus = query.value("processing_status").toString();
    appeal.appealLevel = query.value("appeal_level").toInt();
    appeal.priority = query.value("priority").toString();

    return appeal;
}
// 新增：根据指令ID获取订单
QList<SQL_Order> DatabaseManager::getOrdersByCommandId(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_Order> orders;
    QSqlQuery query;
    query.prepare("SELECT * FROM Orders WHERE command_id = :command_id ORDER BY create_time DESC");
    query.bindValue(":command_id", commandId);

    if (query.exec()) {
        while (query.next()) {
            SQL_Order order = extractOrderFromQuery(query);
            orders.append(order);
        }
    } else {
        qDebug() << "Error fetching orders by command ID:" << query.lastError().text();
    }

    return orders;
}
// 新增：根据指令ID获取第一个订单（如果有多个订单）
SQL_Order DatabaseManager::getFirstOrderByCommandId(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    SQL_Order order;
    QSqlQuery query;
    query.prepare("SELECT * FROM Orders WHERE command_id = :command_id ORDER BY create_time LIMIT 1");
    query.bindValue(":command_id", commandId);

    if (query.exec() && query.next()) {
        order = extractOrderFromQuery(query);
    } else {
        qDebug() << "Error fetching first order by command ID:" << commandId;
    }

    return order;
}

// 新增：检查指令是否已有关联订单
bool DatabaseManager::hasOrdersForCommand(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Orders WHERE command_id = :command_id");
    query.bindValue(":command_id", commandId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 新增：获取订单数量统计（包含command_id统计）
int DatabaseManager::getOrderCountByCommandId(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM Orders WHERE command_id = :command_id");
    query.bindValue(":command_id", commandId);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}
bool DatabaseManager::createTable6()
{
    QSqlQuery query;

    // 创建商城用户表 - 添加inviter_username字段
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS MallUsers (
            username TEXT PRIMARY KEY,
            password TEXT NOT NULL,
            email TEXT,
            invite_code TEXT,
            inviter_username TEXT,  -- 新增：邀请人账号
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            last_login_time TEXT,
            phone TEXT,
            user_level INTEGER DEFAULT 1,
            balance REAL DEFAULT 0.0,
            points INTEGER DEFAULT 0,
            avatar_url TEXT,
            real_name TEXT,
            id_card TEXT,
            address TEXT,
            is_vip INTEGER DEFAULT 0,
            vip_expire_time TEXT,
            status TEXT DEFAULT 'active',  -- active, inactive, banned
            FOREIGN KEY (inviter_username) REFERENCES MallUsers(username) ON DELETE SET NULL
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating table MallUsers: " << query.lastError().text();
        return false;
    }
    qDebug() << "Table MallUsers created successfully.";

    // 创建索引 - 添加对inviter_username的索引
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_malluser_email ON MallUsers(email)",
        "CREATE INDEX IF NOT EXISTS idx_malluser_invite_code ON MallUsers(invite_code)",
        "CREATE INDEX IF NOT EXISTS idx_malluser_inviter ON MallUsers(inviter_username)", // 新增索引
        "CREATE INDEX IF NOT EXISTS idx_malluser_phone ON MallUsers(phone)",
        "CREATE INDEX IF NOT EXISTS idx_malluser_create_time ON MallUsers(create_time)",
        "CREATE INDEX IF NOT EXISTS idx_malluser_status ON MallUsers(status)"
    };

    for (const QString &indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qDebug() << "Error creating index: " << query.lastError().text();
        }
    }

    return true;
}
// 插入商城用户
bool DatabaseManager::insertMallUser(const SQL_MallUser &user)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO MallUsers
        (username, password, email, invite_code, inviter_username, phone,
         user_level, balance, points, create_time)
        VALUES (:username, :password, :email, :invite_code, :inviter_username, :phone,
                :user_level, :balance, :points, :create_time)
    )");

    query.bindValue(":username", user.username);
    query.bindValue(":password", user.password);
    query.bindValue(":email", user.email);
    query.bindValue(":invite_code", user.inviteCode);
    query.bindValue(":inviter_username", user.inviterUsername);  // 新增绑定
    query.bindValue(":phone", user.phone);
    query.bindValue(":user_level", user.userLevel);
    query.bindValue(":balance", user.balance);
    query.bindValue(":points", user.points);
    query.bindValue(":create_time", user.createTime.isEmpty() ?
                                        QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss") : user.createTime);

    if (!query.exec()) {
        qDebug() << "Error inserting mall user: " << query.lastError().text();
        return false;
    }

    qDebug() << "Mall user inserted successfully. Username:" << user.username;
    return true;
}

// 更新商城用户
// 修改updateMallUser函数，添加inviter_username字段
bool DatabaseManager::updateMallUser(const SQL_MallUser &user)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE MallUsers
        SET email = :email,
            invite_code = :invite_code,
            inviter_username = :inviter_username,  -- 新增
            phone = :phone,
            user_level = :user_level,
            balance = :balance,
            points = :points,
            avatar_url = :avatar_url,
            real_name = :real_name,
            id_card = :id_card,
            address = :address,
            is_vip = :is_vip,
            vip_expire_time = :vip_expire_time,
            status = :status
        WHERE username = :username
    )");

    query.bindValue(":username", user.username);
    query.bindValue(":email", user.email);
    query.bindValue(":invite_code", user.inviteCode);
    query.bindValue(":inviter_username", user.inviterUsername);  // 新增绑定
    query.bindValue(":phone", user.phone);
    query.bindValue(":user_level", user.userLevel);
    query.bindValue(":balance", user.balance);
    query.bindValue(":points", user.points);
    // 可以添加更多字段的绑定

    if (!query.exec()) {
        qDebug() << "Error updating mall user: " << query.lastError().text();
        return false;
    }

    qDebug() << "Mall user updated successfully. Username:" << user.username;
    return true;
}
// 删除商城用户
bool DatabaseManager::deleteMallUser(const QString &username)
{
    QMutexLocker locker(&m_mutex);
    QSqlQuery query;
    query.prepare("DELETE FROM MallUsers WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error deleting mall user: " << query.lastError().text();
        return false;
    }

    qDebug() << "Mall user deleted successfully. Username:" << username;
    return true;
}

// 更新密码
bool DatabaseManager::updateMallUserPassword(const QString &username, const QString &newPassword)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("UPDATE MallUsers SET password = :password WHERE username = :username");
    query.bindValue(":username", username);
    query.bindValue(":password", newPassword);

    if (!query.exec()) {
        qDebug() << "Error updating mall user password: " << query.lastError().text();
        return false;
    }

    qDebug() << "Mall user password updated successfully. Username:" << username;
    return true;
}

// 更新最后登录时间
bool DatabaseManager::updateMallUserLastLogin(const QString &username)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("UPDATE MallUsers SET last_login_time = datetime('now', 'localtime') WHERE username = :username");
    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error updating last login time: " << query.lastError().text();
        return false;
    }

    qDebug() << "Last login time updated for user:" << username;
    return true;
}

// 更新余额
bool DatabaseManager::updateMallUserBalance(const QString &username, double amount)
{    QMutexLocker locker(&m_mutex);


    QSqlQuery query;
    query.prepare("UPDATE MallUsers SET balance = balance + :amount WHERE username = :username");
    query.bindValue(":username", username);
    query.bindValue(":amount", amount);

    if (!query.exec()) {
        qDebug() << "Error updating mall user balance: " << query.lastError().text();
        return false;
    }

    qDebug() << "Mall user balance updated. Username:" << username << "Amount:" << amount;
    return true;
}

// 更新积分
bool DatabaseManager::updateMallUserPoints(const QString &username, int points)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("UPDATE MallUsers SET points = points + :points WHERE username = :username");
    query.bindValue(":username", username);
    query.bindValue(":points", points);

    if (!query.exec()) {
        qDebug() << "Error updating mall user points: " << query.lastError().text();
        return false;
    }

    qDebug() << "Mall user points updated. Username:" << username << "Points:" << points;
    return true;
}

// 获取所有商城用户
QList<SQL_MallUser> DatabaseManager::getAllMallUsers()
{    QMutexLocker locker(&m_mutex);

    QList<SQL_MallUser> users;
    QSqlQuery query("SELECT * FROM MallUsers ORDER BY create_time DESC");

    while (query.next()) {
        SQL_MallUser user = extractMallUserFromQuery(query);
        users.append(user);
    }

    qDebug() << "Retrieved" << users.size() << "mall users";
    return users;
}

// 根据用户名获取商城用户
SQL_MallUser DatabaseManager::getMallUserByUsername(const QString &username)
{
    QMutexLocker locker(&m_mutex);

    SQL_MallUser user;
    QSqlQuery query;
    query.prepare("SELECT * FROM MallUsers WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        user = extractMallUserFromQuery(query);
    } else {
        qDebug() << "Error fetching mall user by username:" << username
                 << "Error:" << query.lastError().text();
    }

    return user;
}

// 根据邮箱获取商城用户
SQL_MallUser DatabaseManager::getMallUserByEmail(const QString &email)
{
    QMutexLocker locker(&m_mutex);

     SQL_MallUser user;
    QSqlQuery query;
    query.prepare("SELECT * FROM MallUsers WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        user = extractMallUserFromQuery(query);
    } else {
        qDebug() << "Error fetching mall user by email:" << email;
    }

    return user;
}

// 根据手机号获取商城用户
SQL_MallUser DatabaseManager::getMallUserByPhone(const QString &phone)
{
    QMutexLocker locker(&m_mutex);

    SQL_MallUser user;
    QSqlQuery query;
    query.prepare("SELECT * FROM MallUsers WHERE phone = :phone");
    query.bindValue(":phone", phone);

    if (query.exec() && query.next()) {
        user = extractMallUserFromQuery(query);
    } else {
        qDebug() << "Error fetching mall user by phone:" << phone;
    }

    return user;
}

// 根据邀请码获取商城用户
SQL_MallUser DatabaseManager::getMallUserByInviteCode(const QString &inviteCode)
{
    QMutexLocker locker(&m_mutex);

    SQL_MallUser user;
    QSqlQuery query;
    query.prepare("SELECT * FROM MallUsers WHERE invite_code = :invite_code");
    query.bindValue(":invite_code", inviteCode);

    if (query.exec() && query.next()) {
        user = extractMallUserFromQuery(query);
    } else {
        qDebug() << "Error fetching mall user by invite code:" << inviteCode;
    }

    return user;
}

// 验证用户登录
bool DatabaseManager::validateMallUserLogin(const QString &username, const QString &password)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE username = :username AND password = :password AND status = 'active'");
    query.bindValue(":username", username);
    query.bindValue(":password", password);

    if (query.exec() && query.next()) {
        int count = query.value(0).toInt();
        if (count > 0) {
            // 更新最后登录时间
            updateMallUserLastLogin(username);
            return true;
        }
    }

    return false;
}

// 检查用户名是否存在
bool DatabaseManager::checkMallUserExists(const QString &username)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE username = :username");
    query.bindValue(":username", username);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 检查邮箱是否存在
bool DatabaseManager::checkEmailExists(const QString &email)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE email = :email");
    query.bindValue(":email", email);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 检查手机号是否存在
bool DatabaseManager::checkPhoneExists(const QString &phone)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE phone = :phone");
    query.bindValue(":phone", phone);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 检查邀请码是否存在
bool DatabaseManager::checkInviteCodeExists(const QString &inviteCode)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE invite_code = :invite_code");
    query.bindValue(":invite_code", inviteCode);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}

// 获取用户数量
int DatabaseManager::getMallUserCount()
{
    QMutexLocker locker(&m_mutex);

     QSqlQuery query("SELECT COUNT(*) FROM MallUsers");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 获取按等级统计的用户数量
int DatabaseManager::getMallUserCountByLevel(int level)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE user_level = :level");
    query.bindValue(":level", level);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 获取按状态统计的用户数量
int DatabaseManager::getMallUserCountByStatus(const QString &status)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE status = :status");
    query.bindValue(":status", status);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 获取总余额
double DatabaseManager::getTotalMallUserBalance()
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query("SELECT SUM(balance) FROM MallUsers WHERE status = 'active'");

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}

// 获取总积分
int DatabaseManager::getTotalMallUserPoints()
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query("SELECT SUM(points) FROM MallUsers WHERE status = 'active'");

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 批量插入用户
bool DatabaseManager::batchInsertMallUsers(const QList<SQL_MallUser> &users)
{
    QMutexLocker locker(&m_mutex);

    if (users.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_MallUser &user : users) {
        if (!insertMallUser(user)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch inserted" << users.size() << "mall users successfully";
    return true;
}

// 批量更新用户
bool DatabaseManager::batchUpdateMallUsers(const QList<SQL_MallUser> &users)
{
    QMutexLocker locker(&m_mutex);

    if (users.isEmpty()) {
        return true;
    }

    db.transaction();  // 开始事务

    for (const SQL_MallUser &user : users) {
        if (!updateMallUser(user)) {
            db.rollback();  // 回滚事务
            return false;
        }
    }

    db.commit();  // 提交事务
    qDebug() << "Batch updated" << users.size() << "mall users successfully";
    return true;
}
// 从查询结果中提取商城用户数据
SQL_MallUser DatabaseManager::extractMallUserFromQuery(const QSqlQuery &query)
{
    SQL_MallUser user;
    user.username = query.value("username").toString();
    user.password = query.value("password").toString();
    user.email = query.value("email").toString();
    user.inviteCode = query.value("invite_code").toString();
    user.inviterUsername = query.value("inviter_username").toString();  // 新增
    user.createTime = query.value("create_time").toString();
    user.lastLoginTime = query.value("last_login_time").toString();
    user.phone = query.value("phone").toString();
    user.userLevel = query.value("user_level").toInt();
    user.balance = query.value("balance").toDouble();
    user.points = query.value("points").toInt();

    return user;
}


// 新增：根据邀请人获取用户列表
QList<SQL_MallUser> DatabaseManager::getMallUsersByInviter(const QString &inviterUsername)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_MallUser> users;
    QSqlQuery query;
    query.prepare("SELECT * FROM MallUsers WHERE inviter_username = :inviter_username ORDER BY create_time DESC");
    query.bindValue(":inviter_username", inviterUsername);

    if (query.exec()) {
        while (query.next()) {
            SQL_MallUser user = extractMallUserFromQuery(query);
            users.append(user);
        }
    } else {
        qDebug() << "Error fetching users by inviter:" << query.lastError().text();
    }

    return users;
}

// 新增：获取用户邀请的下级用户数量
int DatabaseManager::getInvitedUserCount(const QString &inviterUsername)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM MallUsers WHERE inviter_username = :inviter_username AND status = 'active'");
    query.bindValue(":inviter_username", inviterUsername);

    if (query.exec() && query.next()) {
        return query.value(0).toInt();
    }

    return 0;
}

// 新增：获取用户邀请的下级用户总消费（通过关联订单表）
double DatabaseManager::getInvitedUsersTotalConsumption(const QString &inviterUsername)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        SELECT SUM(o.total_price)
        FROM Orders o
        JOIN MallUsers u ON o.user = u.username
        WHERE u.inviter_username = :inviter_username
        AND o.status = 'completed'
    )");
    query.bindValue(":inviter_username", inviterUsername);

    if (query.exec() && query.next()) {
        return query.value(0).toDouble();
    }

    return 0.0;
}
bool DatabaseManager::createWithdrawRecord(const QString &withdrawId, const QString &username,
                                           double amount, const QString &alipayAccount,
                                           const QString &remark)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;

    query.prepare(R"(
        INSERT INTO WithdrawRecords
        (withdraw_id, username, amount, alipay_account, remark)
        VALUES (:withdraw_id, :username, :amount, :alipay_account, :remark)
    )");

    query.bindValue(":withdraw_id", withdrawId);
    query.bindValue(":username", username);
    query.bindValue(":amount", amount);
    query.bindValue(":alipay_account", alipayAccount);
    query.bindValue(":remark", remark);

    if (!query.exec()) {
        qDebug() << "Error creating withdraw record: " << query.lastError().text();
        return false;
    }

    qDebug() << "Withdraw record created successfully. ID:" << withdrawId;
    return true;
}
bool DatabaseManager::createWithdrawTable()
{
    QSqlQuery query;

    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS WithdrawRecords (
            withdraw_id TEXT PRIMARY KEY,
            username TEXT NOT NULL,
            amount REAL NOT NULL,
            alipay_account TEXT NOT NULL,
            status TEXT DEFAULT 'pending',  -- pending, processing, completed, failed
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            update_time TEXT DEFAULT (datetime('now', 'localtime')),
            remark TEXT,
            FOREIGN KEY (username) REFERENCES MallUsers(username) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating WithdrawRecords table: " << query.lastError().text();
        return false;
    }

    // 创建索引
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_withdraw_username ON WithdrawRecords(username)",
        "CREATE INDEX IF NOT EXISTS idx_withdraw_status ON WithdrawRecords(status)",
        "CREATE INDEX IF NOT EXISTS idx_withdraw_time ON WithdrawRecords(create_time)"
    };

    for (const QString &indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qDebug() << "Error creating index: " << query.lastError().text();
        }
    }

    qDebug() << "WithdrawRecords table created successfully.";
    return true;
}
QList<SQL_WithdrawRecord> DatabaseManager::getWithdrawRecordsByUsername(const QString &username)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_WithdrawRecord> records;

    QSqlQuery query;
    query.prepare("SELECT * FROM WithdrawRecords WHERE username = :username ORDER BY create_time DESC");
    query.bindValue(":username", username);

    if (query.exec()) {
        while (query.next()) {
            SQL_WithdrawRecord record;
            record.withdrawId = query.value("withdraw_id").toString();
            record.username = query.value("username").toString();
            record.amount = query.value("amount").toDouble();
            record.alipayAccount = query.value("alipay_account").toString();
            record.status = query.value("status").toString();
            record.createTime = query.value("create_time").toString();
            record.updateTime = query.value("update_time").toString();
            record.remark = query.value("remark").toString();

            records.append(record);
        }
    } else {
        qDebug() << "Error fetching withdraw records: " << query.lastError().text();
    }

    return records;
}
// 在 DatabaseManager 类中添加以下方法：

// 获取订单详情（包含截图信息）
SQL_Order DatabaseManager::getOrderWithSnapshot(const QString &orderId)
{
    QMutexLocker locker(&m_mutex);

    SQL_Order order = getOrderById(orderId);

    if (!order.commandId.isEmpty()) {
        // 如果订单有关联的指令ID，查询指令的completed_url作为snapshot
        SQL_CommandHistory command = getCommandById(order.commandId);
        if (!command.completed_url.isEmpty()) {
            order.snapshot = command.completed_url;
        }
    }

    return order;
}
QList<SQL_WithdrawRecord> DatabaseManager::getAllWithdrawRecords()
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_WithdrawRecord> records;

    QSqlQuery query("SELECT * FROM WithdrawRecords ORDER BY create_time DESC");

    if (!query.exec()) {
        qDebug() << "Error fetching withdraw records: " << query.lastError().text();
        return records;
    }

    while (query.next()) {
        SQL_WithdrawRecord record;

        record.withdrawId = query.value("withdraw_id").toString();
        record.username = query.value("username").toString();
        record.amount = query.value("amount").toDouble();
        record.alipayAccount = query.value("alipay_account").toString();
        record.status = query.value("status").toString();
        record.createTime = query.value("create_time").toString();
        record.updateTime = query.value("update_time").toString();
        record.remark = query.value("remark").toString();

        records.append(record);
    }

    qDebug() << "Retrieved" << records.size() << "withdraw records";
    return records;
}
// 获取用户订单列表（包含截图信息）
QList<SQL_Order> DatabaseManager::getUserOrdersWithSnapshots(const QString &username)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_Order> orders = getOrdersByUser(username);

    // 为每个订单获取截图信息
    for (int i = 0; i < orders.size(); ++i) {
        SQL_Order &order = orders[i];

        if (!order.commandId.isEmpty()) {
            SQL_CommandHistory command = getCommandById(order.commandId);
            if (!command.completed_url.isEmpty()) {
                order.snapshot = command.completed_url;
            }
        }

        // 如果商品名称为空，使用默认名称
        if (order.productName.isEmpty() && !order.productId.isEmpty()) {
            order.productName = QString("商品%1").arg(order.productId);
        }
    }

    return orders;
}


bool DatabaseManager::createAppealTable()
{
    QSqlQuery query;

    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS UserAppeals (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            username TEXT NOT NULL,           -- 投诉用户
            order_id TEXT NOT NULL,           -- 投诉订单
            appeal_type TEXT NOT NULL,        -- text 或 picture
            content_path TEXT,                -- 文本或图片的保存路径
            text_content TEXT,                -- 如果是文本投诉，保存原文
            appeal_time TEXT DEFAULT (datetime('now', 'localtime')),  -- 投诉时间
            status TEXT DEFAULT '待处理',     -- 修改：明确的状态值
            operator TEXT,                    -- 处理人员
            result TEXT,                      -- 处理结果
            result_time TEXT,
            processing_status TEXT,           -- 新增：处理状态分类
            appeal_level INTEGER DEFAULT 1,   -- 新增：投诉级别（1-5级）
            priority TEXT DEFAULT 'normal',   -- 新增：优先级（urgent, high, normal, low）
            FOREIGN KEY (username) REFERENCES MallUsers(username) ON DELETE CASCADE,
            FOREIGN KEY (order_id) REFERENCES Orders(order_id) ON DELETE CASCADE
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "Error creating UserAppeals table: " << query.lastError().text();
        return false;
    }

    // 创建索引
    QStringList indexQueries = {
        "CREATE INDEX IF NOT EXISTS idx_appeal_user ON UserAppeals(username)",
        "CREATE INDEX IF NOT EXISTS idx_appeal_order ON UserAppeals(order_id)",
        "CREATE INDEX IF NOT EXISTS idx_appeal_time ON UserAppeals(appeal_time)",
        "CREATE INDEX IF NOT EXISTS idx_appeal_status ON UserAppeals(status)",  // 原有状态
        "CREATE INDEX IF NOT EXISTS idx_appeal_processing_status ON UserAppeals(processing_status)",  // 新增索引
        "CREATE INDEX IF NOT EXISTS idx_appeal_type ON UserAppeals(appeal_type)",
        "CREATE INDEX IF NOT EXISTS idx_appeal_priority ON UserAppeals(priority)",  // 新增索引
        "CREATE INDEX IF NOT EXISTS idx_appeal_level ON UserAppeals(appeal_level)"   // 新增索引
    };

    for (const QString &indexQuery : indexQueries) {
        if (!query.exec(indexQuery)) {
            qDebug() << "Error creating index: " << query.lastError().text();
        }
    }

    qDebug() << "UserAppeals table created successfully.";
    return true;
}

// 插入投诉记录
// 在DatabaseManager.cpp中
// DatabaseManager.cpp

// 根据用户名获取投诉记录
QList<SQL_AppealRecord> DatabaseManager::getAppealsByUser(const QString &username)
{
    QList<SQL_AppealRecord> appeals;

    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM UserAppeals
        WHERE username = :username
        ORDER BY appeal_time DESC
    )");

    query.bindValue(":username", username);

    if (!query.exec()) {
        qDebug() << "Error fetching appeals by user:" << query.lastError().text();
        return appeals;
    }

    qDebug() << "Query executed successfully for user:" << username;

    int count = 0;
    while (query.next()) {
        SQL_AppealRecord appeal;
        appeal.id = query.value("id").toInt();
        appeal.username = query.value("username").toString();
        appeal.orderId = query.value("order_id").toString();
        appeal.appealType = query.value("appeal_type").toString();
        appeal.contentPath = query.value("content_path").toString();
        appeal.textContent = query.value("text_content").toString();
        appeal.appealTime = query.value("appeal_time").toString();
        appeal.status = query.value("status").toString();
        appeal.operatorName = query.value("operator").toString();
        appeal.result = query.value("result").toString();
        appeal.resultTime = query.value("result_time").toString();
        appeal.processingStatus = query.value("processing_status").toString();
        appeal.appealLevel = query.value("appeal_level").toInt();
        appeal.priority = query.value("priority").toString();

        appeals.append(appeal);
        count++;
    }

    qDebug() << "Retrieved" << count << "appeals for user:" << username;
    return appeals;
}

bool DatabaseManager::insertUserAppeal(const QString &username, const QString &orderId,
                                       const QString &appealType, const QString &contentPath,
                                       const QString &textContent,
                                       const QString &priority,  // 这里不要加 = "normal"
                                       int appealLevel)          // 这里不要加 = 1
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO UserAppeals
        (username, order_id, appeal_type, content_path, text_content,
         status, processing_status, priority, appeal_level)
        VALUES (:username, :order_id, :appeal_type, :content_path, :text_content,
                :status, :processing_status, :priority, :appeal_level)
    )");

    query.bindValue(":username", username);
    query.bindValue(":order_id", orderId);
    query.bindValue(":appeal_type", appealType);
    query.bindValue(":content_path", contentPath);
    query.bindValue(":text_content", textContent);
    query.bindValue(":status", AppealStatus::PENDING);
    query.bindValue(":processing_status", "");  // 初始为空
    query.bindValue(":priority", priority);
    query.bindValue(":appeal_level", appealLevel);

    if (!query.exec()) {
        qDebug() << "Error inserting user appeal: " << query.lastError().text();
        return false;
    }

    qDebug() << "User appeal inserted successfully. User:" << username << "Order:" << orderId;
    return true;
}


// 更新处理状态
bool DatabaseManager::updateAppealProcessingStatus(int appealId, const QString &processingStatus)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE UserAppeals
        SET processing_status = :processing_status,
            update_time = datetime('now', 'localtime')
        WHERE id = :id
    )");

    query.bindValue(":id", appealId);
    query.bindValue(":processing_status", processingStatus);

    if (!query.exec()) {
        qDebug() << "Error updating appeal processing status: " << query.lastError().text();
        return false;
    }

    qDebug() << "Appeal processing status updated. ID:" << appealId << "Status:" << processingStatus;
    return true;
}

// 更新优先级
bool DatabaseManager::updateAppealPriority(int appealId, const QString &priority)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE UserAppeals
        SET priority = :priority,
            update_time = datetime('now', 'localtime')
        WHERE id = :id
    )");

    query.bindValue(":id", appealId);
    query.bindValue(":priority", priority);

    if (!query.exec()) {
        qDebug() << "Error updating appeal priority: " << query.lastError().text();
        return false;
    }

    qDebug() << "Appeal priority updated. ID:" << appealId << "Priority:" << priority;
    return true;
}

// 获取特定处理状态的投诉列表
QList<SQL_AppealRecord> DatabaseManager::getAppealsByProcessingStatus(const QString &processingStatus)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_AppealRecord> appeals;
    QSqlQuery query;
    query.prepare("SELECT * FROM UserAppeals WHERE processing_status = :processing_status ORDER BY appeal_time DESC");
    query.bindValue(":processing_status", processingStatus);

    if (query.exec()) {
        while (query.next()) {
            SQL_AppealRecord appeal = extractAppealFromQuery(query);
            appeals.append(appeal);
        }
    } else {
        qDebug() << "Error fetching appeals by processing status: " << query.lastError().text();
    }

    return appeals;
}

// 根据优先级获取投诉列表
QList<SQL_AppealRecord> DatabaseManager::getAppealsByPriority(const QString &priority)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_AppealRecord> appeals;
    QSqlQuery query;
    query.prepare("SELECT * FROM UserAppeals WHERE priority = :priority ORDER BY appeal_time DESC");
    query.bindValue(":priority", priority);

    if (query.exec()) {
        while (query.next()) {
            SQL_AppealRecord appeal = extractAppealFromQuery(query);
            appeals.append(appeal);
        }
    } else {
        qDebug() << "Error fetching appeals by priority: " << query.lastError().text();
    }

    return appeals;
}

// DatabaseManager.cpp 中实现
bool DatabaseManager::updateDeviceAppStatus(const QString &serialNumber, const QString &appName, const QString &status)
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "=== 更新设备应用状态 ===";
    qDebug() << "设备序列号:" << serialNumber;
    qDebug() << "应用名称:" << appName;
    qDebug() << "状态:" << status;

    // 映射应用名称到数据库字段
    QString fieldName;
    QString fieldValue;

    if (appName == "抖音" || appName.toLower() == "tiktok" || appName == "TikTok") {
        fieldName = "tiktok";
    } else if (appName == "B站" || appName.toLower() == "bilibili" || appName == "BILIBILI") {
        fieldName = "bilibili";
    } else if (appName == "小红书" || appName.toLower() == "xhs") {
        fieldName = "xhs";
    } else if (appName == "微博" || appName.toLower() == "weibo") {
        fieldName = "weibo";
    } else if (appName == "快手" || appName.toLower() == "kuaishou") {
        fieldName = "kuaishou";
    } else {
        qDebug() << "未知的应用名称:" << appName;
        return false;
    }

    // 根据状态设置值（文本类型）
    if (status == "finish" || status == "login_success" || status == "running" ||
        status == "success" || status == "completed") {
        fieldValue = "已登录";
    } else if (status == "unlogin" || status == "fail" || status == "stopped" ||
               status == "failed" || status == "error") {
        fieldValue = "未登录";
    } else {
        fieldValue = status;  // 直接使用传入的状态文本
    }

    qDebug() << "对应字段:" << fieldName << "值:" << fieldValue;

    QSqlQuery query;
    query.prepare(QString("UPDATE Devices SET %1 = :value WHERE serial_number = :serial_number").arg(fieldName));
    query.bindValue(":value", fieldValue);
    query.bindValue(":serial_number", serialNumber);

    if (!query.exec()) {
        qDebug() << "更新设备应用状态失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "设备应用状态更新成功";
    return true;
}
bool DatabaseManager::incrementCommandCompletedTasks(const QString &commandId)
{
    QMutexLocker locker(&m_mutex);

    qDebug() << "=== 增加指令完成数 ===";
    qDebug() << "指令ID:" << commandId;

    // 先获取当前的任务信息
    SQL_CommandHistory command = getCommandById(commandId);
    if (command.commandId.isEmpty()) {
        qDebug() << "指令不存在:" << commandId;
        return false;
    }

    // 检查是否已经完成所有任务（成功+失败）
    int totalFinished = command.completed_tasks + command.failed_tasks;
    if (totalFinished >= command.total_tasks) {
        qDebug() << "指令已完成所有任务，无需增加";
        qDebug() << "总任务:" << command.total_tasks;
        qDebug() << "已完成（成功+失败）:" << totalFinished;
        return false;
    }

    // 增加完成数
    int newCompletedTasks = command.completed_tasks + 1;

    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET completed_tasks = :completed_tasks,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");
    query.bindValue(":completed_tasks", newCompletedTasks);
    query.bindValue(":command_id", commandId);

    if (!query.exec()) {
        qDebug() << "增加指令完成数失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "指令完成数更新成功";
    qDebug() << "原完成数:" << command.completed_tasks << "-> 新完成数:" << newCompletedTasks;
    qDebug() << "总任务数:" << command.total_tasks;
    qDebug() << "失败任务数:" << command.failed_tasks;

    // 计算完成率
    int completionRate = 0;
    if (command.total_tasks > 0) {
        completionRate = (newCompletedTasks * 100) / command.total_tasks;
    }
    qDebug() << "完成率:" << completionRate << "%";

    // 更新完成度字段和指令状态（这会自动更新订单状态）
    updateCommandStatus(commandId);

    return true;
}
// 新增：根据指令状态更新关联的订单状态
bool DatabaseManager::updateCommandStatus(const QString &commandId)
{

    SQL_CommandHistory command = getCommandById(commandId);
    if (command.commandId.isEmpty()) {
        qDebug() << "指令不存在，无法更新状态:" << commandId;
        return false;
    }

    QString newStatus = command.status;
    int totalFinished = command.completed_tasks + command.failed_tasks;

    qDebug() << "=== 更新指令状态 ===";
    qDebug() << "指令ID:" << commandId;
    qDebug() << "总任务数:" << command.total_tasks;
    qDebug() << "成功任务数:" << command.completed_tasks;
    qDebug() << "失败任务数:" << command.failed_tasks;
    qDebug() << "已完成总数:" << totalFinished;

    // 判断任务完成情况
    if (totalFinished >= command.total_tasks) {
        if (command.completed_tasks == command.total_tasks && command.failed_tasks == 0) {
            newStatus = "success";  // 全部成功
            qDebug() << "指令状态: 全部成功";
        } else if (command.failed_tasks == command.total_tasks && command.completed_tasks == 0) {
            newStatus = "failed";   // 全部失败
            qDebug() << "指令状态: 全部失败";
        } else {
            newStatus = "partial";  // 部分完成
            qDebug() << "指令状态: 部分完成";
        }
    } else {
        newStatus = "execing";      // 执行中
        qDebug() << "指令状态: 执行中";
    }

    // 计算完成度（基于成功任务数）
    int completionRate = 0;
    if (command.total_tasks > 0) {
        completionRate = (command.completed_tasks * 100) / command.total_tasks;
    }

    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET status = :status,
            completeness = :completeness,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");
    query.bindValue(":status", newStatus);
    query.bindValue(":completeness", QString("%1%").arg(completionRate));
    query.bindValue(":command_id", commandId);

    if (!query.exec()) {
        qDebug() << "更新指令状态失败:" << query.lastError().text();
        return false;
    }

    qDebug() << "指令状态更新成功: " << newStatus;
    qDebug() << "完成度: " << completionRate << "%";

    // 同步更新关联的订单状态
    updateOrderStatusByCommandId(commandId, newStatus);

    return true;
}
// 新增：根据指令状态更新关联的订单状态
bool DatabaseManager::updateOrderStatusByCommandId(const QString &commandId, const QString &commandStatus)
{

    qDebug() << "=== 同步更新订单状态 ===";
    qDebug() << "指令ID:" << commandId;
    qDebug() << "指令状态:" << commandStatus;

    // 获取关联的订单列表
    QList<SQL_Order> orders = getOrdersByCommandId(commandId);
    qDebug() << "找到" << orders.size() << "个关联订单";

    if (orders.isEmpty()) {
        qDebug() << "没有找到关联的订单";
        return false;
    }

    // 映射指令状态到订单状态
    QString orderStatus = mapCommandStatusToOrderStatus(commandStatus);
    qDebug() << "映射后的订单状态:" << orderStatus;

    bool allSuccess = true;

    // 更新所有关联订单的状态
    for (const SQL_Order &order : orders) {
        bool success = updateOrderStatus(order.orderId, orderStatus);
        if (!success) {
            qDebug() << "更新订单状态失败，订单ID:" << order.orderId;
            allSuccess = false;
        } else {
            qDebug() << "订单状态更新成功，订单ID:" << order.orderId << "新状态:" << orderStatus;
        }
    }

    return allSuccess;
}
// 新增：映射指令状态到订单状态
QString DatabaseManager::mapCommandStatusToOrderStatus(const QString &commandStatus)
{

    // 指令状态到订单状态的映射
    static QMap<QString, QString> statusMapping = {
        {"success", "completed"},     // 指令成功 -> 订单已完成
        {"failed", "failed"},         // 指令失败 -> 订单失败
        {"partial", "partial"},       // 指令部分完成 -> 订单部分完成
        {"execing", "processing"},    // 指令执行中 -> 订单处理中
        {"pending", "pending"}        // 指令待处理 -> 订单待处理
    };

    QString orderStatus = statusMapping.value(commandStatus.toLower(), "unknown");

    if (orderStatus == "unknown") {
        qDebug() << "未知的指令状态，使用默认状态: processing";
        return "processing";
    }

    return orderStatus;
}

// 如果需要，可以添加一个更新完成度的方法
bool DatabaseManager::updateCommandCompleteness(const QString &commandId, int completeness)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE CommandHistory
        SET completeness = :completeness,
            updated_at = datetime('now', 'localtime')
        WHERE command_id = :command_id
    )");
    query.bindValue(":completeness", QString("%1%").arg(completeness));
    query.bindValue(":command_id", commandId);

    if (!query.exec()) {
        qDebug() << "更新指令完成度失败:" << query.lastError().text();
        return false;
    }

    return true;
}
// 更新提现记录状态
bool DatabaseManager::updateWithdrawStatus(const QString &withdrawId, const QString &status)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE WithdrawRecords
        SET status = :status,
            update_time = datetime('now', 'localtime')
        WHERE withdraw_id = :withdraw_id
    )");

    query.bindValue(":withdraw_id", withdrawId);
    query.bindValue(":status", status);

    if (!query.exec()) {
        qDebug() << "Error updating withdraw status: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No withdraw record found with ID:" << withdrawId;
        return false;
    }

    qDebug() << "Withdraw status updated successfully. ID:" << withdrawId << "Status:" << status;
    return true;
}

// 更新提现记录（完整更新）
bool DatabaseManager::updateWithdrawRecord(const SQL_WithdrawRecord &record)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE WithdrawRecords
        SET amount = :amount,
            alipay_account = :alipay_account,
            status = :status,
            remark = :remark,
            update_time = datetime('now', 'localtime')
        WHERE withdraw_id = :withdraw_id
    )");

    query.bindValue(":withdraw_id", record.withdrawId);
    query.bindValue(":amount", record.amount);
    query.bindValue(":alipay_account", record.alipayAccount);
    query.bindValue(":status", record.status);
    query.bindValue(":remark", record.remark);

    if (!query.exec()) {
        qDebug() << "Error updating withdraw record: " << query.lastError().text();
        return false;
    }

    if (query.numRowsAffected() == 0) {
        qDebug() << "No withdraw record found with ID:" << record.withdrawId;
        return false;
    }

    qDebug() << "Withdraw record updated successfully. ID:" << record.withdrawId;
    return true;
}

// 根据ID获取提现记录
SQL_WithdrawRecord DatabaseManager::getWithdrawRecordById(const QString &withdrawId)
{
    QMutexLocker locker(&m_mutex);

    SQL_WithdrawRecord record;

    QSqlQuery query;
    query.prepare("SELECT * FROM WithdrawRecords WHERE withdraw_id = :withdraw_id");
    query.bindValue(":withdraw_id", withdrawId);

    if (query.exec() && query.next()) {
        record.withdrawId = query.value("withdraw_id").toString();
        record.username = query.value("username").toString();
        record.amount = query.value("amount").toDouble();
        record.alipayAccount = query.value("alipay_account").toString();
        record.status = query.value("status").toString();
        record.createTime = query.value("create_time").toString();
        record.updateTime = query.value("update_time").toString();
        record.remark = query.value("remark").toString();
    } else {
        qDebug() << "Error fetching withdraw record by ID:" << withdrawId
                 << "Error:" << query.lastError().text();
    }

    return record;
}
bool DatabaseManager::createAppAccountTable()
{
    QSqlQuery query;
    QString createTableQuery = R"(
        CREATE TABLE IF NOT EXISTS AppAccounts (
            id INTEGER PRIMARY KEY AUTOINCREMENT,
            account_name TEXT UNIQUE NOT NULL,
            platform TEXT NOT NULL,
            username TEXT NOT NULL,
            create_time TEXT DEFAULT (datetime('now', 'localtime')),
            status TEXT DEFAULT 'active',
            remark TEXT,
            devserial TEXT,
            update_time TEXT DEFAULT (datetime('now', 'localtime'))
        )
    )";

    if (!query.exec(createTableQuery)) {
        qDebug() << "创建AppAccounts表失败: " << query.lastError().text();
        return false;
    }
    qDebug() << "AppAccounts表创建成功";
    return true;
}

// 2. 添加App账号
bool DatabaseManager::insertAppAccount(const SQL_AppAccount &account)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        INSERT INTO AppAccounts (account_name, platform, username, status, remark, devserial)
        VALUES (:account_name, :platform, :username, :status, :remark, :devserial)
    )");

    query.bindValue(":account_name", account.accountName);
    query.bindValue(":platform", account.platform);
    query.bindValue(":username", account.username);
    query.bindValue(":status", account.status.isEmpty() ? "active" : account.status);
    query.bindValue(":remark", account.remark);
    query.bindValue(":devserial", account.devserial);

    if (!query.exec()) {
        qDebug() << "添加App账号失败: " << query.lastError().text();
        return false;
    }

    qDebug() << "添加App账号成功: " << account.accountName;
    return true;
}

// 3. 删除App账号
bool DatabaseManager::deleteAppAccount(int accountId)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("DELETE FROM AppAccounts WHERE id = :id");
    query.bindValue(":id", accountId);

    if (!query.exec()) {
        qDebug() << "删除App账号失败: " << query.lastError().text();
        return false;
    }

    qDebug() << "删除App账号成功，ID: " << accountId;
    return true;
}

// 4. 修改App账号
bool DatabaseManager::updateAppAccount(const SQL_AppAccount &account)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare(R"(
        UPDATE AppAccounts
        SET platform = :platform,
            username = :username,
            status = :status,
            remark = :remark,
            devserial = :devserial,
            update_time = datetime('now', 'localtime')
        WHERE id = :id
    )");

    query.bindValue(":id", account.id);
    query.bindValue(":platform", account.platform);
    query.bindValue(":username", account.username);
    query.bindValue(":status", account.status);
    query.bindValue(":remark", account.remark);
    query.bindValue(":devserial", account.devserial);

    if (!query.exec()) {
        qDebug() << "修改App账号失败: " << query.lastError().text();
        return false;
    }

    qDebug() << "修改App账号成功，ID: " << account.id;
    return true;
}

// 5. 查询所有App账号
QList<SQL_AppAccount> DatabaseManager::getAllAppAccounts()
{
    QList<SQL_AppAccount> accounts;
    QSqlQuery query("SELECT * FROM AppAccounts ORDER BY create_time DESC");

    while (query.next()) {
        SQL_AppAccount account = extractAppAccountFromQuery(query);
        accounts.append(account);
    }

    return accounts;
}

// 6. 查询单个App账号
SQL_AppAccount DatabaseManager::getAppAccountById(int accountId)
{
    QMutexLocker locker(&m_mutex);

    SQL_AppAccount account;
    QSqlQuery query;
    query.prepare("SELECT * FROM AppAccounts WHERE id = :id");
    query.bindValue(":id", accountId);

    if (query.exec() && query.next()) {
        account = extractAppAccountFromQuery(query);
    }

    return account;
}

// 7. 搜索App账号（只在platform、username、devserial这三个字段搜索）
QList<SQL_AppAccount> DatabaseManager::searchAppAccounts(const QString &keyword)
{
    QMutexLocker locker(&m_mutex);

    QList<SQL_AppAccount> accounts;
    QSqlQuery query;
    query.prepare(R"(
        SELECT * FROM AppAccounts
        WHERE platform LIKE :keyword
           OR username LIKE :keyword
           OR devserial LIKE :keyword
        ORDER BY create_time DESC
    )");
    query.bindValue(":keyword", "%" + keyword + "%");

    if (query.exec()) {
        while (query.next()) {
            SQL_AppAccount account = extractAppAccountFromQuery(query);
            accounts.append(account);
        }
    }

    return accounts;
}

// 8. 从查询结果中提取数据
SQL_AppAccount DatabaseManager::extractAppAccountFromQuery(const QSqlQuery &query)
{
    QMutexLocker locker(&m_mutex);
    SQL_AppAccount account;
    account.id = query.value("id").toInt();
    account.accountName = query.value("account_name").toString();
    account.platform = query.value("platform").toString();
    account.username = query.value("username").toString();
    account.createTime = query.value("create_time").toString();
    account.status = query.value("status").toString();
    account.remark = query.value("remark").toString();
    account.devserial = query.value("devserial").toString();
    return account;
}

// 9. 检查账号名是否存在
bool DatabaseManager::checkAppAccountExists(const QString &accountName)
{
    QMutexLocker locker(&m_mutex);

    QSqlQuery query;
    query.prepare("SELECT COUNT(*) FROM AppAccounts WHERE account_name = :account_name");
    query.bindValue(":account_name", accountName);

    if (query.exec() && query.next()) {
        return query.value(0).toInt() > 0;
    }

    return false;
}
