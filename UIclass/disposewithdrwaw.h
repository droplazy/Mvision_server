#ifndef DISPOSEWITHDRWAW_H
#define DISPOSEWITHDRWAW_H

#include <QDialog>
#include <QFileInfo>
#include <QPixmap>
#include "DatabaseManager.h"

namespace Ui {
class disposewithdrwaw;
}

class disposewithdrwaw : public QDialog
{
    Q_OBJECT

public:
    explicit disposewithdrwaw(QWidget *parent = nullptr,
                              DatabaseManager *dbManager = nullptr,
                              const QString &withdrawId = QString());
    ~disposewithdrwaw();

    void setWithdrawInfo(const QString &withdrawId, const QString &username,
                         double amount, const QString &alipayAccount);

signals:
    void withdrawProcessed(const QString &withdrawId, bool success);

private slots:
    void on_pushButton_browse_clicked();
    void on_pushButton_check_clicked();

protected:
    bool eventFilter(QObject *obj, QEvent *event) override;
private:
    Ui::disposewithdrwaw *ui;
    DatabaseManager *dbManager;
    QString currentWithdrawId;

    void setupDragDrop();
    bool validateImage(const QString &filePath);
    void updateImagePreview(const QString &filePath);
};

#endif // DISPOSEWITHDRWAW_H
