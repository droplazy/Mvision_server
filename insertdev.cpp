#include "insertdev.h"
#include "ui_insertdev.h"

insertdev::insertdev(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::insertdev)
{
    ui->setupUi(this);

    // 设置窗口标题
    setWindowTitle("添加设备");

    // 设置输入框占位符提示
    ui->lineEdit_serialnum->setPlaceholderText("请输入设备序列号");
    ui->lineEdit_checksum->setPlaceholderText("请输入校验码");
    ui->lineEdit_belong->setPlaceholderText("请输入归属用户（可选）");

    // 设置输入验证
    ui->lineEdit_serialnum->setMaxLength(50);
    ui->lineEdit_checksum->setMaxLength(50);
    ui->lineEdit_belong->setMaxLength(50);

    // 设置默认焦点
    ui->lineEdit_serialnum->setFocus();
}

insertdev::~insertdev()
{
    delete ui;
}

QString insertdev::getSerialNumber() const
{
    return ui->lineEdit_serialnum->text().trimmed();
}

QString insertdev::getChecksum() const
{
    return ui->lineEdit_checksum->text().trimmed();
}

QString insertdev::getBelongUser() const
{
    return ui->lineEdit_belong->text().trimmed();
}

void insertdev::clearInputs()
{
    ui->lineEdit_serialnum->clear();
    ui->lineEdit_checksum->clear();
    ui->lineEdit_belong->clear();
    ui->lineEdit_serialnum->setFocus();
}

void insertdev::on_pushButton_ok_clicked()
{
    // 验证必填字段
    if (getSerialNumber().isEmpty()) {
        ui->lineEdit_serialnum->setStyleSheet("border: 1px solid red;");
        ui->lineEdit_serialnum->setFocus();
        return;
    } else {
        ui->lineEdit_serialnum->setStyleSheet("");
    }

    if (getChecksum().isEmpty()) {
        ui->lineEdit_checksum->setStyleSheet("border: 1px solid red;");
        ui->lineEdit_checksum->setFocus();
        return;
    } else {
        ui->lineEdit_checksum->setStyleSheet("");
    }

    // 验证通过，接受对话框
    accept();
}

void insertdev::on_pushButton_cancle_clicked()
{
    // 拒绝对话框
    reject();
}
