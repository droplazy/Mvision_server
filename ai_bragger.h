#ifndef AI_BRAGGER_H
#define AI_BRAGGER_H

#include <QObject>
#include <QThread>
#include "publicheader.h"
class AI_bragger:public QThread
{
    Q_OBJECT
public:
    AI_bragger();

public slots:
    void onProgramInfoGenerated(const ProgramInfo &programInfo);
protected:
    void run() override;
private:
    QVector<ProgramInfo> ProgramList;  // 前端任务容器

};

#endif // AI_BRAGGER_H
