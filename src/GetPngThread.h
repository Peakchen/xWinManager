#ifndef GETPNGTHREAD_H
#define GETPNGTHREAD_H

#include <QThread>
#include "DataStruct.h"

class GetPngThread : public QThread
{
    Q_OBJECT
public:
    explicit GetPngThread(QObject *parent = 0);
    bool Init(QVariantList &lstPackage);
    virtual void run();

signals:
    void sigDeleteGetPngThread(GetPngThread *);

protected:
    QVariantList m_lstPackage;
};

#endif // GETPNGTHREAD_H
