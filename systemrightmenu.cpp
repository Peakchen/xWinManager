#include "systemrightmenu.h"
#include <QSettings>
#include <QDebug>
#include <Windows.h>
#include <shlobj.h>
#include <QProcess>
#include <tlhelp32.h>
#include "global.h"
#include <QDir>

systemRightMenu::systemRightMenu()
{

}

systemRightMenu::~systemRightMenu()
{

}



bool systemRightMenu::addSystemRightMenu()    //添加管家到右键菜单
{
    MyRegedit myReg("HKEY_CLASSES_ROOT\\Directory\\Background\\shell");
    if(!myReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\Directory\\Background\\shell InitReg ERROR"<<endl;
        return false;
    }
    QString strinfo = "乐网管家";
    qDebug()<< strinfo.toStdString().c_str() <<endl;
    if(myReg.CreatReg(strinfo))
    {
        qDebug()<<"HKEY CreatReg ERROR"<<endl;
        return false;
    }

    MyRegedit mreg1("HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\乐网管家");
    if(mreg1.InitReg())
    {
        mreg1.SetReg("Icon", GLOBAL::_DY_DIR_RUNNERSELF + "xbmgr.ico", 2);
    }

    MyRegedit mReg("HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\乐网管家");
    if(!mReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\ InitReg ERROR"<< endl;
        return false;

    }
    QString str = "command";
    if(mReg.CreatReg(str))
    {
        qDebug()<<"command ERROR"<<endl;
        return false;
    }


    MyRegedit reg1("HKEY_CLASSES_ROOT\\Directory\\Background\\shell\\乐网管家\\command");
    if(reg1.InitReg())
    {
        char configPath[500] = {NULL};
        HINSTANCE dllModule = GetModuleHandleA("xbmgr.exe");
        GetModuleFileNameA(dllModule, configPath, sizeof(configPath));
        *(strrchr( configPath, '\\') ) = 0;
        strcat_s(configPath, "\\xbmgr.exe launch");
        reg1.SetReg("",configPath,2);
        return true;
    }

    return false;
}

bool systemRightMenu::deleteSystemRightMenu()     //从右键菜单删除管家
{
    MyRegedit regs("HKEY_CLASSES_ROOT\\Directory\\Background\\shell");
    if(regs.InitReg())
    {
        QString sss= "乐网管家";
        if(regs.DelSubItemReg(sss))
        {
             qDebug()<<"DelSubItemReg ERROR"<<endl;
            return false;
        }
        return true;
    }
    return false;
}

DWORD GetProcessIdByName(LPCTSTR pName)
{
    HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
    if (INVALID_HANDLE_VALUE == hSnapshot)
    {
        return 0;
    }
    PROCESSENTRY32 pe = { sizeof(pe) };
    BOOL fOk;
    for (fOk = Process32First(hSnapshot, &pe); fOk; fOk = Process32Next(hSnapshot, &pe))
    {
        if (!_tcscmp(pe.szExeFile, pName))
        {
            CloseHandle(hSnapshot);
            return pe.th32ProcessID;
        }
    }
    CloseHandle(hSnapshot);
    return 0;
}


BOOL KillProcess(DWORD ProcessId)
{
    HANDLE hProcess=OpenProcess(PROCESS_TERMINATE,FALSE,ProcessId);
    if(hProcess==NULL)
        return FALSE;
    if(!TerminateProcess(hProcess,0))
        return FALSE;
    return TRUE;
}


bool systemRightMenu::addRunwindows()   //添加运行到windows菜单
{
    QString ProcessName = "explorer.exe";
    MyRegedit addRun("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
    if(!addRun.InitReg())
    {
        qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer InitReg ERROR"<< endl;
        return false;
    }
    if(!addRun.SetReg("NoRun","0",1))    //"0"显示
    {
        qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer SetReg TRUE"<< endl;
        if(KillProcess(GetProcessIdByName(ProcessName.toStdWString().c_str())))
        {
            //QProcess::startDetached("C:\\Windows\\explorer.exe",QStringList());
        }
        return true;
    }

    qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer SetReg ERROR"<< endl;
    return false;
}


bool systemRightMenu::deleteRunwindows()  //从windows菜单删除运行
{
    QString ProcessName = "explorer.exe";
    MyRegedit addRun("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer");
    if(!addRun.InitReg())
    {
        qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer InitReg ERROR"<< endl;
        return false;
    }
    if(!addRun.SetReg("NoRun","1",1))  //"1"隐藏
    {
        qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer SetReg TRUE"<< endl;
        if(KillProcess(GetProcessIdByName(ProcessName.toStdWString().c_str())))
        {
            //QProcess::startDetached("C:\\Windows\\explorer.exe",QStringList());
        }
        return true;
    }

    qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer SetReg ERROR"<< endl;
    return false;
}


bool CreatDir(QString nCreatDir)
{
    QString regRoot(nCreatDir);
    int ipos = 0;
    int npos = 0;
    QByteArray bytea = regRoot.toLatin1();

    int iCount = 0;
    char *p = bytea.data();
    while(*p != '\0')
    {
        if(*p == '\\')
            iCount++;
        p++;
    }

    for(int i = 0; i <= iCount; i++)
    {
        QString::SectionFlag flag = QString::SectionSkipEmpty;
        QString svc = regRoot.section("\\",npos,ipos,flag);
        ipos = ipos + 1;
        MyRegedit addIE(svc+"\\");
        if(addIE.InitReg())
        {
            continue;
        }
        else
        {
            QString svcstr = regRoot.section("\\",npos,ipos - 2,flag);
            MyRegedit add(svcstr);
            if(add.InitReg())
            {
                QString ret = regRoot.section("\\",ipos - 1,ipos - 1,flag);
                if(add.CreatReg(ret))
                {
                    return false;
                }
            }
        }
    }
    return true;
}

bool systemRightMenu::addIE()
{
    MyRegedit myReg("HKEY_CLASSES_ROOT\\CLSID");
    if(!myReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID InitReg ERROR"<<endl;
        return false;
    }
    QString strrinfo = "{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}";
    qDebug()<< strrinfo.toStdString().c_str() <<endl;
    if(myReg.CreatReg(strrinfo))
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID CreatReg ERROR"<<endl;
        return false;
    }



    MyRegedit mReg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}");
    if(!mReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B} InitReg ERROR"<< endl;
        return false;

    }
    QString stgr = "DefaultIcon";
    if(mReg.CreatReg(stgr))
    {
        qDebug()<<"DefaultIcon ERROR"<<endl;
        return false;
    }

    MyRegedit mdRfeg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\DefaultIcon");
    if(mdRfeg.InitReg())
    {
       mdRfeg.SetReg("","C:\\Program Files\\Internet Explorer\\iexplore.exe,-32528",2);

    }
    //---------------------------------------------------------------------------------------
    MyRegedit myrReg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}");
    if(!myrReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID InitReg ERROR"<<endl;
        return false;
    }
    QString strinfo = "Shell";
    qDebug()<< strinfo.toStdString().c_str() <<endl;
    if(myrReg.CreatReg(strinfo))
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID CreatReg ERROR"<<endl;
        return false;
    }

    MyRegedit mRreg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\Shell");
    if(!mRreg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B} InitReg ERROR"<< endl;
        return false;

    }
    QString stdr = "Open";
    if(mRreg.CreatReg(stdr))
    {
        qDebug()<<"DefaultIcon ERROR"<<endl;
        return false;
    }

    MyRegedit msReg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\Shell\\Open");
    if(!msReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B} InitReg ERROR"<< endl;
        return false;

    }
    QString str = "Command";
    if(msReg.CreatReg(str))
    {
        qDebug()<<"DefaultIcon ERROR"<<endl;
        return false;
    }

    MyRegedit mdReg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\Shell\\Open\\Command");
    if(mdReg.InitReg())
    {
       mdReg.SetReg("","C:\\Program Files\\Internet Explorer\\iexplore.exe",2);
    }

    //-----------------------------------------------------------------------------------

    MyRegedit eeemyReg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\Shell");
    if(!eeemyReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID InitReg ERROR"<<endl;
        return false;
    }
    QString strrdinfo = "NoAddOns";
    qDebug()<< strrdinfo.toStdString().c_str() <<endl;
    if(eeemyReg.CreatReg(strrdinfo))
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID CreatReg ERROR"<<endl;
        return false;
    }

    MyRegedit mRegg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\Shell\\NoAddOns");
    if(!mRegg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B} InitReg ERROR"<< endl;
        return false;

    }
    QString stgrss = "Command";
    if(mRegg.CreatReg(stgrss))
    {
        qDebug()<<"DefaultIcon ERROR"<<endl;
        return false;
    }

    MyRegedit mdRafeg("HKEY_CLASSES_ROOT\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}\\Shell\\NoAddOns\\Command");
    if(mdRafeg.InitReg())
    {
       mdRafeg.SetReg("","C:\\Program Files\\Internet Explorer\\iexplore.exe about:NoAdd-ons",2);

    }
    //------------------------------------------------------------------------------------

    MyRegedit eeemdyReg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID");
    if(!eeemdyReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID InitReg ERROR"<<endl;
        return false;
    }
    QString strrdxinfo = "{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}";
    qDebug()<< strrdxinfo.toStdString().c_str() <<endl;
    if(eeemdyReg.CreatReg(strrdxinfo))
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID CreatReg ERROR"<<endl;
        return false;
    }

    MyRegedit mdgRafeg("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\CLSID\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}");
    if(mdgRafeg.InitReg())
    {
       mdgRafeg.SetReg("","Internet Explorer",2);

    }
    //-------------------------------------------------------------------------------------

    MyRegedit eeemldyReg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");
    if(!eeemldyReg.InitReg())
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID InitReg ERROR"<<endl;
        return false;
    }
    QString strrdxinfjo = "{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}";
    qDebug()<< strrdxinfjo.toStdString().c_str() <<endl;
    if(eeemldyReg.CreatReg(strrdxinfjo))
    {
        qDebug()<<"HKEY_CLASSES_ROOT\\CLSID CreatReg ERROR"<<endl;
        return false;
    }

    MyRegedit mdgRkafeg("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace\\{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}");
    if(mdgRkafeg.InitReg())
    {
       mdgRkafeg.SetReg("","Windows Media",2);

    }

    QFile vbsFile(":/PinIE.vbs");
    if(!vbsFile.open(QIODevice::ReadOnly | QIODevice::Text))
    {
        return false;
    }

    QFile tempVbsFile(GLOBAL::_DY_DIR_RUNNERSELF + "PinIE.vbs");
    if(!tempVbsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
    {
        vbsFile.close();
        return false;
    }

    tempVbsFile.write(vbsFile.readAll().constData());
    tempVbsFile.close();
    vbsFile.close();

    QProcess Process;
    Process.start("wscript", QStringList() << GLOBAL::_DY_DIR_RUNNERSELF + "PinIE.vbs");
    Process.waitForFinished();
    QFile::remove(GLOBAL::_DY_DIR_RUNNERSELF + "PinIE.vbs");
    PostMessage(HWND_BROADCAST,WM_COMMAND,41504,NULL);  //刷新桌面,速度较快
    return true;
}


bool systemRightMenu::deleteIE()
{
    MyRegedit delItem("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace");
    if(!delItem.InitReg())
    {
        qDebug()<<"HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Desktop\\NameSpace InitReg ERROR"<<endl;
        return false;
    }
    if(!delItem.DelSubItemReg("{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B}"))
    {
        qDebug()<<"{B416D21B-3B22-B6D4-BBD3-BBD452DB3D5B} DelSubItemReg OK"<<endl;
        PostMessage(HWND_BROADCAST,WM_COMMAND,41504,NULL);  //刷新桌面,速度较快

        QFile vbsFile(":/UnpinIE.vbs");
        if(!vbsFile.open(QIODevice::ReadOnly | QIODevice::Text))
        {
            return false;
        }

        QFile tempVbsFile(GLOBAL::_DY_DIR_RUNNERSELF + "UnpinIE.vbs");
        if(!tempVbsFile.open(QIODevice::WriteOnly | QIODevice::Text | QIODevice::Truncate))
        {
            vbsFile.close();
            return false;
        }

        tempVbsFile.write(vbsFile.readAll().constData());
        tempVbsFile.close();
        vbsFile.close();

        QProcess Process;
        Process.start("wscript", QStringList() << GLOBAL::_DY_DIR_RUNNERSELF + "UnpinIE.vbs");
        Process.waitForFinished();
        QFile::remove(GLOBAL::_DY_DIR_RUNNERSELF + "UnpinIE.vbs");
        PostMessage(HWND_BROADCAST,WM_COMMAND,41504,NULL);  //刷新桌面,速度较快
        return true;
    }
    return false;
}


bool systemRightMenu::addMyComputer()
{
    MyRegedit addComputer("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel");
    if(!addComputer.InitReg())
    {
        qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel InitReg ERROR"<<endl;
        return false;
    }

    addComputer.SetReg("{20D04FE0-3AEA-1069-A2D8-08002B30309D}","0",1);  //我的电脑
    addComputer.SetReg("{59031a47-3f72-44a7-89c5-5595fe6b30ee}","0",1);  //网络连接
    addComputer.SetReg("{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}","0",1);  //个人文档
    //addComputer.SetReg("{20D04FE0-3AEA-1069-A2D8-08002B30309D}","1",1);
    PostMessage(HWND_BROADCAST,WM_COMMAND,41504,NULL);
    return true;

}

bool systemRightMenu::deleteMyComputer()
{
    MyRegedit addComputer("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel");
    if(!addComputer.InitReg())
    {
        qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\HideDesktopIcons\\NewStartPanel InitReg ERROR"<<endl;
        return false;
    }

    addComputer.SetReg("{20D04FE0-3AEA-1069-A2D8-08002B30309D}","1",1);  //我的电脑
    addComputer.SetReg("{59031a47-3f72-44a7-89c5-5595fe6b30ee}","1",1);  //网络连接
    addComputer.SetReg("{F02C1A0D-BE21-4350-88B0-7367FC96EF3C}","1",1);  //个人文档
    //addComputer.SetReg("{20D04FE0-3AEA-1069-A2D8-08002B30309D}","1",1);
    PostMessage(HWND_BROADCAST,WM_COMMAND,41504,NULL);
    return true;
}

bool systemRightMenu::addNeverCombine(int val)
{
//    ANIMATIONINFO info;
     QString ProcessName = "explorer.exe";
    MyRegedit addCombine("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced");
    if(val == 0)
    {
        if(!addCombine.InitReg())
        {
            qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced InitReg ERROR"<<endl;
            return false;
        }
        addCombine.SetReg("TaskbarGlomLevel","0",1);  //始终合并
        if(KillProcess(GetProcessIdByName(ProcessName.toStdWString().c_str())))
        {
           //QProcess::startDetached("C:\\Windows\\explorer.exe",QStringList());
        }
//        SystemParametersInfo(SPI_SETANIMATION,sizeof(ANIMATIONINFO),&info,SPIF_SENDCHANGE);
//        SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_IDLIST, NULL, NULL); //刷新桌面,速度较慢
    }else {

        if(!addCombine.InitReg())
        {
            qDebug()<<"HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced InitReg ERROR"<<endl;
            return false;
        }
        addCombine.SetReg("TaskbarGlomLevel","2",1);  //从不合并
        if(KillProcess(GetProcessIdByName(ProcessName.toStdWString().c_str())))
        {
            //QProcess::startDetached("C:\\Windows\\explorer.exe",QStringList());
        }
//        SystemParametersInfo(SPI_SETANIMATION,sizeof(ANIMATIONINFO),&info,SPIF_SENDCHANGE);
//        SHChangeNotify(SHCNE_ALLEVENTS, SHCNF_IDLIST, NULL, NULL); //刷新桌面,速度较慢
    }

    return true;
}
