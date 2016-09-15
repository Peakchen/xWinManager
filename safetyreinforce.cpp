#include "safetyreinforce.h"
#include <QDebug>
#include <string>
#include <QSettings>
#include <shlobj.h>
#include "myregedit.h"
#include<atlbase.h>
#include <iostream>
#include <windows.h>
#include <string>

safetyReinforce::safetyReinforce()
{

}

safetyReinforce::~safetyReinforce()
{

}

bool safetyReinforce::banUpanStart(int val)   //禁止U盘等所有磁盘自启动
{
    int regedit = NULL;
    QSettings *reg = NULL;
    reg = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Policies\\Explorer",QSettings::NativeFormat);
    if(reg == NULL)
    {
        return false;
    }
    regedit = reg->value("NoDriveTypeAutoRun",QVariant()).toInt();
    qDebug()<< regedit <<endl;

    switch (val) {
    case 0:      //关闭
        //regedit = regedit & (0x7FFFFFFF ^ 2);
        reg->setValue("NoDriveTypeAutoRun",255);
        qDebug()<< reg->value("NoDriveTypeAutoRun",QVariant()).toInt() <<endl;
        break;
    case 1:     //开启
        //regedit = regedit | (0 ^ 2);
        reg->setValue("NoDriveTypeAutoRun",0);
        qDebug()<< reg->value("NoDriveTypeAutoRun",QVariant()).toInt() <<endl;
       break;
    default:
        return false;
    }
    delete reg;
    reg = NULL;
    qDebug()<< "------------------------------------------" <<endl;
    return true;
}

bool safetyReinforce::banFileName(int val)   //禁止文件扩展名
{
    QSettings *reg = NULL;
    int HideFileExt = NULL;
    reg = new QSettings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Explorer\\Advanced",QSettings::NativeFormat);
    if(reg == NULL)
    {
        return false;
    }
    HideFileExt = reg->value("HideFileExt",QVariant()).toInt();
    qDebug()<< "HideFileExt = " << HideFileExt << endl;

    switch (val) {
    case 0:     //不隐藏
        reg->setValue("HideFileExt",0);
        qDebug()<< "change ofter HideFileExt = " << reg->value("HideFileExt",QVariant()).toInt() << endl;
        break;
    case 1:   //隐藏
        reg->setValue("HideFileExt",1);
        qDebug()<< "change ofter HideFileExt = " << reg->value("HideFileExt",QVariant()).toInt() << endl;
        break;
    default:
        return false;
    }
    delete reg;
    reg = NULL;
    qDebug()<< "------------------------------------------" <<endl;
    PostMessage(HWND_BROADCAST,WM_COMMAND,41504,NULL);
    return true;
}

bool safetyReinforce::banDiskShare(int val)    //禁止磁盘共享
{
    QSettings *reg = NULL;
    QSettings *Lsa = NULL;
    int AutoShareServer = NULL;
    int AutoShareWks  = NULL;
    int restrictanonymous  = NULL;
    reg = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Services\\lanmanserver\\parameters",QSettings::NativeFormat);
    Lsa = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\Control\\Lsa",QSettings::NativeFormat);
    if(reg == NULL && Lsa == NULL)
    {
        return false;
    }
    AutoShareServer = reg->value("AutoShareServer",QVariant()).toInt();
    AutoShareWks = reg->value("AutoShareWks",QVariant()).toInt();
    restrictanonymous = Lsa->value("restrictanonymous",QVariant()).toInt();
    qDebug()<< "AutoShareServer = " << AutoShareServer <<"  " <<"AutoShareWks = " << AutoShareWks << "restrictanonymous = " <<restrictanonymous<<endl;

    switch (val) {
    case 0:     //关闭
        reg->setValue("AutoShareServer",0);
        reg->setValue("AutoShareWks",0);
        Lsa->setValue("restrictanonymous",1);

        qDebug()<< "change ofter AutoShareServer = " << reg->value("AutoShareServer",QVariant()).toInt()
                <<"  " <<"AutoShareWks = "     << reg->value("AutoShareWks",QVariant()).toInt() <<"  "
               << "restrictanonymous = " <<Lsa->value("restrictanonymous",QVariant()).toInt() << endl;
        break;
    case 1:   //打开
        reg->setValue("AutoShareServer",1);
        reg->setValue("AutoShareWks",0);
        Lsa->setValue("restrictanonymous",0);
        qDebug()<< "change ofter AutoShareServer = " << reg->value("AutoShareServer",QVariant()).toInt()
                <<"  " <<"AutoShareWks = "     << reg->value("AutoShareWks",QVariant()).toInt() <<"  "
               << "restrictanonymous = " <<Lsa->value("restrictanonymous",QVariant()).toInt() << endl;
        break;
    default:
        return false;;
    }
    delete reg;
    delete Lsa;
    reg = NULL;
    Lsa = NULL;
    qDebug()<< "------------------------------------------" <<endl;
    return true;
}

bool safetyReinforce::banNullLink(int val)   //禁止IPC$空链接
{
    QSettings *reg = NULL;
    int SMBDeviceEnabled = NULL;
    reg = new QSettings("HKEY_LOCAL_MACHINE\\SYSTEM\\CurrentControlSet\\services\\NetBT\\Parameters",QSettings::NativeFormat);
    if(reg == NULL)
    {
        return false;
    }
    SMBDeviceEnabled = reg->value("SMBDeviceEnabled",QVariant()).toInt();
    qDebug()<< "SMBDeviceEnabled = " << SMBDeviceEnabled << endl;

    switch (val) {
    case 0:     //关闭
        reg->setValue("SMBDeviceEnabled",0);
        qDebug()<< "change ofter SMBDeviceEnabled = " << reg->value("SMBDeviceEnabled",QVariant()).toInt() << endl;
        break;
    case 1:   //打开
        reg->setValue("SMBDeviceEnabled",1);
        qDebug()<< "change ofter SMBDeviceEnabled = " << reg->value("SMBDeviceEnabled",QVariant()).toInt() << endl;
        break;
    default:
        return false;
    }
    delete reg;
    reg = NULL;
    qDebug()<< "------------------------------------------" <<endl;
    return true;
}

bool safetyReinforce::banAccountControl(int val)   //开启用户windows账户控制
{
    QSettings *reg = NULL;
    int EnableLUA = NULL;
    reg = new QSettings("HKEY_LOCAL_MACHINE\\SOFTWARE\\Microsoft\\Windows\\CurrentVersion\\Policies\\System",QSettings::NativeFormat);
    if(reg == NULL)
    {
        return false;
    }
    EnableLUA = reg->value("EnableLUA",QVariant()).toInt();
    qDebug()<< "EnableLUA = " << EnableLUA << endl;

    switch (val) {
    case 0:     //关闭
        reg->setValue("EnableLUA",0);
        qDebug()<< "change ofter EnableLUA = " << reg->value("EnableLUA",QVariant()).toInt() << endl;
        break;
    case 1:   //打开
        reg->setValue("EnableLUA",1);
        qDebug()<< "change ofter EnableLUA = " << reg->value("EnableLUA",QVariant()).toInt() << endl;
        break;
    default:
        return false;
    }
    delete reg;
    reg = NULL;
    qDebug()<< "------------------------------------------" <<endl;
    return true;
}

void safetyReinforce::searchMenuExt(QString &MenuExtName)
{
    MyRegedit myReg("HKEY_CURRENT_USER\\Software\\Microsoft\\Internet Explorer\\MenuExt");
    if(myReg.InitReg())
    {
        int index = 0;
        TCHAR szKeyName[255] = { 0 };        // 注册表项名称

        DWORD dwKeyLen = sizeof(szKeyName);
        while (ERROR_NO_MORE_ITEMS != RegEnumKeyEx(myReg.getHKEY(), index, szKeyName, &dwKeyLen, 0, NULL, NULL, NULL))
        {
            index++;
            QString szKeyNameinfo = QString::fromStdWString(std::wstring(szKeyName));
            memset(szKeyName,0,sizeof(szKeyName));
            dwKeyLen = 255;
            MenuExtName += (szKeyNameinfo +  ",");
        }
        MenuExtName = MenuExtName.left(MenuExtName.length() - 1);
        qDebug() << "MenuExtName = "<< MenuExtName << endl;
    }
}
