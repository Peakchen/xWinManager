#ifndef MYREGEDIT_H
#define MYREGEDIT_H
#include<QString>
#include<atlbase.h>
class MyRegedit
{
public:
    MyRegedit(QString strReg);
    ~MyRegedit();

    BOOL InitReg();

    int CreatReg(QString strKeyName);                           //返回0则创建成功
    int SetReg(QString strKeyName,QString strValue ,int flag);  //返回0则设置成功

    int SearchReg(QString strKeyName);                          //返回0则是找到

    int DelSubKeyReg(QString strKeyName);                       //返回0则是删除成功
    int DelSubItemReg(QString strItemName);                     //返回0则是删除成功

    int SetBrowserMenu(int flag,QString strMenuNameh);          //返回0则是设置成功

    int RegBackup(QString strRegPath, QString strRegFilePath);  //返回0则是设置成功
    //dwFlags Long，0表示进行常规恢复。REG_WHOLE_HIVE_VOLATILE表示临时恢复信息（系统重新启动时不保存下来）。在这种情况下，hKey必须引用HKEY_LOCAL_MACHINE 或 HKEY_USERS
    static int RegRestore(QString strRegFilePath);  //返回0则是设置成功,flag为1，是暂时恢复，0为永久恢复
    HKEY getHKEY();
private:
    QString strRegData;
    QString strMainRegKey;
    HKEY lMainKey;
    HKEY hKey;
};

#endif // MYREGEDIT_H
