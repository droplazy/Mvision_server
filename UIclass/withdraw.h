#ifndef WITHDRAW_H
#define WITHDRAW_H

#include <QWidget>
#include <QStandardItemModel>
#include "DatabaseManager.h"

namespace Ui {
class withdraw;
}

class withdraw : public QWidget
{
    Q_OBJECT

public:
    explicit withdraw(QWidget *parent = nullptr, DatabaseManager *dbManager = nullptr);
    ~withdraw();

    void setDatabaseManager(DatabaseManager *dbManager);
    void refreshWithdrawTable();

private slots:
    void on_pushButton_clicked();  // 搜索按钮
    void on_processButtonClicked();  // 处理按钮点击

private:
    Ui::withdraw *ui;
    DatabaseManager *dbManager;
    QStandardItemModel *tableModel;

    void setupTableHeader();
    void loadWithdrawRecords(const QString &filterUsername = "");
    void addProcessButtonToRow(int row, const QString &withdrawId);
    void processWithdrawRecord(const QString &withdrawId);
};

#endif // WITHDRAW_H
