#ifndef OSSYSTEMWRAPPER_H
#define OSSYSTEMWRAPPER_H

#include <windows.h>
#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <QString>
#include <QDate>

typedef void (WINAPI *PGetSystemInfo)(LPSYSTEM_INFO);

typedef std::map<std::string, std::string> ItemProperty;
typedef std::map<std::string, ItemProperty> mapSoftwareList;

class OSSystemWrapper
{
protected:
    OSSystemWrapper();

public:
    bool IsOSWin64();
    void GetLocalSystemInfo(SYSTEM_INFO);
    DWORD GetSystemInstalledSoftware(mapSoftwareList &mapSoftwares,DWORD type=0); //type=0(x86,x64),type=1(x86),type=2(x64);
public:
    static OSSystemWrapper *Instance();

protected:
    void GetWinInstalledSoftware(mapSoftwareList &mapSoftwares,DWORD flag, HKEY hKey);
protected:
    PGetSystemInfo GetSystemInfoFunc;
};

#endif // OSSYSTEMWRAPPER_H
