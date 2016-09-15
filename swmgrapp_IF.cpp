#include "swmgrapp.h"
#include <QTextCodec>
#include <QtAlgorithms>
#include "OSSystemWrapper.h"
#include <QSettings>
#include "systemregset.h"
#include "checkuninstallsoftname.h"
#include <QDebug>
#include "safetyreinforce.h"
#include "selfupdate.h"
#include "character.h"
#include "storage.h"
#include "appenv.h"
#include "GetPngThread.h"

void SwmgrApp::docLoadFinish(bool ok){
    if(ok){
        _DataModel->reqQueryUserStatus();

        int openOp = 1;
        char configPath[500] = {NULL};
        HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
        GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
        *(strrchr( configPath, '\\') ) = 0;
        strcat_s(configPath, "\\Close_config.ini");
        openOp = GetPrivateProfileIntA("CLOSE", "openOp", 1, configPath);
        std::string strConfigPath =GetLocalAppDataPath();
        strConfigPath += "\\Close_config.ini";
        openOp = GetPrivateProfileIntA("CLOSE", "openOp", 1, strConfigPath.data());
        if(openOp == 1)
        {
            openOp = 0;
            QString openOp_str = QString::number(openOp, 10);
            WritePrivateProfileStringA("CLOSE", "openOp", openOp_str.toStdString ().c_str (), strConfigPath.data());

            char szUpdateUrl[128] = { 0 };
            int iszUpdateUrl = sizeof(szUpdateUrl);
            GetPrivateProfileStringA("Essential", "EssentialStatus", NULL, szUpdateUrl, iszUpdateUrl, configPath);
            QString qs_data = QString::fromLocal8Bit(szUpdateUrl,128);
            emit statusEssential(qs_data);

            openOptimizationWnd();
        }

        // start service
        _DataModel->StartUserService();
        _DataModel->StartDownloadService();
        _DataModel->StartUpgradeService();
        _DataModel->StartUninstallService();
    }
}

void SwmgrApp::requestSoftCategoryList(){
    emit updateSoftCategory(_DataModel->GetCategory());
}

bool SwmgrApp::FilterPackage(const QVariantMap &package, mapSoftwareList &mapInstalledSoftwares)
{
    QString name = package["name"].toString();
    if(name.isEmpty())
    {
        return true;
    }

    for(mapSoftwareList::iterator it = mapInstalledSoftwares.begin(); it != mapInstalledSoftwares.end(); it++)
    {
        QString displayName = QString::fromStdString(GBKToUTF8(it->second["DisplayName"]));
        if(displayName.isEmpty())
        {
            continue;
        }

        if(name.contains(displayName) || displayName.contains(name))
        {
            return true;
        }
    }

    return false;
}

void SwmgrApp::DeleteGetPngThread(GetPngThread *pThread)
{
    if(pThread)
    {
        if(pThread->isRunning())
        {
            pThread->exit();
            pThread->wait();
        }
        pThread->deleteLater();
    }
}

void SwmgrApp::requestExtraCategoryList(QString szCategoryID, int pageNumber, int count)
{
    if(pageNumber < 1)
    {
        pageNumber = 1;
    }

    if(count <= 0 || count > 20)
    {
        count = 20;
    }

    QVariantList &lstTempPackage = _DataModel->GetPackage(szCategoryID);
    if(lstTempPackage.size())
    {
        GetPngThread *pThread = new GetPngThread();
        if(pThread)
        {
            if(pThread->Init(lstTempPackage))
            {
                QObject::connect(pThread, SIGNAL(sigDeleteGetPngThread(GetPngThread *)), this, SLOT(DeleteGetPngThread(GetPngThread *)), Qt::QueuedConnection);
                pThread->start();
            }
            else
            {
                delete pThread;
                pThread = NULL;
            }
        }
    }

    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    int start = (pageNumber - 1) * count;
    int end = pageNumber * count;

    int index = 0;
    QVariantList lstPackage;
    QVariantMap package;
    QString szPngPath = "";
    foreach(QVariant item, lstTempPackage)
    {
        package = item.toMap();
        if(szCategoryID.compare("hot", Qt::CaseInsensitive) == 0 || szCategoryID.compare("top", Qt::CaseInsensitive) == 0)
        {
            if(FilterPackage(package, mapInstalledSoftwares))
            {
                continue;
            }
            if(index >= start && index < end)
            {
                szPngPath = GLOBAL::_PNGDIR + package["id"].toString() + ".png";
                if(IsFileExist(szPngPath))
                {
                    package["largeIcon"] = QVariant::fromValue("file:///" + szPngPath.replace("\\", "/"));
                }
                lstPackage.append(package);
            }
            index++;
        }
        else
        {
            if(index >= start && index < end)
            {
                szPngPath = GLOBAL::_PNGDIR + package["id"].toString() + ".png";
                if(IsFileExist(szPngPath))
                {
                    package["largeIcon"] = QVariant::fromValue("file:///" + szPngPath.replace("\\", "/"));
                }
                lstPackage.append(package);
            }
            index++;
        }
        if(lstPackage.size() == count)
        {
            break;
        }
    }

    // 如果不满count个, 则追加(从0开始的)软件
    if(lstPackage.size() < count)
    {
        int nRemain = count - lstPackage.size();
        foreach(QVariant item, lstTempPackage)
        {
            if(nRemain <= 0)
            {
                break;
            }
            package = item.toMap();
            szPngPath = GLOBAL::_PNGDIR + package["id"].toString() + ".png";
            if(IsFileExist(szPngPath))
            {
                package["largeIcon"] = QVariant::fromValue("file:///" + szPngPath.replace("\\", "/"));
            }
            lstPackage.append(package);
            nRemain--;
        }
    }

    int countTotal = 100;
    if(szCategoryID.compare("hot", Qt::CaseInsensitive) && szCategoryID.compare("top", Qt::CaseInsensitive))
    {
        countTotal = lstTempPackage.size();
        if(countTotal % count != 0)
        {
            countTotal += (count - countTotal % count);
        }
    }
    int pageTotal = countTotal / count + (countTotal % count > 0 ? 1 : 0);;
    emit updateExtraCategoryList(szCategoryID, lstPackage, pageNumber, pageTotal, countTotal);
}

void SwmgrApp::requestTopPackage()
{
    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    QVariantList &lstAllPackage = _DataModel->GetPackage("hot");

    QVariantList lstPackage;
    QVariantMap package;
    QString szPngPath = "";
    foreach(QVariant item, lstAllPackage)
    {
        package = item.toMap();
        if(!FilterPackage(package, mapInstalledSoftwares))
        {
            szPngPath = GLOBAL::_PNGDIR + package["id"].toString() + ".png";
            if(IsFileExist(szPngPath))
            {
                package["largeIcon"] = QVariant::fromValue("file:///" + szPngPath.replace("\\", "/"));
            }
            lstPackage.append(package);
            if(lstPackage.size() == 8)
            {
                break;
            }
        }
    }

    emit updateTopPackage(lstPackage);
}

void SwmgrApp::requestRegisteUser(QString username, QString password, QString email,QString strMobile){
    _DataModel->reqRegisteUser(username, password, email,strMobile);
}

void SwmgrApp::requestLoginUser(QString username, QString password){
    _DataModel->reqLoginUser(username, password);
}
void SwmgrApp::requestModifyUserInfo(QVariantMap userinfo){
    _DataModel->reqModifyUserInfo(userinfo);
}

void SwmgrApp::requestComfireUpdateUserInfo(QString szOld, QString szNew)
{
    _DataModel->reqUserPasswdUpdate(szOld, szNew);
}

void SwmgrApp::requestBackupSysSoftListInfo()
{
    _DataModel->reqBackupSysSoftListInfo();
}

void SwmgrApp::requestRestoreSysInfo(QString filename)
{
    _DataModel->reqRestoreSysInfo(filename);
}

qint32 SwmgrApp::StringLikeFind(QString str, QString &in_of_str)
{
    if(str.isEmpty())return 0;

    str=str.replace(QRegExp("[<>《》！?*(^)$%~!@#$…&%￥—+=、。；‘’“”：·`]"), "");

    QStringList s=str.split(QString(" "), QString::SkipEmptyParts, Qt::CaseInsensitive);

    for(QStringList::iterator it=s.begin();it!=s.end();){
        if((*it).isEmpty())
        {
            it++;
            continue;
        }
        if((*it).size()<2){
            it = s.erase(it);
            continue;
        }

        it++;
    }

    if(!s.isEmpty()){
        qint32 power=0;
        qint32 k=s.size();

        if(k == 1 && in_of_str.compare(s.at(0)) == 0){
            return 100;
        }
        qint32 flag=0;
        qint32 num;

        qint32 wordnum=s.size();
        qint32 count=in_of_str.count(" ")+1-wordnum;
        if(abs(count)>2){
            return 0;
        }
        for(qint32 i=0;i<k;i++){
            const QString &st=s.at(i);
            num=st.toStdString().size();
            if(in_of_str.contains(st, Qt::CaseInsensitive)){
                if(num<5){
                    power+=2;
                }else if(num<7){
                    power+=3;
                }else if(num<11){
                    power+=4;
                }else{
                    power+=5;
                }

            }else{
                flag++;
            }
        }
        if(!power)return power;

        if(flag<2){
            return power-count*3;
        }else return power-flag*5-count*3;
    }else{
        return 0;
    }}

//--------------------------------------------以下卸载相关
void SwmgrApp::get_str_for_used_date(QString &value, QString &str)
{
    QDate &date=QDate::fromString(value, "yyyyMMdd");
    QDate &cur_date=QDate::currentDate();

    if(cur_date.year()-date.year()){
        str=tr("1年前使用过\0");
    }else if(cur_date.month()-date.month()){
        str=tr("1个月前使用过\0");
    }else{
        int day=cur_date.day()-date.day();
        if(day <= 0)
            str=tr("今天内使用过\0");
        else if(day>0){
            str=QString("%1").arg(day)+tr("天前使用过\0");
        }
    }
}

//卸载相关。这个函数是网页调用qt，用来获取安装数据，然后展现在网页上
void SwmgrApp::requestCanUninstallPackages()
{

    QVariantList jsArray;
    QStringMap map_use;
    _DataModel->reqRefreshSoftList(); // 每次访问 必刷一次， 使得 实时更新 当前卸载 列表（js端控制 是否 进行 数据更新）
    _DataModel->UnstalledSoftInfo();    //获取到安装软件的详细信息，详情请跳转到该函数。
    getUnstallData(map_use);            //获取到最近本机最近使用的软件的名称和最后使用时间。

    QVariantMapMap &jsArrayMap=_DataModel->getUnstallSoftInfo();    //将获取到的软件详细信息取出
    QStringVector InfoSoftIdxMD5=_DataModel->getUnstallSoftInfoIdx();   //将获取到的软件ID的MD5取出。


    //得到不能直接对应上的list

    QList<QString> &list=map_use.keys();
    QStringVector RegNameAll, regName;
    QVector<qint32> Keyvector;

    //循环得到，最近没有运行过的软件名称列表。RegNameAll;
    for(QStringVector::iterator item=InfoSoftIdxMD5.begin();item!=InfoSoftIdxMD5.end();item++){
        QVariantMap &map=jsArrayMap[*item];
        QString &name=map["DisplayName"].toString();
        if(!list.removeOne(name)){
            RegNameAll.append(name);
        }
    }

    QMap<qint32, qint32> keymap;
    qint32 j=0;
    //循环，寻找list中剩下的和RegNameAll中相似的元素。保存在regName，keymap，Keyvector
    for(QStringVector::iterator it=RegNameAll.begin();it!=RegNameAll.end();it++){
        qint32 score=0;
        qint32 key=-1;
        qint32 i=0;
        QString &name=*it;
        for(QList<QString>::iterator lit=list.begin();lit!=list.end();lit++){
            qint32 m=StringLikeFind(*lit, name);

            if(score<m){
                score=m;
                key=i;
            }
            i++;
        }

        if(key>=0){
            if(keymap.find(key) == keymap.end()){
                keymap.insert(key, score);
                regName.append(name);
                Keyvector.append(key);
            }else{
                if(keymap[key]<score){
                    keymap[key]=score;
                    regName.replace(Keyvector.indexOf(key), name);
                }
            }
        }
        j++;
    }

    QStringMap::iterator it;

    QString str;
    for(QStringVector::iterator item=InfoSoftIdxMD5.begin();item!=InfoSoftIdxMD5.end();item++){
        QVariantMap &map=jsArrayMap[*item];
        QString &name=map["DisplayName"].toString();
        it=map_use.find(name);

        if(it!=map_use.end()){

            get_str_for_used_date(it.value(), str);

            if(map.find("UsedDate") == map.end()){
                map.insert("UsedDate", str);
            }else{
                map["UsedDate"]=str;
            }

        }else{
            qint32 k=regName.indexOf(name);
            qint32 key=-1;
            if(k>=0){
                 key=Keyvector.at(k);
            }

            if(key>=0){
               // QString ss=list.at(key);
                it=map_use.find(list.at(key));
                get_str_for_used_date(it.value(), str);

                if(map.find("UsedDate") == map.end()){
                    map.insert("UsedDate", str);
                }else{
                    map["UsedDate"]=str;
                }
            }else{
                map.insert("UsedDate", QString("未知"));
            }
            //qDebug()<<map["UsedDate"]<<endl;
        }
        QString szOldPngPath = map["UnIcoID"].toString();
        QString szPngPath = GLOBAL::_DY_DIR_RUNNERSELF + "png" + QDir::separator() + szOldPngPath;
        map["UnIcoID"] = szPngPath.replace("\\", "/");
        jsArray.append(map);
        map["UnIcoID"] = szOldPngPath;
    }

    //emit updateCanUninstallPackages(jsArray);//给消息页面更新
}

//卸载相关
void SwmgrApp::getUnstallData(QStringMap &map_use)
{

    WaitForSingleObject(mutex_soft_file, INFINITE);
    QString &szAppdataPath=_DataModel->getszAppdataPath();
    QFile file(szAppdataPath+"/softlist.txt");
    //        QFile file("c:\\2.txt");
    if(!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;

    QTextStream in(&file);
    map_use.clear();
    while (!in.atEnd())
    {
        QString &line = in.readLine();
        QStringList list=line.split(QString(", "), QString::SkipEmptyParts, Qt::CaseInsensitive);
        map_use.insert(list.at(0), list.at(1));
    }


    file.close();

    ReleaseMutex(mutex_soft_file);
}

void SwmgrApp::InitMutex_SoftDate()
{
    /*mutex_soft_file = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("Global\\MutexLockSoftDate"));
    if(mutex_soft_file == NULL)
    {
        mutex_soft_file = CreateMutex(NULL, FALSE, TEXT("Global\\MutexLockSoftDate"));
    }
    if(mutex_soft_file == NULL)
    {
        return;
    }*/
}

//卸载相关。卸载某个，或者某几个软件
void SwmgrApp::requestDoUninstall(QString uninstallID){

    /*if(!uninstallID.isEmpty()){

        QVariantMapMap &jsArrayMap=_DataModel->getUnstallSoftInfo();
        UnstallMd5=uninstallID.split(QString(", "), QString::SkipEmptyParts, Qt::CaseInsensitive);
        MultiSoftPath.clear();

        for(QStringList::Iterator it=UnstallMd5.begin();it!=UnstallMd5.end();it++){
            QVariantMap &jsArray=jsArrayMap[*it];
            QVariant &szKEY =jsArray["QuietUninstallString"].toString().isEmpty() ? jsArray["UninstallString"]:jsArray["QuietUninstallString"];
            MultiSoftPath.append(szKEY.toString());
            //得到需要删除的软件的路径
        }
        Md5count=0;
        processIdtime =new QTimer;
        UninstalPackages();//真正的卸载。
    }*/
    if(_DataModel)
        _DataModel->UninstallSoftwares(uninstallID);
}


// 枚举系统当前所有进程信息, 记录能获取到绝对路径的程序信息, 填充mapProgressNow表
BOOL SwmgrApp::CheckProcessId(DWORD pId)
{
    BOOL bRet = FALSE;
    // 定义进程信息结构
    PROCESSENTRY32 pe32 = { sizeof(pe32) };
    // 创建系统当前进程快照
    HANDLE hProcessShot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if(hProcessShot == INVALID_HANDLE_VALUE)
        return false;
   // 循环枚举进程信息
    if(Process32First(hProcessShot, &pe32))
    {
        do {
             // 把宽字符的进程名转化为ANSI字符串
            //WideCharToMultiByte(CP_ACP, 0, pe32.szExeFile, wcslen(pe32.szExeFile), szBuf, sizeof(szBuf), NULL, NULL);
          //  HANDLE hProc = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, pe32.th32ProcessID);
            if(pe32.th32ParentProcessID == pId)
            {
               bRet = TRUE;
               break;
            }

            if(pe32.th32ProcessID == pId)
            {
               bRet = TRUE;
               break;
            }
        } while (Process32Next(hProcessShot, &pe32));
    }
    CloseHandle(hProcessShot);

    return bRet;
}

void SwmgrApp::Uninstall_queue()
{
    if(!CheckProcessId(pid)){
        if(!MultiSoftPath.isEmpty()){
            processIdtime->stop();
            UninstalPackages();
        }else{
            UninstallerFinished(0, QProcess::NormalExit);
        }
    }
}

//卸载软件，一个或者多个。
void SwmgrApp::UninstalPackages()
{
    QProcess *Uninst= new QProcess();
    QStringList arguments;
    QString name;

    name=MultiSoftPath.first();
    MultiSoftPath.pop_front();

    arguments.clear();
    CheckUninstallSoftName *checkuninstall =new CheckUninstallSoftName();
    checkuninstall->GetUninstallNameArg(name, arguments);
    delete checkuninstall;
    //qDebug()<<name<<endl;
    //qDebug()<<arguments<<endl;
    Uninst->start(name, arguments);

    pid=Uninst->processId();

    Q_PID pid2=Uninst->pid();

    //if(!MultiSoftPath.isEmpty()){


    connect(processIdtime, SIGNAL(timeout()), this , SLOT(Uninstall_queue()));
    //processIdtime->singleShot(100, this, SLOT(Uninstall_queue(pid)));
    processIdtime->start(1000);

    //}else QObject::connect(Uninst, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(UninstallerFinished(int, QProcess::ExitStatus)));

    if(pid2 == NULL){//The process does not start;
        QVariantMapMap &jsArrayMap=_DataModel->getUnstallSoftInfo();
        QVariantMap &jsArray=jsArrayMap[UnstallMd5.at(Md5count++)];
        QVariant &szKEY =jsArray["QuietUninstallString"].toString().isEmpty() ? jsArray["UninstallString"]:jsArray["QuietUninstallString"];
        SystemRegSet *reg=new SystemRegSet;

        reg->DelRegUnstallInfo(szKEY.toString());
        delete reg;
    }
}

void SwmgrApp::UninstallerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    processIdtime->stop();
    delete processIdtime;
    if(exitStatus == QProcess::NormalExit){
        _DataModel->reqRefreshSoftList();

        QVariantMapMap &jsArrayMap=_DataModel->getUnstallSoftInfo();
        QStringVector &InfoSoftIdx=_DataModel->getUnstallSoftInfoIdx();
        while(!UnstallMd5.isEmpty())
        {
            if(!MD5asSoftisExist(UnstallMd5.first())){
                jsArrayMap.remove(UnstallMd5.first());
                InfoSoftIdx.remove(InfoSoftIdx.indexOf(UnstallMd5.first()));
            }

            UnstallMd5.pop_front();
        }
        requestCanUninstallPackages();  //更新Uninstall list
    }else if(QProcess::CrashExit == exitStatus){
        ;
    }
}

bool SwmgrApp::MD5asSoftisExist(QString uninstallID)
{
    //do uninstall
    mapSoftwareList &mapSoftwares = _DataModel->getInstalledSoftware();
    for (mapSoftwareList::iterator item = mapSoftwares.begin(); item != mapSoftwares.end();item++){

        QString szKEY = QString::fromStdString(item->second["QuietUninstallString"].size()>0 ? item->second["QuietUninstallString"] : item->second["UninstallString"]);
        if(uninstallID.compare(QString(QCryptographicHash::hash(szKEY.toUtf8(), QCryptographicHash::Md5).toHex()), Qt::CaseInsensitive) == 0){
            return 1 ;
        }
    }

    return 0;
}
//--------------------------------------------以上卸载相关

void SwmgrApp::requestInstallTopPackage(QString szCategoryID, QString szPackageID)
{
    QVariantList &lstPackage = _DataModel->GetPackage("top");

    foreach(QVariant item, lstPackage)
    {
        QVariantMap package = item.toMap();
        if(package.value ("id").toString ().compare(szPackageID, Qt::CaseInsensitive) == 0)
        {
            QString name = package.value ("name").toString ();
            QString versionName = package.value ("versionName").toString();

            QString displayName = "";
            QString displayVersion = "";

            mapSoftwareList mapInstalledSoftware;
            _DataModel->GetCurInstalledSoftware (mapInstalledSoftware);

            for(mapSoftwareList::iterator it = mapInstalledSoftware.begin(); it != mapInstalledSoftware.end(); it++)
            {
                displayName = QString::fromStdString(GBKToUTF8(it->second["DisplayName"]));
                displayVersion = QString::fromStdString(GBKToUTF8(it->second["DisplayVersion"]));
                if(name.compare (displayName) == 0 && versionName.compare (displayVersion) == 0)
                {
                    n_pstatus += (szPackageID + ",");
                    char configPath[500] = {NULL};
                    HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
                    GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
                    *(strrchr( configPath, '\\') ) = 0;
                    strcat_s(configPath, "\\Close_config.ini");
                    qDebug()<<n_pstatus<<endl;
                    WritePrivateProfileStringA("Essential", "EssentialStatus", n_pstatus.toStdString ().c_str (), configPath);
                    return;
                }
            }
        }
    }

    requestStartInstallPackage (szCategoryID, szPackageID);
}


void SwmgrApp::requestStartInstallPackage(QString szCategoryID, QString szPackageID){
    QVariantMap var;
    QVariantList &lstPackage = _DataModel->GetPackage(szCategoryID);

    foreach(QVariant package, lstPackage)
    {
        if(package.toMap().value("id").toString().compare(szPackageID, Qt::CaseInsensitive) == 0)
        {
            var = package.toMap();
            _DataModel->reqAddTask(var);
            break;
        }
    }
}

void SwmgrApp::requestPausePackage(QString szCategoryID, QString szPackageID){
    _DataModel->reqPauseTask(szPackageID);
}

void SwmgrApp::requestResumePackage(QString szCategoryID, QString szPackageID){
    _DataModel->reqResumeTask(szPackageID);
}

void SwmgrApp::requestAllResumePackage(){
    _DataModel->reqResumeAllTask();
}

void SwmgrApp::requestStopDownloadPackage(QString szCategoryID, QString szPackageID){
    _DataModel->reqRemoveTask(szPackageID);
}

void SwmgrApp::requestAllDownloadingTaskPause(){ //Pause all downloading task
    _DataModel->reqPauseAllTask();
}

void SwmgrApp::requestAllDownloadingTaskCancel(){ //Cancel all downloading task
    _DataModel->reqRemoveAllTask();
}

void SwmgrApp::requestLoadPage(QString pageName)
{
    QFile htmlFile(":/tpl." + pageName + ".html");
    htmlFile.open(QIODevice::ReadOnly | QIODevice::Text);
    QVariantList content;
    content.append(htmlFile.readAll().constData());
    htmlFile.close();
    emit updateLoadPage(pageName, content);
}

void SwmgrApp::requestOnPageChange(QString pageName){
    if(pageName.compare("index", Qt::CaseInsensitive) == 0)
    {
        mapSoftwareList mapInstalledSoftwares;
        DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);
        emit updateInstalledCount(mapInstalledSoftwares.size());
    }
    else if(pageName.compare("login", Qt::CaseInsensitive) == 0)
    {
        return;
    }
    else if(pageName.compare("logout", Qt::CaseInsensitive) == 0)
    {
        _DataModel->reqClearUserStatus();
    }
    else if(pageName.compare("task", Qt::CaseInsensitive) == 0)
    {
        _DataModel->reqQueryAllTaskInfo();
    }
    else if(pageName.compare("toolbox", Qt::CaseInsensitive) == 0)
    {
        return;
    }
    else if(pageName.compare("uninstall", Qt::CaseInsensitive) == 0)
    {
        _DataModel->UpdateUninstallList();
    }
    else if(pageName.compare("upgrade", Qt::CaseInsensitive) == 0)
    {
        _DataModel->reqUpgradeData();
    }
    else if(pageName.compare("uprofile", Qt::CaseInsensitive) == 0)
    {
        return;
    }
    else if(pageName.compare("Backup", Qt::CaseInsensitive) == 0)
    {
        _DataModel->reqQueryBackupInfo();
    }
}

void SwmgrApp::requestSystemRegSet(int m)
{
    SystemRegSet *reg=new SystemRegSet();

    switch(m)
    {
        case 0://disable task group
             reg->SetReg(HKEY_CURRENT_USER, tr("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), tr("TaskbarGlomming"), 0);
             reg->SetReg(HKEY_LOCAL_MACHINE, tr("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), tr("TaskbarGlomming"), 0);
        break;
        case 1://enable task group
             reg->SetReg(HKEY_CURRENT_USER, tr("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), tr("TaskbarGlomming"), 1);
             reg->SetReg(HKEY_LOCAL_MACHINE, tr("Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced"), tr("TaskbarGlomming"), 1);
        break;
        default:

        break;
    }
    delete reg;
}

void SwmgrApp::SelectDownLoadFolder()
{
    QFileInfo  oFileInfo;
    QString file_fullPath;
    QString file_name, diskName, newPath;
    file_fullPath = QFileDialog::getExistingDirectory(NULL, tr("select Folder"));
    if(file_fullPath.isEmpty()){
        return;
    }
    oFileInfo = QFileInfo(file_fullPath);
    file_name = oFileInfo.fileName();

    diskName = file_fullPath.left(3);
    quint64 freeSpace = getDiskFreeSpace(diskName);
    newPath = oFileInfo.filePath();
    qDebug()<<"current diskName: "<<diskName<<" free space: "<<freeSpace<<"file_fullPath �� "<<newPath;

    QString defaultConf= SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "downloadPath.conf");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug()<<"write failded.";
    }

    QTextStream out(&file);
    out<<newPath;
    file.close();

    QWebFrame *Settingframe = m_pSettingCenterView->page()->mainFrame();

    QString strVal = QString("setDiskSpaceAndDownLoadPath(\"%1\", \"%2\");").arg(newPath).arg(freeSpace);
    Settingframe->evaluateJavaScript(strVal);
}

quint64 SwmgrApp::getDiskFreeSpace(QString driver)
{
    LPCWSTR lpcwstrDriver=(LPCWSTR)driver.utf16();

    ULARGE_INTEGER liFreeBytesAvailable, liTotalBytes, liTotalFreeBytes;

    if( !GetDiskFreeSpaceEx( lpcwstrDriver, &liFreeBytesAvailable, &liTotalBytes, &liTotalFreeBytes) )
    {
        qDebug() << "ERROR: Call to GetDiskFreeSpaceEx() failed.";
        return 0;
    }
    return (quint64) liTotalFreeBytes.QuadPart/1024/1024/1024;
}

void SwmgrApp::reqDownLoadFolder(QString path, quint64 diskspace)
{
    //emit updateDownLoadFilePath(path, diskspace);
}

void SwmgrApp::exePromptPriceInfoPlugin(QString checkStr)
{
    QString defaultConf = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "PromptPriceInfo.conf");
    if(!file.open(QIODevice::WriteOnly)){
        qDebug()<<"write failded.";
    }

    QTextStream out(&file);
    out<<checkStr;
    file.close();
}

QString SwmgrApp::readPromptInfo()
{
    QString defaultConf= SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "PromptPriceInfo.conf");
    if(!file.open(QIODevice::ReadOnly)){
        qDebug()<<"read failded.";
    }

    QTextStream in(&file);
    QString path;
    if(!in.atEnd()){
        path = in.readLine();
    }
    return path;
}

/****************
 *@brief : get feedback content
 *@param : feedback's name and content
 *@author: cqf
 ****************/
void SwmgrApp::exeGetSuggustionInfo(QString index, QString title, QString content)
{
    QString msg = "";
    switch(index.toInt())       // type
    {
    case 1:
        msg += ("type=" + UrlEncode("question"));
        break;
    case 2:
        msg += ("type=" + UrlEncode("function"));
        break;
    case 3:
        msg += ("type=" + UrlEncode("website"));
        break;
    case 4:
        msg += ("type=" + UrlEncode("virus"));
        break;
    default:
        msg += ("type=" + UrlEncode("question"));
        break;
    }
    msg += ("&username=" + UrlEncode(_DataModel->getUserInstance().getUserName()));           // username
    msg += ("&title=" + UrlEncode(title));
    msg += ("&content=" + UrlEncode(content));

    QByteArray ba;
    ba.append(msg);

    QString curUrl = "";
    curUrl = "http://";
    curUrl += GLOBAL::_SERVER_URL;
    curUrl += "/api/advise";

    std::string recv_msg;
    MyHttpPost(curUrl.toStdString(), 30, ba.data(), recv_msg);
    QJsonParseError err;
    QJsonDocument doc = QJsonDocument::fromJson(recv_msg.data(), &err);
    int iCode = 0;
    if(err.error == QJsonParseError::NoError)
    {
        if(doc.isObject())
        {
            QVariantMap result = doc.toVariant().toMap();
            QVariantMap::iterator jsonCode = result.find("code");
            if(jsonCode != result.end())
            {
                iCode = jsonCode.value().toInt();
            }
        }
    }
    emit updateAdvise(iCode);
}
