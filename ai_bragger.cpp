#include "ai_bragger.h"
#include "QDebug"


AI_bragger::AI_bragger()
{

}

void AI_bragger::run()
{
    while(1)
    {
        qDebug() << "123" ;
        sleep(1);
    }
}
// 槽函数实现
void AI_bragger::onProgramInfoGenerated(const ProgramInfo &programInfo)
{
    qDebug() << "收到节目单信息，commandId:" << programInfo.commandId;
    qDebug() << "节目名称:" << programInfo.programName;
    qDebug() << "设备列表:" << programInfo.deviceList;

    // 添加到容器
    ProgramList.append(programInfo);

    qDebug() << "当前容器大小:" << ProgramList.size();
}
