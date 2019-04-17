#include "myregedit.h"
#include<atlbase.h>
#include <QFile>
#include "OSSystemWrapper.h"
#include <windows.h>
MyRegedit::MyRegedit(QString strReg)
{
    strMainRegKey = "";
    strRegData = strReg;
    hKey = 0;
    lMainKey = 0;
}

MyRegedit::~MyRegedit()
{
    if(hKey!=NULL)
        RegCloseKey(hKey);
}

bool isWOW64 = false;
typedef void (WINAPI *LPFN_PGNSI)(LPSYSTEM_INFO);
bool Is64Bit_OS()
{
    bool bRetVal = false;
    SYSTEM_INFO si = { 0 };
    LPFN_PGNSI pGNSI = (LPFN_PGNSI) GetProcAddress(GetModuleHandle(_T("kernel32.dll")), "GetNativeSystemInfo");
    if (pGNSI == NULL)
    {
        return false;
    }
    pGNSI(&si);
    if (si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_AMD64 || si.wProcessorArchitecture == PROCESSOR_ARCHITECTURE_IA64 )
    {
        bRetVal = true;
    }
    else
    {
         //32 位操作系统
        _tprintf(_T("is 32 bit OS\r\n"));
    }
    return bRetVal;
}


BOOL MyRegedit::InitReg()
{
    if(strRegData.length() < 1 && strMainRegKey.length() < 1)
        return FALSE;
    if(strMainRegKey.length() < 1)
    {
        int iPosi = strRegData.indexOf("\\",0);
        if(iPosi>0)
        {
            strMainRegKey = strRegData.left(iPosi);
            strRegData = strRegData.mid(iPosi+1);
        }
        else
        {
            strMainRegKey = strRegData;
            strRegData = "";
        }
    }


    lMainKey = 0;
    if( "HKEY_CLASSES_ROOT" == strMainRegKey)
        lMainKey = HKEY_CLASSES_ROOT;
    else if("HKEY_CURRENT_USER" == strMainRegKey)
        lMainKey = HKEY_CURRENT_USER;
    else if("HKEY_LOCAL_MACHINE" == strMainRegKey)
        lMainKey = HKEY_LOCAL_MACHINE;
    else if("HKEY_USERS" == strMainRegKey)
        lMainKey = HKEY_USERS;
    else if("HKEY_CURRENT_CONFIG" == strMainRegKey)
        lMainKey = HKEY_CURRENT_CONFIG;

    if(lMainKey==0)
        return FALSE;
				//待添加区分32位/64位
    isWOW64 = Is64Bit_OS();
    if(isWOW64){
        long lResult = RegOpenKeyExA(lMainKey,strRegData.toLocal8Bit().data() , 0, KEY_ALL_ACCESS | KEY_WOW64_64KEY, &hKey);
        if(lResult == 0)
            return TRUE;
        else
            return FALSE;
    }else{
        long lResult = RegOpenKeyExA(lMainKey,strRegData.toLocal8Bit().data() , 0, KEY_ALL_ACCESS | KEY_WOW64_32KEY, &hKey);
        if(lResult == 0)
            return TRUE;
        else
            return FALSE;
    }

}

int MyRegedit::SetReg(QString strKeyName,QString strValue ,int flag)       //flag=1,整型，flag=2,字符串型
{
    if(hKey != NULL)
    {
        long lResult = 10;
        if(1 == flag)
        {
            DWORD dw = strValue.toInt();
            lResult = RegSetValueExA(hKey, strKeyName.toLocal8Bit().data(), 0, REG_DWORD, (BYTE*)&dw, sizeof(DWORD));
        }
        if(2 == flag)
        {
            lResult = RegSetValueExA(hKey, strKeyName.toLocal8Bit().data(), 0, REG_SZ, (BYTE*)(strValue.toLocal8Bit().data()), strValue.length());
        }

        return lResult;
    }
    return -1;
}

int MyRegedit::CreatReg(QString strKeyName)       //flag=1,整型，flag=2,字符串型
{
    //return SetReg(strKeyName,strValue);
    if(hKey != NULL)
    {
        DWORD dw;
        HKEY hNewKey = 0;//创建新的注册表项，并不在本对象中使用，所以创建完成就该关闭了。
		//待添加区分32位/64位
        if(isWOW64){
        long lResult = RegCreateKeyExA(hKey, strKeyName.toLocal8Bit().data(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WOW64_64KEY , NULL, &hNewKey, &dw);
        if(hNewKey)
        {
            RegCloseKey(hNewKey);
        }
        //if(hNewKey != 0 && lResult == 0)
          //  hKey = hNewKey;
        return lResult;
        }else{
            long lResult = RegCreateKeyExA(hKey, strKeyName.toLocal8Bit().data(), 0, REG_NONE, REG_OPTION_NON_VOLATILE, KEY_ALL_ACCESS|KEY_WOW64_32KEY , NULL, &hNewKey, &dw);
            if(hNewKey)
            {
                RegCloseKey(hNewKey);
            }
            //if(hNewKey != 0 && lResult == 0)
              //  hKey = hNewKey;
            return lResult;
        }
    }
    return -1;
}

int MyRegedit::SearchReg(QString strKeyName)
{
    if(hKey != NULL)
    {
        char szPath[MAX_PATH];
        DWORD dwSize = sizeof(szPath);
        long lResult = RegQueryValueExA(hKey, strKeyName.toLocal8Bit().data(), 0, 0, (LPBYTE)szPath, &dwSize);
        return lResult;
    }
    return -1;
}

int MyRegedit::DelSubKeyReg(QString strKeyName)
{
    if(hKey != NULL)
    {
        long lResult = RegDeleteValueA(hKey, strKeyName.toLocal8Bit().data());
        return lResult;
    }
    return -1;
}

int MyRegedit::DelSubItemReg(QString strItemName)
{
    if(hKey != NULL)
    {
        long lResult = SHDeleteKeyA(hKey, strItemName.toLocal8Bit().data());
        return lResult;
    }
    return -1;
}



/*int MyRegedit::RegBackup(QString strRegFilePath,QString strRegPath)  //返回0则是设置成功
{
    // 申请备份权限
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
        return -1;
    LookupPrivilegeValue(NULL, SE_BACKUP_NAME, &tkp.Privileges[0].Luid);//申请SE_BACKUP_NAME权限
    tkp.PrivilegeCount=1;
    tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    if(!AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES)NULL, 0))
        return -1;

    if(hKey != NULL)
    {
        long lResult =  RegSaveKeyA(hKey, strRegFilePath.toLocal8Bit().data(), NULL);
        return lResult;
    }
    return -1;
}*/

/*//Flags ，0表示进行常规恢复。REG_WHOLE_HIVE_VOLATILE表示临时恢复信息（系统重新启动时不保存下来）。在这种情况下，hKey必须引用HKEY_LOCAL_MACHINE 或 HKEY_USERS
int MyRegedit::RegRestore(QString strRegFilePath,  int flag)  //返回0则是设置成功,flag为真，是暂时恢复，假为永久恢复
{
    // 申请权限
    HANDLE hToken;
    TOKEN_PRIVILEGES tkp;
    if(!OpenProcessToken(GetCurrentProcess(),TOKEN_ADJUST_PRIVILEGES|TOKEN_QUERY, &hToken))
        return -1;
    LookupPrivilegeValue(NULL, SE_RESTORE_NAME, &tkp.Privileges[0].Luid);//申请SE_RESTORE_NAME权限
    tkp.PrivilegeCount=1;
    tkp.Privileges[0].Attributes=SE_PRIVILEGE_ENABLED;
    if(!AdjustTokenPrivileges(hToken,FALSE,&tkp,0,(PTOKEN_PRIVILEGES)NULL, 0))
        return -1;

    if(hKey != NULL)//strRegFilePath.toStdString().c_str();
    {
        flag++; //恢复方式不能被指定，只有用这种强制强力恢复才能调用成功，这行代码完全是为了不让警告提示。
        long lResult = RegRestoreKeyA(hKey, strRegFilePath.toLocal8Bit().data(), REG_FORCE_RESTORE);
        return lResult;
    }
    return -1;
}*/


//调用cmd命令的方法来备份注册表
int MyRegedit::RegBackup(QString strRegPath,QString strRegFilePath)  //返回0则是设置成功
{
    if(strRegFilePath.length() < 1 || strRegPath.length() < 1)
        return -1;
    int iPosi = strRegPath.indexOf("\\",0);
    QString strRootKey = "";
    if(iPosi >= 0)
    {
        strRootKey = strRegPath.left(iPosi);
        strRegPath = strRegPath.mid(iPosi);
    }
    else
    {
       strRootKey = strRegPath;
       strRegPath = "";
    }

    if( "HKEY_CLASSES_ROOT" == strRootKey)
        strRootKey = "hkcr";
    else if("HKEY_CURRENT_USER" == strRootKey)
        strRootKey = "hkcu";
    else if("HKEY_LOCAL_MACHINE" == strRootKey)
        strRootKey = "hklm";
    else if("HKEY_USERS" == strRootKey)
        strRootKey = "hku";
    else if("HKEY_CURRENT_CONFIG" == strRootKey)
        strRootKey = "hkcc";

    QString strCmd = "reg export ";
    strCmd += strRootKey + strRegPath + " ";
    strCmd += strRegFilePath;

    TCHAR szApp[MAX_PATH] = {0};
    strCmd.toWCharArray(szApp);

	PROCESS_INFORMATION ProcessInfo = { 0 };
	STARTUPINFO StartupInfo = { sizeof(StartupInfo) };
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.wShowWindow = SW_HIDE;

	if (CreateProcess(NULL, szApp, NULL, NULL, TRUE, NULL, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
		return 0;
	}
	return -1;
}


int MyRegedit::RegRestore(QString strRegFilePath)  //返回0则是设置成功,flag为真，是暂时恢复，假为永久恢复
{
    QFile file(strRegFilePath);
    if (!file.exists())
    {
        //qDebug()<<"文件不存在"<<endl;
        return -1;
    }
    QString strCmd = "reg import ";
    strCmd += strRegFilePath;


    TCHAR szApp[MAX_PATH] = {0};
    strCmd.toWCharArray(szApp);

	PROCESS_INFORMATION ProcessInfo = { 0 };
	STARTUPINFO StartupInfo = { sizeof(StartupInfo) };
	StartupInfo.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	StartupInfo.wShowWindow = SW_HIDE;

	if (CreateProcess(NULL, szApp, NULL, NULL, TRUE, NULL, NULL, NULL, &StartupInfo, &ProcessInfo))
	{
		WaitForSingleObject(ProcessInfo.hProcess, INFINITE);
		CloseHandle(ProcessInfo.hThread);
		CloseHandle(ProcessInfo.hProcess);
		return 0;
	}
	return -1;
}

 HKEY MyRegedit::getHKEY()
 {
    return hKey;
 }


