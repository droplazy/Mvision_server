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

    // 创建定时器（1000ms更新一次，避免过于频繁）
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

    // 1. 更新节目列表（只增不减，避免频繁刷新）
    const QVector<ProgramInfo>& programList = m_aiBragger->ProgramList;
    int currentProgramCount = programModel->rowCount();

    if (currentProgramCount < programList.size()) {
        // 添加新节目
        for (int i = currentProgramCount; i < programList.size(); ++i) {
            const ProgramInfo& program = programList[i];
            QStandardItem *item = new QStandardItem(program.commandId);
            programModel->appendRow(item);
        }
    }

    // 2. 如果有选中的节目，更新其详细信息
    if (m_currentProgramIndex >= 0 && m_currentProgramIndex < programList.size()) {
        const ProgramInfo& program = programList[m_currentProgramIndex];

        // 更新设备列表
        deviceModel->clear();
        for (const QString &device : program.deviceList) {
            QStandardItem *deviceItem = new QStandardItem(device);
            deviceModel->appendRow(deviceItem);
        }

        // === 更新文本内容（追加模式） ===
        static QString lastVoiceText;   // 记录上次的语音文本
        static QString lastBraggerText; // 记录上次的AI评论

        QString currentTime = QDateTime::currentDateTime().toString("hh:mm:ss");

        // 处理 voicetotext（语音识别文本）
        QString currentVoiceText = program.voicetotext;

        if (!currentVoiceText.isEmpty() && currentVoiceText != lastVoiceText) {
            // 获取当前的完整历史
            QString fullHistory = ui->textEdit_up->toPlainText();

            if (!fullHistory.isEmpty()) {
                // 添加时间和分隔符
                fullHistory += QString("\n\n[%1] 语音识别：\n").arg(currentTime);
                fullHistory += currentVoiceText;
            } else {
                // 首次添加
                fullHistory = QString("[%1] 语音识别：\n").arg(currentTime) + currentVoiceText;
            }

            ui->textEdit_up->setPlainText(fullHistory);
            lastVoiceText = currentVoiceText;

            // 滚动到底部
            QTextCursor cursor = ui->textEdit_up->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit_up->setTextCursor(cursor);
        }

        // 处理 bragger（AI评论）
        QString currentBraggerText = program.bragger;

        if (!currentBraggerText.isEmpty() && currentBraggerText != lastBraggerText) {
            // 获取当前的完整历史
            QString fullHistory = ui->textEdit_content->toPlainText();

            if (!fullHistory.isEmpty()) {
                // 添加时间和分隔符
                fullHistory += QString("\n\n[%1] AI评论：\n").arg(currentTime);
                fullHistory += currentBraggerText;
            } else {
                // 首次添加
                fullHistory = QString("[%1] AI评论：\n").arg(currentTime) + currentBraggerText;
            }

            ui->textEdit_content->setPlainText(fullHistory);
            lastBraggerText = currentBraggerText;

            // 滚动到底部
            QTextCursor cursor = ui->textEdit_content->textCursor();
            cursor.movePosition(QTextCursor::End);
            ui->textEdit_content->setTextCursor(cursor);
        }

        // 更新状态（最精简显示）
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
    // 留空，你后续可以添加功能
}

void livingcontrol::on_pushButton_general_clicked()
{
    if (!m_aiBragger) {
        ui->label_status->setText("错误: AI对象为空");
        return;
    }

    // 获取选中的节目索引
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
    QString commandId = selectedProgram.commandId;

    // 重置状态（但不影响UI中的历史记录）
    QString currentTime = QDateTime::currentDateTime().toString("hh:mm:ss");

    // 在重置时在UI中添加重置标记，但不清空历史
    QString upHistory = ui->textEdit_up->toPlainText();
    if (!upHistory.isEmpty()) {
        upHistory += QString("\n\n[%1] 🔄 语音识别已重置\n").arg(currentTime);
        ui->textEdit_up->setPlainText(upHistory);
    }

    QString contentHistory = ui->textEdit_content->toPlainText();
    if (!contentHistory.isEmpty()) {
        contentHistory += QString("\n\n[%1] 🔄 AI评论已重置\n").arg(currentTime);
        ui->textEdit_content->setPlainText(contentHistory);
    }

    // 清空元数据，但不影响UI显示的历史
    selectedProgram.voicetotext.clear();
    selectedProgram.bragger.clear();
    selectedProgram.isListen = false;
    selectedProgram.isGenerating = false;

    // 更新状态显示，但不清空文本框
    QString statusText;
    if (selectedProgram.isStreaming) statusText += "推流中";
    if (selectedProgram.isListen) {
        if (!statusText.isEmpty()) statusText += " | ";
        statusText += "识别中";
    }
    if (selectedProgram.isGenerating) {
        if (!statusText.isEmpty()) statusText += " | ";
        statusText += "AI生成中";
    }
    if (statusText.isEmpty()) statusText = "就绪 (已重置)";

    ui->label_status->setText(statusText);
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

// 添加：当切换不同节目时，需要重置静态变量
void livingcontrol::onProgramDoubleClicked(const QModelIndex &index)
{
    if (!index.isValid()) return;

    m_currentProgramIndex = index.row();

    // 清空静态变量，准备接收新的节目数据
    static QString lastVoiceText;
    static QString lastBraggerText;
    lastVoiceText.clear();
    lastBraggerText.clear();

    // 立即更新一次UI
    updateUI();

    // 选中该项
    ui->listView_program->setCurrentIndex(index);
}
