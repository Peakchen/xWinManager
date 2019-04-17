#ifndef PACKAGERUNNER_H
#define PACKAGERUNNER_H

#include <QObject>
#include <QProcess>

#include "ConfOperation.h"
#include "DataStruct.h"

class LaunchThread;

class PackageRunner : public QObject
{
    Q_OBJECT
public:
    PackageRunner(QObject *parent=0);
    bool Init();
    void UnInit();

protected:
    bool InitMiniXL();
    bool LoadDll();
    void UnloadDll();
    void InitBak();

    void EncodeToVariantMap(const LPDownloadItemData &pItemData, QVariantMap &object); // Encode DownloadItemData to QVariantMap
    void EncodeToDownloadItemData(const QVariantMap &object, LPDownloadItemData &pItemData); // Encode QVariantMap to DownloadItemData

    // read priceInfo , if want to prompt price to take off
    void DealWithPromptPriceInfo(QString Abrief, float Aprice);

    void SendSignal(const LPDownloadItemData &pItemData);

    void DestoryDownloadData();
    void DestoryItemData(LPDownloadItemData &pItemData);
    void DestoryItemTask(LPDownloadItemData &pItemData);

    void AddNewItemData(const QVariantMap &task);
    void SaveBakDataItem();

protected:
    bool m_bPolling;
    DownWrapper *m_downloadWapper;
    QDownloadItemDataMap m_mapDownloadData;

    QBackupDataMap m_mapBackupData;

    // for install
    QProcess *m_pInstaller;
    LPDownloadItemData m_pCurrentItemData;

signals:
    void sigCrash();
    void installTaskStart();

    void sigPromptPriceInfo(QString);
    // backup and Restore
    void sigUpdateOneTaskInfo(QVariantMap);
    void sigupdateBackupSysSoftListInfo(bool, QVariantMap);
    void sigReplyRestoreSysInfo(bool);

public slots:
    void reqAllTaskInfo();
    void reqAllBackupInfo();

    void reqAddTask(QVariantMap task);
    void reqAddTasks(QVariantList tasks);

    void reqPauseTask(QString id);
    void reqPauseAllTask();

    void reqResumeTask(QString id);
    void reqResumeAllTask();

    void reqRemoveTask(QString id);
    void reqRemoveAllTask();
    void reqBackupSysSoftListInfo();
    void reqRestoreSysInfo(QString);

    void DeleteLaunchThread(LaunchThread *pThread);

protected slots:
    void DownloadDataPoll();
    void PeriodInstallTask();
    void InstallerFinished(int exitCode, QProcess::ExitStatus exitStatus);
};

#endif // PACKAGERUNNER_H
