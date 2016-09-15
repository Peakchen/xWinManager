#include "DataControl.h"
#include <QJsonDocument>
#include <QJsonObject>
#include "Storage.h"
#include "Uninstallsoftware.h"
#include "selfupdate.h"
#include "swmgrapp.h"
#include "ThreadToPullSoftwareList.h"
bool compareSWList(QVariantMap&first, QVariantMap &second);

DataControl::DataControl(QObject *parent) : QObject(parent) {
    _UserInRunner = NULL;
    _TaskRunner = NULL;
    m_pUpgradeData = NULL;
    m_pUpgradeDataThread = NULL;
    m_pUpgradeHandler = NULL;
    m_pUninstallSoftware = NULL;
    m_pUninstallSoftwareThread = NULL;

    m_lstCategory.clear();
    m_lstPackage.clear();
    m_szCategoryID = QString("");
}

bool compareSWList(QVariantMap &first, QVariantMap &second)
{
    // 第一排序: rating降序
    int first_rating = first.value("rating").toInt();
    int second_rating = second.value("rating").toInt();
    if(first_rating < second_rating){
        return false;
    }else if(first_rating > second_rating){
        return true;
    }else{
        // 第二排序: weight降序
        int first_weight = first.value("weight").toInt();
        int second_weight = second.value("weight").toInt();
        if(first_weight < second_weight){
            return false;
        }else if(first_weight > second_weight){
            return true;
        }else{
            // 第三排序: dlcount降序dlcount
            qint64 first_dlcount = first.value("dlcount").toLongLong();
            qint64 second_dlcount = second.value("dlcount").toLongLong();
            if(first_dlcount < second_dlcount){
                return false;
            }else if(first_dlcount > second_dlcount){
                return true;
            }else{
                return false;
            }
        }
    }
}

/****************
 *@brief : 通过大小 初始化 软件列表
 *@param : swlist 软件列表
 *@author: cqf
 ****************/
void DataControl::InitSoftWareOrderBySize(QVariantList &SwList)
{
    int maxLen = SwList.size();
    QVector<QVariantMap> vecSort;
    for(int i=0; i<maxLen;i++){
        QVariantMap swMap = SwList.at(i).toMap();
        //int weight = swMap.value("weight").toInt();
        //qDebug()<<" 0_weight:___ "<<weight<<"  list: "<<SwList.at(i).toString();
        vecSort.push_back(swMap);
    }
    if(!vecSort.isEmpty()){
        qSort(vecSort.begin(), vecSort.end(), compareSWList);
    }
    SwList.clear();
    for(int i=0; i<maxLen;i++){
        SwList.append(vecSort.at(i));
    }
    vecSort.clear();
}

void DataControl::LoadSettingProfile() {
    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "swgmgr.conf");
    Storage::getSettingFromFile(szFile, _setting);
}

void DataControl::SaveSettingProfile() {
    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "swgmgr.conf");
    Storage::setSettingToFile(szFile, _setting);
}

QString DataControl::getSettingParameter(QString name, QString defaultValue) {
    if (_setting.isEmpty() || !_setting.contains(name)) {
        _setting.insert(name, defaultValue);
        SaveSettingProfile();
        return defaultValue;
    }
    return _setting.value(name).toString();
}

QVariantList &DataControl::GetCategory()
{
    if(m_lstCategory.isEmpty())
    {
        // from local
        Storage::LoadSoftwareCategory(ConfOperation::Root().getSubpathFile("Data", "SoftwareCategoryAll.list"), m_lstCategory);

        // from server
        if(m_lstCategory.isEmpty())
        {
            QString curUrl = "";
            curUrl = "http://";
            curUrl += GLOBAL::_SERVER_URL;
            curUrl += "/api/swmgr?type=category&cfv=0";

            std::string recv_msg;
            MyHttpGet(curUrl.toStdString(), 300, recv_msg);
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(recv_msg.data(), &err);
            if(err.error == QJsonParseError::NoError)
            {
                if(doc.isObject() && !doc.isEmpty())
                {
                    QJsonObject jsObj = doc.object();
                    if(jsObj.contains("code") && jsObj.value("code").toInt() == 0)
                    {
                        m_lstCategory = jsObj.value("msg").toArray().toVariantList();
                    }
                }
            }
        }

        // from qrc
        if(m_lstCategory.isEmpty())
        {
            QFile categoryFile(":/support/SoftwareCategoryAll.list");
            categoryFile.open(QIODevice::ReadOnly | QIODevice::Text);
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(categoryFile.readAll().data(), &err);
            if(err.error == QJsonParseError::NoError)
            {
                if(doc.isObject() && !doc.isEmpty())
                {
                    QJsonObject jsObj = doc.object();
                    if(jsObj.contains("code") && jsObj.value("code").toInt() == 0)
                    {
                        m_lstCategory = jsObj.value("msg").toArray().toVariantList();
                    }
                }
            }
            categoryFile.close();
        }
    }

    return m_lstCategory;
}

QVariantList &DataControl::GetPackage(const QString szCategoryID)
{
    if(0 == m_szCategoryID.compare(szCategoryID, Qt::CaseInsensitive))
    {
        return m_lstPackage;
    }

    m_lstPackage.clear();
    m_szCategoryID = szCategoryID;

    if(szCategoryID.contains("search:", Qt::CaseInsensitive))
    {
        QStringList list = szCategoryID.split(":", QString::SkipEmptyParts, Qt::CaseInsensitive);
        if(list.size() == 1)
        {
            return m_lstPackage;
        }

        QString szSearchKey = list.at(1);
        QVariantList lstNamePackage;
        QVariantList lstBriefPackage;
        QVariantList lstDescriptionPackage;

        // from local
        bool isExistNullListFile = false;
        foreach(QVariant category, m_lstCategory)
        {
            QVariantList lstTempPackage;
            QString szTempCategoryName = category.toMap().value("id").toString();
            Storage::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", QString("SoftwareCategory") + szTempCategoryName + ".list"), lstTempPackage);
            if(lstTempPackage.isEmpty())
            {
                isExistNullListFile = true;
            }
            foreach(QVariant item, lstTempPackage)
            {
                QVariantMap package = item.toMap();
                if(package.value("name").toString().contains(szSearchKey, Qt::CaseInsensitive) || szSearchKey.contains(package.value("name").toString(), Qt::CaseInsensitive))
                {
                    lstNamePackage.append(item);
                    continue;
                }
                if(package.value("brief").toString().contains(szSearchKey, Qt::CaseInsensitive))
                {
                    lstBriefPackage.append(item);
                    continue;
                }
            }
        }
        if(isExistNullListFile)
        {
            if(lstNamePackage.size() + lstBriefPackage.size() >= 20)
            {
                InitSoftWareOrderBySize(lstNamePackage);
                m_lstPackage = lstNamePackage;
                InitSoftWareOrderBySize(lstBriefPackage);
                m_lstPackage += lstBriefPackage;
            }
        }
        else
        {
            InitSoftWareOrderBySize(lstNamePackage);
            m_lstPackage = lstNamePackage;
            InitSoftWareOrderBySize(lstBriefPackage);
            m_lstPackage += lstBriefPackage;
        }

        // from server
        if(m_lstPackage.isEmpty())
        {
            QString curUrl = "";
            curUrl = "http://";
            curUrl += GLOBAL::_SERVER_URL;
            curUrl += ("/api/swmgr?type=search&count=200&content=" + szSearchKey);
            std::string recv_msg;
            MyHttpGet(curUrl.toStdString(), 300, recv_msg);
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(recv_msg.data(), &err);
            if(err.error == QJsonParseError::NoError)
            {
                if(doc.isObject() && !doc.isEmpty())
                {
                    QJsonObject jsObj = doc.object();
                    if(jsObj.contains("code") && jsObj.value("code").toInt() == 0)
                    {
                        QVariantList &lstTempPackage = jsObj.value("msg").toArray().toVariantList();
                        lstNamePackage.clear();
                        lstBriefPackage.clear();
                        lstDescriptionPackage.clear();
                        foreach(QVariant item, lstTempPackage)
                        {
                            QVariantMap package = item.toMap();
                            if(package.value("name").toString().contains(szSearchKey, Qt::CaseInsensitive) || szSearchKey.contains(package.value("name").toString(), Qt::CaseInsensitive))
                            {
                                lstNamePackage.append(item);
                                continue;
                            }
                            if(package.value("brief").toString().contains(szSearchKey, Qt::CaseInsensitive))
                            {
                                lstBriefPackage.append(item);
                                continue;
                            }
                            lstDescriptionPackage.append(item);
                        }

                        InitSoftWareOrderBySize(lstNamePackage);
                        m_lstPackage = lstNamePackage;
                        InitSoftWareOrderBySize(lstBriefPackage);
                        m_lstPackage += lstBriefPackage;
                        InitSoftWareOrderBySize(lstDescriptionPackage);
                        m_lstPackage += lstDescriptionPackage;
                    }
                }
            }
        }
    }
    else
    {
        Storage::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", QString("SoftwareCategory") + szCategoryID.toUpper() + ".list"), m_lstPackage);
        if(m_lstPackage.isEmpty())
        {
            QString curUrl = "";
            curUrl = "http://";
            curUrl += GLOBAL::_SERVER_URL;
            curUrl += "/api/swmgr?type=";
            if(szCategoryID.compare("top", Qt::CaseInsensitive) == 0)
            {
                curUrl += ("top&count=100&start=0");
            }
            else if(szCategoryID.compare("hot", Qt::CaseInsensitive) == 0)
            {
                curUrl += ("hot&count=200&start=0");
            }
            else
            {
                curUrl += ("category&id=" + szCategoryID + "&count=200&start=0");
            }

            std::string recv_msg;
            MyHttpGet(curUrl.toStdString(), 300, recv_msg);
            QJsonParseError err;
            QJsonDocument doc = QJsonDocument::fromJson(recv_msg.data(), &err);
            if(err.error == QJsonParseError::NoError)
            {
                if(doc.isObject() && !doc.isEmpty())
                {
                    QJsonObject jsObj = doc.object();
                    if(jsObj.contains("code") && jsObj.value("code").toInt() == 0)
                    {
                        m_lstPackage = jsObj.value("msg").toArray().toVariantList();
                    }
                }
            }
        }
    }

    return m_lstPackage;
}

void DataControl::GetPackage2(QString strCategoryID, QVariantList &lstPackage)
{
    lstPackage.clear();
    Storage::LoadArrayOfSoftwareList(ConfOperation::Root().getSubpathFile("Data", QString("SoftwareCategory") + strCategoryID + ".list"), lstPackage);
}

mapSoftwareList &DataControl::getInstalledSoftware() {
    return _mapInstalledSoftwares;
}

QMap<QString,QVariantMap> &DataControl::getUnstallSoftInfo() {
    return _mapUnstallInfoSoft;  //package list by category
}

QVector<QString> &DataControl::getUnstallSoftInfoIdx()
{
    return _VectorInfoSoft;
}

PackageRunner &DataControl::getPackageRunner()
{
    return _DownloadTasks;
}

UserInfo &DataControl::getUserInstance()
{
    return _user;
}

void DataControl::getDownLoadFolder()
{
    emit sigDownLoadFilePath();
}

bool DataControl::initAll() {
    if(m_pThreadToPullSoftwareList = new ThreadToPullSoftwareList())//拉取软件列表。
    {
        m_pThreadToPullSoftwareList->start();
    }

    LoadSettingProfile();

    //InstalledSoftwareChanged();

    //UnstalledSoftInfo();

    if(_UserInRunner = new UserInfoManager())
    {
        _UserInRunner->SetObjects(this, &_user);
    }

    if(_TaskRunner = new TaskManager())
    {
        _TaskRunner->SetObjects(this, &_DownloadTasks);
    }

    if(m_pUpgradeData = new UpgradeData())
    {
        m_pUpgradeData->Init(this);
        if(m_pUpgradeDataThread = new QThread())
        {
            m_pUpgradeData->moveToThread(m_pUpgradeDataThread);
        }
    }

    if(m_pUpgradeHandler = new UpgradeHandler(this))
    {
        m_pUpgradeHandler->Init(this);
    }

    if(m_pUninstallSoftware = new UninstallSoftware())
    {
        m_pUninstallSoftware->Init(this);
        if(m_pUninstallSoftwareThread = new QThread())
        {
            m_pUninstallSoftware->moveToThread(m_pUninstallSoftwareThread);
        }
    }

    return true;
}

void DataControl::StartUserService()
{
    if(_UserInRunner)
    {
        _UserInRunner->start();
    }
}

void DataControl::StartDownloadService()
{
    if(_TaskRunner)
    {
        _TaskRunner->start();
    }
}

void DataControl::StartUpgradeService()
{
    if(m_pUpgradeData && m_pUpgradeDataThread)
    {
        m_pUpgradeDataThread->start();
        m_pUpgradeData->StartTimer();
    }
}

void DataControl::StartUninstallService()
{
    if(m_pUninstallSoftware && m_pUninstallSoftwareThread)
    {
        m_pUninstallSoftwareThread->start();
        m_pUninstallSoftware->StartTimer();
    }
}

void DataControl::unInit() {
    if(_UserInRunner)
    {
        if(_UserInRunner->isRunning())
        {
            _UserInRunner->exit();
            _UserInRunner->wait();
        }
        _UserInRunner->deleteLater();
    }

    if(_TaskRunner)
    {
        if(_TaskRunner->isRunning())
        {
            _TaskRunner->exit();
            _TaskRunner->wait();
        }
        _TaskRunner->deleteLater();
    }

    if(m_pUpgradeData)
    {
        m_pUpgradeData->StopTimer();
    }

    if(m_pUpgradeDataThread)
    {
        if(m_pUpgradeDataThread->isRunning())
        {
            m_pUpgradeDataThread->exit();
            m_pUpgradeDataThread->wait();
        }
        m_pUpgradeDataThread->deleteLater();
    }

    if(m_pUpgradeData)
    {
        m_pUpgradeData->deleteLater();
        m_pUpgradeData = NULL;
    }

    if(m_pUpgradeHandler)
    {
        m_pUpgradeHandler->deleteLater();
    }

    if(m_pUninstallSoftware)
    {
        m_pUninstallSoftware->StopTimer();
    }

    if(m_pUninstallSoftwareThread)
    {
        if(m_pUninstallSoftwareThread->isRunning())
        {
            m_pUninstallSoftwareThread->exit();
            m_pUninstallSoftwareThread->wait();
        }
        m_pUninstallSoftwareThread->deleteLater();
    }

    if(m_pUninstallSoftware)
    {
        m_pUninstallSoftware->deleteLater();
        m_pUninstallSoftware = NULL;
    }

    //if(m_pThreadToPullSoftwareList)   //此处可以不用关闭线程，带主程序结束，线程自动结束，下面方法会有等待时间。
    {
   //     m_pThreadToPullSoftwareList->m_bExit = true;
//        if(m_pThreadToPullSoftwareList->isRunning())
//        {
//            m_pThreadToPullSoftwareList->exit();
//            m_pThreadToPullSoftwareList->wait();
//        }
        //m_pThreadToPullSoftwareList->deleteLater();
    }

    m_lstCategory.clear();
    m_lstPackage.clear();

    _mapInstalledSoftwares.clear();
    _mapUnstallInfoSoft.clear();
    _VectorInfoSoft.clear();
}

void DataControl::GetCurInstalledSoftware(mapSoftwareList &mapInstalledSoftwares)
{
    OSSystemWrapper::Instance()->GetSystemInstalledSoftware(mapInstalledSoftwares,0);
}

void DataControl::InstalledSoftwareChanged() {
    _mapInstalledSoftwares.clear();
    OSSystemWrapper::Instance()->GetSystemInstalledSoftware(_mapInstalledSoftwares,0);
}

//卸载相关：
void DataControl::UnstalledSoftInfo()
{
    QByteArray byMD5;
    CGetIconFromFile cicon; //这个类能在这里配合从中获取到每一个软件的图标，并保存到指定路径。
    int i=0;
    mapSoftwareList &mapSoftwares = getInstalledSoftware();//获取成员变量：_mapInstalledSoftwares，该变量保存从注册表中拿到的已装软件的信息，比较简陋。
    _mapUnstallInfoSoft.clear();
    _VectorInfoSoft.clear();

    //循环将mapSoftwares或者说_mapInstalledSoftwares中的数据转换到_mapUnstallInfoSoft，以及_VectorInfoSoft中，保存map中的key，全部是MD5。
    for (mapSoftwareList::iterator item = mapSoftwares.begin(); item != mapSoftwares.end();item++) {
        QVariantMap objParameter;
        QString map=QString::fromStdString(item->first);

        for (ItemProperty::iterator it = item->second.begin(); it != item->second.end(); it++) {
            objParameter.insert(QString::fromStdString(it->first),QTextCodec::codecForLocale()->toUnicode(it->second.data(), it->second.size()));
        }
        QString szKEY = QString::fromStdString(item->second["QuietUninstallString"].size()>0 ? item->second["QuietUninstallString"] : item->second["UninstallString"]);

        byMD5 = QCryptographicHash::hash(szKEY.toUtf8(), QCryptographicHash::Md5).toHex();
        objParameter.insert(QString("uninstallID"), QString(byMD5));

        objParameter.insert(QString("UnIcoID"), QString(byMD5)+".png");
        objParameter.insert(QString("UnIcoPath"), objParameter["DisplayIcon"].toString());
        objParameter.insert(QString("UnIcoFlag"),0);

        cicon.GetFileNameList(objParameter);
        cicon.SaveOneIcon(objParameter);
        cicon.GetFolersize(objParameter);

        _mapUnstallInfoSoft.insert(QString(byMD5),objParameter);
        _VectorInfoSoft.append(QString(byMD5));
        i++;
    }

    cicon.SetFinishFind_Finish();
    cicon.SaveSoftInfo();

    //------py--11-9
    char buf[1024] = {'\0'};
    szAppdataPath.clear();
    GetEnvironmentVariableA("CommonProgramFiles",buf,1024);
    szAppdataPath.append(buf);
    szAppdataPath.append("\\HurricaneTeam\\xbsoftMgr");
    //------

}

void DataControl::UpdateUninstallList()
{
    if(m_pUninstallSoftware)
    {
        m_pUninstallSoftware->SendUninSoftwareList();
    }
}

void DataControl::UninstallSoftwares(QString &strSoftList)
{
    if(m_pUninstallSoftware)
    {
        m_pUninstallSoftware->UninstallSoftwares(strSoftList);
    }
}

QString & DataControl::getszAppdataPath()
{
    return szAppdataPath;
}

void DataControl::reqRefreshSoftList()
{
    InstalledSoftwareChanged();
}

void DataControl::reqLoginUser(QString username,QString password) { emit sigLoginUser(username,password); }
void DataControl::reqRegisteUser(QString username, QString password, QString email, QString strMobile) { emit sigRegisteUser(username,password,email,strMobile); }
void DataControl::reqModifyUserInfo(QVariantMap userinfo) { emit sigModifyUserInfo(userinfo); }
void DataControl::reqQueryUserStatus(){ emit sigQueryUserState(); }
void DataControl::reqClearUserStatus(){ emit sigClearUserState(); }
void DataControl::reqUserPasswdUpdate(QString szOld,QString szNew){ emit sigUserPasswdUpdate(szOld, szNew); }

void DataControl::reqBackupSysSoftListInfo()
{
    emit sigBackupSysSoftListInfo();
}

void DataControl::reqQueryBackupInfo()
{
    emit sigQueryBackupInfo();
}

void DataControl::reqRestoreSysInfo(QString filename)
{
    emit sigRestoreSysInfo(filename);
}

void DataControl::reqQueryAllTaskInfo() { emit sigQueryAllTaskInfo(); }

void DataControl::reqAddTask(QVariantMap task) { emit sigAddTask(task); }
void DataControl::reqAddTasks(QVariantList tasks) { emit sigAddTasks(tasks); }

void DataControl::reqPauseTask(QString szPackageId) { emit sigPauseTask(szPackageId); }
void DataControl::reqPauseAllTask() { emit sigPauseAllTask(); }

void DataControl::reqResumeTask(QString szPackageId) { emit sigResumeTask(szPackageId); }
void DataControl::reqResumeAllTask() { emit sigResumeAllTask(); }

void DataControl::reqRemoveTask(QString szPackageId) { emit sigRemoveTask(szPackageId); }
void DataControl::reqRemoveAllTask() { emit sigRemoveAllTask(); }

void DataControl::reqUpgradeData() { emit sigRequestUpgradeData(); }

