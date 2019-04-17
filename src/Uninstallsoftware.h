#ifndef UNINSTALLSOFTWARE_H
#define UNINSTALLSOFTWARE_H

#include <QObject>
#include <QDebug>
#include <QTimer>
#include "DataStruct.h"
#include "OSSystemWrapper.h"
#include "checkuninstallsoftname.h"
void GetLinkPath(QString strLink, QString &strPath, QString &strParam);
BOOL  DirectoryList(LPCSTR Path,QStringMap &mapQStr);
std::string GetInfoFromExeAndDll(std::string strFileFullPath ,std::string &strName);
std::string GetTimeNow();
long GetTimeDiffer(std::string strTime1, std::string strTime2);
void GetPathAndCMD(QString &name,QStringList &arguments);

//c语言实现删除制定目录以及目录下文件
BOOL removeDir(const char*  dirPath);

BOOL DelFolder(const char*  dirPath);
class DataControl;
class UninstallSoftThread;
class UninstallSoftware : public QObject
{
    Q_OBJECT
public:
    explicit UninstallSoftware(QObject *parent = 0);
    ~UninstallSoftware();
    void Init(DataControl *pDataControl);
    void StartTimer();
    void StopTimer();
    void GetSoftwareList(mapSoftwareList &mapSoftwareList);
    void GetInstalledSoftwareMap();
    void SendUninSoftwareList();
    void UninstallSoftwares(QString &uninstallID);  //用线程调用卸载进程，卸载软件
    void get_str_for_used_date(QString &value,QString &str);
    qint32 StringLikeFind(QString str,QString &in_of_str);
    QString GetOSProgramsPath(int flag);//获取开始菜单-程序的文件夹路径，根据参数0，1可分别获取当前用户，和系统的
protected:
    void EncodeToVariantList(mapSoftwareList mapUninstallData, QVariantList& objectList);       //将信息的存储格式进行转化
    void getUnstallData(QStringMap &map_use);               //获取软件的使用时间和频率
    void ProcessUninSoftMap();                              //优化要传递给页面的已装软件信息。
    void GetStartMenuSoftInfo(QStringMap &mapQStr);         //获取开始菜单中的软件信息
protected slots:
    void UpdateSoftwareList();
    void UpdateSoftwareListForThread(UninstallSoftThread*, QString,QString);

signals:
    void sigUpdateUninSoftware(QVariantList);
    void sigCleanupResidue(QString,QString);
    void sigUninstallFinished(QString);

protected:
    QTimer m_timer;
    mapSoftwareList m_mapUninSoftwareList;
    QVariantMapMap m_UninsoftList;                  //中间数据
    QVariantList jsArray;                           //要传给页面的map
    QVector<QString>  m_VectorSoftInfo;             //辅助用
    QMap<QString,QString> m_uninstallingSoftName;        //正在卸载中的程序名称
    HANDLE m_Mutex;                                 //用来互斥访问成员变量m_UninsoftList。
    HANDLE mutex_soft_file;                         //用来互斥访问服务提供的文件数据。
    QString confFilePath;                           //配置文件路径。

    BOOL m_bPolling;                                //控制定时器，一次完成才进行下一次。

};

#endif // UNINSTALLSOFTWARE_H
