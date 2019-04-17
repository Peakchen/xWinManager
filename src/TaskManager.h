#ifndef TASKMANAGER_H
#define TASKMANAGER_H

#include <QThread>
class DataControl;
class PackageRunner;

class TaskManager : public QThread
{
    Q_OBJECT
public:
    TaskManager(QObject * parent = 0);

protected:
    PackageRunner *pTaskRunner;
    DataControl *pDataControl;
public:
    void SetObjects(DataControl *,PackageRunner *);
protected:
    virtual void run();
};

#endif // TASKMANAGER_H
