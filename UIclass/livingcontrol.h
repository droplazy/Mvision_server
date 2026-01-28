#ifndef LIVINGCONTROL_H
#define LIVINGCONTROL_H

#include <QWidget>
#include <QVector>
#include <QStandardItemModel>
#include <publicheader.h>

namespace Ui {
class livingcontrol;
}

class livingcontrol : public QWidget
{
    Q_OBJECT

public:
    explicit livingcontrol(QWidget *parent = nullptr);
    ~livingcontrol();

public slots:
    void onProgramInfoGenerated(const ProgramInfo &programInfo);

private slots:
    void on_pushButton_allow_clicked();
    void on_pushButton_general_clicked();
    void onProgramClicked(const QModelIndex &index);

private:
    Ui::livingcontrol *ui;

    // 节目容器
    QVector<ProgramInfo> programList;

    // 列表模型
    QStandardItemModel *programModel;
    QStandardItemModel *deviceModel;
};

#endif // LIVINGCONTROL_H
