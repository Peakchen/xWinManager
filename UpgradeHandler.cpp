#include "UpgradeHandler.h"
#include "Storage.h"
#include "DataControl.h"
#include "character.h"
#include "global.h"

UpgradeHandler::UpgradeHandler(QObject *parent) : QObject(parent)
{
}

UpgradeHandler::~UpgradeHandler()
{
    Storage::SaveUpgradeData(ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);
    for(UpgradeIterator it = m_mapUpgradeData.begin(); it != m_mapUpgradeData.end();)
    {
        //erase space
        LPUpgradeItemData pItemData = it.value();
        if(pItemData)
        {
            delete pItemData;
            pItemData = NULL;
        }
        it = m_mapUpgradeData.erase(it);
    }
}

void UpgradeHandler::Init(DataControl *pDataControl)
{
    Storage::LoadUpgradeData(true, ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);

    QObject::connect(pDataControl, SIGNAL(sigReloadUpgradeData()), this, SLOT(ReloadUpgradeData()), Qt::QueuedConnection);
    QObject::connect(pDataControl, SIGNAL(sigRequestUpgradeData()), this, SLOT(RequestUpgradeData()), Qt::QueuedConnection);
    QObject::connect(pDataControl, SIGNAL(sigUpdateOneTaskInfo(QVariantMap)), this, SLOT(SetUpgradeStatus(QVariantMap)), Qt::QueuedConnection);
    QObject::connect(this, SIGNAL(sigUpdateOneUpgradeInfo(QVariantMap)), pDataControl, SIGNAL(sigUpdateOneUpgradeInfo(QVariantMap)), Qt::QueuedConnection);
}

void UpgradeHandler::EncodeToVariantMap(LPUpgradeItemData itemData, QVariantMap& object) {
    if (!itemData || itemData->id.isNull() || itemData->id.isEmpty())
        return ;

    object.insert(QString("id"),QVariant::fromValue(itemData->id));
    object.insert(QString("category"),QVariant::fromValue(itemData->category));
    object.insert(QString("name"),QVariant::fromValue(itemData->softName));
    object.insert(QString("displayVersion"),QVariant::fromValue(itemData->displayVersion));
    object.insert(QString("versionName"),QVariant::fromValue(itemData->versionName));
    object.insert(QString("updateTime"),QVariant::fromValue(itemData->updateTime.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive).at(0)));
    object.insert(QString("largeIcon"),QVariant::fromValue(itemData->largeIcon));
    object.insert(QString("brief"),QVariant::fromValue(itemData->brief));
    object.insert(QString("size"),QVariant::fromValue(itemData->size));
    object.insert(QString("status"),QVariant::fromValue(itemData->status));
}

void UpgradeHandler::EncodeToUpgradeData(QVariantMap object, LPUpgradeItemData &itemData) {
    if(object.isEmpty())
        return;

    itemData->id = object.find("id").value().toString();
    itemData->category = object.find("category").value().toString();
    itemData->softName = object.find("name").value().toString();
    itemData->displayVersion = object.find("displayVersion").value().toString();
    itemData->versionName = object.find("versionName").value().toString();
    itemData->updateTime = object.find("updateTime").value().toString();
    itemData->largeIcon = object.find("largeIcon").value().toString();
    itemData->brief= object.find("brief").value().toString();
    itemData->size = object.find("size").value().toDouble();
    itemData->status = object.find("status").value().toBool();
}

void UpgradeHandler::SendSignal(const LPUpgradeItemData &pItemData)
{
    if(!pItemData)
    {
        return;
    }

    QVariantMap object;
    EncodeToVariantMap(pItemData, object);
    if(object.isEmpty())
    {
        return;
    }

    QString szPngPath = GLOBAL::_PNGDIR + pItemData->id + ".png";
    if(IsFileExist(szPngPath))
    {
        object["largeIcon"] = QVariant::fromValue("file:///" + szPngPath.replace("\\", "/"));
    }

    emit sigUpdateOneUpgradeInfo(object);
}

void UpgradeHandler::ReloadUpgradeData()
{
    for(QUpgradeItemDataMap::iterator it = m_mapUpgradeData.begin(); it != m_mapUpgradeData.end();)
    {
        //erase space
        LPUpgradeItemData pItemData = it.value();
        if(pItemData)
        {
            delete pItemData;
            pItemData = NULL;
        }
        it = m_mapUpgradeData.erase(it);
    }

    // 获取当前升级数据
    Storage::LoadUpgradeData(false, ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);

    // send signal again
    RequestUpgradeData();
}

bool UpgradeHandler::CompareDisplayVersion(QString szOldVersion, QString szNewVersion){

    szNewVersion.replace(".", "");
    szOldVersion.replace(".", "");

    int OldSize = szOldVersion.trimmed().size();
    int NewSize = szNewVersion.trimmed().size();
    // 比较 版本号
    if(OldSize == NewSize){
        if(szNewVersion.toInt() > szOldVersion.toInt()){
            return true;
        }else{
            return false;
        }
    }else if(OldSize < NewSize){
        int newsizeleft = szNewVersion.left(OldSize).toInt();
        if(newsizeleft > szOldVersion.toInt()){
            return true;
        }else{
            return false;
        }
    }else{
        int oldsizeleft = szOldVersion.left(NewSize).toInt();
        if(oldsizeleft < szNewVersion.toInt()){
            return true;
        }else{
            return false;
        }
    }
}

void UpgradeHandler::RequestUpgradeData()
{
    // get installed softwares
    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    // compare installed software
    for(QUpgradeItemDataMap::iterator it = m_mapUpgradeData.begin(); it != m_mapUpgradeData.end(); it++)
    {
        QString key = it.key();

        bool isExist = false;
        for(mapSoftwareList::iterator temp_it = mapInstalledSoftwares.begin(); temp_it != mapInstalledSoftwares.end(); temp_it++)
        {
            QString displayName = QString::fromStdString(GBKToUTF8(temp_it->second["DisplayName"]));
            QString displayVersion = QString::fromStdString(GBKToUTF8(temp_it->second["DisplayVersion"]));

            if(key.contains(displayName) && key.contains(displayVersion))
            {
                bool bRet = CompareDisplayVersion(displayVersion, it.value()->versionName);
                if(bRet)
                {
                    isExist = true;
                    break;
                }
            }
        }
        if(isExist)
        {
            SendSignal(it.value());
        }
        else
        {
            LPUpgradeItemData pItemData = it.value();
            pItemData->status = PACKAGE_STATUS_REMOVE;      // 移除
            SendSignal(it.value());
        }
    }
}

void UpgradeHandler::SetUpgradeStatus(QVariantMap object)
{
    QString name = object["name"].toString();
    QString versionName = object["versionName"].toString();
    for(UpgradeIterator upgrade_it = m_mapUpgradeData.begin(); upgrade_it != m_mapUpgradeData.end(); upgrade_it++)
    {
        QString key = upgrade_it.key();
        if(key.contains(name) && key.contains(versionName))
        {
            LPUpgradeItemData pItemData = upgrade_it.value();
            int status = object["status"].toInt();
            if(status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
            {
                pItemData->status = PACKAGE_STATUS_CAN_UPGRADE;
            }
            else
            {
                pItemData->status = status;
            }

            Storage::SaveUpgradeData(ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);
            SendSignal(pItemData);
            break;
        }
    }

}
