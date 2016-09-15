#ifndef GLOBAL_H
#define GLOBAL_H

#include <QString>
#include <windows.h>

extern HANDLE mutex;
extern bool ReadConfini();

class GLOBAL
{
public:
    static QString _DY_DIR_RUNNERSELF;
    static QString _SERVER_URL;
    static QString _VERSION;
    static QString _SOFTWARENAME;
    static QString _PNGDIR;
};
#endif // GLOBAL_H

