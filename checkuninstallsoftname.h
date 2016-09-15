#ifndef CHECKUNINSTALLSOFTNAME_H
#define CHECKUNINSTALLSOFTNAME_H



#include <Windows.h>
#include <QString>
#include <QDebug>
#include <QObject>
#include <QThread>

#include <tlhelp32.h>

#include <QtGui>
#include <QtWin>
#include <QGuiApplication>
#include <QScopedArrayPointer>
#include <QStringList>
#include <QPixmap>
#include <QImage>
#include <QFileInfo>
#include <QDir>
#include <iostream>


//#include "swmgrapp.h"



typedef QString*    PQString;

typedef QMap<QString,quint64> ItemSize;
typedef QVector<QString>  QStringVector;

//typedef std::map<std::string,quint64> ItemSize;

class CGetFolderSize : public QObject
{
    Q_OBJECT
public:
    explicit CGetFolderSize(QStringList&,QObject *parent = 0);
    void stop();
    ItemSize FolderSizeMap;
signals:
    void result_ready();
public slots:
protected:
    void get();

    quint64 calc(QString &);
private:
    volatile bool stopped;
    QStringList folderlist;
    QDir dir;
    quint64 folorSize;


};



class CheckUninstallSoftName
{
public:
    explicit CheckUninstallSoftName();
    virtual ~CheckUninstallSoftName();
private:
    const int ROLNUM=5;

    int Getidx(int m);
    QString *checkUnString;
public:
    void GetArgList(QStringList &arguments,QString &argg,QString &find_str);
    bool GetUninstallNameArg(QString &name,QStringList &arguments);
};


class CRunUnstall: public QObject
{
    Q_OBJECT
public:
     explicit CRunUnstall(QObject *parent = 0);
     virtual ~CRunUnstall();

     BOOL CheckProcessId(DWORD pId);
     bool MD5asSoftisExist(QString uninstallID);

protected:


    QStringList MultiSoftPath;
    QStringList UnstallMd5;

    QTimer *processIdtime;
    quint64 pid;

void RunUninstall();

signals:

public slots:

//void Uninstall_queue();
//void UninstallerFinished(int exitCode, QProcess::ExitStatus exitStatus);

};



typedef QMap<QString,QString> QStringMap;
typedef QList<QStringMap> QStringListMap;
typedef QMap<QString,QVariantMap> QVariantMapMap;


class CGetIconFromFile: public QObject
{
    Q_OBJECT
public:
     explicit CGetIconFromFile(QObject *parent = 0);
     virtual ~CGetIconFromFile();

     bool GetFilename(QString &src_name,QString &path,QString &filename,QString &suffix,QString &);
    bool SaveIconFile(QVariantList &);
    bool SaveOneIcon(QVariantMap &);
    bool GetFileNameList(QVariantMap &);
    bool GetUninstallName(QString&);
    void DealWithUnstallString(QString);
    void convertsystemdir(QString&);
    int AnalyzeDealWithAllFile(QString );
    void mult_type_file_icon(QVariant &itId,QVariant &itflag,QString &suffix,QVariantMap &);
    void SetFinishFind_Finish();
    void GetFolersize(QVariantMap&);
    void SaveSoftInfo();
    void ReadSoftInfo();

    QStringListMap *ItemUnstall;
public slots:
    void GetFolersize1();

private:
    void CalcFolersize1(QStringList &);
    void calcfoler(QString &foler,QString &size,QString &date,int flag);
    void remove_no_used_symbol(QString&);

    QThread *thread_calc;

    QStringVector softSizeDate;

    CGetFolderSize *CgetSoftSize;
    const int ROLNUM=5;
    static bool finishFind;
    QString *checkUnString;
    QString runRoot;
    QString fullpathfile;
    QDir dir;
    QFile file;
    int saveflag;
protected:

};




#endif // CHECKUNINSTALLSOFTNAME_H
