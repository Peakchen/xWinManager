#ifndef THREADTOPULLSOFTWARELIST_H
#define THREADTOPULLSOFTWARELIST_H
#include <QObject>
#include <Qthread>

class ThreadToPullSoftwareList : public QThread
{
    Q_OBJECT
public:
    explicit ThreadToPullSoftwareList(QObject *parent = 0);
public:
    bool m_bExit = false;

signals:
protected:
    virtual void run();
    bool chkSoftwareFile(const char *szTempFile, const char * szFile);
    bool chkSoftwareListFile(const char *szTempFile, const char * szFile);
protected:
    std::string strDirTempData;
    std::string strDirData;
    std::string strURL;
public slots:
};

#endif // THREADTOPULLSOFTWARELIST_H
