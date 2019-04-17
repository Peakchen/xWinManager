#include "LaunchThread.h"
#include <QDebug>
#include <QDir>
#include "swmgrapp.h"
#include "character.h"

LaunchThread::LaunchThread(QObject *parent) : QThread(parent)
{
    m_pItemData = NULL;
}

bool LaunchThread::Init(LPDownloadItemData pItemData)
{
    if(!pItemData)
    {
        return false;
    }

    m_pItemData = pItemData;

    return true;
}

void LaunchThread::run()
{
    if(!m_pItemData)
    {
        QThread::exit(1);
    }

    // package exist or not
    QString szPackagePath = "";
    QString szLastString = m_pItemData->downloadPath.right(1);
    if(szLastString.compare(QDir::separator(), Qt::CaseInsensitive) == 0)
    {
        szPackagePath = m_pItemData->downloadPath + m_pItemData->packageName;
    }
    else
    {
        szPackagePath = m_pItemData->downloadPath + QDir::separator() + m_pItemData->packageName;
    }

    QFileInfo *pFileInfo = NULL;
    if(NULL == (pFileInfo = new QFileInfo(szPackagePath)))
    {
        QThread::exit(1);
    }

    if(pFileInfo->exists())
    {
        QProcess *m_pInstaller = new QProcess();
        if(!m_pInstaller)
        {
            QThread::exit(1);
        }

        QString szAllParameter = "/S;/s;/silent;-R;-r;-s;-sms;/s /v \"/qb\";/s /v /qb;";
        QStringList lstParameter = szAllParameter.split(";", QString::SkipEmptyParts, Qt::CaseInsensitive);
        int nParameterCount = lstParameter.size();
        int nParameterIndex = 0;

        if(szPackagePath.contains(".msi", Qt::CaseInsensitive))
        {
            szPackagePath = QDir::toNativeSeparators(szPackagePath);
            m_pInstaller->start("msiexec", QStringList() << "/i" << szPackagePath << "/qn");    //msi文件使用qn静默安装
        }
        else
        {
LOOP:
            if(nParameterIndex >= nParameterCount)
            {
                m_pInstaller->start(szPackagePath, QStringList());
            }
            else
            {
                m_pInstaller->start(szPackagePath, QStringList() << lstParameter.at(nParameterIndex));
            }
        }

        if(m_pInstaller->waitForStarted())
        {
            m_pItemData->status = PACKAGE_STATUS_INSTALL_START;

            if(m_pInstaller->waitForFinished())
            {
                // check software install successfully or not
                // get installed software
                mapSoftwareList mapInstalledSoftwares;
                DataControl::GetCurInstalledSoftware(mapInstalledSoftwares);
                bool isExist = false;

                QString name = m_pItemData->name;
                QString version = m_pItemData->versionName;

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
                        m_pItemData->status = PACKAGE_STATUS_INSTALL_FINISH;
                        break;
                    }

                    int ret = CompareVersion(install_version,  version);
                    if(ret >= 0)
                    {
                        m_pItemData->status = PACKAGE_STATUS_INSTALL_FINISH;
                    }
                    else
                    {
                        m_pItemData->status = PACKAGE_STATUS_INSTALL_CANCEL;
                    }
                    break;
                }

                if(!isExist)
                {
                    m_pItemData->status = PACKAGE_STATUS_INSTALL_CANCEL;
                }
            }
            else
            {
                m_pItemData->status = PACKAGE_STATUS_DOWNLOAD_FINISH;
            }
        }
        else
        {
            if(szPackagePath.contains(".exe", Qt::CaseInsensitive))
            {
                if(nParameterIndex < nParameterCount)
                {
                    nParameterIndex++;
                    goto LOOP;
                }
            }

            m_pItemData->status = PACKAGE_STATUS_INSTALL_START_ERR;
        }
    }
    else        // can not find package, need to re-download and re-install
    {
        m_pItemData->status = PACKAGE_STATUS_NEW_DOWNLOAD;
    }

    delete pFileInfo;
    pFileInfo = NULL;

    m_pItemData = NULL;

    emit sigDeleteLaunchThread(this);
    QThread::exit(0);
}
