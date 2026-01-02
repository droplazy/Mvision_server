#include "mallproducts.h"
#include "ui_mallproducts.h"
#include "DatabaseManager.h"
#include <QHeaderView>
#include <QMessageBox>
#include <QPushButton>
#include <QHBoxLayout>
#include <QWidget>
#include <QDesktopServices>
#include <QUrl>
#include <QFile>
#include <QFileInfo>
#include "addproduct.h"


mallproducts::mallproducts(DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::mallproducts)
    , dbManager(dbManager)
    , model(new QStandardItemModel(this))  // 初始化model
{
    ui->setupUi(this);

    // 1. 禁止拉伸
    setFixedSize(this->size());
    setWindowTitle("商城商品管理");

    if (!dbManager) {
        QMessageBox::critical(this, "错误", "数据库管理器为空！");
        return;
    }

    // 设置表格
    setupTableView();

    // 加载商品数据
    loadProducts();
}

mallproducts::~mallproducts()
{
    delete ui;
}

void mallproducts::setupTableView()
{
    // 设置表头 - 添加分类ID列
    QStringList headers;
    headers << "商品ID" << "商品名称" << "分类ID" << "分类名称" << "价格" << "库存"
            << "状态" << "action" << "subaction" << "操作" << "删除";

    model->setHorizontalHeaderLabels(headers);

    // 设置表格属性
    ui->tableView->setModel(model);
    ui->tableView->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tableView->setAlternatingRowColors(true);
    ui->tableView->setSortingEnabled(true);
    ui->tableView->setEditTriggers(QAbstractItemView::NoEditTriggers);  // 禁止编辑

    // 设置列宽
    ui->tableView->horizontalHeader()->setStretchLastSection(true);
    ui->tableView->setColumnWidth(0, 120);  // 商品ID
    ui->tableView->setColumnWidth(1, 180);  // 商品名称
    ui->tableView->setColumnWidth(2, 80);   // 分类ID
    ui->tableView->setColumnWidth(3, 100);  // 分类名称
    ui->tableView->setColumnWidth(4, 70);   // 价格
    ui->tableView->setColumnWidth(5, 60);   // 库存
    ui->tableView->setColumnWidth(6, 70);   // 状态
    ui->tableView->setColumnWidth(7, 80);   // action
    ui->tableView->setColumnWidth(8, 100);  // subaction
    ui->tableView->setColumnWidth(9, 80);   // 操作（详情修改）
    ui->tableView->setColumnWidth(10, 80);  // 删除
}

void mallproducts::loadProducts()
{
    if (!dbManager) return;

    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 从数据库获取所有商品
    QList<SQL_Product> products = dbManager->getAllProducts();

    // 填充数据
    for (int i = 0; i < products.size(); i++) {
        const SQL_Product &product = products[i];
        QList<QStandardItem*> rowItems;

        // 显示指定的列
        rowItems.append(new QStandardItem(product.productId));
        rowItems.append(new QStandardItem(product.productName));
        rowItems.append(new QStandardItem(product.categoryId.isEmpty() ? "无" : product.categoryId));
        rowItems.append(new QStandardItem(product.categoryName.isEmpty() ? "未命名" : product.categoryName));
        rowItems.append(new QStandardItem(QString::number(product.unitPrice, 'f', 2)));
        rowItems.append(new QStandardItem(QString::number(product.stock)));
        rowItems.append(new QStandardItem(product.status.isEmpty() ? "active" : product.status));
        rowItems.append(new QStandardItem(product.action.isEmpty() ? "" : product.action));
        rowItems.append(new QStandardItem(product.subaction.isEmpty() ? "" : product.subaction));

        // 操作列数据 - 存储商品信息但不显示
        rowItems.append(new QStandardItem(""));

        // 删除列数据
        rowItems.append(new QStandardItem(""));

        model->appendRow(rowItems);

        // 为操作列添加详情修改按钮（第9列）
        addEditButton(i, product);
        // 为删除列添加删除按钮（现在是第10列）
        addDeleteButton(i);
    }
}

void mallproducts::addEditButton(int row, const SQL_Product &product)
{
    QPushButton *editBtn = new QPushButton("详情修改");
    editBtn->setStyleSheet("QPushButton {"
                           "background-color: #4a90e2;"
                           "color: white;"
                           "border: none;"
                           "padding: 4px 8px;"
                           "border-radius: 3px;"
                           "}"
                           "QPushButton:hover {"
                           "background-color: #3a80d2;"
                           "}"
                           "QPushButton:pressed {"
                           "background-color: #2a70c2;"
                           "}");

    // 设置按钮大小策略
    editBtn->setFixedSize(70, 24);

    // 创建一个widget来包含按钮
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(editBtn);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);

    // 将widget设置到表格（第9列是操作列）
    ui->tableView->setIndexWidget(model->index(row, 9), widget);

    // 连接按钮点击信号 - 传递商品信息
    connect(editBtn, &QPushButton::clicked, [this, product, row]() {
        editProduct(row, product);
    });
}

void mallproducts::editProduct(int row, const SQL_Product &product)
{
    // 创建编辑商品对话框
    addproduct *editDialog = new addproduct(dbManager, this);

    // 设置为编辑模式并填充现有数据
    editDialog->setEditMode(product);

    // 设置对话框标题
    editDialog->setWindowTitle("编辑商品 - " + product.productName);

    // 显示模态对话框
    int result = editDialog->exec();

    if (result == QDialog::Accepted) {
        // 获取更新后的商品信息
        SQL_Product updatedProduct = editDialog->getProduct();

        // 更新数据库
        if (dbManager->updateProduct(updatedProduct)) {
            // 更新表格中的数据显示
            updateProductInTable(row, updatedProduct);

            // 更新按钮的商品信息
            addEditButton(row, updatedProduct);

            QMessageBox::information(this, "成功", "商品信息更新成功！");
        } else {
            QMessageBox::critical(this, "错误", "商品信息更新失败！");
        }
    }

    // 清理资源
    editDialog->deleteLater();
}

void mallproducts::updateProductInTable(int row, const SQL_Product &product)
{
    // 更新表格中的数据
    model->item(row, 0)->setText(product.productId);
    model->item(row, 1)->setText(product.productName);
    model->item(row, 2)->setText(product.categoryId.isEmpty() ? "无" : product.categoryId);
    model->item(row, 3)->setText(product.categoryName.isEmpty() ? "未命名" : product.categoryName);
    model->item(row, 4)->setText(QString::number(product.unitPrice, 'f', 2));
    model->item(row, 5)->setText(QString::number(product.stock));
    model->item(row, 6)->setText(product.status.isEmpty() ? "active" : product.status);
    model->item(row, 7)->setText(product.action.isEmpty() ? "" : product.action);
    model->item(row, 8)->setText(product.subaction.isEmpty() ? "" : product.subaction);
}

void mallproducts::addDeleteButton(int row)
{
    QPushButton *deleteBtn = new QPushButton("删除");
    deleteBtn->setStyleSheet("QPushButton {"
                             "background-color: #ff6b6b;"
                             "color: white;"
                             "border: none;"
                             "padding: 4px 8px;"
                             "border-radius: 3px;"
                             "}"
                             "QPushButton:hover {"
                             "background-color: #ff5252;"
                             "}"
                             "QPushButton:pressed {"
                             "background-color: #ff3d3d;"
                             "}");

    // 设置按钮大小策略
    deleteBtn->setFixedSize(60, 24);

    // 创建一个widget来包含按钮
    QWidget *widget = new QWidget();
    QHBoxLayout *layout = new QHBoxLayout(widget);
    layout->addWidget(deleteBtn);
    layout->setAlignment(Qt::AlignCenter);
    layout->setContentsMargins(0, 0, 0, 0);
    widget->setLayout(layout);

    // 将widget设置到表格（第10列是删除列）
    ui->tableView->setIndexWidget(model->index(row, 10), widget);

    // 连接按钮点击信号
    connect(deleteBtn, &QPushButton::clicked, [this, row]() {
        deleteProduct(row);
    });
}

void mallproducts::deleteProduct(int row)
{
    // 获取商品ID
    QModelIndex index = model->index(row, 0);
    QString productId = model->data(index).toString();

    if (productId.isEmpty()) {
        QMessageBox::warning(this, "警告", "商品ID为空！");
        return;
    }

    // 确认删除
    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "确认删除",
                                  QString("确定要删除商品 '%1' 吗？\n此操作不可撤销！").arg(productId),
                                  QMessageBox::Yes | QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // 从数据库删除
        if (dbManager->deleteProduct(productId)) {
            // 从表格中删除
            model->removeRow(row);

            // 重新为后面的行添加按钮
            for (int i = row; i < model->rowCount(); i++) {
                // 获取商品信息并重新添加编辑按钮
                SQL_Product product = getProductFromTable(i);
                addEditButton(i, product);
                addDeleteButton(i);
            }

            QMessageBox::information(this, "成功", "商品删除成功！");
        } else {
            QMessageBox::critical(this, "错误", "删除商品失败！");
        }
    }
}

SQL_Product mallproducts::getProductFromTable(int row)
{
    SQL_Product product;

    // 从表格中获取商品信息
    product.productId = model->item(row, 0)->text();
    product.productName = model->item(row, 1)->text();
    product.categoryId = model->item(row, 2)->text();
    product.categoryName = model->item(row, 3)->text();
    product.unitPrice = model->item(row, 4)->text().toDouble();
    product.stock = model->item(row, 5)->text().toInt();
    product.status = model->item(row, 6)->text();
    product.action = model->item(row, 7)->text();
    product.subaction = model->item(row, 8)->text();

    return product;
}

void mallproducts::on_pushButton_search_clicked()
{
    QString keyword = ui->lineEdit->text().trimmed();

    if (keyword.isEmpty()) {
        loadProducts();
        return;
    }

    if (!dbManager) return;

    // 清空现有数据
    model->removeRows(0, model->rowCount());

    // 搜索商品
    QList<SQL_Product> products = dbManager->searchProducts(keyword);

    // 填充数据
    for (int i = 0; i < products.size(); i++) {
        const SQL_Product &product = products[i];
        QList<QStandardItem*> rowItems;

        // 显示指定的列
        rowItems.append(new QStandardItem(product.productId));
        rowItems.append(new QStandardItem(product.productName));
        rowItems.append(new QStandardItem(product.categoryId.isEmpty() ? "无" : product.categoryId));
        rowItems.append(new QStandardItem(product.categoryName.isEmpty() ? "未命名" : product.categoryName));
        rowItems.append(new QStandardItem(QString::number(product.unitPrice, 'f', 2)));
        rowItems.append(new QStandardItem(QString::number(product.stock)));
        rowItems.append(new QStandardItem(product.status.isEmpty() ? "active" : product.status));
        rowItems.append(new QStandardItem(product.action.isEmpty() ? "" : product.action));
        rowItems.append(new QStandardItem(product.subaction.isEmpty() ? "" : product.subaction));

        // 操作列数据
        rowItems.append(new QStandardItem(""));

        // 删除列数据
        rowItems.append(new QStandardItem(""));

        model->appendRow(rowItems);

        // 为操作列添加详情修改按钮
        addEditButton(i, product);
        // 为删除列添加删除按钮
        addDeleteButton(i);
    }
}

void mallproducts::on_pushButton_add_clicked()
{
    // 创建添加商品对话框，传入数据库指针
    addproduct *addDialog = new addproduct(dbManager, this);

    // 设置模态对话框
    addDialog->setModal(true);

    // 显示对话框并等待结果
    int result = addDialog->exec();

    // 检查对话框返回值
    if (result == QDialog::Accepted) {
        // 如果用户点击确定并添加成功，重新加载商品列表
        loadProducts();

        // 可选：显示成功消息
        QMessageBox::information(this, "成功", "商品添加成功！商品列表已刷新。");
    } else if (result == QDialog::Rejected) {
        // 用户取消了操作，不需要做任何事
        qDebug() << "用户取消了商品添加操作";
    }

    // 对话框关闭后自动删除（防止内存泄漏）
    addDialog->deleteLater();
}
