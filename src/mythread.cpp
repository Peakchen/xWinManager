#include <windows.h>
#include <string>
#include <direct.h>
#include <io.h>
#include <QDebug>
#include <QFile>
#include "swmgrapp.h"
#include "mythread.h"
#include "selfupdate.h"
#include "character.h"
#include "appenv.h"
using std::string;
MyThread::MyThread()
: QThread()
{
    isExit = false;
}
MyThread::~MyThread()
{

}
void MyThread::run()
{
    std::string strUpdateUrl = "";
    std::string strVersion = "";
    std::string strFileName = "";

    strUpdateUrl = "http://";
    strUpdateUrl += GLOBAL::_SERVER_URL.toStdString();
    strUpdateUrl += "/api/update";
    strVersion = GLOBAL::_VERSION.toStdString();
    strFileName = GLOBAL::_SOFTWARENAME.toStdString();

    int i = 3590 * 10;
    while(!isExit)
    {
        Sleep(100);
        i++;
        if(i == 3600 * 10)
        {
            i = 0;

            string strReqData = "version=" + strVersion;
            strReqData += "&softname=";
            strReqData += strFileName;
            string strFileUrl;
            string  strFileSize;
            string strRecvVersion;
            //qDebug()<<strCurrentDir.data()<<"wo cong service shou dao ~~~~~~~~~~~~~~~~~~\n\n\n\n\n\n\n~~~~~~~~~~~~~:"<<strRspData.data()<<"\n"<<strUpdateUrl.data()<<"\n"<<strReqData.data()<<endl;
            if(IsNeedUpdate(strUpdateUrl,strReqData,strFileUrl,strFileSize,strRecvVersion) && CompareVersion(QString::fromStdString(strRecvVersion), QString::fromStdString(strVersion)) > 0)                               //如果需要更新，则下载更新。
            {
                string strFilePath = GetLocalAppDataPath() + "\\TempGuanJia.exe";
                emit SendUpdateMsgToGUI(strFilePath.data());
                break;
            }
        }
    }

    return;
}
