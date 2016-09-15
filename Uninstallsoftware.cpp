#include <shlobj.h>
#include <string>
#include <time.h>
#include <direct.h>
#include <stdio.h>
#include <io.h>
#include <QMessageBox>
#include "character.h"
#include "DataControl.h"
#include "Uninstallsoftware.h"
#include "character.h"
#include "uninstallsoftthread.h"
#include "logwriter.h"
#include "global.h"

using std::string;
#pragma execution_character_set("utf-8")

//写日志
//int WriteToLog(char* str)
//{
//    FILE* pfile;
//    fopen_s(&pfile, FILE_PATH, "a+");
//    if (pfile == NULL)
//    {
//        return -1;
//    }
//    string strTime = GetTimeNow();
//    fprintf_s(pfile,"time:%s\n",strTime.data());
//    fprintf_s(pfile, "%s---\n", str);
//    fclose(pfile);
//    return 0;
//}

UninstallSoftware::UninstallSoftware(QObject *parent) : QObject(parent)
{
    m_Mutex = CreateMutex(NULL, FALSE, TEXT("SoftListMutex"));
    mutex_soft_file = OpenMutex(MUTEX_ALL_ACCESS, FALSE, TEXT("Global\\MutexLockSoftDate"));
    if (mutex_soft_file == NULL)
    {
        mutex_soft_file = CreateMutex(NULL, FALSE, TEXT("Global\\MutexLockSoftDate"));
    }
    m_bPolling =FALSE;
}

UninstallSoftware::~UninstallSoftware()
{
    if(m_timer.isActive())
    {
        m_timer.stop();
    }
    CloseHandle(m_Mutex);
    CloseHandle(mutex_soft_file);
}
void UninstallSoftware::Init(DataControl* pDataControl)
{
    QObject::connect(this, SIGNAL(sigUpdateUninSoftware(QVariantList)), pDataControl, SIGNAL(sigUpdateUninSoftware(QVariantList)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sigCleanupResidue(QString,QString)), pDataControl, SIGNAL(sigCleanupResidue(QString,QString)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sigUninstallFinished(QString)), pDataControl, SIGNAL(sigUninstallFinished(QString)), Qt::QueuedConnection);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(UpdateSoftwareList()), Qt::QueuedConnection);
    confFilePath.clear();
    char buf[1024] = {'\0'};
    GetEnvironmentVariableA("CommonProgramFiles",buf,1024);
    confFilePath.append(buf);
    confFilePath.append("\\HurricaneTeam\\xbsoftMgr");

    // 还原default.png
    QString szFilePath = GLOBAL::_DY_DIR_RUNNERSELF + "png" + QDir::separator() + "default.png";
    if(!QFile::exists(szFilePath))
    {
        QString szFileDir = GLOBAL::_DY_DIR_RUNNERSELF + "png";
        QDir *tmp = new QDir;
        if(tmp == NULL)
        {
            return;
        }

        if(!tmp->exists(szFileDir))
        {
            if(!tmp->mkdir(szFileDir))
            {
                delete tmp;
                tmp = NULL;
                return;
            }
            QFile::copy(":/png/default.png", szFilePath);
        }
        delete tmp;
        tmp = NULL;
    }
}

void UninstallSoftware::StartTimer()
{
    m_timer.start(1000);
}

void UninstallSoftware::StopTimer()
{
    if(m_timer.isActive())
    {
        m_timer.stop();
    }
}

/*QString softwareParam[]={
    "DisplayName",
    "DisplayVersion",
    "InstallDate",
    "InstallLocation",
    "Publisher",
    "UninstallString",
    "QuietUninstallString",
    "Version",
    "VersionMajor",
    "VersionMinor",
    "WindowsInstaller",
    "EstimatedSize",
    "DisplayIcon",
    "DisplayArch"
};*/

QString softwareParam[]={
    "DisplayName",
    "DisplayVersion",
    "InstallDate",
    "InstallLocation",
    "Publisher",
    "UninstallString",
    "QuietUninstallString",
    "EstimatedSize",
    "DisplayIcon",
};

//暂时用不到，待注释或者删除，，，
void UninstallSoftware::EncodeToVariantList(mapSoftwareList mapUninstallData, QVariantList& objectList)
{
    for(mapSoftwareList::iterator it = mapUninstallData.begin(); it != mapUninstallData.end(); it++)
    {
        QVariantMap object;
        for(int i = 0;i<14;i++)
        {
            //object.insert(QString("DisplayName"),QVariant::fromValue(QString::fromStdString(GBKToUTF8(it->second["DisplayName"]))));
            object.insert(softwareParam[i],QVariant::fromValue(QString::fromStdString(GBKToUTF8(it->second[softwareParam[i].toStdString()]))));
        }
        objectList.append(object);
    }
}

void UninstallSoftware::UpdateSoftwareList()//定时获取注册表信息，以判断是否需要更新内存。
{
    if(m_bPolling)
        return;
    m_bPolling = TRUE;
    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    //对比成员变量 m_mapUninSoftwareList，决定是否更新。
    BOOL isNeedUpdate = FALSE;
    if(m_mapUninSoftwareList.size() == 0)
        isNeedUpdate = TRUE;

    std::map<std::string, ItemProperty>::iterator item;
    if(!isNeedUpdate)
    {
        for(item = mapInstalledSoftwares.begin();item != mapInstalledSoftwares.end();item++)
        {
            std::map<std::string, ItemProperty>::iterator itemOnPage = m_mapUninSoftwareList.find(item->first);
            if(itemOnPage==m_mapUninSoftwareList.end())
            {
                isNeedUpdate = TRUE;
                break;
            }
        }
    }

    if(!isNeedUpdate)
    {
        for(item = m_mapUninSoftwareList.begin();item != m_mapUninSoftwareList.end();item++)
        {
            std::map<std::string, ItemProperty>::iterator itemOnPage = mapInstalledSoftwares.find(item->first);
            if(itemOnPage==mapInstalledSoftwares.end())
            {
                isNeedUpdate = TRUE;
                break;
            }
        }
    }
    //qDebug()<<"--------------------------isNeedUpadate:"<<isNeedUpdate<<"---mapInstalledSoftwares"<<mapInstalledSoftwares.size()<<endl;
    if(isNeedUpdate)
    {
        m_mapUninSoftwareList.clear();
        m_mapUninSoftwareList = mapInstalledSoftwares;
        m_UninsoftList.clear();
        //EncodeToVariantList(m_mapUninSoftwareList, m_UninsoftList);
        GetInstalledSoftwareMap();

        WaitForSingleObject(m_Mutex, INFINITE);
        ProcessUninSoftMap();
        ReleaseMutex(m_Mutex);

        SendUninSoftwareList();
    }
    m_bPolling = FALSE;
}

//向DataControl发送信号，更新卸载页面。
void UninstallSoftware::SendUninSoftwareList()
{
    //qDebug()<<"--------------------------mapInstalledSoftwares"<<m_mapUninSoftwareList.size()<<endl;
    WaitForSingleObject(m_Mutex, INFINITE);
    //qDebug()<<jsArray<<"-------------------------asdasd\n"<<endl;
    if(!jsArray.isEmpty())
        emit sigUpdateUninSoftware(jsArray);
    ReleaseMutex(m_Mutex);
}

//扩展待用
void UninstallSoftware::GetSoftwareList(mapSoftwareList &mapSoftwareList)
{
    WaitForSingleObject(m_Mutex, INFINITE);
    mapSoftwareList =  m_mapUninSoftwareList;
    ReleaseMutex(m_Mutex);
}

void UninstallSoftware::GetInstalledSoftwareMap()
{
    QByteArray byMD5;
    CGetIconFromFile cicon; //这个类能在这里配合从中获取到每一个软件的图标，并保存到指定路径。
    int i = 0;
    m_UninsoftList.clear();
    m_VectorSoftInfo.clear();

    //循环将mapSoftwares或者说_mapInstalledSoftwares中的数据转换到_mapUnstallInfoSoft，以及_VectorInfoSoft中，保存map中的key，全部是MD5。
    for (mapSoftwareList::iterator item = m_mapUninSoftwareList.begin(); item != m_mapUninSoftwareList.end();item++) {
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

        m_UninsoftList.insert(QString(byMD5),objParameter);
        m_VectorSoftInfo.append(QString(byMD5));
        i++;
    }
    cicon.SetFinishFind_Finish();
    cicon.SaveSoftInfo();
}

void UninstallSoftware::getUnstallData(QStringMap &map_use)
{
    WaitForSingleObject(mutex_soft_file, INFINITE);
    QString &szAppdataPath=confFilePath;
    QFile file(szAppdataPath+"/softlist.txt");
    //        QFile file("c:\\2.txt");
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text))
        return;
    QTextStream in(&file);
    map_use.clear();
    while (!in.atEnd())
    {
        QString &line = in.readLine();
        QStringList list=line.split(QString(","), QString::SkipEmptyParts, Qt::CaseInsensitive);
        map_use.insert(list.at(0), list.at(1));
    }
    file.close();
    ReleaseMutex(mutex_soft_file);
}

void UninstallSoftware::ProcessUninSoftMap()    //
{
    QStringMap mapStartMenuSoftInfo;
    GetStartMenuSoftInfo(mapStartMenuSoftInfo);//获取开始菜单中的软件信息（主要包括，软件名称（对应displayname）,软件路径），将能对应上的信息加入map。

    jsArray.clear();
    QVariantMapMap &jsArrayMap=m_UninsoftList;    //将获取到的软件详细信息取出
    QStringVector InfoSoftIdxMD5=m_VectorSoftInfo;   //将获取到的软件ID的MD5取出。
    QStringMap map_use;

    //------------------------------------------------------获取软件的使用频率，时间。
    getUnstallData(map_use);

    //得到不能直接对应上的list
    QList<QString> &list=map_use.keys();
    QStringVector RegNameAll,regName;
    QVector<qint32> Keyvector;

    //循环得到，最近没有运行过的软件名称列表。RegNameAll;
    for(QStringVector::iterator item=InfoSoftIdxMD5.begin();item!=InfoSoftIdxMD5.end();item++){
        QVariantMap &map=jsArrayMap[*item];
        QString &name=map["DisplayName"].toString();
        if(!list.removeOne(name)){
            RegNameAll.append(name);
        }
    }

    QMap<qint32,qint32> keymap;
    qint32 j=0;
    //循环，寻找list中剩下的和RegNameAll中相似的元素。保存在regName，keymap，Keyvector
    for(QStringVector::iterator it=RegNameAll.begin();it!=RegNameAll.end();it++){
        qint32 score=0;
        qint32 key=-1;
        qint32 i=0;
        QString &name=*it;
        for(QList<QString>::iterator lit=list.begin();lit!=list.end();lit++){
            qint32 m=StringLikeFind(*lit,name);

            if(score<m){
                score=m;
                key=i;
            }
            i++;
        }

        if(key>=0){
            if(keymap.find(key)==keymap.end()){
                keymap.insert(key,score);
                regName.append(name);
                Keyvector.append(key);
            }else{
                if(keymap[key]<score){
                    keymap[key]=score;
                    regName.replace(Keyvector.indexOf(key),name);
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
        if(it!=map_use.end())
        {

            get_str_for_used_date(it.value(),str);

            if(map.find("UsedDate")==map.end()){
                map.insert("UsedDate",str);
            }else{
                map["UsedDate"]=str;
            }
        }
        else
        {
            qint32 k=regName.indexOf(name);
            qint32 key=-1;
            if(k>=0)
            {
                key=Keyvector.at(k);
            }

            if(key>=0)
            {
                // QString ss=list.at(key);
                it=map_use.find(list.at(key));
                get_str_for_used_date(it.value(),str);
                map["UsedDate"]=str;
            }
            else
            {
                map.insert("UsedDate",QString("未知"));
            }
        }
        //------------------------------------------------------获取软件的使用频率，时间。

        //-------------------用开始菜单中的快捷方式信息，startmenu中的信息，补全map，安装目录。
        QStringMap::iterator itemQStringMap =  mapStartMenuSoftInfo.find(map["DisplayName"].toString());
        if(itemQStringMap != mapStartMenuSoftInfo.end())
        {
            if(map["InstallLocation"].toString().length()<1)
            {

                if(itemQStringMap.value().toStdString().length()>0)
                    map["InstallLocation"] = itemQStringMap.value();
            }
        }
        if(strcmp(map["UsedDate"].toString().toStdString().data(),"未知")==0)//过滤 未知
        {
            QString strUsedDate;
            QString strTime1 = map["InstallDate"].toString();
            if(strTime1.indexOf("-")>=0)
            {
                strTime1.replace("-","");
                strTime1+="0000";
                long lRet = GetTimeDiffer(strTime1.toStdString(),GetTimeNow()); //两个时间所差分钟数
                lRet = lRet/1440;                                               //转化为天数
                if(lRet>30)
                {
                    lRet = lRet/30;
                    if(lRet>11)
                    {
                        strUsedDate.sprintf("未知",lRet);
                    }
                    else
                        strUsedDate.sprintf("%d个月前使用过",lRet);
                }
                else if(lRet>=1)
                    strUsedDate.sprintf("%d天前使用过",lRet);
                else
                    strUsedDate.sprintf("今天使用过");

            }
            else
                strUsedDate.sprintf("3个月前使用过");
            map["UsedDate"] = strUsedDate;
        }
        //-------------------用开始菜单中的快捷方式信息，startmenu中的信息，补全map，安装目录。

        //-------------------区分静默卸载和卸载
        if(map["QuietUninstallString"].toString().length()>0)
        {
            map["showNameOnPage"] = tr("一键卸载");
        }
        else
        {
            map["showNameOnPage"] = tr("卸载");
        }
        //-------------------区分静默卸载和卸载

        QString szOldPngPath = map["UnIcoID"].toString();
        QString szPngPath = GLOBAL::_DY_DIR_RUNNERSELF + "png" + QDir::separator() + szOldPngPath;
        map["UnIcoID"] = szPngPath.replace("\\", "/");
        jsArray.append(map);
        map["UnIcoID"] = szOldPngPath;
    }
}

void UninstallSoftware::get_str_for_used_date(QString &value,QString &str)//获取使用频率的显示数据，此处有问题，，，待续
{
    QDate &date=QDate::fromString(value,"yyyyMMdd");
    QDate &cur_date=QDate::currentDate();
    int iRet = cur_date.toJulianDay() - date.toJulianDay();
    if(iRet>365)
    {
        str=tr("1年前使用过\0");
    }
    else if(iRet>30)
    {
        int iMonth = iRet/30;
        str=QString("%1").arg(iMonth) + tr("个月前使用过\0");
    }
    else if(iRet>0)
    {
        str=QString("%1").arg(iRet)+tr("天前使用过\0");
    }
    else
        str=tr("今天内使用过\0");
//    qDebug()<<"in data -----------------------"<<endl;
//    qDebug()<<value<<endl;
//    qDebug()<<date.toJulianDay()<<endl;
//    qDebug()<<"today:"<<cur_date.toJulianDay()<<endl;
}

//判断两个字符串的相似度
qint32 UninstallSoftware::StringLikeFind(QString str,QString &in_of_str)
{
    if(str.isEmpty())return 0;

    str=str.replace(QRegExp("[<>《》！?*(^)$%~!@#$…&%￥—+=、。；‘’“”：·`]"),"");

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

        if(k==1 && in_of_str.compare(s.at(0))==0){
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
    }
}

//获取开始菜单中的软件信息
void UninstallSoftware::GetStartMenuSoftInfo(QStringMap &mapQStr)
{
    QString strUserPath =GetOSProgramsPath(0);
    DirectoryList(strUserPath.toStdString().data(),mapQStr);
    QString strOSPath = GetOSProgramsPath(1);
    DirectoryList(strOSPath.toStdString().data(),mapQStr);
}

QString UninstallSoftware::GetOSProgramsPath(int flag)//获取开始菜单-程序的文件夹路径，根据参数0，1可分别获取当前用户，和系统的.默认1
{
    int iParameter = -1;
    if(flag == 0)
        iParameter = CSIDL_COMMON_STARTMENU;
    else
        iParameter = CSIDL_STARTMENU;
    char szPath[MAX_PATH];
    BOOL iRet;
    memset(szPath, 0, sizeof(szPath));
    iRet = SHGetSpecialFolderPathA(NULL, szPath, iParameter, 0);
    if(iRet == TRUE)
        return szPath;
    else
        return "";
}

//卸载软件
void UninstallSoftware::UninstallSoftwares(QString &uninstallID)
{
    if(!uninstallID.isEmpty()){

        //QVariantMapMap &jsArrayMap=_DataModel->getUnstallSoftInfo();
        QStringList UnstallMd5=uninstallID.split(QString(","), QString::SkipEmptyParts, Qt::CaseInsensitive);
       // QStringList MultiSoftPath;

        for(QStringList::Iterator it=UnstallMd5.begin();it!=UnstallMd5.end();it++){
            QVariantMap &jsArray=m_UninsoftList[*it];
            //if(jsArray!=m_UninsoftList.end()->toStdMap())
            {
                if(m_uninstallingSoftName.find(jsArray["DisplayName"].toString()) == m_uninstallingSoftName.end())
                {
                    m_uninstallingSoftName[jsArray["DisplayName"].toString()] = jsArray["DisplayName"].toString();
                }
                else
                {
                    qDebug()<<"重复点击，过滤"<<endl;
                    continue;
                }
                QVariant &szKEY =jsArray["QuietUninstallString"].toString().isEmpty() ? jsArray["UninstallString"]:jsArray["QuietUninstallString"];
                //解析命令行，将命令行解析成带参数命令，但是下面的写的有问题，，，，用修改后的函数，应该没问题了。
                QString strTowrite = jsArray["QuietUninstallString"].toString().isEmpty() ? jsArray["UninstallString"].toString():jsArray["QuietUninstallString"].toString();
                QString name = szKEY.toString();
//                FILE *fp;
//                fopen_s(&fp,"uninstallLog.txt","a+");
//                if(fp)
//                {
//                    string strtime = GetTimeNow();
//                    fprintf(fp,"time:%s\n",strtime.data());
//                    fprintf(fp,"name:%s,uninstallPath:%s\n",name.toStdString().data(),jsArray["InstallLocation"].toString().toStdString().data());
//                }
                QStringList arguments;
                GetPathAndCMD(name,arguments);
                //LogWriter::getLogCenter()->SaveFileLog(LOG_INFO, "befor--CMD:%s.name:%s,softName:%s,file:%s,line:%d\n",strTowrite.toStdString().data(), name.toStdString().data(),jsArray["DisplayName"].toString().toStdString().data(),__FILE__, __LINE__);
//                if(fp)
//                {
//                    fprintf(fp,"name:%s\n",name.toStdString().data());
//                    fclose(fp);
//                }
                if(name.length()>0)
                {
                    UninstallSoftThread *myThread = new UninstallSoftThread();
                    QObject::connect(myThread, SIGNAL(update(UninstallSoftThread *,QString,QString)), this, SLOT(UpdateSoftwareListForThread(UninstallSoftThread *,QString,QString)), Qt::QueuedConnection);
                    myThread->Init(name,arguments,jsArray["InstallLocation"].toString(),jsArray["DisplayName"].toString());
                    myThread->start();
                }
                else
                {
                    QMessageBox message(QMessageBox::Warning,"提示","未找到卸载命令，请手动卸载",QMessageBox::Yes,NULL);
                    message.exec();
                }
                qDebug()<<name<<endl;
                qDebug()<<arguments<<endl;
            }
        }
    }
}


void UninstallSoftware::UpdateSoftwareListForThread(UninstallSoftThread*mythread,QString strInstallLocation,QString strDisPlayName)
{
    if(m_uninstallingSoftName.find(strDisPlayName)!=m_uninstallingSoftName.end())//维护只调用一次卸载
    {
        m_uninstallingSoftName.remove(strDisPlayName);
    }
    if(mythread)
    {
       if(mythread->isRunning())
       {
           mythread->exit();
           mythread->wait();
       }
       delete mythread;
       mythread = NULL;
    }
    UpdateSoftwareList();           //更新界面，更新成员变量
    qDebug()<<strInstallLocation<<endl;
    //判断是否是真的卸载了
    bool isFind = false;
    QList<QVariant>::iterator item;
    for(item = jsArray.begin();item != jsArray.end();item++)
    {
        if(!item->toMap()["DisplayName"].toString().compare(strDisPlayName))
        {
            //qDebug() << "registe software name: " << item->toMap()["DisplayName"].toString();
            //qDebug() << "find software name: " << strDisPlayName;
            isFind = true;
            break;
        }
    }
    if(isFind)
    {
        //未卸载
    }
    else
    {
        //真卸载，，，
        emit sigUninstallFinished(strDisPlayName);
        if(strInstallLocation.length()>0)
        {
            if(!removeDir(strInstallLocation.toStdString().data()))
                emit sigCleanupResidue(strInstallLocation, strDisPlayName);
        }
    }
}

//c语言实现删除制定目录以及目录下文件
BOOL  removeDir(const char*  dirPath)
{
    struct _finddata_t fb;   //查找相同属性文件的存储结构体
    char  path[250];
    long    handle;
    int  resultone;
    int   noFile;            //对系统隐藏文件的处理标记

    noFile = 0;
    handle = 0;
    //制作路径
    strcpy_s(path, dirPath);
    strcat_s(path, "/*");
    handle = _findfirst(path, &fb);
    // 找到第一个匹配的文件
    if (handle != 0 && handle != -1)
    {
        //当可以继续找到匹配的文件，继续执行
        while (0 == _findnext(handle, &fb))
        {
            //windows下，常有个系统文件，名为“..”,对它不做处理
            noFile = strcmp(fb.name, "..");
            noFile &= strcmp(fb.name, ".");
            if (0 != noFile)
            {
                //制作完整路径
                memset(path, 0, sizeof(path));
                strcpy_s(path, dirPath);
                strcat_s(path, "\\");
                strcat_s(path, fb.name);
                //属性值为16，则说明是文件夹，迭代
                if (fb.attrib == 16)
                {
                    if (!removeDir(path))
                    {
                        _findclose(handle);
                        return FALSE;
                    }
                }
                //非文件夹的文件，直接删除。对文件属性值的情况没做详细调查，可能还有其他情况。
                else
                {
                    int iRet = remove(path);
                    if (iRet == -1)
                    {
                        _findclose(handle);
                        return FALSE;
                    }
                }
            }
        }
        //关闭文件夹，只有关闭了才能删除。找这个函数找了很久，标准c中用的是closedir
        //经验介绍：一般产生Handle的函数执行后，都要进行关闭的动作。
        _findclose(handle);
        //移除文件夹
        resultone = _rmdir(dirPath);
    }
    return  TRUE;
}

// 深度优先递归遍历目录中所有的文件
BOOL  DirectoryList(LPCSTR Path,QStringMap &mapQStr)
{
    WIN32_FIND_DATAA FindData;
    HANDLE hError;
    int FileCount = 0;
    char FilePathName[256] = {0};
    // 构造路径
    char FullPathName[256] = {0};
    strcpy_s(FilePathName, Path);
    strcat_s(FilePathName, "\\*.*");
    hError = FindFirstFileA(FilePathName, &FindData);
    if (hError == INVALID_HANDLE_VALUE)
    {
        //printf("搜索失败!\n");
        return 0;
    }
    while (::FindNextFileA(hError, &FindData))
    {
        // 过虑.和..
        if (strcmp(FindData.cFileName, ".") == 0|| strcmp(FindData.cFileName, "..") == 0)
        {
            continue;
        }
        // 构造完整路径
        sprintf_s(FullPathName, "%s\\%s", Path, FindData.cFileName);
        FileCount++;
        // 获取文件信息
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //printf("%d  %s\n  ", FileCount, FullPathName);
            //printf("<文件夹>");
            DirectoryList(FullPathName,mapQStr);
        }
        else
        {
            //获取信息存入map；
            std::string strFileName; strFileName.append(FullPathName);
            //qDebug() << QString::fromStdString(GBKToUTF8(strFileName));
            QString lin1,lin2;
            QString lin0 = QString::fromStdString(GBKToUTF8(strFileName)) ;
            GetLinkPath(lin0,lin1,lin2);
            if(lin1.length()>0 && lin2.length()>0)
            {
                string strSoftName;
                GetInfoFromExeAndDll(lin1.toStdString(),strSoftName);//通过绝对路径获取软件名称
                if(strSoftName.length()>0)
                {
                    mapQStr[QString::fromStdString(strSoftName)] = lin2;
                }
            }
        }
    }
    return 0;
}

string GetInfoFromExeAndDll(string strFileFullPath, std::string &strRt)
{
    strRt = "";
    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    };
    DWORD dwSize = 0;
    UINT uiSize = GetFileVersionInfoSizeA(strFileFullPath.data(), &dwSize);
    if (0 == uiSize)
    {
        return strRt;
    }
    PTSTR pBuffer = new TCHAR[uiSize];
    if (NULL == pBuffer)
    {
        return strRt;
    }
    memset((void*)pBuffer, 0, uiSize);
    //获取exe 或 DLL 的资源信息，存放在pBuffer内
    if (!GetFileVersionInfoA(strFileFullPath.data(), 0, uiSize, (PVOID)pBuffer))
    {
        return strRt;
    }
    LANGANDCODEPAGE *pLanguage = NULL;  //这里这样设置没关系了。
    UINT  uiOtherSize = 0;
    //获取资源相关的 codepage 和language
    if (!VerQueryValueA(pBuffer, "\\VarFileInfo\\Translation", (PVOID*)&pLanguage, &uiOtherSize))
    {
        return strRt;
    }

    //重点
    char* pTmp = NULL;
    char SubBlock[MAX_PATH];
    memset((void*)SubBlock, 0, sizeof(SubBlock));
    UINT uLen = 0;
    //获取每种 CodePage 和 Language 资源的相关信息
    int ret = uiOtherSize / sizeof(LANGANDCODEPAGE);
    if (ret > 0)
    {
        sprintf_s(SubBlock, "\\StringFileInfo\\%04x%04x\\ProductName", pLanguage[0].wLanguage, pLanguage[0].wCodePage);
        if (VerQueryValueA(pBuffer, SubBlock, (PVOID*)&pTmp, &uLen))
        {
            if (strlen(pTmp) > 0)
                strRt = pTmp;
        }
    }
    delete[]pBuffer;
    pBuffer = NULL;
    return strRt;
}

void GetLinkPath(QString strLink, QString &strPath, QString &strParam)
{
    if(strLink.endsWith(".lnk") || strLink.endsWith(".lnk") || strLink.endsWith(".lnk"))
    {
        HRESULT   hres;
        IShellLink*   psl;
        wchar_t   szGotPath[MAX_PATH] = {0};
        wchar_t   szArguement[MAX_PATH] = {0};
        wchar_t szLink[MAX_PATH] = {0};
        WIN32_FIND_DATA   wfd;
        CoInitialize(0);
        hres = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (LPVOID*)&psl);
        if (SUCCEEDED(hres))
        {
            IPersistFile*   ppf;
            hres = psl->QueryInterface(IID_IPersistFile, (void**)&ppf);
            if (SUCCEEDED(hres))
            {
                strLink.toWCharArray(szLink);
                hres = ppf->Load(szLink, STGM_READ);
                if (SUCCEEDED(hres))   {
                    int i = 1;
                    i = i<<16;
                    hres = psl->Resolve(0,i | SLR_NO_UI);//不弹窗，一毫秒等待
                    if (SUCCEEDED(hres))
                    {
                        hres = psl->GetPath(szGotPath, MAX_PATH, (WIN32_FIND_DATA*)&wfd, SLGP_SHORTPATH);
                        if (SUCCEEDED(hres))
                        {
                            strPath = QString::fromWCharArray(szGotPath);
                        }
                        hres = psl->GetWorkingDirectory(szArguement, 256);
                        if (SUCCEEDED(hres))
                        {
                            strParam = QString::fromWCharArray(szArguement);
                        }
                    }
                }
                ppf->Release();
            }
            psl->Release();
        }
        CoUninitialize();
    }
}

//获取当前系统时间
string GetTimeNow()
{
    SYSTEMTIME st;
    ::GetLocalTime(&st);
    char szTemp[24] = { 0 };
    sprintf_s(szTemp, "%04u%02u%02u%02u%02u", st.wYear, st.wMonth, st.wDay, st.wHour, st.wMinute);
    return szTemp;
}

long GetTimeDiffer(string strTime1, string strTime2)
{
    long ret = -1;
    //string strTimeNow = GetTimeNow();
    strTime1 += "00";				//加上默认秒数；
    struct tm tm1;
    time_t time1;
    sscanf_s(strTime1.c_str(), "%4d%2d%2d%2d%2d%2d", &tm1.tm_year, &tm1.tm_mon, &tm1.tm_mday, &tm1.tm_hour, &tm1.tm_min, &tm1.tm_sec);
    tm1.tm_year -= 1900;
    tm1.tm_mon--;
    tm1.tm_isdst = -1;
    time1 = mktime(&tm1);

    strTime2 += "00";				//加上默认秒数；
    struct tm tm2;
    time_t time2;
    sscanf_s(strTime2.c_str(), "%4d%2d%2d%2d%2d%2d", &tm2.tm_year, &tm2.tm_mon, &tm2.tm_mday, &tm2.tm_hour, &tm2.tm_min, &tm2.tm_sec);
    tm2.tm_year -= 1900;
    tm2.tm_mon--;
    tm2.tm_isdst = -1;
    time2 = mktime(&tm2);
    char szLinShi[16] = { 0 };
    sprintf_s(szLinShi, "%lld", abs(time2 - time1) / 60);
    ret = atoi(szLinShi);
    return ret;
}

void GetPathAndCMD(QString &name,QStringList &arguments)
{
    if(name.length()<1)
        return;
    name=name.trimmed();//用以删除字符串两边的空白字符(注意，空白字符包括空格、Tab以及换行符，而不仅仅是空格)
    int iPos = -1;
    if(name.at(0)=='\"')
    {
            name.remove(0,1);       //"uninstallString" param1 param2
            iPos = name.indexOf("\"");
    }
    else
    {
        iPos = name.lastIndexOf("\\");   //uninstallString param1 param2
        QString strLinshi = name.mid(iPos+1);
        int iLinshi = strLinshi.indexOf(" ");
        if(iLinshi>0)
            iPos = iPos + iLinshi + 1;
        else
            iPos = -1;
    }
    if(iPos>0)
    {
        QString strLinshi = name.mid(0,iPos);

        name = name.mid(iPos+1);
        name = name.trimmed();
        arguments = name.split(QString(" "), QString::SkipEmptyParts, Qt::CaseInsensitive);
        name = strLinshi;
    }
    //uninstallString
}

BOOL DelFolder(const char *szPath)
{
    WIN32_FIND_DATAA FindData;
    HANDLE hError;
    int FileCount = 0;
    char FilePathName[256] = { 0 };
    // 构造路径
    char FullPathName[256] = { 0 };
    strcpy_s(FilePathName, szPath);
    strcat_s(FilePathName, "\\*.*");
    hError = FindFirstFileA(FilePathName, &FindData);
    if (hError == INVALID_HANDLE_VALUE)
    {
        //printf("搜索失败!\n");
        return TRUE;
    }
    while (::FindNextFileA(hError, &FindData))
    {
        // 过虑.和..
        if (strcmp(FindData.cFileName, ".") == 0 || strcmp(FindData.cFileName, "..") == 0)
        {
            continue;
        }
        // 构造完整路径
        sprintf_s(FullPathName, "%s\\%s", szPath, FindData.cFileName);
        FileCount++;
        // 获取文件信息
        if (FindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)
        {
            //printf("%d  %s\n  ", FileCount, FullPathName);
            //printf("<文件夹>");
            if(!DelFolder(FullPathName))
                return FALSE;
        }
        else
        {
            int iRet=remove(FullPathName);
            if(iRet == -1)
                return FALSE;
        }

    }
    return TRUE;
}
