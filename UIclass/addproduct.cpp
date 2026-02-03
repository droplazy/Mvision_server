#include "addproduct.h"
#include "ui_addproduct.h"
#include "DatabaseManager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QFileInfo>
#include <QDebug>
#include <QDir>
#include <QProcess>
#include <QDesktopServices>
#include <QUrl>

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
    setFixedSize(1000,620);

    if (!dbManager) {
        QMessageBox::critical(this, "错误", "数据库管理器为空！");
    }

    // lineEdit_imagepath 设置为只读，禁止编辑
    ui->lineEdit_imagepath->setReadOnly(true);

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

    // 编辑模式下，imagepath保持可查看但不可编辑
    ui->lineEdit_imagepath->setReadOnly(true);
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
    ui->lineEdit_stock->setText(QString::number(product.stock));  // 添加库存显示
    ui->lineEdit_action->setText(product.action);
    ui->lineEdit_actionsub->setText(product.subaction);
    ui->lineEdit_describe->setText(product.description);

    // 显示相对路径（如果是绝对路径，转换为相对路径）
    QString imagePath = product.imageUrl;
    if (QFileInfo(imagePath).isAbsolute()) {
        // 如果是绝对路径，尝试转换为相对于当前目录的路径
        QDir currentDir(QDir::currentPath());
        imagePath = currentDir.relativeFilePath(imagePath);
    }
    ui->lineEdit_imagepath->setText(imagePath);

    // 在编辑模式下，商品ID不可编辑
    ui->lineEdit_productId->setEnabled(false);
}

void addproduct::setupConnections()
{
    // 可以在这里添加额外的信号连接
    connect(ui->lineEdit_productId, &QLineEdit::textChanged, [this](const QString &text) {
        if (!isEditMode) {
            // 添加模式下，商品ID改变时更新图片路径
            updateImagePath();
        }
    });

    connect(ui->lineEdit_categoryid, &QLineEdit::textChanged, [this](const QString &text) {
        if (!isEditMode) {
            // 添加模式下，分类ID改变时更新图片路径
            updateImagePath();
        }
    });

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
        ui->lineEdit_stock->setFocus();  // 添加库存输入框的焦点切换
    });

    connect(ui->lineEdit_stock, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_action->setFocus();
    });

    connect(ui->lineEdit_action, &QLineEdit::returnPressed, [this]() {
        ui->lineEdit_actionsub->setFocus();
    });

    connect(ui->lineEdit_actionsub, &QLineEdit::returnPressed, [this]() {
        on_ok_button_clicked();
    });
}

// 更新图片路径（添加模式下自动生成）
void addproduct::updateImagePath()
{
    if (!isEditMode) {
        QString productId = ui->lineEdit_productId->text().trimmed();
        QString categoryId = ui->lineEdit_categoryid->text().trimmed();

        if (!productId.isEmpty() && !categoryId.isEmpty()) {
            // 生成相对路径：Products/分类ID_商品ID
            QString relativePath = QString("Products/%1_%2").arg(categoryId).arg(productId);
            ui->lineEdit_imagepath->setText(relativePath);
        } else if (!productId.isEmpty()) {
            // 只有商品ID，使用 Products/商品ID
            ui->lineEdit_imagepath->setText(QString("Products/%1").arg(productId));
        } else {
            // 都没有，使用默认路径
            ui->lineEdit_imagepath->setText("Products");
        }
    }
}

void addproduct::on_image_browse_button_clicked()
{
    // 获取当前设置的相对路径
    QString relativePath = ui->lineEdit_imagepath->text().trimmed();

    // 如果路径为空，使用默认路径
    if (relativePath.isEmpty()) {
        // 添加模式下，尝试自动生成路径
        if (!isEditMode) {
            updateImagePath();
            relativePath = ui->lineEdit_imagepath->text().trimmed();
        }

        // 如果还是为空，使用默认路径
        if (relativePath.isEmpty()) {
            relativePath = "Products";
        }
    }

    // 转换为绝对路径
    QString absolutePath = QDir::current().absoluteFilePath(relativePath);
    QDir targetDir(absolutePath);

    // 确保目录存在，如果不存在则创建
    if (!targetDir.exists()) {
        if (!targetDir.mkpath(".")) {
            QMessageBox::warning(this, "警告",
                                 QString("无法创建目录：\n%1").arg(absolutePath));
            return;
        }
    }

    // 使用 QDesktopServices 打开文件夹（跨平台）
    if (QDesktopServices::openUrl(QUrl::fromLocalFile(absolutePath))) {
        qDebug() << "成功打开文件夹:" << absolutePath;

        // 显示提示信息
        QMessageBox::information(this, "提示",
                                 QString("已打开文件夹：\n%1\n\n请将商品图片放入此文件夹。").arg(relativePath));
    } else {
        // 如果 QDesktopServices 失败，尝试使用平台特定命令
        bool success = false;

#ifdef Q_OS_WINDOWS
        QString cmd = "explorer";
        QString nativePath = QDir::toNativeSeparators(absolutePath);

        // 移除末尾的斜杠
        if (nativePath.endsWith('\\') || nativePath.endsWith('/')) {
            nativePath.chop(1);
        }

        QStringList args;
        args << nativePath;
        success = QProcess::startDetached(cmd, args);

#elif defined(Q_OS_MAC)
        success = QProcess::startDetached("open", QStringList() << absolutePath);
#else
        success = QProcess::startDetached("xdg-open", QStringList() << absolutePath);
#endif

        if (!success) {
            QMessageBox::warning(this, "错误",
                                 QString("无法打开文件夹：\n%1\n\n请检查路径是否正确。").arg(relativePath));
        } else {
            qDebug() << "使用系统命令打开文件夹:" << absolutePath;
            QMessageBox::information(this, "提示",
                                     QString("已打开文件夹：\n%1\n\n请将商品图片放入此文件夹。").arg(relativePath));
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

        // 在添加模式下，创建图片文件夹
        QString imagePath = ui->lineEdit_imagepath->text().trimmed();
        if (!imagePath.isEmpty()) {
            QString absolutePath = QDir::current().absoluteFilePath(imagePath);
            QDir dir(absolutePath);
            if (!dir.exists()) {
                if (!dir.mkpath(".")) {
                    QMessageBox::warning(this, "注意",
                                         QString("无法创建图片文件夹：\n%1\n\n商品仍会被添加，但需要手动创建文件夹。").arg(imagePath));
                } else {
                    qDebug() << "创建图片文件夹:" << absolutePath;
                }
            }
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

    // 验证价格是否为有效数字
    bool ok;
    double price = ui->lineEdit_unitprice->text().toDouble(&ok);
    if (!ok || price < 0) {
        QMessageBox::warning(this, "警告", "请输入有效的价格！");
        ui->lineEdit_unitprice->setFocus();
        ui->lineEdit_unitprice->selectAll();
        return false;
    }

    // 验证库存是否为有效数字
    if (ui->lineEdit_stock->text().trimmed().isEmpty()) {
        QMessageBox::warning(this, "警告", "库存不能为空！");
        ui->lineEdit_stock->setFocus();
        return false;
    }

    int stock = ui->lineEdit_stock->text().toInt(&ok);
    if (!ok || stock < 0) {
        QMessageBox::warning(this, "警告", "请输入有效的库存数量！");
        ui->lineEdit_stock->setFocus();
        ui->lineEdit_stock->selectAll();
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
    product.stock = ui->lineEdit_stock->text().toInt();  // 从界面获取库存
    product.action = ui->lineEdit_action->text().trimmed();
    product.subaction = ui->lineEdit_actionsub->text().trimmed();
    product.description = ui->lineEdit_describe->text().trimmed();

    // 获取相对路径（imagepath是只读的，显示相对路径）
    QString imagePath = ui->lineEdit_imagepath->text().trimmed();

    // 确保路径是相对路径
    if (QFileInfo(imagePath).isAbsolute()) {
        // 如果是绝对路径，转换为相对路径
        QDir currentDir(QDir::currentPath());
        imagePath = currentDir.relativeFilePath(imagePath);
    }

    product.imageUrl = imagePath;  // 保存相对路径

    // 其他字段保持原值或使用默认值
    product.status = "active";
    product.minOrder = 1;
    product.maxOrder = 9999;
    product.salesCount = 0;
    product.rating = 0.0;
    product.ratingCount = 0;
    product.tags = "";
    product.specifications = "";
    product.createTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    product.updateTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");

    return product;
}
