#ifndef SYSTEMRIGHTMENU_H
#define SYSTEMRIGHTMENU_H

#include "myregedit.h"

class systemRightMenu
{
public:
    systemRightMenu();
    ~systemRightMenu();
    bool addSystemRightMenu();
    bool deleteSystemRightMenu();
    bool addRunwindows();
    bool deleteRunwindows();
    bool addIE();
    bool deleteIE();
    bool addMyComputer();
    bool deleteMyComputer();
    bool addNeverCombine(int val);
};

#endif // SYSTEMRIGHTMENU_H
