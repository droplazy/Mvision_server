#ifndef ADDPRODUCT_H
#define ADDPRODUCT_H

#include <QDialog>
#include "DatabaseManager.h"

namespace Ui {
class addproduct;
}

class addproduct : public QDialog
{
    Q_OBJECT

public:
    explicit addproduct(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~addproduct();

    // 设置编辑模式
    void setEditMode(const SQL_Product &product);

    // 获取商品信息
    SQL_Product getProduct() const;

private slots:
    void on_image_browse_button_clicked();
    void on_ok_button_clicked();

private:
    Ui::addproduct *ui;
    DatabaseManager *dbManager;
    bool isEditMode;  // 是否为编辑模式
    QString originalProductId;  // 原始商品ID（用于编辑模式）

    bool validateInput();  // 验证输入
    void setupConnections();  // 设置信号连接
    void fillProductData(const SQL_Product &product);  // 填充商品数据
    QString getDefaultStatus();
    void updateImagePath();
};

#endif // ADDPRODUCT_H
