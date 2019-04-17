#ifndef LAUNCHTHREAD_H
#define LAUNCHTHREAD_H

#include <QThread>
#include <QProcess>
#include "DataStruct.h"

class LaunchThread : public QThread
{
    Q_OBJECT
public:
    explicit LaunchThread(QObject *parent = 0);
    virtual void run();
    bool Init(LPDownloadItemData pItemData);

signals:
    void sigDeleteLaunchThread(LaunchThread *);

protected:
    LPDownloadItemData m_pItemData;
};

#endif // LAUNCHTHREAD_H
