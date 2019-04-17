#include "swmgrapp.h"
#include <QApplication>
#include "curl/curl.h"
#include "global.h"
#include "selfupdate.h"

SwmgrApp::SwmgrApp(QObject *parent) : QObject(parent)
{
    m_bCurlStatus = FALSE;

    traySystem = NULL;
    trayIconMenu = NULL;
    quitAction = NULL;
    fullAction = NULL;
    miniAction = NULL;
    appTrayIcon = NULL;

    m_pMainView = NULL;
    m_pCloseView = NULL;
    m_pFeedbackView = NULL;
    m_pSettingCenterView = NULL;
    m_pUpgradeView = NULL;
    m_pAboutUsView = NULL;
    m_pOptimizeView = NULL;
    m_pAutoUpgrade = NULL;

    _DataModel = NULL;
    processIdtime = NULL;
    mythread = NULL;

    sizemoveAnimation = NULL;
}

SwmgrApp::~SwmgrApp() {
    if (m_bCurlStatus) {
        ::curl_global_cleanup();
    }
}

SwmgrApp *SwmgrApp::Instance() {
    static SwmgrApp *_Instance = new SwmgrApp(qApp);
    return _Instance;
}

BOOL SwmgrApp::InitAppEnv() {
    InitDir(SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()));
    if (!InitCurl()) {
        return FALSE;
    }

    // ----------
    InitObjects();
    InitIcons();
    InitMenuActions();

    InitSlots();

    InitDataModel();

    InitMutex_SoftDate();

    InitTray();
    InitWnd();

    //启动自我更新检测。
    mythread = NULL;
    //StartSelfUpdate();
    if(mythread == NULL)
    {
        mythread = new MyThread;
        connect(mythread,SIGNAL(SendUpdateMsgToGUI(QString)),this,SLOT(ShowUpdateMSG(QString)),Qt::QueuedConnection);
        mythread->start();
    }

    return TRUE;
}
