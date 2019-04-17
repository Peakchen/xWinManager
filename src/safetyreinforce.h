#ifndef SAFETYREINFORCE_H
#define SAFETYREINFORCE_H
#include <QString>

class safetyReinforce
{
public:
    safetyReinforce();
    ~safetyReinforce();

public:
    bool banUpanStart(int);
    bool banFileName(int);
    bool banDiskShare(int);
    bool banNullLink(int);
    bool banAccountControl(int);
    void searchMenuExt(QString &MenuExtName);
};

#endif // SAFETYREINFORCE_H
