#ifndef COMMANDLSIT_H
#define COMMANDLSIT_H

#include <QWidget>
#include "DatabaseManager.h"

namespace Ui {
class commandlsit;
}

class commandlsit : public QWidget
{
    Q_OBJECT

public:
    explicit commandlsit(DatabaseManager* dbManager, QWidget *parent = nullptr);
    ~commandlsit();

private slots:
    void on_pushButton_clicked();
    void onDeleteButtonClicked(const QString &commandId);

    void on_pushButton_clear_clicked();

private:
    Ui::commandlsit *ui;
    DatabaseManager* m_dbManager;

    void loadCommandList();
    void setupTableWidget();
    void addDeleteButtonToRow(int row, const QString &commandId);
    void filterCommands(const QString &keyword);
    void onViewButtonClicked(const QString &imagePath);
    void addViewButtonToRow(int row, const QString &imagePath);
};

#endif // COMMANDLSIT_H
