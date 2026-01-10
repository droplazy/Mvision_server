#include "orderlist.h"
#include "ui_orderlist.h"
#include <QDebug>
#include <QMessageBox>
#include <QHeaderView>
#include <QSqlRecord>
#include <QSqlError>
#include <QDateTime>

orderlist::orderlist(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::orderlist)
    , m_dbManager(nullptr)
    , m_model(nullptr)
    , m_proxyModel(nullptr)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("订单查询");

    // 初始化表格
    initTableView();

    // 设置搜索框提示文本
    ui->lineEdit_search->setPlaceholderText("输入订单号、用户名或产品ID搜索...");

    // 连接回车键搜索
    connect(ui->lineEdit_search, &QLineEdit::returnPressed, this, &orderlist::on_pushButton_search_clicked);

    // 连接表格双击事件
    connect(ui->tableView, &QTableView::doubleClicked, this, &orderlist::on_tableView_doubleClicked);
}

orderlist::~orderlist()
{
    if (m_model) {
        delete m_model;
    }
    if (m_proxyModel) {
        delete m_proxyModel;
    }
    delete ui;
}

void orderlist::setDatabaseManager(DatabaseManager* dbManager)
{
    m_dbManager = dbManager;

    // 如果数据库管理器有效，初始加载所有订单
    if (m_dbManager) {
        loadOrders();
    }
}

void orderlist::initTableView()
{
    // 创建数据模型
    m_model = new QSqlQueryModel(this);
    m_proxyModel = new QSortFilterProxyModel(this);
    m_proxyModel->setSourceModel(m_model);

    // 设置表格模型
    ui->tableView->setModel(m_proxyModel);

    // 启用排序
    ui->tableView->setSortingEnabled(true);

    // 设置选择行为
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setSelectionMode(QAbstractItemView::SingleSelection);

    // 设置交替行颜色
    ui->tableView->setAlternatingRowColors(true);

    // 设置列宽自适应
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->verticalHeader()->setDefaultSectionSize(30);

    // 设置表格属性
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);
}

void orderlist::loadOrders(const QString &keyword)
{
    if (!m_dbManager) {
        QMessageBox::warning(this, "错误", "数据库连接未初始化");
        return;
    }

    QSqlDatabase db = QSqlDatabase::database();
    if (!db.isValid() || !db.isOpen()) {
        QMessageBox::warning(this, "错误", "数据库连接无效或未打开");
        return;
    }

    // 构建SQL查询
    QString sql;
    QSqlQuery query(db);

    if (keyword.isEmpty()) {
        // 查询所有订单
        sql = "SELECT "
              "order_id, "
              "product_id, "
              "user, "
              "unit_price, "
              "quantity, "
              "total_price, "
              "status, "
              "create_time, "
              "update_time "
              "FROM Orders "
              "ORDER BY create_time DESC";

        if (!query.exec(sql)) {
            QMessageBox::warning(this, "查询错误",
                                 QString("执行查询失败：\n%1").arg(query.lastError().text()));
            return;
        }
    } else {
        // 搜索订单
        sql = "SELECT "
              "order_id, "
              "product_id, "
              "user, "
              "unit_price, "
              "quantity, "
              "total_price, "
              "status, "
              "create_time, "
              "update_time "
              "FROM Orders "
              "WHERE order_id LIKE :keyword "
              "   OR user LIKE :keyword "
              "   OR product_id LIKE :keyword "
              "ORDER BY create_time DESC";

        query.prepare(sql);
        query.bindValue(":keyword", "%" + keyword + "%");

        if (!query.exec()) {
            QMessageBox::warning(this, "搜索错误",
                                 QString("执行搜索失败：\n%1").arg(query.lastError().text()));
            return;
        }
    }

    // 设置查询结果到模型
    m_model->setQuery(std::move(query));

    if (m_model->lastError().isValid()) {
        QMessageBox::warning(this, "模型错误",
                             QString("设置模型数据失败：\n%1").arg(m_model->lastError().text()));
        return;
    }

    // 设置列标题
    QStringList headers;
    headers << "订单号" << "产品ID" << "用户" << "单价" << "数量"
            << "总价" << "状态" << "创建时间" << "更新时间";

    for (int i = 0; i < headers.size() && i < m_model->columnCount(); ++i) {
        m_model->setHeaderData(i, Qt::Horizontal, headers[i]);
    }

    // 调整列宽
    ui->tableView->resizeColumnsToContents();

    // 显示统计信息
    int orderCount = m_model->rowCount();
    QString statusText = keyword.isEmpty() ?
                             QString("共 %1 条订单").arg(orderCount) :
                             QString("搜索到 %1 条订单").arg(orderCount);

    // 这里您可以在界面上添加一个QLabel来显示状态，如果没有的话可以打印到控制台
    qDebug() << statusText;
}

void orderlist::on_pushButton_search_clicked()
{
    QString keyword = ui->lineEdit_search->text().trimmed();
    loadOrders(keyword);
}

void orderlist::on_tableView_doubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) {
        return;
    }

    // 获取源模型的索引
    QModelIndex sourceIndex = m_proxyModel->mapToSource(index);

    // 获取订单号（假设订单号在第一列）
    QString orderId = m_model->data(m_model->index(sourceIndex.row(), 0)).toString();

    if (orderId.isEmpty()) {
        return;
    }

    // 显示订单详情
    showOrderDetail(orderId);
}

void orderlist::showOrderDetail(const QString &orderId)
{
    if (!m_dbManager) {
        return;
    }

    // 获取订单详细信息
    SQL_Order order = m_dbManager->getOrderById(orderId);

    if (order.orderId.isEmpty()) {
        QMessageBox::information(this, "提示", "未找到订单信息");
        return;
    }

    // 如果商品名称为空，尝试获取商品信息
    if (order.productName.isEmpty() && !order.productId.isEmpty()) {
        SQL_Product product = m_dbManager->getProductById(order.productId);
        if (!product.productId.isEmpty()) {
            order.productName = product.productName;
        }
    }

    // 如果有指令ID，获取截图信息
    if (order.snapshot.isEmpty() && !order.commandId.isEmpty()) {
        SQL_CommandHistory command = m_dbManager->getCommandById(order.commandId);
        if (!command.completed_url.isEmpty()) {
            order.snapshot = command.completed_url;
        }
    }

    // 构建详情信息
    QString detail = QString(
                         "订单详细信息\n\n"
                         "订单号: %1\n"
                         "指令ID: %2\n"
                         "产品ID: %3\n"
                         "产品名称: %4\n"
                         "单价: ¥%5\n"
                         "数量: %6\n"
                         "总价: ¥%7\n"
                         "用户: %8\n"
                         "联系方式: %9\n"
                         "备注: %10\n"
                         "状态: %11\n"
                         "创建时间: %12\n"
                         "更新时间: %13\n"
                         "截图链接: %14")
                         .arg(order.orderId)
                         .arg(order.commandId.isEmpty() ? "无" : order.commandId)
                         .arg(order.productId)
                         .arg(order.productName.isEmpty() ? "未知" : order.productName)
                         .arg(QString::number(order.unitPrice, 'f', 2))
                         .arg(order.quantity)
                         .arg(QString::number(order.totalPrice, 'f', 2))
                         .arg(order.user)
                         .arg(order.contactInfo.isEmpty() ? "未提供" : order.contactInfo)
                         .arg(order.note.isEmpty() ? "无" : order.note)
                         .arg(order.status)
                         .arg(order.createTime)
                         .arg(order.updateTime)
                         .arg(order.snapshot.isEmpty() ? "无" : order.snapshot);

    QMessageBox::information(this, "订单详情", detail);
}
