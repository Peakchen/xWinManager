#include <windows.h>
#include <direct.h>
#include <fstream>
#include <Shlwapi.h>
#include <QDebug>
#include <QFile>
#include <QJsonDocument>
#include <QJsonObject>
#include <QJsonArray>
#include <QJsonValue>
#include "ThreadToPullSoftwareList.h"
#include "selfupdate.h"
#include "Storage.h"
#include "global.h"
const int packageitem_count = 21;
const int categoryitem_count = 4;
std::string categoryItems[categoryitem_count] = {
    "id",
    "name",
    "alias",
    "total"
};

std::string packageItems[packageitem_count] = {
    "id",
    "packageName",
    "windowsVersion",
    "arch",
    "name",
    "category",
    "largeIcon",
    "incomeShare",
    "rating",
    "versionName",
    "priceInfo",
    "size",
    "updateTime",
    "language",
    "brief",
    "isAd",
    "status",
    "weight",
    "ptdownloadUrl",
    "dlcount",
    "baiduid"
};



ThreadToPullSoftwareList::ThreadToPullSoftwareList(QObject *parent) : QThread(parent)
{
    char buf[1024] = {'\0'};
    std::string szAppdataPath;
    GetEnvironmentVariableA("CommonProgramFiles", buf, 1024);
    szAppdataPath.append(buf);
    szAppdataPath.append("\\HurricaneTeam");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr");
    _mkdir(szAppdataPath.data());
    strDirData = szAppdataPath + "\\Data";          //软件列表文件夹
    _mkdir(strDirData.data());
    strDirTempData = szAppdataPath + "\\Temp";      //临时软件列表文件夹
    _mkdir(strDirTempData.data());

    strURL = "http://";
    strURL += GLOBAL::_SERVER_URL.toStdString();
}


void ThreadToPullSoftwareList::run()
{
    int iCount = 36000;
    while(1)
    {
        if(iCount == 36000)
        {
            while(!ReadConfini())
            {
                Sleep(1000 * 5);
            }

            iCount=0;
            //拉取列表--------------------
//            "http://112.74.194.33/api/swmgr?type=top"
//            "http://112.74.194.33/api/swmgr?type=hot"
//            "http://112.74.194.33/api/swmgr?type=category&cfv=0"
//            "http://112.74.194.33/api/swmgr?type=category&id=1"
//            "http://112.74.194.33/api/swmgr?type=category&cfv=0"			//拉取分类。SoftwareCategory1.list

            //拉取ALL：http://112.74.194.33/api/swmgr?type=all
            char szTempFile[256] = {0};
            sprintf_s(szTempFile,"%s\\SoftwareCategoryAll.list",strDirTempData.data());
            char szFile[256] = {0};
            sprintf_s(szFile,"%s\\SoftwareCategoryAll.list",strDirData.data());
            char szURL[256] = {0};
            sprintf_s(szURL,"%s/api/swmgr?type=category&cfv=0",strURL.data());

            MyHttpDownload(szURL,30,szTempFile);
            chkSoftwareFile(szTempFile,szFile);

            if(m_bExit)
                break;
            //拉取Top：http://112.74.194.33/api/swmgr?type=top
            sprintf_s(szTempFile,"%s\\SoftwareCategoryTOP.list",strDirTempData.data());
            sprintf_s(szFile,"%s\\SoftwareCategoryTOP.list",strDirData.data());
            sprintf_s(szURL,"%s/api/swmgr?type=top",strURL.data());
            MyHttpDownload(szURL,30,szTempFile);
            chkSoftwareFile(szTempFile,szFile);
            if(m_bExit)
                break;
            //拉取hot：http://112.74.194.33/api/swmgr?type=hot
            sprintf_s(szTempFile,"%s\\SoftwareCategoryHOT.list",strDirTempData.data());
            sprintf_s(szFile,"%s\\SoftwareCategoryHOT.list",strDirData.data());
            sprintf_s(szURL,"%s/api/swmgr?type=hot",strURL.data());
            MyHttpDownload(szURL,30,szTempFile);
            chkSoftwareFile(szTempFile,szFile);
            if(m_bExit)
                break;
            //分类别拉取列表。
            for(int i = 1;i<20;i++)
            {
                if(m_bExit)
                    break;
                char szFile[256] = {0};
                sprintf_s(szFile,"%s\\SoftwareCategory%d.list",strDirData.data(),i);
                char szTempFile[256] = {0};
                sprintf_s(szTempFile,"%s\\SoftwareCategory%d.list",strDirTempData.data(),i);
                char szURL[256] = {0};
                sprintf_s(szURL,"%s/api/swmgr?type=category&id=%d",strURL.data(),i);

                MyHttpDownload(szURL,300,szTempFile);
                chkSoftwareFile(szTempFile,szFile);
            }

            if(m_bExit)
                break;
        }
        Sleep(100);
        iCount++;
    }
}

bool ThreadToPullSoftwareList::chkSoftwareFile(const char *szTempFile,const char *szFile )
{
    QJsonDocument doc;
    bool bIsFileOK = false;
    QFile loadFile(szTempFile);
    if(loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        QJsonParseError json_error;
        doc = QJsonDocument::fromJson(fileBuf,&json_error);

        if(json_error.error == QJsonParseError::NoError)
        {
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                if(obj.contains("code"))
                {
                    if(0==obj.value("code").toInt())
                    {
                        bIsFileOK = true;

                    }

                    //                            if(obj.contains("msg"))//这段代码可以解析msg中的内容
                    //                            {
                    //                                QVariantList mapItems = obj.value("msg").toArray().toVariantList();
                    //                                foreach(QVariant item, mapItems)
                    //                                {
                    //                                    QVariantMap data = item.toMap();
                    //                                    qDebug() << data.value("name").toString();
                    //                                }
                    //                            }
                }
            }
        }
        if(bIsFileOK)
        {
            loadFile.close();
            MoveFileExA(szTempFile, szFile, MOVEFILE_REPLACE_EXISTING);
        }
        else
        {
            loadFile.remove();
            loadFile.close();
        }
    }
    return bIsFileOK;
}

bool ThreadToPullSoftwareList::chkSoftwareListFile(const char *szTempFile,const char *szFile )
{
    QJsonDocument doc;
    bool bIsFileOK = false;
    QFile loadFile(szTempFile);
    if(loadFile.open(QIODevice::ReadWrite)) {
        QByteArray fileBuf = loadFile.readAll();
        QJsonParseError json_error;
        doc = QJsonDocument::fromJson(fileBuf,&json_error);

        if(json_error.error == QJsonParseError::NoError)
        {
            if(doc.isObject())
            {
                QJsonObject obj = doc.object();
                if(obj.contains("code"))
                {
                    if(0==obj.value("code").toInt())
                    {
                        bIsFileOK = true;

                    }
                }
            }
        }

        if(bIsFileOK)
        {
            loadFile.close();
            MoveFileExA(szTempFile, szFile, MOVEFILE_REPLACE_EXISTING);
        }
        else
        {
            loadFile.remove();
            loadFile.close();
        }
    }
    return bIsFileOK;
}
