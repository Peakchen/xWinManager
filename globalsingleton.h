#ifndef GLOBALSINGLETON_H
#define GLOBALSINGLETON_H

#include <QString>
#include <windows.h>
class GlobalSingleton
{
protected:
    GlobalSingleton();
	~GlobalSingleton();
public:
    static GlobalSingleton *Instance();
protected:
    HANDLE m_Mutex;
protected:
    void setGlobalMutex(HANDLE hMutex);
};

#endif // GLOBALSINGLETON_H
