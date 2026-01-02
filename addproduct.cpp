#include "addproduct.h"
#include "ui_addproduct.h"
#include "DatabaseManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>

// 在构造函数中添加
addproduct::addproduct(DatabaseManager *dbManager, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::addproduct)
    , dbManager(dbManager)
    , isEditMode(false)
{
    ui->setupUi(this);

    // 设置窗口属性
    setWindowTitle("添加商品");
    setFixedSize(this->size());

    if (!dbManager) {
        QMessageBox::critical(this, "错误", "数据库管理器为空！");
    }

    // 设置连接
    setupConnections();
}
// 设置编辑模式
void addproduct::setEditMode(const SQL_Product &product)
{
    isEditMode = true;
    originalProductId = product.productId;
    setWindowTitle("编辑商品 - " + product.productName);

    // 填充商品数据
    fillProductData(product);
}
addproduct::~addproduct()
{
    delete ui;
}
// 填充商品数据到界面
void addproduct::fillProductData(const SQL_Product &product)
{
    ui->lineEdit_productId->setText(product.productId);
    ui->lineEdit_productname->setText(product.productName);
    ui->lineEdit_categoryid->setText(product.categoryId);
    ui->lineEdit_categoryname->setText(product.categoryName);
    ui->lineEdit_unitprice->setText(QString::number(product.unitPrice, 'f', 2));
    ui->lineEdit_action->setText(product.action);
    ui->lineEdit_actionsub->setText(product.subaction);
    ui->lineEdit_describe->setText(product.description);
    ui->lineEdit_imagepath->setText(product.imageUrl);

    // 在编辑模式下，商品ID不可编辑
    ui->lineEdit_productId->setEnabled(false);
}
void addproduct::setupConnections()
{
    // 可以在这里添加额外的信号连接
    connect(ui->lineEdit_productId, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_productname->setFocus();
    });

    connect(ui->lineEdit_productname, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_categoryid->setFocus();
    });

    connect(ui->lineEdit_categoryid, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_categoryname->setFocus();
    });

    connect(ui->lineEdit_categoryname, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_unitprice->setFocus();
    });

    connect(ui->lineEdit_unitprice, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_action->setFocus();
    });

    connect(ui->lineEdit_action, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_actionsub->setFocus();
    });

    connect(ui->lineEdit_actionsub, &QLineEdit::returnPressed, [this]() {
        on_ok_button_clicked();
    });
}

void addproduct::on_image_browse_button_clicked()
{
    // 获取商品信息
    QString productId = ui->lineEdit_productId->text().trimmed();
    QString categoryId = ui->lineEdit_categoryid->text().trimmed();

    // 确定目标目录路径
    QString targetDir;

    if (!productId.isEmpty() && !categoryId.isEmpty()) {
        // 如果商品ID和分类ID都不为空，则使用 Products/categoryId_productId/ 目录
        targetDir = QString("Products/%1_%2").arg(categoryId).arg(productId);
    } else if (!productId.isEmpty()) {
        // 如果只有商品ID，则使用 Products/productId/ 目录
        targetDir = QString("Products/%1").arg(productId);
    } else {
        // 如果商品ID为空，使用默认的Products目录
        targetDir = "Products";
    }

    // 确保目录存在
    QDir dir;
    if (!dir.exists(targetDir)) {
        if (dir.mkpath(targetDir)) {
            qDebug() << "创建目录:" << targetDir;
        } else {
            QMessageBox::warning(this, "警告",
                                 QString("无法创建目录：\n%1\n\n将使用当前目录。").arg(targetDir));
            targetDir = QDir::currentPath();
        }
    }

    // 打开文件对话框，直接定位到目标目录
    QString filePath = QFileDialog::getOpenFileName(
        this,
        "选择商品图片",
        targetDir,
        "图片文件 (*.jpg *.jpeg *.png *.bmp *.gif *.webp);;所有文件 (*.*)"
        );

    if (!filePath.isEmpty()) {
        // 显示选中的文件路径
        ui->lineEdit_imagepath->setText(filePath);

        // 获取文件信息
        QFileInfo fileInfo(filePath);
        QString fileName = fileInfo.completeBaseName(); // 不含扩展名的文件名
        QString fileDir = fileInfo.dir().path(); // 文件所在目录

        // 检查文件是否在正确的目录中
        if (!fileDir.contains(targetDir)) {
            // 如果文件不在目标目录中，询问是否要复制
            QMessageBox::StandardButton reply;
            reply = QMessageBox::question(this, "确认",
                                          QString("图片不在商品目录中。\n是否要复制到商品目录？\n\n原路径：%1\n目标目录：%2")
                                              .arg(filePath)
                                              .arg(targetDir),
                                          QMessageBox::Yes | QMessageBox::No);

            if (reply == QMessageBox::Yes) {
                // 复制文件到商品目录
                QString newFilePath = QDir(targetDir).filePath(fileInfo.fileName());
                if (QFile::copy(filePath, newFilePath)) {
                    ui->lineEdit_imagepath->setText(newFilePath);
                    QMessageBox::information(this, "成功", "图片已复制到商品目录！");
                } else {
                    QMessageBox::warning(this, "警告", "复制图片失败！");
                }
            }
        }

        // 自动填充图片信息（只在添加模式下）
        if (!isEditMode) {
            // 如果商品名称为空，可以用图片文件名作为建议
            if (ui->lineEdit_productname->text().trimmed().isEmpty()) {
                ui->lineEdit_productname->setText(fileName);
            }

            // 如果商品ID为空，可以用图片文件名生成ID（去除特殊字符）
            if (ui->lineEdit_productId->text().trimmed().isEmpty()) {
                QString suggestedId = fileName.replace(QRegularExpression("[^a-zA-Z0-9_]"), "_");
                ui->lineEdit_productId->setText(suggestedId.toUpper());
            }

            // 如果分类ID为空，可以用目录名作为建议
            if (ui->lineEdit_categoryid->text().trimmed().isEmpty()) {
                // 尝试从目录名提取分类信息
                QString dirName = fileInfo.dir().dirName();
                if (dirName.contains('_')) {
                    QString category = dirName.split('_').first();
                    ui->lineEdit_categoryid->setText(category);
                    ui->lineEdit_categoryname->setText(category + "分类");
                }
            }
        }
    }
}

// 修改 on_ok_button_clicked 函数
void addproduct::on_ok_button_clicked()
{
    // 验证输入
    if (!validateInput()) {
        return;
    }

    SQL_Product product = getProduct();

    if (isEditMode) {
        // 编辑模式：更新商品
        if (dbManager->updateProduct(product)) {
            QMessageBox::information(this, "成功", "商品更新成功！");
            accept();
        } else {
            QMessageBox::critical(this, "错误", "商品更新失败！");
        }
    } else {
        // 添加模式：检查商品ID是否已存在
        SQL_Product existingProduct = dbManager->getProductById(product.productId);
        if (!existingProduct.productId.isEmpty()) {
            QMessageBox::warning(this, "警告",
                                 QString("商品ID '%1' 已存在！\n请使用不同的商品ID。").arg(product.productId));
            ui->lineEdit_productId->setFocus();
            ui->lineEdit_productId->selectAll();
            return;
        }

        // 插入数据库
        if (dbManager->insertProduct(product)) {
            QMessageBox::information(this, "成功", "商品添加成功！");
            accept();
        } else {
            QMessageBox::critical(this, "错误", "商品添加失败！");
        }
    }
}
bool addproduct::validateInput()
{
    // 检查必填字段
    if (ui->lineEdit_productId->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "商品ID不能为空！");
        ui->lineEdit_productId->setFocus();
        return false;
    }

    // 商品ID格式检查（只允许字母、数字、下划线）
    QString productId = ui->lineEdit_productId->text().trimmed();
    QRegularExpression regex("^[a-zA-Z0-9_]+$");
    if (!regex.match(productId).hasMatch()) {
        QMessageBox::warning(this, "警告",
                             "商品ID只能包含字母、数字和下划线！\n不能包含空格或特殊字符。");
        ui->lineEdit_productId->setFocus();
        ui->lineEdit_productId->selectAll();
        return false;
    }

    if (ui->lineEdit_productname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "商品名称不能为空！");
        ui->lineEdit_productname->setFocus();
        return false;
    }

    if (ui->lineEdit_categoryid->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "分类ID不能为空！");
        ui->lineEdit_categoryid->setFocus();
        return false;
    }

    if (ui->lineEdit_categoryname->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "分类名称不能为空！");
        ui->lineEdit_categoryname->setFocus();
        return false;
    }

    if (ui->lineEdit_unitprice->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "价格不能为空！");
        ui->lineEdit_unitprice->setFocus();
        return false;
    }

    if (ui->lineEdit_action->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "action不能为空！");
        ui->lineEdit_action->setFocus();
        return false;
    }

    // actionsub可以为空，不需要验证

    return true;
}

QString addproduct::getDefaultStatus()
{
    // 可以根据需要返回不同的默认状态
    // 例如：return "active"; 或 return "inactive";
    return "active";  // 默认状态为活跃
}

// 获取商品信息
SQL_Product addproduct::getProduct() const
{
    SQL_Product product;

    product.productId = ui->lineEdit_productId->text().trimmed();
    product.productName = ui->lineEdit_productname->text().trimmed();
    product.categoryId = ui->lineEdit_categoryid->text().trimmed();
    product.categoryName = ui->lineEdit_categoryname->text().trimmed();
    product.unitPrice = ui->lineEdit_unitprice->text().toDouble();
    product.action = ui->lineEdit_action->text().trimmed();
    product.subaction = ui->lineEdit_actionsub->text().trimmed();
    product.description = ui->lineEdit_describe->text().trimmed();
    product.imageUrl = ui->lineEdit_imagepath->text().trimmed();

    // 其他字段保持原值或使用默认值
    product.stock = 0;  // 可以根据需要从界面获取
    product.status = "active";
    product.minOrder = 1;
    product.maxOrder = 9999;
    product.salesCount = 0;
    product.rating = 0.0;
    product.ratingCount = 0;
    product.tags = "";
    product.specifications = "";

    return product;
}
