#ifndef USERINFOMANAGER_H
#define USERINFOMANAGER_H

#include <QThread>

class DataControl;
class UserInfo;

class UserInfoManager : public QThread
{
    Q_OBJECT
public:
    explicit UserInfoManager(QObject *parent = 0);
protected:
    UserInfo *pUserInfo;
    DataControl *pDataControl;
public:
    void SetObjects(DataControl *,UserInfo *);
protected:
    virtual void run();
signals:

public slots:
};

#endif // USERINFOMANAGER_H
