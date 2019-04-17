#include "TaskManager.h"
#include <QTimer>
#include <QVariantMap>
#include "DataControl.h"
#include "PackageRunner.h"

TaskManager::TaskManager(QObject * parent) : QThread(parent)
{
}

void TaskManager::SetObjects(DataControl *dataControl,PackageRunner *runner) {
    pTaskRunner = runner;
    pDataControl= dataControl;
    pTaskRunner->moveToThread(this);

    // request
    QObject::connect(pDataControl,SIGNAL(sigQueryAllTaskInfo()),pTaskRunner,SLOT(reqAllTaskInfo()),Qt::QueuedConnection);

    QObject::connect(pDataControl,SIGNAL(sigAddTask(QVariantMap)),pTaskRunner,SLOT(reqAddTask(QVariantMap)),Qt::QueuedConnection);
    QObject::connect(pDataControl,SIGNAL(sigAddTasks(QVariantList)),pTaskRunner,SLOT(reqAddTasks(QVariantList)),Qt::QueuedConnection);

    QObject::connect(pDataControl,SIGNAL(sigPauseTask(QString)),pTaskRunner,SLOT(reqPauseTask(QString)),Qt::QueuedConnection);
    QObject::connect(pDataControl,SIGNAL(sigPauseAllTask()),pTaskRunner,SLOT(reqPauseAllTask()),Qt::QueuedConnection);

    QObject::connect(pDataControl,SIGNAL(sigResumeTask(QString)),pTaskRunner,SLOT(reqResumeTask(QString)),Qt::QueuedConnection);
    QObject::connect(pDataControl,SIGNAL(sigResumeAllTask()),pTaskRunner,SLOT(reqResumeAllTask()),Qt::QueuedConnection);

    QObject::connect(pDataControl,SIGNAL(sigRemoveTask(QString)),pTaskRunner,SLOT(reqRemoveTask(QString)),Qt::QueuedConnection);
    QObject::connect(pDataControl,SIGNAL(sigRemoveAllTask()),pTaskRunner,SLOT(reqRemoveAllTask()),Qt::QueuedConnection);

    QObject::connect(pTaskRunner, SIGNAL(sigCrash()),pDataControl, SIGNAL(sigCrash()),Qt::QueuedConnection);

    QObject::connect(pTaskRunner, SIGNAL(sigUpdateOneTaskInfo(QVariantMap)), pDataControl, SIGNAL(sigUpdateOneTaskInfo(QVariantMap)), Qt::QueuedConnection);

    QObject::connect(pDataControl, SIGNAL(sigBackupSysSoftListInfo()), pTaskRunner, SLOT(reqBackupSysSoftListInfo()) ,Qt::QueuedConnection);

    QObject::connect(pTaskRunner, SIGNAL(sigupdateBackupSysSoftListInfo(bool, QVariantMap)), pDataControl, SIGNAL(sigupdateBackupSysSoftListInfo(bool, QVariantMap)),Qt::QueuedConnection);

    QObject::connect(pDataControl, SIGNAL(sigQueryBackupInfo()), pTaskRunner, SLOT(reqAllBackupInfo()));

    QObject::connect(pDataControl, SIGNAL(sigRestoreSysInfo(QString)), pTaskRunner, SLOT(reqRestoreSysInfo(QString)) ,Qt::QueuedConnection);
    QObject::connect(pTaskRunner, SIGNAL(sigReplyRestoreSysInfo(bool)), pDataControl, SIGNAL(sigReplyRestoreSysInfo(bool)) ,Qt::QueuedConnection);

}

void TaskManager::run() {
    if (pTaskRunner!=NULL && pDataControl!=NULL) {
        // load task list
        pTaskRunner->Init();
        QTimer downloadPeriod ,installPeriod;

        // 每2秒轮询一次下载任务列表
        QObject::connect(&downloadPeriod,SIGNAL(timeout()),pTaskRunner,SLOT(DownloadDataPoll()));
        // 每5秒发送一次installTaskStart的信号, 用来启动安装
        QObject::connect(&installPeriod,SIGNAL(timeout()),pTaskRunner,SIGNAL(installTaskStart()));

        downloadPeriod.start(1000);
//        installPeriod.start(5000);
        QThread::exec();
        downloadPeriod.stop();
//        installPeriod.stop();
        pTaskRunner->UnInit();
        if(pTaskRunner != NULL)
        {
            pTaskRunner = NULL;
        }
    }
    else{
        QThread::exit(1);
    }
}
