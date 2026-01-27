#include "livingcontrol.h"
#include "ui_livingcontrol.h"

livingcontrol::livingcontrol(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::livingcontrol)
{
    ui->setupUi(this);
}

livingcontrol::~livingcontrol()
{
    delete ui;
}

void livingcontrol::on_pushButton_allow_clicked()
{

}


void livingcontrol::on_pushButton_general_clicked()
{

}

