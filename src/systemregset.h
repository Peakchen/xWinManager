#ifndef SYSTEMREGSET_H
#define SYSTEMREGSET_H

#include <QObject>
#include <Windows.h>
#include "OSSystemWrapper.h"
#include <QSettings>
#include<atlbase.h>

class SystemRegSet : public QObject
{
//    Q_OBJECT
public:
    explicit SystemRegSet(QObject *parent = 0);
    ~SystemRegSet();

    void SetRegQT(QString key,QString childkey,QString Value);
    void SetRegQT(QString key,QString childkey,unsigned int Value);
    void DelRegKeyQT(QString key,QString childkey);

    bool SetReg(HKEY hKey,QString lpSubKey,QString name,QString Value);
    bool SetReg(HKEY hKey,QString childkey,QString name,unsigned int Value);
    bool DelRegUnstallInfo(QString &);


    bool isWOW64;
protected:
    std::vector <DWORD> Access;
    std::vector <std::string> findkey;
    std::string softpath;
    bool find;
    qint32 ResultCount;
    bool FindReg(HKEY &MAINKEY,char *SubKey,DWORD RegAccessMask);
    bool FindReg1(HKEY &MAINKEY,char *SubKey,DWORD RegAccessMask);
    bool StealReg(const char *KeyValue,const char *Virus);
signals:

public slots:

};

#endif // SYSTEMREGSET_H
