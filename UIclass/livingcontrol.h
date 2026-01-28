#ifndef LIVINGCONTROL_H
#define LIVINGCONTROL_H

#include <QWidget>
#include <QTimer>
#include <QStandardItemModel>
#include <publicheader.h>
#include "ai_bragger.h"  // 包含AI_bragger头文件

namespace Ui {
class livingcontrol;
}

class livingcontrol : public QWidget
{
    Q_OBJECT

public:
    explicit livingcontrol(QWidget *parent = nullptr, AI_bragger *aiBragger = nullptr);
    ~livingcontrol();

    void setAIbragger(AI_bragger *aiBragger);

private slots:
    void on_pushButton_allow_clicked();
    void on_pushButton_general_clicked();
    void onProgramDoubleClicked(const QModelIndex &index);
    void updateUI();


    void on_pushButton_para_clicked();

private:
    Ui::livingcontrol *ui;

    // 数据
    AI_bragger *m_aiBragger = nullptr;

    // 模型
    QStandardItemModel *programModel;
    QStandardItemModel *deviceModel;

    // 定时器
    QTimer *m_updateTimer;
    int m_currentProgramIndex = -1;
};

#endif // LIVINGCONTROL_H
