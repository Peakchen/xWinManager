#include "PackageRunner.h"
#include "swmgrapp.h"
#include "character.h"
#include "Storage.h"
#include "LaunchThread.h"
#include <QtDebug>

PackageRunner::PackageRunner(QObject *parent) : QObject(parent)
{
    m_bPolling = false;
    m_pInstaller = NULL;
    m_pCurrentItemData = NULL;
}

bool PackageRunner::Init()
{
    if(!InitMiniXL())
    {
        emit sigCrash();
        return false;
    }

    m_pInstaller = new QProcess(this);

    QObject::connect(this, SIGNAL(installTaskStart()), SLOT(PeriodInstallTask()));
    QObject::connect(m_pInstaller, SIGNAL(finished(int, QProcess::ExitStatus)), this, SLOT(InstallerFinished(int, QProcess::ExitStatus)));
    Storage::LoadDownloadData(true, ConfOperation::Root().getSubpathFile("Conf", "download.dat"), m_mapDownloadData);
    InitBak();
    return true;
}

void PackageRunner::UnInit()
{
    UnloadDll();
    DestoryDownloadData();
    if(!m_pInstaller)
    {
        delete m_pInstaller;
        m_pInstaller = NULL;
    }
    m_pCurrentItemData = NULL;
}

bool PackageRunner::InitMiniXL()
{
    if(!LoadDll())
    {
        UnloadDll();
        return false;
    }
    return true;
}

bool PackageRunner::LoadDll()
{
    WCHAR szDllpath[512] = { 0 };
    QString szLoadPath = GLOBAL::_DY_DIR_RUNNERSELF + "xbspeed";
    szLoadPath.append( QDir::separator() + QString("xldl.dll"));
    szLoadPath = QDir::toNativeSeparators(szLoadPath);

    StrCpyW(szDllpath, szLoadPath.toStdWString().data());
    try
    {
        m_downloadWapper = new DownWrapper(szDllpath);
    }
    catch(wchar_t e[])
    {
        m_downloadWapper = NULL;
        wchar_t msg[1024] = {0};
        StrCpyW(msg, e);
        StrCatW(msg, L"\r\n");
        StrCatW(msg, L"乐网软件已被恶意破坏, 请前往官网下载最新版乐网软件");
        MessageBox(NULL, msg, L"乐网软件", MB_ICONEXCLAMATION);
        return false;
    }
    return m_downloadWapper->Init();
}

void PackageRunner::UnloadDll()
{
    if(!m_downloadWapper)
    {
        return;
    }
    m_downloadWapper->UnInit();
    delete m_downloadWapper;
    m_downloadWapper = NULL;
}

void PackageRunner::InitBak()
{
    QString defaultConf = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QString curDatFile = defaultConf + "Backup.dat";

    QFile file(curDatFile);
    if(!file.open(QIODevice::ReadOnly))
    {
        if(!file.open(QIODevice::WriteOnly)){
            qDebug() << "WRITE failded.";
            return ;
        }
    }
    file.close();

    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "Backup.dat");
    QVariantList objectList;
    Storage::LoadFromConfArray(szFile, objectList);
    int count = 0;
    for(QVariantList::iterator it = objectList.begin(); it != objectList.end(); it++)
    {
        QVariantMap object = it->toMap();
        if(object.isEmpty())
        {
            continue;
        }
        LPBackupData pBackup = new BackupDataItem;
        if(!pBackup){
            continue;
        }
        pBackup->id = object.value("id").toString();
        pBackup->filename = object.value("filename").toString();

        QDateTime current_date_time = QDateTime::currentDateTime();
        QString current_date = current_date_time.toString("yyyy_MM_dd_hh_mm_ss_ddd");
        QString key = tr("date %1: %2 ;").arg(QString::number((++ count))).arg(current_date);
        m_mapBackupData.insert(key, pBackup);
    }
    qDebug()<<"_______InitBak_______"<<m_mapBackupData.count();
}

void PackageRunner::EncodeToVariantMap(const LPDownloadItemData &pItemData, QVariantMap &object)
{
    if(!pItemData)
        return;

    object.clear();
    object.insert(QString("id"), QVariant::fromValue(pItemData->id));
    object.insert(QString("category"), QVariant::fromValue(pItemData->category));
    object.insert(QString("name"), QVariant::fromValue(pItemData->name));
    object.insert(QString("largeIcon"), QVariant::fromValue(pItemData->largeIcon));
    object.insert(QString("brief"), QVariant::fromValue(pItemData->brief));
    object.insert(QString("size"), QVariant::fromValue(pItemData->size));
    object.insert(QString("downloadUrl"), QVariant::fromValue(pItemData->downloadUrl));
    object.insert(QString("rating"), QVariant::fromValue(pItemData->rating));
    object.insert(QString("isAd"), QVariant::fromValue(pItemData->isAd));
    object.insert(QString("priceInfo"), QVariant::fromValue(pItemData->priceInfo));
    object.insert(QString("updateTime"), QVariant::fromValue(pItemData->updateTime));
    object.insert(QString("versionName"), QVariant::fromValue(pItemData->versionName));
    object.insert(QString("packageName"), QVariant::fromValue(pItemData->packageName));
    object.insert(QString("downloadPath"), QVariant::fromValue(pItemData->downloadPath));
    object.insert(QString("percent"), QVariant::fromValue(pItemData->percent));
    object.insert(QString("speed"), QVariant::fromValue(pItemData->speed));
    object.insert(QString("status"), QVariant::fromValue(pItemData->status));
    object.insert(QString("unfinishedCount"), QVariant::fromValue(pItemData->unfinishedCount));
    object.insert(QString("finishedCount"), QVariant::fromValue(pItemData->finishedCount));
}

void PackageRunner::EncodeToDownloadItemData(const QVariantMap &object, LPDownloadItemData &pItemData)
{
    if(object.isEmpty())
        return;

    pItemData->id = object.find("id").value().toString();
    pItemData->category = object.find("category").value().toString();
    pItemData->name = object.find("name").value().toString();
    pItemData->largeIcon = object.find("largeIcon").value().toString();
    pItemData->brief = object.find("brief").value().toString();
    pItemData->size = object.find("size").value().toDouble();
    pItemData->downloadUrl = object.find("downloadUrl").value().toString();
    pItemData->rating = object.find("rating").value().toInt();
    pItemData->isAd = object.find("isAd").value().toBool();
    pItemData->priceInfo = object.find("priceInfo").value().toFloat();
    pItemData->updateTime = object.find("updateTime").value().toString();
    pItemData->versionName = object.find("versionName").value().toString();
    pItemData->packageName = object.find("packageName").value().toString();
    pItemData->downloadPath = object.find("downloadPath").value().toString();
    pItemData->percent = object.find("percent").value().toFloat();
    pItemData->speed = object.find("speed").value().toLongLong();
    pItemData->status = object.find("status").value().toInt();
    pItemData->unfinishedCount = object.find("unfinishedCount").value().toInt();
    pItemData->finishedCount = object.find("finishedCount").value().toInt();
}

void PackageRunner::DestoryItemTask(LPDownloadItemData &pItemData)
{
    if(!pItemData || !pItemData->hTaskHandle)
        return;

    m_downloadWapper->TaskPause(pItemData->hTaskHandle);
    m_downloadWapper->TaskDelete(pItemData->hTaskHandle);
    pItemData->hTaskHandle = NULL;
    m_downloadWapper->DelTempFile(pItemData->downTaskParam);
    memset(&(pItemData->downTaskParam), 0x00, sizeof(pItemData->downTaskParam));

    return;
}

void PackageRunner::DestoryItemData(LPDownloadItemData &pItemData)
{
    if(!pItemData)
    {
        return;
    }

    DestoryItemTask(pItemData);

    delete pItemData;
    pItemData = NULL;
}

void PackageRunner::DestoryDownloadData()
{
    Storage::SaveDownloadData(ConfOperation::Root().getSubpathFile("Conf", "download.dat"), m_mapDownloadData);
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        DestoryItemData(it.value());
        it = m_mapDownloadData.erase(it);
    }
}

void PackageRunner::SendSignal(const LPDownloadItemData &pItemData)
{
    if(!pItemData)
    {
        return;
    }

    int finishedCount = 0;
    int unfinishedCount = 0;
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end(); it++)
    {
        LPDownloadItemData pTempItemData = it.value();
        if(!pTempItemData || pTempItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
        {
            continue;
        }
        if(pTempItemData->status < PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            unfinishedCount++;
        }
        else
        {
            finishedCount++;
        }
    }
    pItemData->unfinishedCount = unfinishedCount;
    pItemData->finishedCount = finishedCount;

    QVariantMap object;
    EncodeToVariantMap(pItemData, object);
    if(object.isEmpty())
    {
        return;
    }

    QString szPngPath = GLOBAL::_PNGDIR +  pItemData->id + ".png";
    if(IsFileExist(szPngPath))
    {
        object["largeIcon"] = QVariant::fromValue("file:///" + szPngPath.replace("\\", "/"));
    }

    emit sigUpdateOneTaskInfo(object);
}

void PackageRunner::DownloadDataPoll()
{
    if(m_bPolling)
    {
        return;
    }

    m_bPolling = true;
    static bool bWrite = true;

    LPDownloadItemData pItemData = NULL;

    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        if(!(pItemData = it.value()))
        {
            it = m_mapDownloadData.erase(it);
            continue;
        }

        if(pItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
        {
            DestoryItemData(pItemData);
            it = m_mapDownloadData.erase(it);
            continue;
        }

        if(pItemData->status >= PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            DestoryItemTask(pItemData);
            it++;
            if(pItemData->status == PACKAGE_STATUS_DOWNLOAD_FINISH)
            {
                LaunchThread *pThread = new LaunchThread();
                if(pThread)
                {
                    if(pThread->Init(pItemData))
                    {
                        QObject::connect(pThread, SIGNAL(sigDeleteLaunchThread(LaunchThread *)), this, SLOT(DeleteLaunchThread(LaunchThread *)), Qt::QueuedConnection);
                        pThread->start();
                        pItemData->status = PACKAGE_STATUS_INSTALL_START;
                    }
                    else
                    {
                        delete pThread;
                        pThread = NULL;
                    }
                }
            }
            SendSignal(pItemData);
            continue;
        }

        if(pItemData->status == PACKAGE_STATUS_NEW_DOWNLOAD ||
                pItemData->status == PACKAGE_STATUS_DOWNLOAD_START ||
                pItemData->status == PACKAGE_STATUS_DOWNLOAD_START_ERR ||
                pItemData->status == PACKAGE_STATUS_DOWNLOAD_PAUSE)
        {
            if(!pItemData->hTaskHandle)
            {
                StrCpyW(pItemData->downTaskParam.szTaskUrl, pItemData->downloadUrl.toStdWString().data());
                StrCpyW(pItemData->downTaskParam.szSavePath, pItemData->downloadPath.toStdWString().data());
                StrCpyW(pItemData->downTaskParam.szFilename, pItemData->packageName.toStdWString().data());
                pItemData->downTaskParam.IsOnlyOriginal = FALSE;
                pItemData->downTaskParam.DisableAutoRename = TRUE;
                pItemData->downTaskParam.IsResume = TRUE;          // use resume

                if(!(pItemData->hTaskHandle = m_downloadWapper->TaskCreate(pItemData->downTaskParam)))
                {
                    it++;
                    continue;
                }
            }
        }

        if(pItemData->status == PACKAGE_STATUS_NEW_DOWNLOAD)
        {
            if(m_downloadWapper->TaskStart(pItemData->hTaskHandle))
            {
                pItemData->status = PACKAGE_STATUS_DOWNLOAD_START;
            }
            else
            {
                pItemData->status = PACKAGE_STATUS_DOWNLOAD_START_ERR;
            }

            it++;
            continue;
        }

        DownTaskInfo info;
        memset( &info, 0, sizeof(info));
        if(!m_downloadWapper->TaskQueryEx(pItemData->hTaskHandle, info))
        {
            it++;
            continue;
        }

        pItemData->speed = info.nSpeed;
        pItemData->percent = info.fPercent;

        if(pItemData->status == PACKAGE_STATUS_DOWNLOAD_PAUSE)
        {
            pItemData->speed = 0;
            it++;
            continue;
        }

        if(info.stat == NOITEM && info.fPercent == 1.0f && info.nSpeed == 0)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_FINISH;
        }
        else if(info.stat == TSC_ERROR)
        {
            ;
        }
        else if(info.stat == TSC_PAUSE)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_PAUSE;
        }
        else if(info.stat == TSC_DOWNLOAD)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_START;
        }
        else if(info.stat == TSC_STARTPENDING)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_START;
        }
        else if(info.stat == TSC_COMPLETE)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_FINISH;
        }
        else if(TSC_STOPPENDING)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_PAUSE;
        }

        SendSignal(pItemData);
        it++;
    }

    if(m_mapDownloadData.size())
    {
        Storage::SaveDownloadData(ConfOperation::Root().getSubpathFile("Conf", "download.dat"), m_mapDownloadData);
        bWrite = true;
    }
    else
    {
        if(bWrite)  // 避免重复写空的数据, 只写一次
        {
            Storage::SaveDownloadData(ConfOperation::Root().getSubpathFile("Conf", "download.dat"), m_mapDownloadData);
            bWrite = false;
        }
    }

    m_bPolling = false;      // 下个定时器可以工作
}

void PackageRunner::PeriodInstallTask()
{
    if(!m_pInstaller || m_pInstaller->state() != QProcess::NotRunning)
    {
        return;
    }

    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData || pItemData->status != PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            it++;
            continue;
        }

        // package exist or not
        QString isSlash = pItemData->downloadPath.right(1);
        QFileInfo *fileInfo = NULL;
        QString sWPath = "";
        if(isSlash.compare(QDir::separator()) == 0){
            sWPath = pItemData->downloadPath + pItemData->packageName;
            fileInfo = new QFileInfo(sWPath);
        }else{
            sWPath = pItemData->downloadPath + QDir::separator() + pItemData->packageName;
            fileInfo = new QFileInfo(sWPath);
        }
        if(fileInfo->exists())
        {
            m_pCurrentItemData = pItemData;
            if(sWPath.contains(".msi", Qt::CaseInsensitive)){
                sWPath = QDir::toNativeSeparators(sWPath);
                m_pInstaller->start("msiexec", QStringList()<<"/i"<<sWPath<<"/qn"); //msi文件使用qn静默安装
            }else{
                m_pInstaller->start(sWPath, QStringList());
            }
            if(!m_pInstaller->waitForStarted(1000))       // start failed
            {
                pItemData->status = PACKAGE_STATUS_INSTALL_START_ERR;
                SendSignal(pItemData);
            }
            else
            {
                pItemData->status = PACKAGE_STATUS_INSTALL_START;
                SendSignal(pItemData);
                break;
            }
        }
        else        // can not find package, need to re-download and re-install
        {
            QString id = pItemData->id;
            QString category = pItemData->category;
            DestoryItemData(pItemData);
            it = m_mapDownloadData.erase(it);
            SwmgrApp::Instance()->requestStartInstallPackage(category, id);
            continue;
        }
    }
}

void PackageRunner::InstallerFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    Q_UNUSED(exitCode);
    Q_UNUSED(exitStatus);

    if(!m_pInstaller || !m_pCurrentItemData)
    {
        return;
    }

    // check software install successfully or not
    // get installed software
    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);
    bool isExist = false;

    QString name = m_pCurrentItemData->name;
    QString version = m_pCurrentItemData->versionName;

    for(mapSoftwareList::iterator it = mapInstalledSoftwares.begin(); it != mapInstalledSoftwares.end(); it++)
    {
        QString install_name = QString::fromStdString(GBKToUTF8(it->second["DisplayName"]));
        QString install_version = QString::fromStdString(GBKToUTF8(it->second["DisplayVersion"]));
        if(install_name.isEmpty() || (!name.contains(install_name, Qt::CaseInsensitive) && !install_name.contains(name, Qt::CaseInsensitive)))
        {
            continue;
        }

        isExist = true;

        if(install_version.isEmpty())
        {
            m_pCurrentItemData->status = PACKAGE_STATUS_INSTALL_FINISH;
            break;
        }

        int ret = CompareVersion(install_version,  version);
        if(ret >= 0)
        {
            m_pCurrentItemData->status = PACKAGE_STATUS_INSTALL_FINISH;
        }
        else
        {
            m_pCurrentItemData->status = PACKAGE_STATUS_INSTALL_CANCEL;
        }
        break;
    }

    if(!isExist)
    {
        m_pCurrentItemData->status = PACKAGE_STATUS_INSTALL_CANCEL;
    }

    SendSignal(m_pCurrentItemData);
    m_pCurrentItemData = NULL;

    emit installTaskStart();
}

// add multi-task
void PackageRunner::reqAddTasks(QVariantList taskList)
{
    foreach(QVariant task, taskList)
    {
        if(task.type() == QVariant::Map)
        {
            reqAddTask(task.toMap());
        }
    }
}

// add one task
void PackageRunner::reqAddTask(QVariantMap task)
{
    if(task.isEmpty())
    {
        return;
    }

    QString key = task.value("id").toString() + ":;" + task.value("name").toString() + ":;" + task.value("versionName").toString();
    DownloadIterator it = m_mapDownloadData.find(key);
    if(it != m_mapDownloadData.end())  // already exist
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData || pItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
        {
            it = m_mapDownloadData.erase(it);
            AddNewItemData(task);
            return;
        }
        if(pItemData->status == PACKAGE_STATUS_INSTALL_START || pItemData->status == PACKAGE_STATUS_INSTALL_START_ERR)
        {
            return;
        }

        if(pItemData->status > PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_FINISH;
        }
    }
    else
    {
        AddNewItemData(task);
    }
}

void PackageRunner::AddNewItemData(const QVariantMap &task)
{
    LPDownloadItemData pItemData = new DownloadItemData();
    pItemData->id = task.value("id").toString();
    pItemData->category = task.value("category").toString();
    pItemData->name = task.value("name").toString();
    pItemData->largeIcon = task.value("largeIcon").toString();
    pItemData->brief = task.value("brief").toString();
    pItemData->size = task.value("size").toDouble();

    pItemData->downloadUrl = task.value("ptdownloadUrl").toString();
    if(pItemData->downloadUrl.isEmpty() || !pItemData->downloadUrl.compare("0"))
    {
        pItemData->downloadUrl = task.value("downloadUrl").toString();
    }

    pItemData->rating = task.value("rating").toInt();
    pItemData->isAd = task.value("isAd").toBool();
    pItemData->priceInfo = task.value("priceInfo").toFloat();
    pItemData->updateTime = task.value("updateTime").toString();
    pItemData->versionName = (task.value("versionName").isNull() || (task.value("versionName").type() == QVariant::String && task.value("versionName").toString().size() == 0)) ? QString("1.0.0.0") : task.value("versionName").toString();
    pItemData->percent = 0.0f;
    pItemData->speed = 0;

    // pItemData->packageName = task.value("packageName").toString();
    // 常用安装包后缀
    QString suffix = "";
    if(pItemData->downloadUrl.contains(".exe", Qt::CaseInsensitive))
    {
        suffix = ".exe";
    }
    else if(pItemData->downloadUrl.contains(".bin", Qt::CaseInsensitive))
    {
        suffix = ".bin";
    }
    else if(pItemData->downloadUrl.contains(".msi", Qt::CaseInsensitive))
    {
        suffix = ".msi";
    }
    else if(pItemData->downloadUrl.contains(".cab", Qt::CaseInsensitive))
    {
        suffix = ".cab";
    }
    else if(pItemData->downloadUrl.contains(".iso", Qt::CaseInsensitive))
    {
        suffix = ".iso";
    }
    else if(pItemData->downloadUrl.contains(".rar", Qt::CaseInsensitive))
    {
        suffix = ".rar";
    }
    else if(pItemData->downloadUrl.contains(".zip", Qt::CaseInsensitive))
    {
        suffix = ".zip";
    }
    else
    {
        suffix = ".exe";

    }
    // 组建安装包名称
    int arch = task.value("arch").toInt();
    if(arch == 64)
    {
        pItemData->packageName = pItemData->name + "_" + pItemData->versionName + "_x64" + suffix;
    }
    else if(arch == 32)
    {
        pItemData->packageName = pItemData->name + "_" + pItemData->versionName + "_x86" + suffix;
    }
    else if(arch == 96)
    {
        pItemData->packageName = pItemData->name + "_" + pItemData->versionName + "_x86_64" + suffix;
    }

    pItemData->downloadPath = SwmgrApp::GetDownloadPath();
    pItemData->status = PACKAGE_STATUS_NEW_DOWNLOAD;
    pItemData->hTaskHandle = NULL;

    QString key = pItemData->id + ":;" + pItemData->name + ":;" + pItemData->versionName;
    m_mapDownloadData.insert(key, pItemData);
}

void PackageRunner::SaveBakDataItem()
{
    QString szFile = ConfOperation::Root().getSubpathFile("Conf", "Backup.dat");
    QVariantList objectList;
    for(QBackupIterator it = m_mapBackupData.begin(); it != m_mapBackupData.end();)
    {
        LPBackupData pItemData = it.value();
        if(!pItemData){
            it = m_mapBackupData.erase(it);
            continue;
        }
        QVariantMap object;
        QString filename = pItemData->filename;
        QString id = pItemData->id;
        object.insert(QString("filename"), filename);
        object.insert(QString("id"), id);
        if(!object.isEmpty()){
            objectList.append(object);
        }
        it++;
    }
    Storage::SaveToConfArray(szFile, objectList);
}

// resume one task
void PackageRunner::reqResumeTask(QString id)
{
    bool isExist = true;
    QString key = "";
    DownloadIterator it = m_mapDownloadData.begin();
    for(; it != m_mapDownloadData.end(); it++)
    {
        if(it.key().contains(id, Qt::CaseInsensitive))
        {
            isExist = true;
            break;
        }
    }
    if(!isExist)
    {
        return;
    }

    LPDownloadItemData pItemData = it.value();
    if(!pItemData)
    {
        it = m_mapDownloadData.erase(it);
        return;
    }

    if(m_downloadWapper->TaskStart(pItemData->hTaskHandle))
    {
        pItemData->status = PACKAGE_STATUS_DOWNLOAD_START;
    }
    else
    {
        pItemData->status = PACKAGE_STATUS_DOWNLOAD_START_ERR;
    }
    SendSignal(pItemData);
}

// resume all task
void PackageRunner::reqResumeAllTask()
{
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData)
        {
            it = m_mapDownloadData.erase(it);
            continue;
        }
        if(pItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL || pItemData->status >= PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            it++;
            continue;
        }
        if(m_downloadWapper->TaskStart(pItemData->hTaskHandle))
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_START;
        }
        else
        {
            pItemData->status = PACKAGE_STATUS_DOWNLOAD_START_ERR;
        }
        SendSignal(pItemData);
        it++;
    }
}

// pause one task
void PackageRunner::reqPauseTask(QString id)
{
    bool isExist = true;
    QString key = "";
    DownloadIterator it = m_mapDownloadData.begin();
    for(; it != m_mapDownloadData.end(); it++)
    {
        if(it.key().contains(id, Qt::CaseInsensitive))
        {
            isExist = true;
            break;
        }
    }
    if(!isExist)
    {
        return;
    }

    LPDownloadItemData pItemData = it.value();
    if(!pItemData)
    {
        it = m_mapDownloadData.erase(it);
        return;
    }

    m_downloadWapper->TaskPause(pItemData->hTaskHandle);
    pItemData->status = PACKAGE_STATUS_DOWNLOAD_PAUSE;
    pItemData->speed = 0;
    SendSignal(pItemData);
}

// pause all task
void PackageRunner::reqPauseAllTask()
{
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData)
        {
            it = m_mapDownloadData.erase(it);
            continue;
        }
        if(pItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL || pItemData->status >= PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            it++;
            continue;
        }

        m_downloadWapper->TaskPause(pItemData->hTaskHandle);
        pItemData->status = PACKAGE_STATUS_DOWNLOAD_PAUSE;
        pItemData->speed = 0;
        SendSignal(pItemData);
        it++;
    }
}

// cancel one task
void PackageRunner::reqRemoveTask(QString id)
{
    bool isExist = true;
    QString key = "";
    DownloadIterator it = m_mapDownloadData.begin();
    for(; it != m_mapDownloadData.end(); it++)
    {
        if(it.key().contains(id, Qt::CaseInsensitive))
        {
            isExist = true;
            break;
        }
    }
    if(!isExist)
    {
        return;
    }

    LPDownloadItemData pItemData = it.value();
    if(!pItemData)
    {
        it = m_mapDownloadData.erase(it);
        return;
    }

    DestoryItemTask(pItemData);
    pItemData->status = PACKAGE_STATUS_DOWNLOAD_CANCEL;
    SendSignal(pItemData);
}

// cancel all task
void PackageRunner::reqRemoveAllTask()
{
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData)
        {
            it = m_mapDownloadData.erase(it);
            continue;
        }

        if(pItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL || pItemData->status >= PACKAGE_STATUS_DOWNLOAD_FINISH)
        {
            it++;
            continue;
        }

        DestoryItemTask(pItemData);
        pItemData->status = PACKAGE_STATUS_DOWNLOAD_CANCEL;
        SendSignal(pItemData);
        it++;
    }
}

/****************
 *@brief : back up sys, so as to backup download.dat
 *@param : no
 *@author: cqf
 ****************/
void PackageRunner::reqBackupSysSoftListInfo()
{
    QDateTime current_date_time = QDateTime::currentDateTime();
    static int count = 0;
    QString current_date = current_date_time.toString("yyyy_MM_dd_hh_mm_ss_ddd");
    QString fileName = SwmgrApp::GetSoftwareName() + current_date;
    QString defaultConf = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QString curDatFile = defaultConf + fileName.append(".conf");
    //    QDir dir;
    //    dir.mkpath(curDatFile);
    QFile file(curDatFile);
    if(!file.open(QIODevice::WriteOnly))
    {
        qDebug() << "WRITE failded.";
    }
    file.close();
    QDownloadItemDataMap BackupData;
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end();)
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData)
        {
            it = m_mapDownloadData.erase(it);
            continue;
        }
        //if(pItemData->status == PACKAGE_STATUS_INSTALL_FINISH){  // 针对 安装 成功的 软件进行 备份设置
            QString key = pItemData->id + ":;" + pItemData->name + ":;" + pItemData->versionName;
            BackupData.insert(key, pItemData);
       // }
        it++;
    }
    if(!BackupData.isEmpty()){
        Storage::SaveDownloadData(ConfOperation::Root().getSubpathFile("Conf", fileName), BackupData);
    }
    LPBackupData pBackup = new BackupDataItem();
    QString key = tr("date %1: %2 ;").arg(QString::number((++ count))).arg(current_date);

    pBackup->id = current_date;
    pBackup->filename = fileName;
    m_mapBackupData.insert(key, pBackup);

    QVariantMap Fileobj;
    Fileobj.insert("id", key);
    Fileobj.insert("filename", fileName);
    if(Fileobj.isEmpty()){
        return ;
    }
    SaveBakDataItem();
    emit sigupdateBackupSysSoftListInfo(true, Fileobj);
}

/****************
 *@brief : Restore Sys Soft list
 *@param : filename
 *@author: cqf
 ****************/
void PackageRunner::reqRestoreSysInfo(QString filename)
{
    bool rRestoreInfo = false;
    if(!filename.isEmpty()){
        m_mapDownloadData.clear();
        Storage::LoadDownloadData(false, ConfOperation::Root().getSubpathFile("Conf", filename), m_mapDownloadData);
        QString szFile = ConfOperation::Root().getSubpathFile("Conf", "download.dat");
        QFile file(szFile);
        file.remove();
        if(!file.open(QIODevice::WriteOnly))
        {
            qDebug() << "WRITE failded.";
        }
        file.close();
        Storage::SaveDownloadData(szFile, m_mapDownloadData);
        rRestoreInfo = true;
        emit reqAllTaskInfo();
        emit sigReplyRestoreSysInfo(rRestoreInfo);
    }else{
        emit sigReplyRestoreSysInfo(rRestoreInfo);
    }
}

/****************
 *@brief : deal with to Prompt price info and plugin
 *@param : brief, price
 *@author : cqf
 ****************/
void PackageRunner::DealWithPromptPriceInfo(QString Abrief, float Aprice)
{
    QString defaultConf = SwmgrApp::GetProgramProfilePath(SwmgrApp::GetSoftwareName()) + QDir::separator() + QString("Conf") + QDir::separator();
    QFile file(defaultConf + "PromptPriceInfo.conf");
    if(!file.open(QIODevice::ReadOnly))
    {
        qDebug() << "READ failded.";
    }

    QTextStream in( &file);
    QString Promptflag;
    if(!in.atEnd())
    {
        Promptflag = in.readLine();
    }
    file.close();
    // flag : 1 to check price and plugin
    if(Promptflag.toInt())
    {
        QString infoAppend;
        bool bplugin = Abrief.contains("插件", Qt::CaseInsensitive);
        bool bprice = Aprice > 0.00;
        if(bplugin)
        {
            infoAppend = infoAppend + "1";
        }
        if(bprice)
        {
            infoAppend = infoAppend + "0";
        }

        if(!infoAppend.isEmpty())
        {
            emit sigPromptPriceInfo(infoAppend);
        }
    }
}

void PackageRunner::reqAllTaskInfo()
{
    for(DownloadIterator it = m_mapDownloadData.begin(); it != m_mapDownloadData.end(); it++)
    {
        if(it.value() == NULL || it.value()->status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
        {
            continue;
        }

        SendSignal(it.value());
    }
}

void PackageRunner::reqAllBackupInfo()
{
    qDebug()<<"_______reqAllBackupInfo_______"<<m_mapBackupData.count();
    for(QBackupIterator it = m_mapBackupData.begin(); it != m_mapBackupData.end(); it++)
    {
        LPBackupData pItemData = it.value();

        QVariantMap object;
        object.insert(QString("id"), QVariant::fromValue(pItemData->id));
        object.insert(QString("filename"), QVariant::fromValue(pItemData->filename));

        emit sigupdateBackupSysSoftListInfo(false, object);
    }
}

void PackageRunner::DeleteLaunchThread(LaunchThread *pThread)
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
