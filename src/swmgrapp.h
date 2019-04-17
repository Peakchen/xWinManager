#ifndef SWMGRAPP_H
#define SWMGRAPP_H
#include <Shlwapi.h>
#pragma comment(lib, "shlwapi.lib")

#include <QObject>
#include <QAction>
#include <QSystemTrayIcon>
#include <QProcessEnvironment>
#include <QDesktopServices>
#include <QDesktopWidget>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>

#include <QWebView>
#include <QWebPage>
#include <QWebFrame>
#include <QFileDialog>
#include <QMutex>

#include "global.h"

#include "ConfOperation.h"
#include "DataControl.h"

#include "MainWnd.h"
#include "safetyreinforce.h"
#include "systemrightmenu.h"
#include "browsertools.h"
#include "selfupdate.h"
#include "mythread.h"
#include "GetPngThread.h"

#pragma execution_character_set("utf-8")


class SwmgrApp : public QObject
{
    Q_OBJECT
public:
    explicit SwmgrApp(QObject *parent = 0);
    ~SwmgrApp();
public:
    static SwmgrApp *Instance();
    static const QString &GetEnvVar();
    static const QString &GetCompanyName();
    static const QString &GetSoftwareName();

    static QString GetAppDataPath(QString szCompany);
    static QString GetProgramProfilePath(QString name);
    static QString GetFilePathFromFile(QString szFile);
    static QString GetCookieFile();
    static QString GetUserLoginUrl();
    static QString GetUserRegisteUrl();
    static QString GetDownloadPath();
public:
    Q_PROPERTY(QString userToken READ getUserToken)
    QString getUserToken();
public:
    QString getSettingParameter(QString name, QString defaultValue);
    qint32 StringLikeFind(QString, QString&);

    void safetyCurrentStatus(QString&);
    void RightMenuCurrentStatus(QString&);
    void SetIECurrentStatus(QString&);
public:
    // ------------------------
    QString n_pstatus;
    BOOL InitAppEnv();
    void InitDir(QString szAppDir);
    BOOL InitCurl();
protected:
    void InitObjects();
    void InitIcons();
    void InitMenuActions();
    void InitSlots();
    void InitDataModel();
    void InitTray();
    void InitWnd();
    void DumpEnv();

    // 获取当前 需要更新的 软件列表
    bool MD5asSoftisExist(QString );
    void InitMutex_SoftDate();
    void get_str_for_used_date(QString&, QString&);

    quint64 getDiskFreeSpace(QString driver);
    QString readPromptInfo();

protected:

    systemRightMenu systemR;
    safetyReinforce ssafeR;

    QWebView *m_pMainView;              // 主窗口
    QWebView *m_pCloseView;             // 退出窗口
    QWebView *m_pFeedbackView;          // 反馈建议
    QWebView *m_pSettingCenterView;     // 设置中心
    QWebView *m_pUpgradeView;           // 检测更新
    QWebView *m_pAboutUsView;           // 关于我们
    QWebView *m_pOptimizeView;          // 优化设置向导
    QWebView *m_pAutoUpgrade;           // 自动升级
    QSystemTrayIcon *traySystem;
    QMenu *trayIconMenu;
    QAction *quitAction;
    QAction *fullAction;
    QAction *miniAction;
    QIcon   *appTrayIcon;

    DataControl *_DataModel;
    BOOL m_bCurlStatus;

    QStringList MultiSoftPath;
    QStringList UnstallMd5;
    qint32 Md5count;
    QTimer *processIdtime;
    quint64 pid;
    HANDLE mutex_soft_file;

    MyThread *mythread;
    QString strPackagePath;


    QPropertyAnimation *sizemoveAnimation;
    int head_menu_x;
    int head_menu_y;
    int head_menu_width;
    int head_menu_height;
protected slots:
    void initWebViewHost();
    void docLoadFinish(bool);
public slots:
    // system control ex:show close hide
    void appquit();
    void appsettingsquit();
    void showFullWnd();
    void showMiniWnd();
    void trayActivated(QSystemTrayIcon::ActivationReason);

    //msic js bind signal
    void execOpenSystemBrowser(QString urlAddress);//use system default browser open a http://website

    void execOpenLocalFolder(QString localAddress);//open local disk folder on explorer
    void execOpenLocalDownloadFolder();//open local disk download folder on explorer
    void execOpenSetDownloadFolder(QString path);
    void ForcedExit();

    void reqDefaultDownLoadPath();
    void exePromptPriceInfoPlugin(QString checkStr);

// For UI interface begin
signals:
    void updateSoftCategory(QVariantList lstCategory);
    void updateExtraCategoryList(QString szCategoryID, QVariantList swCategory, int pageNumber, int pageTotal, int countTotal);
    void updateTopPackage(QVariantList swPackageList);
    void updateOnePackageInfo(QVariantMap);

    //about user
    void updateRegisteUser(QVariantMap userinfo);
    void updateLoginUser(QVariantMap userinfo);
    void updateModifyUserInfo(QVariantMap userinfo);
    void updateComfireUserInfo(bool);
    void updateBackupSysSoftListInfo(bool, QVariantMap);
    void updateReplyRestoreSysInfo(bool);

    // load page
    void updateLoadPage(QString, QVariantList);

    // 软件大全
    void updateInstalledCount(int);

    //soft package operation
    void updateOneTaskInfo(QVariantMap downloadInfo);
    void updateOneUpgradeInfo(QVariantMap downloadInfo);
    void updateUpgradeCount(int upgradeCount);

    //卸载
    void UpdateUninSoftware(QVariantList softwareList);
    void updateCanUninstallPackages(QVariantList swCategory);

    void updateDownLoadFilePath();  //QString, quint64
    void updatePromptPriceInfo(QString);
    void setclosestatus();
    void BrowserPageSignals(QVariantList BrowserList, QString StartPage);
    void SearchEngineSignals(QVariantList SearchList, int isNewSearch, QString SUrl);

    // advise
    void updateAdvise(int);
    void statusEssential(QString);
    void firstEssential(int);

    void sigDeleteGetPngThread(GetPngThread *);

public slots:
    //void UpdateUninSoftware(QVariantList asd);
    //about software
    void requestSoftCategoryList();
    void requestExtraCategoryList(QString szCategoryID, int pageNumber=0, int count=0);
    void requestTopPackage();

    //about user
    void requestRegisteUser(QString username, QString password, QString email,QString strMobile);
    void requestLoginUser(QString username, QString password);
    void requestModifyUserInfo(QVariantMap userinfo);
    void requestComfireUpdateUserInfo(QString szOld, QString szNew);
    void requestBackupSysSoftListInfo();
    void requestRestoreSysInfo(QString);

    //soft package operation
    void requestInstallTopPackage(QString szCategoryID, QString szPackageID);
    void requestStartInstallPackage(QString szCategoryID, QString szPackageID); // software package download and install
    void requestBatStartInstallPackage(QVariantList lstPackage){}  // bat install
    void requestPausePackage(QString szCategoryID, QString szPackageID); //pause someone package
    void requestResumePackage(QString szCategoryID, QString szPackageID); //resume someone package
    void requestAllResumePackage(); //resume all package
    void requestStopDownloadPackage(QString szCategoryID, QString szPackageID); // remove processing task for download/installtion

    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++^
    void requestAllDownloadingTaskPause(); //Pause all downloading task
    void requestAllDownloadingTaskCancel(); //Cancel all downloading task
    //++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++$

    void requestCanUninstallPackages(); //can be uninstall pacakge list
    void requestDoUninstall(QString uninstallID);

    void UninstalPackages();
    void UninstallerFinished(int exitCode, QProcess::ExitStatus exitStatus);
    BOOL CheckProcessId(DWORD pId);
    void Uninstall_queue();
    void getUnstallData(QStringMap&);

    void requestLoadPage(QString pageName);
    void requestOnPageChange(QString pageName); //Webkit (dynamic dom page) change to {pageName}

    void requestSystemRegSet(int);

    // 设置head menu坐标
    void setHeadMenuPos(int x, int y, int width, int height);

    // For UI interface end
    // 退出窗口
    void openCloseWnd();
    void loadCloseFinished(bool);

    // 反馈建议
    void openFeedbackWnd();
    void closeFeedbackWnd();

    // 设置中心
    void openSettingCenterWnd();
    void loadSettingCenterFinished(bool);
    void closeSettingCenterWnd();

    // 检测更新
    void openUpgradeWnd();
    void loadUpgradeFinished(bool);
    void closeUpgradeWnd();

    // 关于我们
    void openAboutUsWnd();
    void loadAboutUsFinished(bool);
    void closeAboutUsWnd();

    // 优化设置向导
    void openOptimizationWnd();
    void loadOptimizationFinished(bool);
    void closeOptimizationWnd();
    void closeOptimizationView();

    void Upan(int status);
    void fileName(int status);
    void DiskShare(int status);
    void NullLink(int status);
    void AccountControl(int status);
    void requestAddIE();
    void requestDelIE();
    void requestAddSystemRightMenu();
    void requestaDeleteSystemRightMenu();
    void requestaAddMyComputer();
    void requestaDeleteMyComputer();
    void requestAddRunwindows();
    void requestDeleteRunwindows();
    void requestSetHomePage(int status);
    void requestSetSearchEngine(int status);
    void requestAddNeverCombine(int status);
    void requestFinished();
    void requestSearchMenuExt(QString MenuExt);

    void SelectDownLoadFolder();
    void reqDownLoadFolder(QString, quint64);

    void ShowUpdateMSG(QString packagePath);
    void requestCloseAutoUpgrade();
    void requestAddQuickSettings();
    void requestDelQuickSettings();
    void exeGetSuggustionInfo(QString index, QString title, QString content);
    void requestgb();
    void requestChangeStasu(int radio1, int checkbox1);
    void requestBrowserPage();
    void requsetChangeBrowserPage(QString);
    void requsetSearchEngine();
    void requsetChangeSearchEngine(QString, QString);

    void CleanupResidue(QString strResidueFilePath,QString strDisplayName);          //软件卸载----清理残留文件
    void requestEssential();
    void requestn_nocheck();

    void DeleteGetPngThread(GetPngThread *pThread);
protected:
    bool FilterPackage(const QVariantMap &package, mapSoftwareList &mapInstalledSoftwares);
};

#endif // SWMGRAPP_H
