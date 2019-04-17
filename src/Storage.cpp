#include "Storage.h"
#include <QtDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "DataControl.h"
#include "character.h"

Storage::Storage(QObject *parent) : QObject(parent)
{

}

void Storage::getSettingFromFile(QString szFile, QVariantMap &setting) {
    QByteArray fileBuf;
    QJsonDocument doc;
    QFile saveFile(szFile);
    if(saveFile.open(QIODevice::ReadWrite)) {
        fileBuf = saveFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        saveFile.close();
    }
    if(!doc.isEmpty() &&doc.isObject()) {
        setting = doc.object().toVariantMap();
    }
}

void Storage::setSettingToFile(QString szFile, QVariantMap &setting) {
    QJsonDocument doc(QJsonObject::fromVariantMap(setting));

    QFile saveFile(szFile);
    if(saveFile.open(QIODevice::ReadWrite)) {
        saveFile.resize(0);
        saveFile.write(doc.toJson(QJsonDocument::Compact));
        saveFile.close();
    }
}

bool Storage::LoadSoftwareCategory(QString szCategoryFile, QVariantList &mapCategory) {
    QByteArray fileBuf;
    QJsonObject jsObj;
    QJsonDocument doc;

    if(!QFile::exists(szCategoryFile)) {
        return false;
    }

    QFile loadFile(szCategoryFile);
    if(loadFile.open(QIODevice::ReadOnly)) {
        fileBuf = loadFile.readAll();
        QJsonParseError err;
        doc = QJsonDocument::fromJson(fileBuf, &err);
        loadFile.close();
    }
    else {
        return false;
    }
    if(doc.isEmpty() || !doc.isObject()) {
        return false;
    }
    jsObj = doc.object();
    if(!jsObj.contains("code") || !jsObj.value("code").isDouble() || !jsObj.contains("msg") || !jsObj.value("msg").isArray()) {
        return false;
    }
    if(jsObj.value("code").toInt() != 0) {
        return false;
    }
    mapCategory = jsObj.value("msg").toArray().toVariantList();

    return true;
}

bool Storage::LoadCategorySoftwareList(QString szCategoryListFile, QString szCategoryID, QMap<QString, QVariantList> &mapPackageByCategoryID) {
    QByteArray fileBuf;
    QJsonObject jsObj;
    QJsonDocument doc;

    QFile loadFile(szCategoryListFile);
    if(loadFile.open(QIODevice::ReadOnly)) {
        fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        loadFile.close();
    }
    else {
        return false;
    }
    if(doc.isEmpty() || !doc.isObject()) {
        return false;
    }
    jsObj = doc.object();
    if(!jsObj.contains("code") || !jsObj.value("code").isDouble() || !jsObj.contains("msg") || !jsObj.value("msg").isArray()) {
        return false;
    }
    if(jsObj.value("code").toInt() != 0) {
        return false;
    }
    mapPackageByCategoryID.insert(szCategoryID, jsObj.value("msg").toArray().toVariantList());
    //mapPackageByCategoryID[szCategoryID] = jsObj.value("msg").toArray().toVariantList();
    return true;
}

bool Storage::LoadArrayOfSoftwareList(QString szSoftListFile, QVariantList &arrPackage) {
    QByteArray fileBuf;
    QJsonDocument doc;
    QJsonObject jsObj;

    QFile loadFile(szSoftListFile);
    if(loadFile.open(QIODevice::ReadOnly)) {
        fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        loadFile.close();
    }
    else {
        return false;
    }
    if(doc.isEmpty() || !doc.isObject()) {
        return false;
    }
    jsObj = doc.object();
    if(!jsObj.contains("code") || !jsObj.value("code").isDouble() || !jsObj.contains("msg") || !jsObj.value("msg").isArray()) {
        return false;
    }
    if(jsObj.value("code").toInt() != 0) {
        return false;
    }
    arrPackage = jsObj.value("msg").toArray().toVariantList();
    return true;
}

void Storage::LoadFromConfArray(QString szFile, QVariantList &mapItems) {
    QJsonDocument doc;

    QFile loadFile(szFile);
    if(loadFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        if(!doc.isEmpty() &&doc.isArray()) {
            mapItems = doc.array().toVariantList();
        }
        else {
            loadFile.resize(0);
        }
        loadFile.close();
    }
}

void Storage::SaveToConfArray(QString szFile, QVariantList &mapItems) {
    QJsonDocument doc;
    QFile loadFile(szFile);
    if(loadFile.open(QIODevice::ReadWrite | QIODevice::Truncate)) {
        QByteArray fileBuf = loadFile.readAll();
        doc = QJsonDocument::fromJson(fileBuf);
        loadFile.resize(0);
        doc.setArray(QJsonArray::fromVariantList(mapItems));
        loadFile.write(doc.toJson(QJsonDocument::Compact));
        loadFile.close();
    }
}

void Storage::LoadDownloadData(bool bFirst, const QString &szFile, QDownloadItemDataMap &mapDownloadData)
{
    QVariantList objectList;
    LoadFromConfArray(szFile, objectList);

    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    for(QVariantList::iterator it = objectList.begin(); it != objectList.end(); it++)
    {
        QVariantMap object = it->toMap();
        if(object.isEmpty())
        {
            continue;
        }

        int status = object.value("status").toInt();
        if(status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
        {
            continue;
        }
        if(status == PACKAGE_STATUS_DOWNLOAD_START ||
                status == PACKAGE_STATUS_DOWNLOAD_START_ERR ||
                status == PACKAGE_STATUS_DOWNLOAD_PAUSE)
        {
            status = PACKAGE_STATUS_NEW_DOWNLOAD;
        }

        if(bFirst && (status == PACKAGE_STATUS_INSTALL_START || status == PACKAGE_STATUS_DOWNLOAD_START_ERR))
        {
            status = PACKAGE_STATUS_INSTALL_CANCEL;
        }

        if(status == PACKAGE_STATUS_INSTALL_FINISH)
        {
            bool isInstalled = false;
            QString name = object.value("name").toString();
            for(mapSoftwareList::iterator install_it = mapInstalledSoftwares.begin(); install_it != mapInstalledSoftwares.end(); install_it++)
            {
                QString displayName = QString::fromStdString(GBKToUTF8(install_it->second["DisplayName"]));
                if(displayName.isEmpty())
                {
                    continue;
                }

                if(name.contains(displayName, Qt::CaseInsensitive) || displayName.contains(name, Qt::CaseInsensitive))
                {
                    isInstalled = true;
                    break;
                }
            }
            if(isInstalled)
            {
                status = PACKAGE_STATUS_INSTALL_FINISH;
            }
            else
            {
                status = PACKAGE_STATUS_INSTALL_CANCEL;
            }
        }

        if(status == PACKAGE_STATUS_INSTALL_CANCEL)
        {
            QString szPackagePath = "";
            QString szLastString = object.value("downloadPath").toString().right(1);
            if(szLastString.compare(QDir::separator(), Qt::CaseInsensitive) == 0)
            {
                szPackagePath = object.value("downloadPath").toString() + object.value("packageName").toString();
            }
            else
            {
                szPackagePath = object.value("downloadPath").toString() + QDir::separator() + object.value("packageName").toString();
            }
            if(QFile(szPackagePath).exists())
            {
                status = PACKAGE_STATUS_INSTALL_CANCEL;
            }
            else
            {
                continue;
            }
        }

        LPDownloadItemData pItemData = new DownloadItemData();
        if(!pItemData)
        {
            continue;
        }

        pItemData->id = object.value("id").toString();
        pItemData->category = object.value("category").toString();
        pItemData->name = object.value("name").toString();
        pItemData->largeIcon = object.value("largeIcon").toString();
        pItemData->brief = object.value("brief").toString();
        pItemData->size = object.value("size").toDouble();
        pItemData->downloadUrl = object.value("downloadUrl").toString();
        pItemData->rating = object.value("rating").toInt();
        pItemData->isAd = object.value("isAd").toBool();
        pItemData->priceInfo = object.value("priceInfo").toFloat();
        pItemData->updateTime = object.value("updateTime").toString();
        pItemData->versionName = object.value("versionName").toString();
        pItemData->packageName = object.value("packageName").toString();
        pItemData->downloadPath = object.value("downloadPath").toString();
        pItemData->percent = object.value("percent").toFloat();
        pItemData->speed = object.value("speed").toLongLong();
        pItemData->status = status;
        pItemData->hTaskHandle = NULL;
        memset(&pItemData->downTaskParam, 0x00, sizeof(pItemData->downTaskParam));

        // 为了保证可以下载相同软件的不同版本号, 采用 id:;name:;versionName 的格式作为下载数据的key
        QString key = pItemData->id + ":;" + pItemData->name + ":;" + pItemData->versionName;
        mapDownloadData.insert(key, pItemData);
    }
}

void Storage::SaveDownloadData(const QString &szFile, QDownloadItemDataMap &mapDownloadData)
{
    QVariantList objectList;
    for(DownloadIterator it = mapDownloadData.begin(); it != mapDownloadData.end();)
    {
        LPDownloadItemData pItemData = it.value();
        if(!pItemData || pItemData->status == PACKAGE_STATUS_DOWNLOAD_CANCEL)
        {
            continue;
        }

        QVariantMap object;
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
        object.insert(QString("speed"), QVariant::fromValue(0));
        object.insert(QString("status"), QVariant::fromValue(pItemData->status));
        if(!object.isEmpty())
        {
            objectList.append(object);
        }
        it++;
    }
    SaveToConfArray(szFile, objectList);
}

void Storage::LoadUpgradeData(const bool bFirst, const QString &szFile, QUpgradeItemDataMap &mapUpgradeData)
{
    QVariantList objectList;
    LoadFromConfArray(szFile, objectList);

    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    for(QVariantList::iterator it = objectList.begin(); it != objectList.end(); it++)
    {
        QVariantMap object = it->toMap();
        if(object.isEmpty())
        {
            continue;
        }

        QString displayName = object.value("displayName").toString();
        QString displayVersion = object.value("displayVersion").toString();

        // filter non-install software, and set arch
        for(mapSoftwareList::iterator temp_it = mapInstalledSoftwares.begin(); temp_it != mapInstalledSoftwares.end(); temp_it++)
        {
            QString tempDisplayName = QString::fromStdString(GBKToUTF8(temp_it->second["DisplayName"]));
            QString tempDisplayVersion = QString::fromStdString(GBKToUTF8(temp_it->second["DisplayVersion"]));
            int tempDisplayArch = QString::fromStdString(GBKToUTF8(temp_it->second["DisplayArch"])).toInt();
            if(!displayName.compare(tempDisplayName, Qt::CaseInsensitive) &&
                    !displayVersion.compare(tempDisplayVersion, Qt::CaseInsensitive))
            {

                LPUpgradeItemData pItemData = new UpgradeItemData();
                if(!pItemData)
                {
                    continue;
                }

                pItemData->id = object.value("id").toString();
                pItemData->category = object.value("category").toString();
                pItemData->softName = object.value("softName").toString();
                pItemData->displayName = object.value("displayName").toString();
                pItemData->displayVersion = object.value("displayVersion").toString();
                pItemData->name = object.value("name").toString();
                pItemData->versionName = object.value("versionName").toString();
                pItemData->updateTime = object.value("updateTime").toString();
                pItemData->largeIcon = object.value("largeIcon").toString();
                pItemData->brief = object.value("brief").toString();
                pItemData->size = object.value("size").toDouble();

                if(object.value("arch").isNull())
                {
                    pItemData->arch = tempDisplayArch;
                }
                else
                {
                    pItemData->arch = object.value("arch").toInt();
                }

                int status = object.value("status").toInt();
                if(bFirst && (status == PACKAGE_STATUS_INSTALL_START || status == PACKAGE_STATUS_DOWNLOAD_START_ERR))
                {
                    status = PACKAGE_STATUS_INSTALL_CANCEL;
                }
                pItemData->status = status;

                QString key = pItemData->id + ":;" + pItemData->displayName + ":;" + pItemData->displayVersion + ":;" + pItemData->name + ":;" + pItemData->versionName;
                mapUpgradeData.insert(key, pItemData);

                break;
            }
        }
    }
}

void Storage::SaveUpgradeData(const QString &szFile, QUpgradeItemDataMap &mapUpgradeData)
{
    QVariantList objectList;
    for(UpgradeIterator it = mapUpgradeData.begin(); it != mapUpgradeData.end(); it++)
    {
        LPUpgradeItemData pItemData = it.value();
        if(!pItemData)
        {
            continue;
        }

        QVariantMap object;
        object.insert(QString("id"), QVariant::fromValue(pItemData->id));
        object.insert(QString("category"), QVariant::fromValue(pItemData->category));
        object.insert(QString("softName"), QVariant::fromValue(pItemData->softName));
        object.insert(QString("displayName"), QVariant::fromValue(pItemData->displayName));
        object.insert(QString("displayVersion"), QVariant::fromValue(pItemData->displayVersion));
        object.insert(QString("name"), QVariant::fromValue(pItemData->name));
        object.insert(QString("versionName"), QVariant::fromValue(pItemData->versionName));
        object.insert(QString("updateTime"), QVariant::fromValue(pItemData->updateTime));
        object.insert(QString("largeIcon"), QVariant::fromValue(pItemData->largeIcon));
        object.insert(QString("brief"), QVariant::fromValue(pItemData->brief));
        object.insert(QString("size"), QVariant::fromValue(pItemData->size));
        object.insert(QString("arch"), QVariant::fromValue(pItemData->arch));
        object.insert(QString("status"), QVariant::fromValue(pItemData->status));
        if(!object.isEmpty())
        {
            objectList.append(object);
        }
    }
    SaveToConfArray(szFile, objectList);
}
