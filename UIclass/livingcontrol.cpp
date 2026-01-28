#include "livingcontrol.h"
#include "ui_livingcontrol.h"
#include <QDebug>

livingcontrol::livingcontrol(QWidget *parent)
    : QWidget  (parent)
    , ui(new Ui::livingcontrol)
{
    ui->setupUi(this);

    // 初始化节目列表模型
    programModel = new QStandardItemModel(this);
    ui->listView_program->setModel(programModel);

    // 初始化设备列表模型
    deviceModel = new QStandardItemModel(this);
    ui->listView_device->setModel(deviceModel);

    // 连接节目列表的点击信号
    connect(ui->listView_program, &QListView::clicked,
            this, &livingcontrol::onProgramClicked);
}

livingcontrol::~livingcontrol()
{
    delete ui;
}

void livingcontrol::onProgramInfoGenerated(const ProgramInfo &programInfo)
{
    qDebug() << "收到节目信息：" << programInfo.programName;

    // 将节目信息添加到容器
    programList.append(programInfo);

    // 添加到节目列表视图
    QStandardItem *item = new QStandardItem(programInfo.programName);
    programModel->appendRow(item);

    // 如果是第一个节目，自动选中它
    if (programList.size() == 1) {
        QModelIndex firstIndex = programModel->index(0, 0);
        ui->listView_program->setCurrentIndex(firstIndex);
        onProgramClicked(firstIndex);
    }
}

void livingcontrol::onProgramClicked(const QModelIndex &index)
{
    if (!index.isValid() || index.row() >= programList.size()) {
        return;
    }

    // 获取选中的节目索引
    int selectedIndex = index.row();

    // 清空设备列表
    deviceModel->clear();

    // 获取对应的节目信息
    const ProgramInfo &program = programList[selectedIndex];

    qDebug() << "选中节目：" << program.programName
             << "，包含设备数量：" << program.deviceList.size();

    // 添加设备到设备列表
    for (const QString &device : program.deviceList) {
        QStandardItem *deviceItem = new QStandardItem(device);
        deviceModel->appendRow(deviceItem);
    }
}

void livingcontrol::on_pushButton_allow_clicked()
{

}

void livingcontrol::on_pushButton_general_clicked()
{

}
