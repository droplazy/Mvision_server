#ifndef MALLPRODUCTS_H
#define MALLPRODUCTS_H

#include <QDialog>
#include <QStandardItemModel>
#include "publicheader.h"
class DatabaseManager;

namespace Ui {
class mallproducts;
}

class mallproducts : public QDialog
{
    Q_OBJECT

public:
    explicit mallproducts(DatabaseManager *dbManager, QWidget *parent = nullptr);
    ~mallproducts();

private slots:
    void on_pushButton_search_clicked();
    void on_pushButton_add_clicked();

private:
    Ui::mallproducts *ui;
    DatabaseManager *dbManager;
    QStandardItemModel *model;

    void setupTableView();              // 设置表格
    void loadProducts();                // 加载商品数据
    void addDeleteButton(int row);      // 添加删除按钮
    void deleteProduct(int row);        // 删除商品

    // 辅助函数
    QString formatCreateTime(const QString &timeStr);  // 格式化创建时间
    void addImageButton(int row, const QString &imageUrl);
    void openImage(const QString &imageUrl);
    SQL_Product getProductFromTable(int row);
    void updateProductInTable(int row, const SQL_Product &product);
    void editProduct(int row, const SQL_Product &product);
    void addEditButton(int row, const SQL_Product &product);
};

#endif // MALLPRODUCTS_H
