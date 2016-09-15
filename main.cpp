#include <QApplication>
#include <QUrl>
#include <QResource>
#include <QSystemSemaphore>
#include <QtDebug>
#include <io.h>
#include <direct.h>
#include <windows.h>
#include "swmgrapp.h"
#include "global.h"
#include "globalsingleton.h"
#include "selfupdate.h"
#include "logwriter.h"
#include "character.h"

#pragma comment(lib, "Version.lib")
QString GLOBAL::_DY_DIR_RUNNERSELF="";
QString GLOBAL::_VERSION="1.0.0.0";
QString GLOBAL::_SERVER_URL="";
QString GLOBAL::_SOFTWARENAME="guanjia";
QString GLOBAL::_PNGDIR="";

void adjustProcessPrivilege()
{
    HANDLE hProcess = OpenProcess(MAXIMUM_ALLOWED, FALSE, GetCurrentProcessId());
    HANDLE hPToken = INVALID_HANDLE_VALUE;
    LUID luid;
    TOKEN_PRIVILEGES tp;
    if(hProcess != INVALID_HANDLE_VALUE && ::OpenProcessToken(hProcess, TOKEN_ADJUST_PRIVILEGES | TOKEN_QUERY | TOKEN_DUPLICATE | TOKEN_ASSIGN_PRIMARY | TOKEN_ADJUST_SESSIONID | TOKEN_READ | TOKEN_WRITE, &hPToken))
    {
        //获得令牌句柄
        if(hPToken != INVALID_HANDLE_VALUE && LookupPrivilegeValue(NULL, SE_DEBUG_NAME, &luid))
        {
            tp.PrivilegeCount = 1;
            tp.Privileges[0].Luid = luid;
            tp.Privileges[0].Attributes = SE_PRIVILEGE_ENABLED;
            if(AdjustTokenPrivileges(hPToken, FALSE, &tp, sizeof(TOKEN_PRIVILEGES), (PTOKEN_PRIVILEGES)NULL, NULL))
            {
                qDebug() << "adjust process privileges.";
            }
            else {
                qDebug() << GetLastError();
            }
            CloseHandle(hPToken);
        }
        else
        {
            qDebug() << GetLastError();;
        }
        CloseHandle(hProcess);
    }
    else
    {
        qDebug() << GetLastError();;
    }
}

//----------------------------------------add by liyong 2015.10.24
HANDLE mutex;
UINT ThreadFunc()
{
    char buf[1024] = {'\0'};
    std::string szAppdataPath;
    GetEnvironmentVariableA("CommonProgramFiles", buf, 1024);
    szAppdataPath.append(buf);
    szAppdataPath.append("\\HurricaneTeam");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr.lock");

    while (1)
    {
        WaitForSingleObject(mutex, INFINITE);
        FILE *fp;
        fopen_s(&fp, szAppdataPath.data(), "w");
        if(fp)
        {
            fprintf(fp, "1");
            fclose(fp);
         //   printf("我在正常运行~\n");
        }
        ReleaseMutex(mutex);
        Sleep(2000);
    }
    return 0;
}

UINT ThreadFuncGetSoftList()            //定期杀死并重新启动拉取服务端列表的进程。
{
    KillProgressByName("xbmgrsvc.exe");
    ShellExecuteA(NULL, "open", "xbmgrsvc.exe", "normal", NULL, SW_HIDE);
    StartSC();                  //启动服务
    return 0;
}

void InitMutexLock()
{
    mutex = OpenMutex(SYNCHRONIZE, FALSE, TEXT("Global\\MutexLockXbmgr"));
    if(mutex == NULL)
    {
        mutex = CreateMutex(NULL, FALSE, TEXT("Global\\MutexLockXbmgr"));
    }
    if(mutex == NULL)
    {
        return ;
    }

    CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFunc, 0, 0, 0);//定时更新文件信息
   // CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFuncGetSoftList, 0, 0, 0);//定期杀死并重新启动拉取服务端列表的进程。
}

void KillProgressByName(char* strProName)
{
    //进程列举
    int id = -1;
    HANDLE hSnApshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (hSnApshot != INVALID_HANDLE_VALUE)
    {
        PROCESSENTRY32 te = { sizeof(te) };
        BOOL f0k = Process32First(hSnApshot, &te);
        for (; f0k; f0k = Process32Next(hSnApshot, &te))
        {
            //WStringToString(te.szExeFile, strExeFile);
            // 把宽字符的进程名转化为ANSI字符串
            char szBuf[512] = { 0 };
            WideCharToMultiByte(CP_ACP, 0, te.szExeFile, wcslen(te.szExeFile), szBuf, sizeof(szBuf), NULL, NULL);
            if (strcmp(strProName, szBuf) == 0)
            {
                id = te.th32ProcessID;
                if (id >= 0)
                {
                    HANDLE hProcess = NULL;
                    //打开目标进程
                    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, id);
                    if (hProcess == NULL)
                    {
                        printf("\nOpen Process fAiled:%d\n", GetLastError());
                        return;
                    }
                    //结束目标进程
                    DWORD ret = TerminateProcess(hProcess, 0);
                    if (ret == 0)
                    {
                        printf("%d", GetLastError());
                    }

                }
            }
        }
    }
    CloseHandle(hSnApshot);
    return;
}

bool ReadConfini()
{
    QFile iniFile(":/support/conf.ini");
    if(!iniFile.open(QIODevice::ReadOnly | QIODevice::Truncate))
    {
        GLOBAL::_VERSION = "1.0.0.0";
        GLOBAL::_SERVER_URL = "";
        GLOBAL::_SOFTWARENAME = "guanjia";
        return false;
    }

    bool bVersion = false;
    bool bServerUrl = false;
    bool bSoftwareName = false;
    QTextStream in(&iniFile);
    QString line = "";
    while(!in.atEnd())
    {
        if(bVersion && bServerUrl && bSoftwareName)
        {
            break;
        }
        line = in.readLine();
        if(line.contains("SelfVersion"))
        {
            bVersion = true;
            GLOBAL::_VERSION = line.split("=", QString::SkipEmptyParts).at(1);
            continue;
        }
        if(line.contains("SelfUpdateUrl"))
        {
            bServerUrl = true;
            std::string strMultiIP = line.split("=", QString::SkipEmptyParts).at(1).toStdString();
            std::string strURLHead = "http://";
            std::string strURL;
            bool bIsIPOK = false;
            while (strMultiIP.length() > 0)
            {
                int iPos = strMultiIP.find(",");
                std::string strIP;
                if (iPos > 0)
                {
                    strIP = strMultiIP.substr(0, iPos);
                    strMultiIP = strMultiIP.substr(iPos + 1, strMultiIP.length());
                }
                else
                {
                    strIP = strMultiIP;
                    strMultiIP = "";
                }
                strURL = strURLHead + strIP + "/api/swmgr?type=category&id=1&count=1&start=0";
                std::string strRecv;
                MyHttpGet(strURL, 1, strRecv);
                int iRet = strRecv.find("\"code\":0");
                if (iRet >= 0)
                {
                    GLOBAL::_SERVER_URL= strIP.data();
                    bIsIPOK = true;
                    break;
                }
            }
            if(!bIsIPOK)
            {
                GLOBAL::_SERVER_URL= "";
            }
            continue;
        }
        if(line.contains("SelfFileName"))
        {
            bSoftwareName = true;
            GLOBAL::_SOFTWARENAME = line.split("=", QString::SkipEmptyParts).at(1);
            continue;
        }
    }
    if(GLOBAL::_VERSION.isEmpty())
    {
        GLOBAL::_VERSION = "1.0.0.0";
    }
    if(GLOBAL::_SOFTWARENAME.isEmpty())
    {
        GLOBAL::_SOFTWARENAME = "guanjia";
    }
    if(GLOBAL::_SERVER_URL.isEmpty())
    {
        return false;
    }
    return true;
}

int main(int argc, char *argv[])
{
    adjustProcessPrivilege();   //进程提权
    InitMutexLock();            //启动互斥锁，供服务监控软件运行状态

    QApplication a(argc, argv);

    GLOBAL::_DY_DIR_RUNNERSELF = QApplication::applicationDirPath();
    if(GLOBAL::_DY_DIR_RUNNERSELF.right(1).compare(QDir::separator()))
    {
        GLOBAL::_DY_DIR_RUNNERSELF += QDir::separator();
    }

    if(!ReadConfini())
    {
        MessageBox(NULL, L"无法连接服务器， 正在努力连接服务器中...", L"乐网软件", MB_OK);
    }

    /*
     * Add lib path, must be it!
     * Because production env not find plugins for qt plugin window dll.
     * PS: Or use qt.conf fix it problem. Recommend use QApplication::addLibraryPath
     */
    QApplication::addLibraryPath(GLOBAL::_DY_DIR_RUNNERSELF);
    QApplication::addLibraryPath(GLOBAL::_DY_DIR_RUNNERSELF + "plugins");

    GlobalSingleton *_instance = GlobalSingleton::Instance();
    if(_instance == NULL)
    {
        qDebug() << "instance already exist!";
        return 1;
    }

    if(!SwmgrApp::Instance()->InitAppEnv())
    {
        qDebug() << "Initial error!";
        return 1;
    }

    return a.exec();
}
