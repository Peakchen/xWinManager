#include "quicksettings.h"
#include <stdio.h>
#include <windows.h>
#include <shlobj.h>
#include <io.h>
#include <QString>
#pragma comment(lib, "shell32.lib")

QuickSettings::QuickSettings()
{

}

//得到当前桌面路径
BOOL GetDesktopPath(char *pszDesktopPath)
{
     LPITEMIDLIST  ppidl = NULL;
     if (SHGetSpecialFolderLocation(NULL, CSIDL_DESKTOP, &ppidl) == S_OK)
     {
         BOOL flag = SHGetPathFromIDListA(ppidl, pszDesktopPath);
         CoTaskMemFree(ppidl);
         return flag;
     }
     return FALSE;
}

//得到快速启动栏的路径
//BOOL GetIEQuickLaunchPath(char *pszIEQueickLaunchPath)
//{
//    LPITEMIDLIST  ppidl;
//    if (SHGetSpecialFolderLocation(NULL, CSIDL_APPDATA, &ppidl) == S_OK)
//    {
//        BOOL flag = SHGetPathFromIDListA(ppidl, pszIEQueickLaunchPath);
//        strcat(pszIEQueickLaunchPath, "\\Microsoft\\Internet Explorer\\Quick Launch");
//        CoTaskMemFree(ppidl);
//        return flag;
//    }
//    return FALSE;
//}
//
//得到 开始->程序组 的路径

//BOOL GetProgramsPath(char *pszProgramsPath)
//{
//    LPITEMIDLIST  ppidl;
//    if (SHGetSpecialFolderLocation(NULL, CSIDL_PROGRAMS, &ppidl) == S_OK)
//    {
//        BOOL flag = SHGetPathFromIDListA(ppidl, pszProgramsPath);
//        CoTaskMemFree(ppidl);
//        return flag;
//    }
//    return FALSE;
//}

/*
 函数功能：对指定文件在指定的目录下创建其快捷方式
 函数参数：
 lpszFileName    指定文件，为NULL表示当前进程的EXE文件。
 lpszLnkFileDir  指定目录，不能为NULL。
 lpszLnkFileName 快捷方式名称，为NULL表示EXE文件名。
 wHotkey         为0表示不设置快捷键
 pszDescription  备注
 iShowCmd        运行方式，默认为常规窗口
 */

BOOL CreateFileShortcut(LPCSTR lpszFileName, LPCSTR lpszLnkFileDir, LPCSTR lpszLnkFileName, LPCWSTR lpszWorkDir, WORD wHotkey, LPCTSTR lpszDescription, int iShowCmd = SW_SHOWNORMAL)
{
    if (lpszLnkFileDir == NULL)
        return FALSE;
    HRESULT hr;
    IShellLink     *pLink;  //IShellLink对象指针
    IPersistFile   *ppf; //IPersisFil对象指针
    //创建IShellLink对象
    hr = CoCreateInstance(CLSID_ShellLink, NULL, CLSCTX_INPROC_SERVER, IID_IShellLink, (void**)&pLink);
    if (FAILED(hr))
        return FALSE;
    //从IShellLink对象中获取IPersistFile接口
    hr = pLink->QueryInterface(IID_IPersistFile, (void**)&ppf);
    if (FAILED(hr))
    {
        pLink->Release();
        return FALSE;
    }
    //目标
    if (lpszFileName == NULL)
    {
        WCHAR wszClassName[256] = {0};
        memset(wszClassName,0,sizeof(wszClassName));
        MultiByteToWideChar(CP_ACP,0,_pgmptr,strlen(_pgmptr)+1,wszClassName,sizeof(wszClassName)/sizeof(wszClassName[0]));
        pLink->SetPath(wszClassName);
    }
    else
    {
        WCHAR sName[256] = {0};
        memset(sName,0,sizeof(sName));
        MultiByteToWideChar(CP_ACP,0,lpszFileName,strlen(lpszFileName)+1,sName,sizeof(sName)/sizeof(sName[0]));
        pLink->SetPath(sName);
    }
    //工作目录
    if (lpszWorkDir != NULL)
        pLink->SetPath(lpszWorkDir);
    //快捷键
    if (wHotkey != 0)
        pLink->SetHotkey(wHotkey);
    //备注
    if (lpszDescription != NULL)
        pLink->SetDescription(lpszDescription);
    //显示方式
    pLink->SetShowCmd(iShowCmd);
    //快捷方式的路径 + 名称
    char szBuffer[MAX_PATH];
    if (lpszLnkFileName != NULL) //指定了快捷方式的名称
        sprintf(szBuffer, "%s\\%s", lpszLnkFileDir, lpszLnkFileName);
    else
    {
        //没有指定名称，就从取指定文件的文件名作为快捷方式名称。
       const char *pstr;
        if (lpszFileName != NULL)
            pstr = strrchr(lpszFileName, '\\');
        else
            pstr = strrchr(_pgmptr, '\\');
        if (pstr == NULL)
        {
            ppf->Release();
            pLink->Release();
            return FALSE;
        }
        //注意后缀名要从.exe改为.lnk
        sprintf(szBuffer, "%s\\%s", lpszLnkFileDir, pstr);
        int nLen = strlen(szBuffer);
        szBuffer[nLen - 3] = 'l';
        szBuffer[nLen - 2] = 'n';
        szBuffer[nLen - 1] = 'k';
    }
    //保存快捷方式到指定目录下
    WCHAR  wsz[MAX_PATH];  //定义Unicode字符串
    MultiByteToWideChar(CP_ACP, 0, szBuffer, -1, wsz, MAX_PATH);
    hr = ppf->Save(wsz, TRUE);
    ppf->Release();
    pLink->Release();
    return SUCCEEDED(hr);
}

void QuickSettings::addXbmgrQuickSettings ()
{
    char  szPath[MAX_PATH];
    CoInitialize(NULL);
    GetDesktopPath(szPath);
    //QString name = QStringLiteral("乐网管家");
    if (CreateFileShortcut(NULL, szPath,NULL, NULL, MAKEWORD(VK_F12, HOTKEYF_CONTROL), L"That is a xbmgr"))
        printf("创建成功\n");
    CoUninitialize();
    return;
}

void QuickSettings::delXbmgrQuickSettings ()
{
     char  szPath[MAX_PATH];
     GetDesktopPath(szPath);
     //*(strrchr( szPath, '\\') ) = 0;
     strcat_s(szPath, "\\xbmgr.lnk");

     if(DeleteFileA("C:\\Users\\YH\\Desktop\\xbmgr.lnk")){
         printf("创建成功\n");
     }
     else{
         printf("删除成功\n");
     }
}

int QuickSettings::find_file_exists ()
{
    char  szPath[MAX_PATH];
    GetDesktopPath(szPath);
    //*(strrchr( szPath, '\\') ) = 0;
    strcat_s(szPath, "\\xbmgr.lnk");
    if(_access(szPath,0)==-1)
    {
        return 1;   //文件不存在
    }else{
        return 0;
    }
}
