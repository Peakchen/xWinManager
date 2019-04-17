#ifndef DATACONTROL_H
#define DATACONTROL_H

#include <QObject>

#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <QLocalServer>
#include <QLocalSocket>
#include <QFileSystemWatcher>
#include <QTextCodec>
#include <QCryptographicHash>
#include <QVariantMap>
#include <QFileDialog>
#include <QFileInfo>

#include "OSSystemWrapper.h"
#include "ConfOperation.h"
#include "UserInfo.h"
#include "UserInfoManager.h"
#include "PackageRunner.h"
#include "TaskManager.h"
#include "checkuninstallsoftname.h"
#include "UpgradeData.h"
#include "UpgradeHandler.h"

class UninstallSoftware;
class ThreadToPullSoftwareList;

class DataControl : public QObject
{
    Q_OBJECT
public:
    DataControl(QObject *parent=0);

    void unInit();
    bool initAll();
    void LoadSettingProfile();
    void SaveSettingProfile();

    void StartUserService();
    void StartUpgradeService();
    void StartDownloadService();
    void StartUninstallService();

    QVariantList &GetCategory();
    QVariantList &GetPackage(const QString szCategoryID);
    void GetPackage2(const QString szCategoryID, QVariantList &lstPackage);

    QString getSettingParameter(QString name, QString defaultValue);
    QString &getszAppdataPath();
    QVariantMapMap &getUnstallSoftInfo();
    QVector<QString> &getUnstallSoftInfoIdx();
    mapSoftwareList &getInstalledSoftware();
    QString getUserToken(){return _user.getUserToken();}
    PackageRunner &getPackageRunner();
    UserInfo &getUserInstance();
    void InitSoftWareOrderBySize(QVariantList &);
    static void GetCurInstalledSoftware(mapSoftwareList &mapInstalledSoftwares);
    void UpdateUninstallList();
    void UninstallSoftwares(QString &strSoftList);

public slots:
    // 用户
    void reqLoginUser(QString username,QString password);
    void reqRegisteUser(QString username,QString password,QString email,QString strMobile);
    void reqModifyUserInfo(QVariantMap userinfo);
    void reqQueryUserStatus();
    void reqClearUserStatus();
    void reqUserPasswdUpdate(QString szOld,QString szNew);
    void reqBackupSysSoftListInfo();
    void reqQueryBackupInfo();
    void reqRestoreSysInfo(QString filename);

    // 卸载
    void reqRefreshSoftList();

    // 下载管理
    void reqQueryAllTaskInfo();
    void reqAddTask(QVariantMap task);
    void reqAddTasks(QVariantList tasks);
    void reqPauseTask(QString szPackageId);
    void reqPauseAllTask();
    void reqResumeTask(QString szPackageId);
    void reqResumeAllTask();
    void reqRemoveTask(QString szPackageId);
    void reqRemoveAllTask();

    // 升级
    void reqUpgradeData();

    // 设置中心
    void getDownLoadFolder();
    void UnstalledSoftInfo();

    // 安装
    void InstalledSoftwareChanged();

signals:
    // 异常
    void sigCrash();

    // 用户
    void updateLoginUser(QVariantMap userinfo);
    void updateRegisteUser(QVariantMap userinfo);
    void updateModifyUserInfo(QVariantMap userinfo);
    void sigLoginUser(QString username,QString password);
    void sigRegisteUser(QString username,QString password,QString email,QString strMobile);
    void sigModifyUserInfo(QVariantMap userinfo);
    void sigQueryUserState();
    void sigClearUserState();
    void updateComfireUserInfo(bool);
    void sigUserPasswdUpdate(QString szOld, QString szNew);
    void sigBackupSysSoftListInfo();
    void sigQueryBackupInfo();
    void sigRestoreSysInfo(QString filename);
    void sigupdateBackupSysSoftListInfo(bool, QVariantMap);
    void sigReplyRestoreSysInfo(bool);

    void sigLoginUserDef();

    // 软件大全
    void sigSendHotList(QVariantList hotlist);
    void sigSendTopList(QVariantList toplist);
    void sigSendAllSoftPackages(QMap<QString,QVariantList> SoftPackages);
    void sigDownLoadFilePath();

    // 下载管理
    void sigQueryAllTaskInfo();
    void sigAddTask(QVariantMap task);
    void sigAddTasks(QVariantList tasks);
    void sigPauseTask(QString szPackageId);
    void sigPauseAllTask();
    void sigResumeTask(QString szPackageId);
    void sigResumeAllTask();
    void sigRemoveTask(QString szPackageId);
    void sigRemoveAllTask();
    void sigUpdateOneTaskInfo(QVariantMap);

    // 升级
    void sigRequestUpgradeData();
    void sigUpdateOneUpgradeInfo(QVariantMap);
    void sigUpdateUpgradeCount(int);
    void sigReloadUpgradeData();

    //卸载
    void sigUpdateUninSoftware(QVariantList);//更新软件列表
    void sigCleanupResidue(QString,QString);//清理残留文件
    void sigUninstallFinished(QString);     //卸载完毕

    // 设置中心
    void updateDownLoadFilePath(QString,quint64);

protected:
    QVariantMap  _setting;              // 程序设置
    UserInfo     _user;                 // 用户信息
    UserInfoManager* _UserInRunner;     // 用户管理
    PackageRunner _DownloadTasks;       // 下载数据和下载操作
    TaskManager* _TaskRunner;           // 下载管理
    UpgradeData *m_pUpgradeData;        // 升级数据
    QThread *m_pUpgradeDataThread;      // 升级线程
    UpgradeHandler *m_pUpgradeHandler;  // 升级操作

    UninstallSoftware *m_pUninstallSoftware;//软件卸载。
    QThread *m_pUninstallSoftwareThread;//卸载线程。

    ThreadToPullSoftwareList * m_pThreadToPullSoftwareList; //拉取软件列表的线程。

    // 软件大全
    QVariantList m_lstCategory;
    QString m_szCategoryID;
    QVariantList m_lstPackage;

    // 卸载
    mapSoftwareList _mapInstalledSoftwares;
    QVariantMapMap _mapUnstallInfoSoft;
    QVector<QString>  _VectorInfoSoft;

    // 程序
    QString szAppdataPath;
};

#endif // DATACONTROL_H
