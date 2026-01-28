#ifndef AI_BRAGGER_H
#define AI_BRAGGER_H

#include <QObject>
#include <QThread>
#include "publicheader.h"
#include <QJsonObject>
#include <QTimer>

class AI_bragger : public QThread
{
    Q_OBJECT
public:
    AI_bragger();
    ~AI_bragger();

    void sethostpath(QString ip, QString port);
    QVector<ProgramInfo> ProgramList;  // 前端任务容器

public slots:
    void onProgramInfoGenerated(const ProgramInfo &programInfo);
    void checkProgramList();  // 定时检查节目列表

protected:
    void run() override;

private:
    QString host_ip;
    QString host_port;
    QTimer *checkTimer;

    QString generateRandomSuffix();  // 生成随机后缀
    QString getCurrentTimestamp();   // 获取当前时间戳
    void checkAndDistributeBraggers();
    QStringList splitBraggerByDevices(const QString &bragger, int deviceCount);
    QTimer* aiCooldownTimer;  // 新增：AI冷却计时器
    void checkCoolDown();
signals:
    void sCommadSend(QString topic, QString msg);
private slots:
    void resetAIState();
};

#endif // AI_BRAGGER_H
