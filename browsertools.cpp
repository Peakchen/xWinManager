#include<direct.h>
#include<atlbase.h>
#include<QDebug>
#include"browsertools.h"
#include"myregedit.h"
#include"appenv.h"
using namespace std;

//设置IE主页；
int SetHomePage(QString strHomePageURL)
{
    //修改前，将主页写入配置文件，服务根据配置文件来进行所动守护。
    //待续，，
    std::string szLoadPath = GetAppdataPath("HurricaneTeam");
    szLoadPath.append("\\xbsoftMgr");
    _mkdir(szLoadPath.data());

    std::string strConfFilePath = szLoadPath;
    strConfFilePath.append("\\conf.ini");
    WritePrivateProfileStringA("URL","HomePage",strHomePageURL.toLocal8Bit().data(),strConfFilePath.data());

    //修改user主键下的IE主页；
     MyRegedit myReg("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MAIN");
    if(myReg.InitReg())
    {
        QString strKey = "Start Page";
        return myReg.SetReg(strKey,strHomePageURL, 2);
    }
    else
        return -1;

    //注：组策略实质也是修改注册表，不过这里的这种修改方法，无法在组策略设置中提现出来，但是有效果，因为修改组策略的代码在qt中无法调试通过。
    HKEY hKey;
    long status = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Policies\\Microsoft\\Internet Explorer\\Main", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (status == 0)
    {
        DWORD dw = strHomePageURL.length();
        status = RegSetValueExA(hKey,  "Start Page", NULL,REG_SZ, (BYTE*)strHomePageURL.toLocal8Bit().data(), dw);
        RegCloseKey(hKey);
    }
    status = RegCreateKeyExA(HKEY_CURRENT_USER, "Software\\Policies\\Microsoft\\Internet Explorer\\Control Panel", 0, NULL, REG_OPTION_NON_VOLATILE, KEY_WRITE, NULL, &hKey, NULL);
    if (status == 0)
    {
        DWORD lpData = 1;
        status = RegSetValueExA(hKey, "HomePage", NULL,REG_DWORD, (BYTE*)&lpData, 4);
        RegCloseKey(hKey);
    }
    return 0;
}

//设置默认浏览器；
int SetDefaultBrowser(QString strbrowserPath)
{
    //要在HKEY_CLASSES_ROOT下创建一个自己的浏览器路径，如：HKEY_CLASSES_ROOT\chrome\Shell\open\command，然后command默认键值为浏览器路径，然后将chrome设置为
    //HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice下的Progid键的值
    QString strRegPath = "HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\Shell\\Associations\\UrlAssociations\\http\\UserChoice";
    QString strNewBrowserName = "chrome";
    QString strRootKey = "HKEY_CLASSES_ROOT\\";

    MyRegedit regDefaultBrowser(strRegPath);
    MyRegedit regRoot(strRootKey);

    if(!regDefaultBrowser.InitReg())
        return -1;
    if(regRoot.InitReg())
    {
        if(0 == regRoot.CreatReg(strNewBrowserName))
        {
            QString strRegBrowserName = strRootKey + "\\" + strNewBrowserName;
            MyRegedit regNewBrowserName(strRegBrowserName);
            if(regNewBrowserName.InitReg())
            {
                if(0 == regNewBrowserName.CreatReg("Shell"))
                {
                    QString strBrowserShell = strRegBrowserName + "\\" + "Shell";
                    MyRegedit regBrowserShell(strBrowserShell);
                    if(regBrowserShell.InitReg())
                    {
                        if(0 == regBrowserShell.CreatReg("open"))
                        {
                            QString strBrowserOpen = strBrowserShell + "\\" + "open";
                            MyRegedit regBrowserOpen(strBrowserOpen);
                            if(regBrowserOpen.InitReg())
                            {
                                if(0 == regBrowserOpen.CreatReg("command"))
                                {
                                    QString strBrowserCommand = strBrowserOpen + "\\" + "command";
                                    MyRegedit regBrowserCommand(strBrowserCommand);
                                    if(regBrowserCommand.InitReg())
                                    {
                                        QString strBrowserFullPath = "\"" + strbrowserPath + "\"" + "\"" + "%1" + "\"";
                                        if(0 == regBrowserCommand.SetReg("",strBrowserFullPath, 2))
                                        {
                                            return regDefaultBrowser.SetReg("Progid",strNewBrowserName, 2);
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }
    else
        return -1;
    return -1;
}


//设置IE的默认搜索引擎；
int SetSearchEngine(QString strSearchEngineName,QString strSearchEngineURL)
{
    /*//此方法在win7下不行，xp和win8未测试
     *
     * MyRegedit myReg("HKEY_LOCAL_MACHINE\\Software\\Microsoft\\Internet Explorer\\MAIN");
    if(myReg.InitReg())
    {
        QString strKey = "Search Page";
        //return
        int ret = myReg.SetReg(strKey,strSearchEngineName);
        ret++;
    }
    else
        return -1;*/

    QString strPath = "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Internet Explorer\\SearchScopes";
    MyRegedit myReg(strPath);
    if(myReg.InitReg())
    {
        //创建一个搜索引擎的子项；
        QString strRegSubItem = "SearchProvider_baidu";
        int ret = myReg.CreatReg(strRegSubItem);
        if(ret == ERROR_SUCCESS)
        {
            MyRegedit mySubReg(strPath + "\\" + strRegSubItem);
            if(mySubReg.InitReg())
            {
                ret = mySubReg.SetReg("DisplayName",strSearchEngineName, 2);
                ret = mySubReg.SetReg("Url",strSearchEngineURL, 2);
                if(ret == ERROR_SUCCESS)
                {
                    //将子项填充后，设置为SearchScopes子键DefaultScope的默认值
                    ret = myReg.SetReg("DefaultScope",strRegSubItem, 2);
                    return ret;
                }
            }
        }
    }
    return -1;
}

int SetBrowserMenu(int flag,QString strMenuNameh,QString strFileName)       //设置IE右键菜单的时候，除了要设置在注册表中的键名，还要在键值中加入一个配置html文件路径，或者随便填写一个字符串。
{
    QString strPath = "HKEY_CURRENT_USER\\SOFTWARE\\Microsoft\\Internet Explorer\\MenuExt";
    MyRegedit myReg(strPath);
    if(myReg.InitReg())
    {
        if(1 == flag)
        {
            return myReg.DelSubItemReg(strMenuNameh);
        }
        if(2 == flag)
        {
            int ret = myReg.CreatReg(strMenuNameh);
            if(0 == ret)
            {
                QString strMenuPath = strPath + "\\" + strMenuNameh;
                MyRegedit regMenu(strMenuPath);
                if(regMenu.InitReg())
                {
                    ret = regMenu.SetReg("", strFileName, 2);
                    if(ret != 0)
                        return ret;
                    regMenu.SetReg("Contexts", "", 2);
                    return ret;
                }
            }
        }
    }
    return -1;
}

