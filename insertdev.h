#ifndef INSERTDEV_H
#define INSERTDEV_H

#include <QDialog>

namespace Ui {
class insertdev;
}

class insertdev : public QDialog
{
    Q_OBJECT

public:
    explicit insertdev(QWidget *parent = nullptr);
    ~insertdev();

    // 获取输入数据的接口
    QString getSerialNumber() const;
    QString getChecksum() const;
    QString getBelongUser() const;

    // 清空输入框
    void clearInputs();

private slots:
    void on_pushButton_ok_clicked();
    void on_pushButton_cancle_clicked();

private:
    Ui::insertdev *ui;
};

#endif // INSERTDEV_H
