#include "swmgrapp.h"
#include <QApplication>
#include <QMenu>
#include <QDir>
#include <direct.h>
#include <QMessageBox>
#include <QDomDocument>
#include "quicksettings.h"
#include "Storage.h"
#include "mythread.h"
#include "appenv.h"
#include "character.h"

void SwmgrApp::InitObjects() {
    appTrayIcon = new QIcon();

    trayIconMenu = new QMenu(NULL);
    fullAction = new QAction(QString("显示窗口"), this);
    miniAction = new QAction(QString("隐藏窗口"), this);
    quitAction = new QAction(QString("退出"), this);
    traySystem = new QSystemTrayIcon(this);

    _DataModel = new DataControl(this);

    QWebSettings::globalSettings()->setAttribute(QWebSettings::DeveloperExtrasEnabled, true);
    QWebSettings::globalSettings()->setMaximumPagesInCache(0);
    QWebSettings::globalSettings()->setObjectCacheCapacities(0, 0, 0);
    QWebSettings::globalSettings()->setOfflineStorageDefaultQuota(0);
    QWebSettings::globalSettings()->setOfflineWebApplicationCacheQuota(0);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessRemoteUrls, true);
    QWebSettings::globalSettings()->setAttribute(QWebSettings::LocalContentCanAccessFileUrls, true);
    QString strCacheDir= GetProgramProfilePath(QString("xbsoftMgr")) + "\\BrowserCache";
    QDir d(strCacheDir);
    if(!d.exists()){
        d.mkpath(strCacheDir);
    }
    QWebSettings::globalSettings()->enablePersistentStorage(strCacheDir);

    m_pMainView = new MainWnd();
}

void SwmgrApp::InitIcons() {
    QString szFilePath = GLOBAL::_DY_DIR_RUNNERSELF + "xbmgr.ico";
    if(!QFile::exists(szFilePath))
    {
        QFile::copy(":/xbmgr.ico", szFilePath);
    }

    appTrayIcon->addFile(":/xbmgr.ico");
}

void SwmgrApp::InitMenuActions() {
    trayIconMenu->addAction(fullAction);
    trayIconMenu->addAction(miniAction);
    trayIconMenu->addSeparator();
    trayIconMenu->addAction(quitAction);
}

void SwmgrApp::InitSlots() {
    // 托盘菜单
    QObject::connect(quitAction, SIGNAL(triggered(bool)), this, SLOT(appquit()));
    QObject::connect(fullAction, SIGNAL(triggered(bool)), this, SLOT(showFullWnd()));
    QObject::connect(miniAction, SIGNAL(triggered(bool)), this, SLOT(showMiniWnd()));
    QObject::connect(traySystem, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),this,SLOT(trayActivated(QSystemTrayIcon::ActivationReason)));

    // 异常
    QObject::connect(_DataModel,SIGNAL(sigCrash()),this,SLOT(appquit()));

    // 用户
    QObject::connect(_DataModel,SIGNAL(updateLoginUser(QVariantMap)),this,SIGNAL(updateLoginUser(QVariantMap)));
    QObject::connect(_DataModel,SIGNAL(updateRegisteUser(QVariantMap)),this,SIGNAL(updateRegisteUser(QVariantMap)));
    QObject::connect(_DataModel,SIGNAL(updateModifyUserInfo(QVariantMap)),this,SIGNAL(updateModifyUserInfo(QVariantMap)));
    QObject::connect(_DataModel,SIGNAL(updateComfireUserInfo(bool)),this,SIGNAL(updateComfireUserInfo(bool)));
    QObject::connect(_DataModel,SIGNAL(sigupdateBackupSysSoftListInfo(bool, QVariantMap)), this, SIGNAL(updateBackupSysSoftListInfo(bool, QVariantMap)));
    QObject::connect(_DataModel, SIGNAL(sigReplyRestoreSysInfo(bool)), this, SIGNAL(updateReplyRestoreSysInfo(bool)));

    // 软件大全
    QObject::connect(_DataModel, SIGNAL(sigUpdateOneTaskInfo(QVariantMap)), this, SIGNAL(updateOnePackageInfo(QVariantMap)));
    QObject::connect(_DataModel, SIGNAL(sigUpdateOneUpgradeInfo(QVariantMap)), this, SIGNAL(updateOnePackageInfo(QVariantMap)));

    // 升级
    QObject::connect(_DataModel, SIGNAL(sigUpdateOneUpgradeInfo(QVariantMap)), this, SIGNAL(updateOneUpgradeInfo(QVariantMap)));
    QObject::connect(_DataModel, SIGNAL(sigUpdateUpgradeCount(int)), this, SIGNAL(updateUpgradeCount(int)));

    //卸载
    //qRegisterMetaType<mapSoftwareList>("mapSoftwareList");     //向注册自定义数据类型
    QObject::connect(_DataModel, SIGNAL(sigUpdateUninSoftware(QVariantList)), this, SIGNAL(UpdateUninSoftware(QVariantList)));//更新显示软件列表
    QObject::connect(_DataModel, SIGNAL(sigCleanupResidue(QString,QString)), this, SLOT(CleanupResidue(QString,QString)));//其实用户清理残留文件。

    // 下载管理
    QObject::connect(_DataModel, SIGNAL(sigUpdateOneTaskInfo(QVariantMap)), this, SIGNAL(updateOneTaskInfo(QVariantMap)));
}

//测试用
//void SwmgrApp::UpdateUninSoftware(QVariantList asd)
//{
//    QList<QVariant>::iterator map = asd.begin();
//    QVariantMap asdd = map->toMap();

//    qDebug()<<asdd["UsedDate"]<<"w d ce shi+++++++++++++++++++++++++++++++++"<<endl;
//    qDebug()<<"<asd.begin()->first"<<asd.size()<<endl;
//}

void SwmgrApp::InitDataModel() {
    _DataModel->initAll();
}

void SwmgrApp::InitTray() {
    traySystem->setIcon(*appTrayIcon);
    traySystem->setContextMenu(trayIconMenu);
    traySystem->show();
}

void SwmgrApp::InitWnd() {
    m_pMainView->setWindowTitle("乐网管家");
    m_pMainView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
    m_pMainView->setFixedSize(963, 627);

    QFile htmlFile(":/index.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pMainView->setHtml(htmlFile.readAll(), QUrl("qrc:///index.html"));
    htmlFile.close();

    QObject::connect(m_pMainView->page()->mainFrame(), SIGNAL(javaScriptWindowObjectCleared()), this, SLOT(initWebViewHost()));
    QObject::connect(m_pMainView, SIGNAL(loadFinished(bool)), this, SLOT(docLoadFinish(bool)));
    QObject::connect(&_DataModel->getPackageRunner(), SIGNAL(sigPromptPriceInfo(QString )), this, SIGNAL(updatePromptPriceInfo(QString)));
    m_pMainView->show();
}

void SwmgrApp::appquit() {
    //----------------------------------------add by liyong 2015.10.24
     WaitForSingleObject(mutex, INFINITE);
    char buf[1024] = {'\0'};
    std::string szAppdataPath;
    GetEnvironmentVariableA("CommonProgramFiles",buf,1024);
    szAppdataPath.append(buf);
    szAppdataPath.append("\\HurricaneTeam");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr.lock");
    FILE *fp;
    fopen_s(&fp, szAppdataPath.data(), "w");
    if (fp)
    {
        fprintf(fp,"2");//
        fclose(fp);
    };
    ReleaseMutex(mutex);
    //----------------------------------------add by liyong 2015.10.24
    if(appTrayIcon)
    {
        delete appTrayIcon;
        appTrayIcon = NULL;
    }
    if(trayIconMenu)
    {
        trayIconMenu->deleteLater();
    }
    if(fullAction)
    {
        fullAction->deleteLater();
    }
    if(miniAction)
    {
        miniAction->deleteLater();
    }
    if(quitAction)
    {
        quitAction->deleteLater();
    }
    if(traySystem)
    {
        traySystem->deleteLater();
    }
    if(sizemoveAnimation)
    {
        sizemoveAnimation->deleteLater();
    }
    if(m_pAutoUpgrade)
    {
        m_pAutoUpgrade->hide();
        m_pAutoUpgrade->close();
        m_pAutoUpgrade->deleteLater();
    }
    if(m_pCloseView)
    {
        m_pCloseView->hide();
        m_pCloseView->close();
        m_pCloseView->deleteLater();
    }
    if(m_pFeedbackView)
    {
        m_pFeedbackView->hide();
        m_pFeedbackView->close();
        m_pFeedbackView->deleteLater();
    }
    if(m_pSettingCenterView)
    {
        m_pSettingCenterView->hide();
        m_pSettingCenterView->close();
        m_pSettingCenterView->deleteLater();
    }
    if(m_pUpgradeView)
    {
        m_pUpgradeView->hide();
        m_pUpgradeView->close();
        m_pUpgradeView->deleteLater();
    }
    if(m_pAboutUsView)
    {
        m_pAboutUsView->hide();
        m_pAboutUsView->close();
        m_pAboutUsView->deleteLater();
    }
    if(m_pOptimizeView)
    {
        m_pOptimizeView->hide();
        m_pOptimizeView->close();
        m_pOptimizeView->deleteLater();
    }
    if(m_pMainView)
    {
        m_pMainView->hide();
        m_pMainView->close();
        m_pMainView->deleteLater();
    }

    QWebSettings::globalSettings()->clearMemoryCaches();
    if(_DataModel)
    {
        _DataModel->unInit();
        _DataModel->deleteLater();
    }
    if(mythread)
    {
        if(mythread->isRunning())
        {
            mythread->isExit = true;
            mythread->exit();
            mythread->wait();
        }
        mythread->deleteLater();
    }
    if(processIdtime)
    {
        if(processIdtime->isActive())
        {
            processIdtime->stop();
        }
        processIdtime->deleteLater();
    }

    qApp->quit();
}

int radioNow = 1;
int checkboxNow = 4;
void SwmgrApp::requestChangeStasu(int radio1,int checkbox1)
{
    radioNow = radio1;
    checkboxNow = checkbox1;

    QString strRadioNow = QString::number(radioNow, 10);
    QString strCheckboxNow = QString::number(checkboxNow, 10);

    char configPath[500] = {NULL};
    HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
    GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
    *(strrchr( configPath, '\\') ) = 0;
    strcat_s(configPath, "\\Close_config.ini");

    std::string strConfigPath =GetLocalAppDataPath();
    strConfigPath += "\\Close_config.ini";
    WritePrivateProfileStringA("CLOSE","checkboxNow",strCheckboxNow.toStdString ().c_str (),strConfigPath.data());
    WritePrivateProfileStringA("CLOSE","radioNow",strRadioNow.toStdString ().c_str (),strConfigPath.data());

    //-------------------------------------
    if(checkboxNow == 3)
    {
        if(radioNow == 1){
            requestgb();
            showMiniWnd();
        }else{
            requestgb();
            appquit();
        }
    }else{
        if(radioNow == 1){
            requestgb();
            showMiniWnd();
        }else{
            requestgb();
            appquit();
        }
    }
}

void SwmgrApp::requestgb()
{
    m_pCloseView->hide();
    m_pCloseView->close();
    emit setclosestatus();
}


void SwmgrApp::appsettingsquit()
{
     m_pSettingCenterView->close();
}

void SwmgrApp::showFullWnd() {
    if (m_pMainView->isVisible())
        return;
    m_pMainView->show();
    m_pMainView->activateWindow();
}

void SwmgrApp::showMiniWnd() {
    if (!m_pMainView->isVisible())
        return;
    m_pMainView->hide();
}

void SwmgrApp::trayActivated(QSystemTrayIcon::ActivationReason reason) {
    if (reason==QSystemTrayIcon::DoubleClick) {
        showFullWnd();
    }
}

void SwmgrApp::initWebViewHost() {
    if(m_pMainView)
    {
        m_pMainView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC",this);
    }
}


/**
 * Use system default browser open url address
 * @brief SwmgrApp::openSystemBrowser
 * @param urlAddress
 */
void SwmgrApp::execOpenSystemBrowser(QString urlAddress){
    QDesktopServices::openUrl(QUrl::fromUserInput(urlAddress));
}

/**
 * Use explorer open local disk folder
 * @brief SwmgrApp::execOpenLocalFolder
 * @param localAddress
 */
void SwmgrApp::execOpenLocalFolder(QString localAddress){
    QDesktopServices::openUrl(QUrl::fromUserInput(localAddress));
}


/**
 * Use explorer open local download folder
 * @brief SwmgrApp::execOpenLocalDownloadFolder
 */
void SwmgrApp::execOpenLocalDownloadFolder(){
    QDir dir;
    QString defaultConf= SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "downloadPath.conf");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"read failded.";
    }
    QTextStream in(&file);
    QString path;
    if(!in.atEnd()){
        path = in.readLine();
    }
    if(path.isEmpty()){
        QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
        defaultRepository = QDir::toNativeSeparators(defaultRepository);
        defaultRepository = getSettingParameter(QString("Repository"), defaultRepository);
        dir.mkpath(defaultRepository);
        QTextStream out(&file);
        path = defaultRepository;
        out<<path;
    }

    QDesktopServices::openUrl(QUrl::fromUserInput(path));
}

void SwmgrApp::execOpenSetDownloadFolder(QString path)
{
    if(path.isEmpty()){
        execOpenLocalDownloadFolder();
    }else
    {
        QDesktopServices::openUrl(QUrl::fromUserInput(path));
    }
}

// ---------------------------------------
/****************
 *@brief : open settings, show lewangCenter
 *@param : Urladdress(html url), width( W ) , height ( H )
 *@author: cqf
 ****************/

void SwmgrApp::requestAddQuickSettings()
{
    QuickSettings Settings;
    Settings.addXbmgrQuickSettings();
}

void SwmgrApp::requestDelQuickSettings()
{
    QuickSettings Settings;
    Settings.delXbmgrQuickSettings();
}

//----------------------------------------add by liyong 2015.10.29
void SwmgrApp::ForcedExit()
{

     WaitForSingleObject(mutex, INFINITE);
    char buf[1024] = {'\0'};
    std::string szAppdataPath;
    GetEnvironmentVariableA("CommonProgramFiles",buf,1024);
    szAppdataPath.append(buf);
    szAppdataPath.append("\\HurricaneTeam");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr.lock");
    FILE *fp;
    fopen_s(&fp, szAppdataPath.data(), "w");
    if (fp)
    {
        fprintf(fp,"2");//
        fclose(fp);
    };
    ReleaseMutex(mutex);

    qApp->quit();
}

void SwmgrApp::reqDefaultDownLoadPath()
{
    QString defaultConf= SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "downloadPath.conf");
    if(!file.open(QIODevice::ReadWrite)){
        qDebug()<<"read failded.";
    }
    QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
    QString Repository = defaultRepository.replace(QString("\\"), QString("/"));
    QString path = Repository;
    QTextStream out(&file);
    out<<path;
    file.close();
    QString diskName = path.left(3);
    quint64 freeSpace = getDiskFreeSpace(diskName);
    QWebFrame *Settingframe = m_pSettingCenterView->page()->mainFrame();
    QString strVal = QString("ReadDefaultDownLoadPath(\"%1\",\"%2\");").arg(path).arg(freeSpace);
    Settingframe->evaluateJavaScript(strVal);
}


void SwmgrApp::safetyCurrentStatus(QString &safeStatus)
{
    //---------------------------------------Upan STATUS
    QSettings *Upan = NULL;
    Upan = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",QSettings::NativeFormat);
    if(Upan == NULL)
    {
        ;
    }
    int regedit = Upan->value("NoDriveTypeAutoRun",QVariant()).toInt();
    if(regedit == 0){     //开启
        safeStatus = "T,";
    }else{   //关闭
        safeStatus = "F,";
    }
    delete Upan;
    //---------------------------------------filename
    QSettings *fileReg = NULL;
    fileReg = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",QSettings::NativeFormat);
    if(fileReg == NULL)
    {
        ;
    }
    int HideFileExt = fileReg->value("HideFileExt",QVariant()).toInt();
    if(HideFileExt == 0)   //隐藏
    {
        safeStatus += "F,";
    }else {     //不隐藏
        safeStatus += "T,";
    }
    delete fileReg;
    //---------------------------------------DiskShare
    QSettings *DiskShare = NULL;
    DiskShare = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\parameters",QSettings::NativeFormat);
    int AutoShareServer = DiskShare->value("AutoShareServer",QVariant()).toInt();
    if(DiskShare == NULL)
    {
        ;
    }
    if(AutoShareServer == 0)   //关闭
    {
        safeStatus += "T,";
    }else {     //打开
        safeStatus += "F,";
    }
    delete DiskShare;
    //---------------------------------------NullLink
    QSettings *NullLink = NULL;
    NullLink = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\NetBT\\Parameters",QSettings::NativeFormat);
    if(NullLink == NULL)
    {
        ;
    }
    int SMBDeviceEnabled = NullLink->value("SMBDeviceEnabled",QVariant()).toInt();
    if(SMBDeviceEnabled == 0)   //关闭
    {
        safeStatus += "T,";
    }else {     //打开
        safeStatus += "F,";
    }
    delete NullLink;
    //---------------------------------------
    QSettings *AccountControl = NULL;
    AccountControl = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",QSettings::NativeFormat);
    if(AccountControl == NULL)
    {
        ;
    }
    int EnableLUA = AccountControl->value("EnableLUA",QVariant()).toInt();
    if(EnableLUA == 0)   //关闭
    {
        safeStatus += "F";
    }else {     //打开
        safeStatus += "T";
    }
    delete AccountControl;
    qDebug() << "safeStatus = " << safeStatus << endl;
    return;
}

void SwmgrApp::RightMenuCurrentStatus(QString &RightMenuStatus)
{
    //---------------------------------------NeverCombine
    QSettings *NeverCombine = NULL;
    NeverCombine = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",QSettings::NativeFormat);
    if(NeverCombine == NULL)
    {
        ;
    }
    int TaskbarGlomLevel = NeverCombine->value("TaskbarGlomLevel",QVariant()).toInt();
    if(TaskbarGlomLevel == 0)   //合并
    {
        RightMenuStatus = "F,";
    }else {     //不合并
        RightMenuStatus = "T,";
    }
    delete NeverCombine;
    //---------------------------------------addIE
    MyRegedit delItem("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}");
    if(!delItem.InitReg())
    {
        RightMenuStatus += "F,";
    }else{
        RightMenuStatus += "T,";
    }

//    //---------------------------------------addSystemRightMenu
    MyRegedit RightMenu("HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\乐网管家");
    if(!RightMenu.InitReg())
    {
        RightMenuStatus += "F,";
    }else{
     RightMenuStatus += "T,";
    }
    //---------------------------------------MyComputer
    QSettings *MyComputer = NULL;
    MyComputer = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel",QSettings::NativeFormat);
    if(MyComputer == NULL)
    {
        ;
    }
    int A2D8 = MyComputer->value("{20D04FE0-3AEA-1069-A2D8-08002B30309D}",QVariant()).toInt();
    if(A2D8 == 0)   //打开
    {
        RightMenuStatus += "T,";
    }else {     //关闭
        RightMenuStatus += "F,";
    }
    delete MyComputer;
    //---------------------------------------Runwindows
    QSettings *Runwindows = NULL;
    Runwindows = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",QSettings::NativeFormat);
    if(Runwindows == NULL)
    {
        ;
    }
    int NoRun = Runwindows->value("NoRun",QVariant()).toInt();
    if(NoRun == 0)   //显示
    {
        RightMenuStatus += "T";
    }else {     //隐藏
        RightMenuStatus += "F";
    }
    delete Runwindows;
    qDebug() << "RightMenuStatus = " << RightMenuStatus << endl;
    return;
}

void SwmgrApp::SetIECurrentStatus(QString &SetIEStatus)
{
    QSettings *SetHome = NULL;
    SetHome = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MAIN",QSettings::NativeFormat);
    if(SetHome == NULL)
    {
        ;
    }
    std::string StartPage = SetHome->value("Start Page",QVariant()).toString().toStdString();
    delete SetHome;

    QSettings *SetSearch = NULL;
    SetSearch = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Internet Explorer\\SearchScopes\\SearchProvider_baidu",QSettings::NativeFormat);
    if(SetSearch == NULL)
    {
        ;
    }
    std::string StartUrl = SetSearch->value("Url",QVariant()).toString().toStdString();
    if(StartUrl == "http://www.baidu.com/s?wd={searchTerms}")   //baidu
    {
        SetIEStatus += "4";
    }else if(StartUrl == "http://www.google.cn/search?hl=zh-CN&q={searchTerms}"){     //google
        SetIEStatus += "5";
    }else {     //原有
        SetIEStatus += "6";
    }
    delete SetSearch;

    qDebug() << "SetIEStatus = " << SetIEStatus << endl;
    return;
}

int getlen(const char *result){
    int i=0;
    while(result[i]!='\0'){
        i++;
    }
    std::cout<<i<<endl;
    return i;
}

QString getSpecialsymbolStr(QString website){
    std::string webStr = website.toStdString();
    const char * cweb = webStr.c_str();
    int len = getlen(cweb);
    char bakWeb[100];
    memset(bakWeb,0,sizeof(bakWeb));
    int count = 0;
    bool flag = false;
    bool hasPoint = false;
    int slashIndex = website.indexOf("//");
    if(website.contains("https") || website.contains("http")){
        ;
    }else{
        return website;
    }
    int curIndex = 0;
    for(int i = 0; i < len; i++){
        if((cweb[i] == '.') && (count == 0)){
            if((cweb[i + 3] == '/') || (cweb[i + 4] == '/')){
                hasPoint = false;
            }else{
                hasPoint = true;
            }
            curIndex = i;
            count ++;
            flag = true;
            continue;
        }else if((cweb[i] == '.') && (count != 0)){
            flag = false;
            break;
        }
        if((count == 1) && flag){
            if(hasPoint){
                bakWeb[i - curIndex - 1] = cweb[i];
            }else{
                return website.mid( slashIndex + 2, curIndex - slashIndex - 2);
            }
        }
    }
    QString tempStr = QString(QLatin1String(bakWeb));
    return tempStr;
}

void SwmgrApp::requestBrowserPage()
{
    QVariantList allItems;
    Storage::LoadFromConfArray(":/support/BrowserHomePage.conf",allItems);

    QSettings *SetHome = NULL;
    SetHome = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MAIN",QSettings::NativeFormat);
    if(SetHome == NULL)
    {
        return ;
    }
    QString StartPage = SetHome->value("Start Page",QVariant()).toString();

    delete SetHome;

    if(!allItems.isEmpty ())
        emit BrowserPageSignals(allItems,StartPage);
}

void SwmgrApp::requsetSearchEngine()
{
    QVariantList allItems;
    Storage::LoadFromConfArray(":/support/SearchEngine.conf",allItems);
    QSettings *SetSearchEngine = NULL;
    SetSearchEngine = new QSettings("HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Internet Explorer\\SearchScopes\\SearchProvider_baidu", QSettings::NativeFormat);
    if(SetSearchEngine == NULL)
    {
        return ;
    }
    QString SDisplayName = SetSearchEngine->value("DisplayName",QVariant()).toString ();
    QString SUrl = SetSearchEngine->value("Url",QVariant()).toString ();
    QVariantList SearchList;
    QVariantMap curSearchMap;
    int isNewSearchEngine = 0;
    curSearchMap.insert("DisplayName", QVariant::fromValue(SDisplayName));
    curSearchMap.insert("Url", QVariant::fromValue(SUrl));
    SearchList.append(curSearchMap);
    delete SetSearchEngine;
    int pageCount = 0;
    for(QVariantList::Iterator it=allItems.begin();it!=allItems.end();it++){
        QVariantMap temp = it->toMap();
        if(temp["nameText"].toString().contains(SDisplayName)){
            break;
        }else{
            pageCount ++;
        }
    }
    if(allItems.count() == pageCount){
        isNewSearchEngine = 2;
        allItems.append(SearchList);
    }else{
        isNewSearchEngine = 1;
    }
    if(!allItems.isEmpty ())
        emit SearchEngineSignals(allItems, isNewSearchEngine, SUrl);
}

void SwmgrApp::requsetChangeBrowserPage(QString BrowserPage)
{
    SetHomePage(BrowserPage);
}

void SwmgrApp::requsetChangeSearchEngine(QString SearchEnginestr, QString SearchEngine)
{
    qDebug()<<SearchEnginestr.toStdString ().c_str ()<<" "<< SearchEngine.toStdString ().c_str ();
    SetSearchEngine(SearchEnginestr,SearchEngine);
}

void SwmgrApp::setHeadMenuPos(int x, int y, int width, int height)
{
    head_menu_x = x;
    head_menu_y = y;
    head_menu_width = width;
    head_menu_height = height;
}

/* 退出窗口 *///点击窗口叉号的响应函数。
void SwmgrApp::openCloseWnd()
{
    char configPath[500] = {NULL};
    HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
    GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
    *(strrchr( configPath, '\\') ) = 0;
    strcat_s(configPath, "\\Close_config.ini");
    std::string strConfigPath =GetLocalAppDataPath();
    strConfigPath += "\\Close_config.ini";
    checkboxNow = GetPrivateProfileIntA("CLOSE","checkboxNow",4,strConfigPath.data());
    radioNow = GetPrivateProfileIntA("CLOSE","radioNow",2,strConfigPath.data());

    if(checkboxNow == 3)
    {
        if(radioNow == 1){
            showMiniWnd();
            return;
        }else{
            appquit();
            return;
        }
    }

    if(!m_pCloseView)
    {
        m_pCloseView = new MainWnd();
        if(!m_pCloseView)
        {
            qDebug() << "create close window failed";
            return;
        }

        QObject::connect(m_pCloseView, SIGNAL(loadFinished(bool)), this, SLOT(loadCloseFinished(bool)));

        m_pCloseView->setObjectName("close");
        m_pCloseView->setWindowTitle("管家退出");
        m_pCloseView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        m_pCloseView->setWindowModality(Qt::ApplicationModal);
    }

    QFile htmlFile(":/close.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pCloseView->setHtml(htmlFile.readAll(), QUrl("qrc:///close.html"));
    htmlFile.close();

    m_pCloseView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC", this);

    int width = 420;
    int height = 230;
    int x = m_pMainView->x() + m_pMainView->width() / 2 - width / 2;
    int y = m_pMainView->y() + m_pMainView->height() / 2 - height / 2;
    m_pCloseView->setGeometry(QRect(x, y, width, height));

    m_pCloseView->show();
}

void SwmgrApp::loadCloseFinished(bool)
{
    QString strRadioNow = QString::number(radioNow, 10);
    QString strCheckboxNow = QString::number(checkboxNow, 10);
    QWebFrame *clsoeNuw = m_pCloseView->page()->mainFrame();
    QString strClsoe = QString("setstatus(\"%1\",\"%2\");").arg(strRadioNow).arg(strCheckboxNow);
    clsoeNuw->evaluateJavaScript(strClsoe);
}
/* end */

/* 反馈建议 */
void SwmgrApp::openFeedbackWnd()
{
    if(!m_pFeedbackView)
    {
        m_pFeedbackView = new MainWnd();
        if(!m_pFeedbackView)
        {
            qDebug() << "create feedback window failed";
            return;
        }

        m_pFeedbackView->setObjectName("feedback");
        m_pFeedbackView->setWindowTitle("反馈建议");
        m_pFeedbackView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        m_pFeedbackView->setWindowModality(Qt::ApplicationModal);
    }

    QFile htmlFile(":/_dialog_feedback.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pFeedbackView->setHtml(htmlFile.readAll(), QUrl("qrc:///_dialog_feedback.html"));
    htmlFile.close();

    m_pFeedbackView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC", this);

    int width = 400;
    int height = 390;
    int x = m_pMainView->x() + m_pMainView->width() / 2 - width / 2;
    int y = m_pMainView->y() + m_pMainView->height() / 2 - height / 2;
    m_pFeedbackView->setGeometry(QRect(x, y, width, height));

    m_pFeedbackView->show();
}

void SwmgrApp::closeFeedbackWnd()
{
    if(m_pFeedbackView)
    {
        m_pFeedbackView->hide();
        m_pFeedbackView->close();
    }
}
/* end */

/* 设置中心 */
void SwmgrApp::openSettingCenterWnd()
{
    if(!m_pSettingCenterView)
    {
        m_pSettingCenterView = new MainWnd();
        if(!m_pSettingCenterView)
        {
            qDebug() << "create setting center window failed";
            return;
        }

        QObject::connect(m_pSettingCenterView, SIGNAL(loadFinished(bool)), this, SLOT(loadSettingCenterFinished(bool)));

        m_pSettingCenterView->setObjectName("setting-center");
        m_pSettingCenterView->setWindowTitle("设置中心");
        m_pSettingCenterView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        m_pSettingCenterView->setWindowModality(Qt::ApplicationModal);
    }

    QFile htmlFile(":/SettingCenter.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pSettingCenterView->setHtml(htmlFile.readAll(), QUrl("qrc:///SettingCenter.html"));
    htmlFile.close();

    m_pSettingCenterView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC", this);

    int width = 458;
    int height = 380;
    int x = m_pMainView->x() + m_pMainView->width() / 2 - width / 2;
    int y = m_pMainView->y() + m_pMainView->height() / 2 - height / 2;
    m_pSettingCenterView->setGeometry(QRect(x, y, width, height));

    m_pSettingCenterView->show();
}

void SwmgrApp::loadSettingCenterFinished(bool)
{
    QString defaultConf= SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "downloadPath.conf");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"read failded.";
    }

    QTextStream in(&file);
    QString path;
    if(!in.atEnd()){
        path = in.readLine();
    }

    if(path.isEmpty()){
        QTextStream out(&file);
        QString defaultRepository = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Repository") + QDir::separator();
        QString Repository = defaultRepository.replace(QString("\\"), QString("/"));
        path = Repository;
        out<<path;
    }
    file.close();
    QWebFrame *Settingframe = m_pSettingCenterView->page()->mainFrame();
    QString diskName = path.left(3);
    quint64 freeSpace = getDiskFreeSpace(diskName);
    QString strVal = QString("ReadDefaultDownLoadPath(\"%1\",\"%2\");").arg(path).arg(freeSpace);
    Settingframe->evaluateJavaScript(strVal);
    //QString strInit = QString("InitAll();");
    //Settingframe->evaluateJavaScript(strInit);

    QString pricecheck = readPromptInfo();
    QString strPrice = QString("PromptPriceInfoCheck(\"%1\");").arg(pricecheck);
    Settingframe->evaluateJavaScript(strPrice);

    QuickSettings Settings;
    QString status = "";
    if(Settings.find_file_exists())
    {
        status = "F";
    }else{
        status = "T";
    }
    QString strStatus = QString("setStatus(\"%1\");").arg(status);
    Settingframe->evaluateJavaScript(strStatus);
}

void SwmgrApp::closeSettingCenterWnd()
{
    if(m_pSettingCenterView)
    {
        m_pSettingCenterView->hide();
        m_pSettingCenterView->close();
    }
}
/* end */

/* 检测更新 */
void SwmgrApp::openUpgradeWnd()
{
    if(!m_pUpgradeView)
    {
        m_pUpgradeView = new MainWnd();
        if(!m_pUpgradeView)
        {
            qDebug() << "create check upgrade window failed";
            return;
        }

        QObject::connect(m_pUpgradeView, SIGNAL(loadFinished(bool)), this, SLOT(loadUpgradeFinished(bool)));

        m_pUpgradeView->setObjectName("check-upgrade");
        m_pUpgradeView->setWindowTitle("检测更新");
        m_pUpgradeView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        m_pUpgradeView->setWindowModality(Qt::ApplicationModal);
    }

    QFile htmlFile(":/Upgrade.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pUpgradeView->setHtml(htmlFile.readAll(), QUrl("qrc:///Upgrade.html"));
    htmlFile.close();

    m_pUpgradeView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC", this);

    int width = 420;
    int height = 310;
    int x = m_pMainView->x() + m_pMainView->width() / 2 - width / 2;
    int y = m_pMainView->y() + m_pMainView->height() / 2 - height / 2;
    m_pUpgradeView->setGeometry(QRect(x, y, width, height));

    m_pUpgradeView->show();
}

struct StructUpdateInfo
{
    std::string strFileUrl;
    std::string strFilePath;
    std::string strFileSize;
};

UINT ManualUpdate(PVOID pParam)
{
    StructUpdateInfo *p = (StructUpdateInfo*)pParam;
    MyHttpDownload(p->strFileUrl,300,p->strFilePath);
    QFile file(p->strFilePath.data());
    if(file.open(QIODevice::ReadOnly))
    {
        long iFileSize = file.size();
        long iRecvFileSize = atoi(p->strFileSize.data());

        if(iFileSize == iRecvFileSize)
        {
            QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
            //::MessageBox(0,L"最新版本下载完毕，将为您更新至最新版本\0",L"提示\0",0);
            ShellExecuteA(NULL, "open", p->strFilePath.data(), NULL, NULL, SW_SHOW);       //运行安装包，并退出当程序，
            delete p;
            //此处应该强行退出当前程序，以保证安装不会冲突。
            //SwmgrApp::Instance()->ForcedExit();
            SwmgrApp::Instance()->appquit();
        }
    }
    delete p;
    return 0;
}

void SwmgrApp::loadUpgradeFinished(bool)
{
    std::string strUpdateUrl = "";
    std::string strVersion = "";
    std::string strFileName = "";

    strUpdateUrl = "http://";
    strUpdateUrl += GLOBAL::_SERVER_URL.toStdString();
    strUpdateUrl += "/api/update";
    strVersion = GLOBAL::_VERSION.toStdString();
    strFileName = GLOBAL::_SOFTWARENAME.toStdString();

    std::string strReqData = "version=" + strVersion;
    strReqData += "&softname=";
    strReqData += strFileName;

    std::string strFileUrl;
    std::string strFileSize;
    std::string strRecvVersion;

    QString Q_strVersionNow,Q_strVersionSC,Q_strFlag;
    Q_strVersionNow = GLOBAL::_VERSION;

    QWebFrame *frame = m_pUpgradeView->page()->mainFrame();
    if(IsNeedUpdate(strUpdateUrl,strReqData,strFileUrl,strFileSize,strRecvVersion) && CompareVersion(QString::fromStdString(strRecvVersion), Q_strVersionNow) > 0)
    {
        Q_strVersionSC = QString(QString::fromLocal8Bit(strRecvVersion.c_str()));
        Q_strFlag = "1";
        QString strVal = QString("IsNeedUpdate_fan(\"%1\",\"%2\",\"%3\");").arg(Q_strVersionNow).arg(Q_strVersionSC).arg(Q_strFlag);
        frame->evaluateJavaScript(strVal);
        std::string strFilePath = GetLocalAppDataPath() + "\\TempGuanJia.exe";

        StructUpdateInfo* UpdateInfo = new StructUpdateInfo;
        UpdateInfo->strFileUrl = strFileUrl;
        UpdateInfo->strFilePath = strFilePath;
        UpdateInfo->strFileSize = strFileSize;
        CreateThread(NULL,0,(LPTHREAD_START_ROUTINE)ManualUpdate,UpdateInfo,0,0);
    }
    else
    {
        Q_strFlag = "0";
        QString strVal = QString("IsNeedUpdate_fan(\"%1\",\"%2\",\"%3\");").arg(Q_strVersionNow).arg(Q_strVersionSC).arg(Q_strFlag);
        frame->evaluateJavaScript(strVal);
    }
}

void SwmgrApp::closeUpgradeWnd()
{
    if(m_pUpgradeView)
    {
        m_pUpgradeView->hide();
        m_pUpgradeView->close();
    }
}
/* end */

/* 关于我们 */
void SwmgrApp::openAboutUsWnd()
{
    if(!m_pAboutUsView)
    {
        m_pAboutUsView = new MainWnd();
        if(!m_pAboutUsView)
        {
            qDebug() << "create about us window failed";
            return;
        }

        QObject::connect(m_pAboutUsView, SIGNAL(loadFinished(bool)), this, SLOT(loadAboutUsFinished(bool)));

        m_pAboutUsView->setObjectName("about-us");
        m_pAboutUsView->setWindowTitle("关于我们");
        m_pAboutUsView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        m_pAboutUsView->setWindowModality(Qt::ApplicationModal);
    }

    QFile htmlFile(":/about-us.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pAboutUsView->setHtml(htmlFile.readAll(), QUrl("qrc:///about-us.html"));
    htmlFile.close();

    m_pAboutUsView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC", this);

    int width = 420;
    int height = 342;
    int x = m_pMainView->x() + m_pMainView->width() / 2 - width / 2;
    int y = m_pMainView->y() + m_pMainView->height() / 2 - height / 2;
    m_pAboutUsView->setGeometry(QRect(x, y, width, height));
    m_pAboutUsView->show();
}

void SwmgrApp::loadAboutUsFinished(bool)
{
    QString  Q_strVersion = GLOBAL::_VERSION;
    QWebFrame *frame = m_pAboutUsView->page()->mainFrame();
    QString _strVersion = QString("set_Q_strVersion(\"%1\");").arg(Q_strVersion);
    frame->evaluateJavaScript(_strVersion);
}

void SwmgrApp::closeAboutUsWnd()
{
    if(m_pAboutUsView)
    {
        m_pAboutUsView->hide();
        m_pAboutUsView->close();
    }
}
/* end */

/* 优化设置向导 */
void SwmgrApp::openOptimizationWnd()
{
    if(!m_pOptimizeView)
    {
        m_pOptimizeView = new MainWnd();
        if(!m_pOptimizeView)
        {
            qDebug() << "create optimizate window failed";
            return;
        }

        QObject::connect(m_pOptimizeView, SIGNAL(loadFinished(bool)), this, SLOT(loadOptimizationFinished(bool)));

        m_pOptimizeView->setObjectName("optimization");
        m_pOptimizeView->setWindowTitle("优化向导");
        m_pOptimizeView->setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint | Qt::WindowMinimizeButtonHint);
        m_pOptimizeView->setWindowModality(Qt::ApplicationModal);
    }

    QFile htmlFile(":/_dialog_toolbox.html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    m_pOptimizeView->setHtml(htmlFile.readAll(), QUrl("qrc:///_dialog_toolbox.html"));
    htmlFile.close();

    m_pOptimizeView->page()->mainFrame()->addToJavaScriptWindowObject("DYBC", this);

    int width = 700;
    int height = 475;
    int x = m_pMainView->x() + m_pMainView->width() / 2 - width / 2;
    int y = m_pMainView->y() + m_pMainView->height() / 2 - height / 2;
    m_pOptimizeView->setGeometry(QRect(x, y, width, height));

    m_pOptimizeView->show();
    emit firstEssential(0);
}

void SwmgrApp::loadOptimizationFinished(bool)
{
    QString safeStatus;
    safetyCurrentStatus(safeStatus);

    QString RightMenuStatus;
    RightMenuCurrentStatus(RightMenuStatus);

    QString SetIEStatus;
    SetIECurrentStatus(SetIEStatus);

    QString MenuExtName;
    safetyReinforce sts;
    sts.searchMenuExt(MenuExtName);

    QWebFrame *frame = m_pOptimizeView->page()->mainFrame();
    QString strVal = QString("setStatus(\"%1\",\"%2\",\"%3\");").arg(safeStatus).arg(RightMenuStatus).arg(SetIEStatus);
    QString findMenuExt = QString("findMenuExtName(\"%1\");").arg(MenuExtName);
    frame->evaluateJavaScript(strVal);
    frame->evaluateJavaScript(findMenuExt);
}

void SwmgrApp::closeOptimizationWnd()
{
    sizemoveAnimation = new QPropertyAnimation(m_pOptimizeView, "geometry");
    sizemoveAnimation->setDuration(500);
    sizemoveAnimation->setStartValue(QRect(m_pOptimizeView->x(), m_pOptimizeView->y(), m_pOptimizeView->width(), m_pOptimizeView->height()));
    sizemoveAnimation->setEndValue(QRect(head_menu_x + m_pMainView->x(), head_menu_y + m_pMainView->y(), head_menu_width, head_menu_height));
    sizemoveAnimation->start();
    QObject::connect(sizemoveAnimation, SIGNAL(finished()), this, SLOT(closeOptimizationView()));
}

void SwmgrApp::closeOptimizationView()
{
    if(m_pOptimizeView)
    {
        m_pOptimizeView->hide();
        m_pOptimizeView->close();
    }

    if(sizemoveAnimation)
    {
        sizemoveAnimation->deleteLater();
        sizemoveAnimation = NULL;
    }
}
/* end */

void SwmgrApp::Upan(int status)
{
    ssafeR.banUpanStart(status);
}

void SwmgrApp::fileName(int status)
{
    ssafeR.banFileName(status);
}

void SwmgrApp::DiskShare(int status)
{
    ssafeR.banDiskShare(status);
}

void SwmgrApp::NullLink(int status)
{
    ssafeR.banNullLink(status);
}

void SwmgrApp::AccountControl(int status)
{
    ssafeR.banAccountControl(status);
}

void SwmgrApp::requestDelIE()
{
    systemR.deleteIE();
}

void SwmgrApp::requestAddIE()
{
    systemR.addIE();
}

void SwmgrApp::requestAddSystemRightMenu()
{
    systemR.addSystemRightMenu();
}

void SwmgrApp::requestaDeleteSystemRightMenu()
{
    systemR.deleteSystemRightMenu();
}

void SwmgrApp::requestaAddMyComputer()
{
    systemR.addMyComputer();
}

void SwmgrApp::requestaDeleteMyComputer()
{
    systemR.deleteMyComputer();
}

void SwmgrApp::requestAddRunwindows()
{
    systemR.addRunwindows();
}

void SwmgrApp::requestDeleteRunwindows()
{
    systemR.deleteRunwindows();
}

void SwmgrApp::requestSetHomePage(int val)
{
    if(val == 1){
        QString hao123 = "http://www.hao123.com/";
        SetHomePage(hao123);
    }else if(val == 2){
        QString www2345 = "http://www.2345.com/";
        SetHomePage(www2345);
    }else if(val == 3){
        QString sogou123 = "http://123.sogou.com/";
        SetHomePage(sogou123);
    }
}

void SwmgrApp::requestSetSearchEngine(int val)
{
    if(val == 4){
        QString Searchbaidu = "http://www.baidu.com/s?wd={searchTerms}";
        SetSearchEngine("baidu",Searchbaidu);
    }else if(val == 5){
        QString google = "http://www.google.cn/search?hl=zh-CN&q={searchTerms}";
        SetSearchEngine("google",google);
    }else if(val == 6){
        return;
    }
}

void SwmgrApp::requestAddNeverCombine(int status)
{
    systemR.addNeverCombine(status);
}

void SwmgrApp::requestFinished()
{
    m_pOptimizeView->close();
}

void SwmgrApp::requestSearchMenuExt(QString MenuExt)
{
    qDebug()<< "MenuExt=" << MenuExt << endl;
    MyRegedit myReg("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MenuExt");
    if(myReg.InitReg())
    {
       myReg.DelSubItemReg(MenuExt);
    }
}


//----------------------------------------------------GetInfo




void SwmgrApp::ShowUpdateMSG(QString packagePath)
{
    strPackagePath = packagePath;
    openUpgradeWnd();
}

void SwmgrApp::requestCloseAutoUpgrade()
{
    if(m_pAutoUpgrade)
    {
        m_pAutoUpgrade->hide ();
        m_pAutoUpgrade->close();
        QFile file(strPackagePath);
        if(file.open(QIODevice::ReadOnly))
        {
            file.close();
            delete m_pAutoUpgrade;
            m_pAutoUpgrade = NULL;
            //,,,关闭主程序，启动安装包。
            ShellExecuteA(NULL, "open", strPackagePath.toStdString().data(), NULL, NULL, SW_HIDE);       //运行安装包，并退出当程序，
            //此处应该强行退出当前程序，以保证安装不会冲突。
            appquit();
        }
    }
}

//软件卸载----清理残留文件
void SwmgrApp::CleanupResidue(QString strResidueFilePath, QString strDisplayName)
{
    Q_UNUSED(strDisplayName);
    QMessageBox message(QMessageBox::Warning,"提示","清理残留文件失败，是否手动清理？",QMessageBox::Yes|QMessageBox::No,NULL);
    if (message.exec()==QMessageBox::Yes)
    {
        QString strCMD;
        strCMD = strResidueFilePath;
        ShellExecuteA(NULL, "open", strCMD.toStdString().data(), NULL, NULL, SW_SHOW);
    }
}

void SwmgrApp::requestEssential()
{
    char configPath[500] = {NULL};
    HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
    GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
    *(strrchr( configPath, '\\') ) = 0;
    strcat_s(configPath, "\\Close_config.ini");
    char szUpdateUrl[128] = { 0 };
    int iszUpdateUrl = sizeof(szUpdateUrl);
    GetPrivateProfileStringA("Essential", "EssentialStatus", NULL, szUpdateUrl, iszUpdateUrl, configPath);
    QString qs_data = QString::fromLocal8Bit(szUpdateUrl,128);
    emit statusEssential(qs_data);
}

void SwmgrApp::requestn_nocheck()
{
    char configPath[500] = {NULL};
    HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
    GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
    *(strrchr( configPath, '\\') ) = 0;
    strcat_s(configPath, "\\Close_config.ini");
    n_pstatus = "";
    WritePrivateProfileStringA("Essential", "EssentialStatus", n_pstatus.toStdString ().c_str (), configPath);
}
