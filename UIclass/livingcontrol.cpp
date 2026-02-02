#include "livingcontrol.h"
#include "ui_livingcontrol.h"
#include <QDebug>

livingcontrol::livingcontrol(QWidget *parent, AI_bragger *aiBragger)
    : QWidget(parent)
    , ui(new Ui::livingcontrol)
    , m_aiBragger(aiBragger)
{
    ui->setupUi(this);

    // 初始化模型
    programModel = new QStandardItemModel(this);
    deviceModel = new QStandardItemModel(this);

    ui->listView_program->setModel(programModel);
    ui->listView_device->setModel(deviceModel);

    // 连接双击信号
    connect(ui->listView_program, &QListView::doubleClicked,
            this, &livingcontrol::onProgramDoubleClicked);

    // 创建定时器（1000ms更新一次）
    m_updateTimer = new QTimer(this);
    m_updateTimer->setInterval(1000);
    connect(m_updateTimer, &QTimer::timeout, this, &livingcontrol::updateUI);
    m_updateTimer->start();
}

livingcontrol::~livingcontrol()
{
    delete ui;
}

void livingcontrol::setAIbragger(AI_bragger *aiBragger)
{
    m_aiBragger = aiBragger;
}

void livingcontrol::updateUI()
{
    if (!m_aiBragger) return;

    // 更新节目列表
    const QVector<ProgramInfo>& programList = m_aiBragger->ProgramList;
    int currentProgramCount = programModel->rowCount();

    if (currentProgramCount < programList.size()) {
        for (int i = currentProgramCount; i < programList.size(); ++i) {
            const ProgramInfo& program = programList[i];
            QStandardItem *item = new QStandardItem(program.commandId);
            programModel->appendRow(item);
        }
    }

    // 更新选中节目的详细信息
    if (m_currentProgramIndex >= 0 && m_currentProgramIndex < programList.size()) {
        const ProgramInfo& program = programList[m_currentProgramIndex];

        // 更新设备列表
        deviceModel->clear();
        for (const QString &device : program.deviceList) {
            QStandardItem *deviceItem = new QStandardItem(device);
            deviceModel->appendRow(deviceItem);
        }

        // 直接显示历史记录
        ui->textEdit_up->setPlainText(program.historyVoice);
        ui->textEdit_content->setPlainText(program.historyAI);

        // 滚动到底部
        QTextCursor cursor = ui->textEdit_up->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textEdit_up->setTextCursor(cursor);

        cursor = ui->textEdit_content->textCursor();
        cursor.movePosition(QTextCursor::End);
        ui->textEdit_content->setTextCursor(cursor);

        // 更新状态
        QString statusText;
        if (program.isStreaming) statusText += "推流中";
        if (program.isListen) {
            if (!statusText.isEmpty()) statusText += " | ";
            statusText += "识别中";
        }
        if (program.isGenerating) {
            if (!statusText.isEmpty()) statusText += " | ";
            statusText += "AI生成中";
        }
        if (statusText.isEmpty()) statusText = "就绪";

        ui->label_status->setText(statusText);
    }
}

void livingcontrol::on_pushButton_allow_clicked()
{
    // 留空
}

void livingcontrol::on_pushButton_general_clicked()
{
    if (!m_aiBragger) {
        ui->label_status->setText("错误: AI对象为空");
        return;
    }

    QModelIndex currentIndex = ui->listView_program->currentIndex();
    if (!currentIndex.isValid()) {
        ui->label_status->setText("请先选择一个节目");
        return;
    }

    int selectedIndex = currentIndex.row();
    QVector<ProgramInfo>& programList = m_aiBragger->ProgramList;

    if (selectedIndex >= programList.size()) {
        ui->label_status->setText("错误: 索引越界");
        return;
    }

    // 获取选中的节目
    ProgramInfo& selectedProgram = programList[selectedIndex];

    // 直接清空历史记录（在ProgramInfo中已经保存了）
    selectedProgram.voicetotext.clear();
    selectedProgram.bragger.clear();
    selectedProgram.isListen = false;
    selectedProgram.isGenerating = false;

    ui->label_status->setText("就绪 (已重置)");
}

void livingcontrol::on_pushButton_para_clicked()
{
    if (!m_aiBragger) {
        ui->label_status->setText("错误: AI对象为空");
        return;
    }

    if (m_currentProgramIndex < 0) {
        ui->label_status->setText("请先双击选择一个节目");
        return;
    }

    QVector<ProgramInfo>& programList = m_aiBragger->ProgramList;
    int programCount = programList.size();

    if (m_currentProgramIndex >= programCount) {
        ui->label_status->setText("错误: 节目索引无效");
        return;
    }

    // 获取当前节目引用
    ProgramInfo& program = programList[m_currentProgramIndex];

    // 获取UI参数
    QString theme = ui->textEdit_theme->toPlainText().trimmed();
    QString scene = ui->textEdit_scene->toPlainText().trimmed();
    QString motion = ui->textEdit_motion->toPlainText().trimmed();
    QString guideword = ui->textEdit_head->toPlainText().trimmed();

    // 保存到节目
    program.theme = theme;
    program.scene = scene;
    program.motion = motion;
    program.guideword = guideword;

    // 显示状态
    QString status = QString("参数已保存到: %1").arg(program.commandId);
    if (!theme.isEmpty()) status += QString("\n主题: %1").arg(theme);
    if (!scene.isEmpty()) status += QString(" 场景: %1").arg(scene);

    ui->label_status->setText(status);
}

void livingcontrol::onProgramDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    m_currentProgramIndex = index.row();
    updateUI();
    ui->listView_program->setCurrentIndex(index);
}
