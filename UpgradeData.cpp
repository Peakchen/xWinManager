#include "UpgradeData.h"
#include "Storage.h"
#include "ConfOperation.h"
#include "DataControl.h"
#include "character.h"

UpgradeData::UpgradeData(QObject *parent) : QObject(parent)
{
    m_bPolling = false;
    m_bQuit = false;
    m_pDataControl = NULL;
}

UpgradeData::~UpgradeData()
{
    if(m_timer.isActive())
    {
        m_timer.stop();
    }
    for(UpgradeIterator it = m_mapUpgradeData.begin(); it != m_mapUpgradeData.end();)
    {
        DelUpgradeItemData(it.value());
        it = m_mapUpgradeData.erase(it);
    }
}

void UpgradeData::Init(DataControl* pDataControl)
{
    m_pDataControl = pDataControl;
    connect(this, SIGNAL(sigUpdateUpgradeCount(int)), pDataControl, SIGNAL(sigUpdateUpgradeCount(int)), Qt::QueuedConnection);
    connect(this, SIGNAL(sigReloadUpgradeData()), pDataControl, SIGNAL(sigReloadUpgradeData()), Qt::QueuedConnection);
    connect(&m_timer, SIGNAL(timeout()), this, SLOT(PollUpgradeData()), Qt::QueuedConnection);
}

void UpgradeData::StartTimer()
{
    m_timer.start(1000);
}

void UpgradeData::StopTimer()
{
    m_bQuit = true;
    if(m_timer.isActive())
    {
        m_timer.stop();
    }
}


void UpgradeData::AddUpgradeItemData(const QVariant &package, const QString &softName, const QString &displayName, const QString &displayVersion)
{
    LPUpgradeItemData pItemData = new UpgradeItemData();
    if(!pItemData)
        return;

    QMap<QString, QVariant> map = package.toMap();
    pItemData->id = map.value("id").toString();
    pItemData->category = map.value("category").toString();
    pItemData->softName = softName;
    pItemData->displayName = displayName;
    pItemData->displayVersion = displayVersion;
    pItemData->name = map.value("name").toString();
    pItemData->versionName = map.value("versionName").toString();
    pItemData->updateTime = map.value("updateTime").toString();
    pItemData->largeIcon = map.value("largeIcon").toString();
    pItemData->brief = map.value("brief").toString();
    pItemData->size = map.value("size").toDouble();
    pItemData->arch = map.value("arch").toInt();
    pItemData->status = PACKAGE_STATUS_CAN_UPGRADE;

    QString key = pItemData->id + ":;" + displayName + ":;" + displayVersion + ":;" + pItemData->name + ":;" + pItemData->versionName;
    m_mapUpgradeData.insert(key, pItemData);
}

void UpgradeData::DelUpgradeItemData(LPUpgradeItemData &pItemData)
{
    if(pItemData)
    {
        delete pItemData;
        pItemData = NULL;
    }
}

void UpgradeData::SendUpgradeCountSignal()
{
    int count = 0;
    QUpgradeItemDataMap mapUpgradeData;
    Storage::LoadUpgradeData(false, ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), mapUpgradeData);
    for(UpgradeIterator it = mapUpgradeData.begin(); it != mapUpgradeData.end();)
    {
        if(it.value()->status < PACKAGE_STATUS_INSTALL_FINISH)
        {
            count++;
        }
        DelUpgradeItemData(it.value());
        it = mapUpgradeData.erase(it);
    }

    emit sigUpdateUpgradeCount(count);
}

void UpgradeData::PollUpgradeData()
{
    if(m_bPolling)
    {
        return;
    }

    m_bPolling = true;

    SendUpgradeCountSignal();

    // get installed softwares
    mapSoftwareList mapInstalledSoftwares;
    DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);

    // compare softwares with softwares
    for(mapSoftwareList::iterator it = mapInstalledSoftwares.begin(); it != mapInstalledSoftwares.end(); it++)
    {
        if(m_bQuit)
        {
            return;
        }

        QString displayName = QString::fromStdString(GBKToUTF8(it->second["DisplayName"]));
        if(displayName.isEmpty())
        {
            continue;
        }

        QString displayVersion = QString::fromStdString(GBKToUTF8(it->second["DisplayVersion"]));
        int displayArch = QString::fromStdString(GBKToUTF8(it->second["DisplayArch"])).toInt();

        QString softName = "";
        QString versionFromName = "";
        if(!SeparateNameAndVersion(displayName, softName, versionFromName))
        {
            continue;
        }

        if(softName.isEmpty())
        {
            softName = displayName;
        }

        if(versionFromName.isEmpty() && !displayVersion.isEmpty())
        {
            versionFromName = displayVersion;
        }
        else if(displayVersion.isEmpty())
        {
            displayVersion = versionFromName;
        }

        // compare with all packages
        foreach(QVariant category, m_pDataControl->GetCategory())
        {
            QString szCategoryID = category.toMap().value("id").toString();
            QVariantList lstPackage;
            m_pDataControl->GetPackage2(szCategoryID, lstPackage);

            foreach(QVariant package, lstPackage)
            {
                if(m_bQuit)
                {
                    return;
                }
                USleep(50);             // 休眠50微秒, 避免CPU长时间占用

                // is windows or not
                QString windowsVersion = package.toMap().value("windowsVersion").toString();
                if(!windowsVersion.contains("win", Qt::CaseInsensitive))
                {
                    continue;
                }

                QString id = package.toMap().value("id").toString();
                QString name = package.toMap().value("name").toString();
                QString versionName = package.toMap().value("versionName").toString();
                int arch = package.toMap().value("arch").toInt();

                QString tempName = "";
                QString tempVersionFromName = "";
                if(!SeparateNameAndVersion(name, tempName, tempVersionFromName))
                {
                    continue;
                }

                if(tempName.isEmpty())
                {
                    tempName = name;
                }

                if(tempVersionFromName.isEmpty() && !versionName.isEmpty())
                {
                    tempVersionFromName = versionName;
                }
                else if(versionName.isEmpty())
                {
                    versionName = tempVersionFromName;
                }

                // compare name
                if(!tempName.contains(softName, Qt::CaseInsensitive))
                {
                    continue;
                }

                // if the first string in name is not equal with the first string in softName, return false, such as: python and activepython
                if(tempName.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive).at(0).compare(softName.split(" ", QString::SkipEmptyParts, Qt::CaseInsensitive).at(0), Qt::CaseInsensitive))
                {
                    continue;
                }

                // compare version from name
                int verCompareRet = CompareVersion(tempVersionFromName,  versionFromName);
                if(verCompareRet < 0)
                {
                    continue;
                }

                // yes, you find a package that its name contains displayName and its version is bigger than or equal with version from displayName, then compare version
                int ret = CompareVersion(versionName, displayVersion);
                if(ret <= 0)
                {
                    continue;
                }

                // compare arch
                if(displayArch == 32)       // 已安装的软件是32位的, 只能升级32或者96的包
                {
                    if(arch != 32 && arch != 96)
                    {
                        continue;
                    }
                }
                else if(displayArch == 64)  // 已安装的软件是64位的, 只能升级64的包
                {
                    if(arch != 64)
                    {
                        continue;
                    }
                }
                else
                {
                    continue;
                }

                bool isExist = false;
                Storage::LoadUpgradeData(false, ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);
                for(UpgradeIterator upgrade_it = m_mapUpgradeData.begin(); upgrade_it != m_mapUpgradeData.end();)
                {
                    if(m_bQuit)
                    {
                        return;
                    }

                    QString key = upgrade_it.key();
                    if(key.contains(displayName, Qt::CaseInsensitive))
                    {
                        if(key.contains(displayVersion, Qt::CaseInsensitive))
                        {
                            if(key.contains(name, Qt::CaseInsensitive))
                            {
                                if(key.contains(versionName, Qt::CaseInsensitive))
                                {
                                    if(key.contains(id, Qt::CaseInsensitive))
                                    {
                                        LPUpgradeItemData pItemData = upgrade_it.value();
                                        if(pItemData->arch == 0)
                                        {
                                            pItemData->arch = package.toMap().value("arch").toInt();
                                            Storage::SaveUpgradeData(ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);
                                            emit sigReloadUpgradeData();
                                            SendUpgradeCountSignal();
                                        }
                                        isExist = true;
                                        break;
                                    }
                                }
                            }
                        }

                        DelUpgradeItemData(upgrade_it.value());
                        upgrade_it = m_mapUpgradeData.erase(upgrade_it);
                        AddUpgradeItemData(package, softName, displayName, displayVersion);
                        Storage::SaveUpgradeData(ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);
                        emit sigReloadUpgradeData();
                        SendUpgradeCountSignal();
                        break;
                    }

                    upgrade_it++;
                }

                if(!isExist)
                {
                    AddUpgradeItemData(package, softName, displayName, displayVersion);
                    Storage::SaveUpgradeData(ConfOperation::Root().getSubpathFile("Conf", "upgrade.dat"), m_mapUpgradeData);
                    emit sigReloadUpgradeData();
                    SendUpgradeCountSignal();
                }

                for(UpgradeIterator upgrade_it = m_mapUpgradeData.begin(); upgrade_it != m_mapUpgradeData.end();)
                {
                    DelUpgradeItemData(upgrade_it.value());
                    upgrade_it = m_mapUpgradeData.erase(upgrade_it);
                }
                break;
            }
            lstPackage.clear();
        }
    }

    m_bPolling = false;
}
