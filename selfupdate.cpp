#include <windows.h>
#include <QApplication>
#include <string>
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>
#include <QMessageBox>
#include <QFile>
#include <direct.h>
#include <io.h>
#include "curl\curl.h"
#include "selfupdate.h"
#include "appenv.h"
#include "swmgrapp.h"
#include "DataControl.h"
#include "character.h"
using namespace std;
# pragma execution_character_set("utf-8")

//用于定时检测软件自身更新的线程回调函数。
UINT ThreadFuncSelfUpdate()
{
    std::string strUpdateUrl = "";
    std::string strVersion = "";
    std::string strFileName = "";

    strUpdateUrl = "http://";
    strUpdateUrl += GLOBAL::_SERVER_URL.toStdString();
    strUpdateUrl += "/api/update";
    strVersion = GLOBAL::_VERSION.toStdString();
    strFileName = GLOBAL::_SOFTWARENAME.toStdString();

    while(1)
    {
        Sleep(1000*10);
        string strReqData = "version=" + strVersion;
        strReqData += "&softname=";
        strReqData += strFileName;
        string strFileUrl;
        string  strFileSize;
        string strRecvVersion;
        if(IsNeedUpdate(strUpdateUrl,strReqData,strFileUrl,strFileSize,strRecvVersion) && CompareVersion(QString::fromStdString(strRecvVersion), QString::fromStdString(strVersion)) > 0)
        {
            string strFilePath = GetLocalAppDataPath() + "\\TempGuanJia.exe";
            MyHttpDownload(strFileUrl,300,strFilePath);
            QFile file(strFilePath.data());
            if(file.open(QIODevice::ReadOnly))
            {
                long iFileSize = file.size();
                long iRecvFileSize = atoi(strFileSize.data());
                file.close();
                if(iFileSize == iRecvFileSize)
                {
                    QTextCodec::setCodecForLocale(QTextCodec::codecForName("utf8"));
                    //::MessageBox(0,L"发现新版本，将为您更新至最新版本\0",L"提示\0",0);
                    ShellExecuteA(NULL, "open", strFilePath.data(), NULL, NULL, SW_HIDE);
                    //此处应该强行退出当前程序，以保证安装不会冲突。
                    SwmgrApp::Instance()->appquit();
                }
            }
            else
            {
                qDebug()<<"下载到的文件大小不符合要求\0"<<endl;
            }
        }
        Sleep(3600*1000);
    }
    return 0;
}

BOOL IsNeedUpdate(std::string strUpdateUrl,std::string strReqData,std::string &strRspData,std::string &strFileSize,std::string &strFileVersion)
{
    strRspData = "";
    strFileSize = "";
    strFileVersion = "";
    string strLinShiRspData;
    MyHttpPost(strUpdateUrl,30,strReqData,strLinShiRspData);

    if(strLinShiRspData.length()>0)                               //如果post之后有数据返回，用qt自带的json解析库解析，
    {
        QByteArray byte_array = strLinShiRspData.data();
        QJsonParseError json_error;
        QJsonDocument parse_doucment = QJsonDocument::fromJson(byte_array, &json_error);
        if(json_error.error == QJsonParseError::NoError)
        {
            if(parse_doucment.isObject())
            {
                QVariantMap result = parse_doucment.toVariant().toMap();
                QVariantMap::iterator jsonCode = result.find("code");
                if(jsonCode != result.end())
                {
                    int iCode = jsonCode.value().toInt();
                    if(iCode == 1)
                    {
                        QVariantMap msgMap = result["msg"].toMap();
                        if(!msgMap.isEmpty())
                        {
                            strRspData = msgMap["file_url"].toString().toStdString();
                            strFileSize = msgMap["file_size"].toString().toStdString();
                            strFileVersion = msgMap["version"].toString().toStdString();
                            return TRUE;
                        }
                    }
                }
            }
        }
    }
    return FALSE;
}

void StartSelfUpdate()
{
    //CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)ThreadFuncSelfUpdate, 0, 0, 0);//定时更新文件信息
}

size_t write_file(void *buffer, size_t size, size_t nmemb, void *userp) {
    FILE *fptr = (FILE *)userp;
    return fwrite(buffer, size, nmemb, fptr);
}

size_t write_data(void *buffer, size_t size, size_t nmemb, void *userp) {
    string *str = (string *)userp;
    size_t len = nmemb * size;
    str->append((char *)buffer, len);
    return len;
}

//int my_progress_func(char *progress_data,double t, /* dltotal */double d, /* dlnow */double ultotal,double ulnow)
/*{
    string strOutput;
    //sprintf(strOutput.c_str(),"%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);
    //printf("%s %g / %g (%g %%)\n", progress_data, d, t, d*100.0/t);
    qDebug()<<"xia zai jin du~~~~~~~~"<<strOutput.data()<<endl;
    //ShowProgressBar(1);
    return 0;
}*/

void MyHttpGet(const string &url, int timeout, string &rspData)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if (curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 0);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rspData);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

void MyHttpPost(const string &url, int timeout, const string &reqBody, string &rspData)
{
    CURL *curl;
    CURLcode res;
    curl = curl_easy_init();
    if(curl)
    {
        curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
        curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, reqBody.size());
        curl_easy_setopt(curl, CURLOPT_POSTFIELDS, reqBody.data());
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &rspData);
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
    }
}

void MyHttpDownload(const string &url, int timeout, const string &path)
{
    CURL *curl;
    CURLcode res;
    FILE *fp;

    fp = fopen(path.c_str(), "wb");
    curl = curl_easy_init();
    if(fp)
    {
        if (curl)
        {
            curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, timeout);
            curl_easy_setopt(curl, CURLOPT_POST, 0);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1);
            curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_file);
            //curl_easy_setopt(curl, CURLOPT_PROGRESSDATA, progress_data);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
        }
        fclose(fp);
    }
}

void KillProgressByName(string strProName)
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
            string strExeFile;
            //WStringToString(te.szExeFile, strExeFile);
            // 把宽字符的进程名转化为ANSI字符串
            char szBuf[512] = { 0 };
            WideCharToMultiByte(CP_ACP, 0, te.szExeFile, wcslen(te.szExeFile), szBuf, sizeof(szBuf), NULL, NULL);
            strExeFile = szBuf;
            if (strcmp(strProName.data(), strExeFile.data()) == 0)
            {
                id = te.th32ProcessID;
                if (id >= 0)
                {
                    HANDLE hProcess = NULL;
                    //打开目标进程
                    hProcess = OpenProcess(PROCESS_TERMINATE, FALSE, id);
                    if (hProcess == NULL)
                    {
                        wprintf(L"\nOpen Process fAiled:%d\n", GetLastError());
                        return;
                    }
                    //结束目标进程
                    DWORD ret = TerminateProcess(hProcess, 0);
                    if (ret == 0)
                    {
                        wprintf(L"%d", GetLastError());
                    }

                }
            }
        }
    }
    CloseHandle(hSnApshot);
    return;
}





string GetVersion(string strFileName)
{
    string strVersion = "";
    struct LANGANDCODEPAGE
    {
        WORD wLanguage;
        WORD wCodePage;
    };
    DWORD dwSize = 0;
    UINT uiSize = GetFileVersionInfoSizeA(strFileName.data(), &dwSize);
    if (0 == uiSize)
    {
        return strVersion;
    }
    PTSTR pBuffer = new TCHAR[uiSize];
    if (NULL == pBuffer)
    {
        return strVersion;
    }
    memset((void*)pBuffer, 0, uiSize);
    //获取exe 或 DLL 的资源信息，存放在pBuffer内
    if (!GetFileVersionInfoA(strFileName.data(), 0, uiSize, (PVOID)pBuffer))
    {
        return strVersion;
    }
    LANGANDCODEPAGE *pLanguage = NULL;  //这里这样设置没关系了。
    UINT  uiOtherSize = 0;
    //获取资源相关的 codepage 和language
    if (!VerQueryValueA(pBuffer, "\\VarFileInfo\\Translation",(PVOID*)&pLanguage, &uiOtherSize))
    {
        return strVersion;
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
        sprintf(SubBlock,"\\StringFileInfo\\%04x%04x\\FileVersion",pLanguage[0].wLanguage,pLanguage[0].wCodePage);
        VerQueryValueA(pBuffer, SubBlock, (PVOID*)&pTmp, &uLen);
        char szVersion[64] = {0};
        memcpy(szVersion,pTmp,uLen);
        strVersion.append(szVersion);
    }
    return strVersion;
}

void StartSC()
{
    //配置文件中的版本号;
    char szSCVersion[64] = {0};
    int szLen = sizeof (szSCVersion);
    GetPrivateProfileStringA("","",NULL,szSCVersion,szLen,"");

    //exe文件自身的版本号,对比
    char buf[1024] = {'\0'};
    std::string szAppdataPath;
    GetEnvironmentVariableA("CommonProgramFiles",buf,1024);
    szAppdataPath.append(buf);

    szAppdataPath.append("\\HurricaneTeam");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\xbsoftMgr");
    _mkdir(szAppdataPath.data());
    szAppdataPath.append("\\PCToolsTest.exe");

    string strVersionRuning = GetVersion(szAppdataPath);

    //QString src = QApplication::applicationDirPath() + "/SCProgressLog.exe";//这个函数获取不到当前的绝对路径。
    string src = GetModulePath();
    src += "\\PCToolsTest.exe";
    string strVersionNew = GetVersion(src);

    SHELLEXECUTEINFOA ShExecInfo = { 0 };
    ShExecInfo.cbSize = sizeof(SHELLEXECUTEINFO);
    ShExecInfo.fMask = SEE_MASK_NOCLOSEPROCESS;
    ShExecInfo.hwnd = NULL;
    ShExecInfo.lpVerb = NULL;
    ShExecInfo.lpFile = src.data();//此处应填写本文件夹下的服务exe的路径，如果需要更新，服务会自动将文件自身复制到指定目录下
    ShExecInfo.lpParameters = "";
    ShExecInfo.lpDirectory = NULL;
    ShExecInfo.nShow = SW_HIDE;
    ShExecInfo.hInstApp = NULL;

    if(strcmp(strVersionNew.data(),strVersionRuning.data())==0)
    {
        ShExecInfo.lpParameters = "start";
    }
    else
    {
        ShExecInfo.lpParameters = "update";
        KillProgressByName("PCToolsTest.exe");         //此处需要杀死进程，否则复制替换文件将失败。
    }
    ShellExecuteExA(&ShExecInfo);
    //WaitForSingleObject(ShExecInfo.hProcess, INFINITE);
    CloseHandle(ShExecInfo.hProcess);
}
//----------------------------------------add by liyong 2015.11.03
