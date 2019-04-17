#include "GetPngThread.h"
#include <QDebug>
#include <QDir>
#include "selfupdate.h"
#include "global.h"
#include "swmgrapp.h"

GetPngThread::GetPngThread(QObject *parent)  : QThread(parent)
{
}

bool GetPngThread::Init(QVariantList &lstPackage)
{
    if(lstPackage.size() == 0)
    {
        return false;
    }

    m_lstPackage = lstPackage;
    return true;
}

void GetPngThread::run()
{
    QVariantMap package;
    QString szPngPath = "";
    QString szIconUrl = "";

    foreach(QVariant item, m_lstPackage)
    {
        package = item.toMap();
        szPngPath = GLOBAL::_PNGDIR + package["id"].toString() + ".png";
        szIconUrl = package["largeIcon"].toString();

        QFile file(szPngPath);
        if(!file.exists())
        {
            MyHttpDownload(szIconUrl.toStdString(), 1,  szPngPath.toStdString());
        }
    }

    emit sigDeleteGetPngThread(this);
}

