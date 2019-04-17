#ifndef UNINSTALLSOFTTHREAD_H
#define UNINSTALLSOFTTHREAD_H
#include <QThread>
class UninstallSoftware;

class UninstallSoftThread : public QThread
{
    Q_OBJECT
public:
    UninstallSoftThread();
    ~UninstallSoftThread();
    void Init(QString name, QStringList param, QString strInstallLocation,QString strDisplayName);
protected:
    void run();
signals:
    void update(UninstallSoftThread*,QString,QString);
protected:
    QString m_name;
    QStringList m_param;
    QString m_strInstallLocation;   //安装路径
    QString m_strDisPlayName;
};

#endif // UNINSTALLSOFTTHREAD_H
