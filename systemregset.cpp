#include "systemregset.h"
#include <windows.h>

SystemRegSet::SystemRegSet(QObject *parent) : QObject(parent)
{
    Access.push_back(KEY_WOW64_32KEY);
    Access.push_back(KEY_WOW64_64KEY);
    isWOW64 =OSSystemWrapper:: Instance()->IsOSWin64();
}

SystemRegSet::~SystemRegSet()
{

};


void SystemRegSet::SetRegQT(QString key,QString childkey,QString Value)
{
    //   QString path = "HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall";
    //   QSettings *settings = new QSettings(path, QSettings::NativeFormat);

       //Format涓篞Settings::NativeFormat
       QSettings *settings = new QSettings(key, QSettings::NativeFormat);

       //鍐橦KEY_CURRENT_USER/regedit/test,璁剧疆test鍊间负red
       settings->setValue(childkey, Value);

       delete settings;

       //    QStringList ckey= settings->childGroups();
       //    QVariant name=settings->value("DisplayName");
}

void SystemRegSet::SetRegQT(QString key,QString childkey,unsigned int Value)
{
       //Format涓篞Settings::NativeFormat
       QSettings *settings = new QSettings(key, QSettings::NativeFormat);
        QStringList list=settings->allKeys();

       settings->setValue(childkey, Value);

       delete settings;
}

void SystemRegSet::DelRegKeyQT(QString key,QString childkey)
{
       //Format涓篞Settings::NativeFormat
       QSettings *settings = new QSettings(key, QSettings::NativeFormat);

        if(settings->contains(childkey)){
            settings->remove(childkey);
        }

       delete settings;
}


bool SystemRegSet::SetReg(HKEY hKey,QString lpSubKey,QString name,QString Value)
{

    long lReturn;

    DWORD dwDisposition; //鏌ョ湅鏄惁宸茬粡鏈夌殑锛岃繕鏄柊寤虹殑
    DWORD RegAccessMask;


    if(isWOW64){
    	RegAccessMask=KEY_WOW64_64KEY|KEY_ALL_ACCESS;
    }else{
    	RegAccessMask=KEY_WOW64_32KEY|KEY_ALL_ACCESS;
    }
	HKEY hkResult;            // 灏嗚鎵撳紑閿殑鍙ユ焺
	//lReturn = RegOpenKeyEx(key, (LPCTSTR)lpSubKey.utf16(), 0, KEY_ALL_ACCESS, &hkResult);
    lReturn = ::RegCreateKeyEx(hKey,(LPCTSTR)lpSubKey.utf16(),0,NULL,REG_OPTION_NON_VOLATILE,RegAccessMask,NULL,&hkResult,&dwDisposition);
    if(lReturn==ERROR_SUCCESS){

     lReturn=RegSetValueEx(hkResult,(LPCTSTR)name.utf16(),0,REG_SZ,(CONST BYTE*)Value.data(),Value.size());
     RegCloseKey(hkResult);
     return lReturn==ERROR_SUCCESS;
    }else{
      return  lReturn=-1;
    }
}



bool SystemRegSet::SetReg(HKEY hKey,QString lpSubKey,QString name,unsigned int Value)
{

    long lReturn;

    QString SValue=Value;
    DWORD dwDisposition; //鏌ョ湅鏄惁宸茬粡鏈夌殑锛岃繕鏄柊寤虹殑
    DWORD RegAccessMask;


    if(isWOW64){
        RegAccessMask=KEY_WOW64_64KEY|KEY_ALL_ACCESS;
    }else{
        RegAccessMask=KEY_WOW64_32KEY|KEY_ALL_ACCESS;
    }
    HKEY hkResult;            // 灏嗚鎵撳紑閿殑鍙ユ焺
    //lReturn = RegOpenKeyEx(key, (LPCTSTR)lpSubKey.utf16(), 0, KEY_ALL_ACCESS, &hkResult);
    lReturn = ::RegCreateKeyEx(hKey,(LPCTSTR)lpSubKey.utf16(),0,NULL,REG_OPTION_NON_VOLATILE,RegAccessMask,NULL,&hkResult,&dwDisposition);
    if(lReturn==ERROR_SUCCESS){

         lReturn=RegSetValueEx(hkResult,(LPCTSTR)name.utf16(),0,REG_DWORD,(const BYTE*)SValue.data(),4);
         RegCloseKey(hkResult);
         return lReturn==ERROR_SUCCESS;
    }else{
        return  lReturn=-1;
    }
}


bool SystemRegSet::DelRegUnstallInfo(QString &UnstallKeyValue)
{
    std::vector<HKEY> key;
    char szRegUninstall[MAX_PATH]=("SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Uninstall");
    HKEY hUninstall;
    DWORD RegAccessMask;
    softpath=UnstallKeyValue.toStdString();
    key.push_back(HKEY_LOCAL_MACHINE);
    key.push_back(HKEY_CURRENT_USER);
    findkey.clear();
    findkey.push_back("QuietUninstallString");
    findkey.push_back("UninstallString");

    for(std::vector<DWORD>::iterator it=Access.begin();it!=Access.end();it++)
    {
        RegAccessMask=KEY_ALL_ACCESS|*it;
        for(std::vector<HKEY>::iterator itkey=key.begin();itkey!=key.end();itkey++){
            ResultCount=0;
            FindReg(*itkey,szRegUninstall,RegAccessMask);
        }

    }
    return true;
}

bool SystemRegSet::StealReg(const char *KeyValue,const char *Virus)
{

    if(strcmp(KeyValue,Virus)==0)
    {
        return true;
    }else{

        return false;
    }
}


bool SystemRegSet::FindReg1(HKEY &MAINKEY,char *SubKey,DWORD RegAccessMask)
{
    char temp[MAX_PATH];
    HKEY hKey = NULL;
    char str[MAX_PATH];
    char achValue[MAX_PATH];
    DWORD num = sizeof(str),index = 0,rc;
    DWORD num1=MAX_PATH;

    rc = ::RegOpenKeyExA(MAINKEY,SubKey,0,KEY_ALL_ACCESS,&hKey);
    if(rc == ERROR_SUCCESS)
    {
        while(RegEnumValueA(hKey,index,str,&num,NULL,NULL,(LPBYTE)achValue, &num1)==0 )
        {
            //printf("\t%s\n",str);
            if(StealReg(str,"E:\\Program Files"))
            {
                ResultCount++;
            }
            index++;
            num = MAX_PATH;
            num1=MAX_PATH;
        }

//        for(std::vector<std::string>::iterator it=findkey.begin();it!=findkey.end();it++){
//            if ((RegQueryValueExA(hKey, (*it).c_str(), NULL, NULL, (LPBYTE)achValue, &num) == ERROR_SUCCESS)&& strlen(achValue) > 0)
//            {
//                //处理

//                if(!softpath.compare(achValue)){
//                    return true;
//                }

//            }
//            num = MAX_PATH;
//        }

        index = 0;
        while( RegEnumKeyExA(hKey,index,str,&num,NULL,NULL,NULL,NULL)==0 )
        {   //然后遍历子项后进行递归
            //printf("%s\n",str);
            strcpy(temp,SubKey);
            strcat(temp,"\\");
            strcat(temp,str);
           // FindReg(MAINKEY,temp,RegAccessMask);          //递归

            index++;
            num = MAX_PATH;
        }
    }
    else
    {
        printf("Can't Open The Key!\n");
    }

    RegCloseKey(hKey);

    return false;
}


#define MAX_KEY_LENGTH 255
#define MAX_VALUE_NAME 16383

bool SystemRegSet::FindReg(HKEY &MAINKEY,char *SubKey,DWORD RegAccessMask)
{
    DWORD i = 0, j = 0;
     char achKey[MAX_KEY_LENGTH];
     char achValue[MAX_VALUE_NAME];
     DWORD cbKey = MAX_KEY_LENGTH;
     DWORD cchValue = MAX_VALUE_NAME;
     DWORD dwDataType = 0;
     bool flag=0;
     DWORD cSubKeys = 0;

     HKEY hUninstall, hUninstallSubKey;

     std::vector<std::string> softwareItems;

    if (RegOpenKeyExA(MAINKEY, SubKey, 0, RegAccessMask, &hUninstall) == ERROR_SUCCESS) {
        if (RegQueryInfoKeyA(hUninstall, NULL, NULL, NULL, &cSubKeys, NULL, NULL, NULL, NULL, NULL, NULL, NULL)== ERROR_SUCCESS) {
            for (i = 0; i < cSubKeys; i++) {
                achKey[0] = '\0';
                cbKey = MAX_VALUE_NAME;

                if (RegEnumKeyExA(hUninstall, i, achKey, &cbKey, NULL, NULL, NULL, NULL) == ERROR_SUCCESS) {
                    softwareItems.push_back(std::string(achKey,cbKey));
                }
            }
        }
        qint32 k=softwareItems.size();
        for (i = 0; i < k; i++) {
            achValue[0] = '\0';
            cchValue = MAX_VALUE_NAME;
            if (RegOpenKeyExA(hUninstall, softwareItems[i].data(), 0, KEY_ALL_ACCESS, &hUninstallSubKey) == ERROR_SUCCESS) {
                std::string notContain[3] = { "SystemComponent", "ParentKeyName", "ParentDisplayName" };
                bool bNotContain = false;
                for (j = 0; j < 3; j++) {
                    achValue[0] = '\0';
                    cchValue = MAX_VALUE_NAME;
                    if ((RegQueryValueExA(hUninstallSubKey, notContain[j].c_str(), NULL, NULL, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS) && strlen(achValue) > 0) {
                        bNotContain = true;
                        break;
                    }
                }
                if (bNotContain){
                    continue;
                }


                for(std::vector<std::string>::iterator it=findkey.begin();it!=findkey.end();it++){

                    achValue[0] = '\0';
                    cchValue = MAX_VALUE_NAME;
                    if ((RegQueryValueExA(hUninstallSubKey, (*it).data(), NULL, &dwDataType, (LPBYTE)achValue, &cchValue) == ERROR_SUCCESS) && strlen(achValue) > 0) {

                        if(!softpath.compare(achValue)){
                            //delete key
                             SHDeleteKeyA(hUninstall, softwareItems[i].data());
                            //SHDeleteKeyA(hUninstall, "111");
                            flag=1;
                            break;
                        }
                    }

                }

                RegCloseKey(hUninstallSubKey);
            }
            if(flag){
                break;
            }
        }
        RegCloseKey(hUninstall);
    }
    return flag;
}
