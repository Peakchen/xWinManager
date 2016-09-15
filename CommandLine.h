#ifndef COMMANDLINE_H
#define COMMANDLINE_H

#include <QObject>
#include <QString>
#include <QStringList>
#include <QVariant>
#include <QVariantMap>

class CommandLine : public QObject
{
    Q_OBJECT
public:
    explicit CommandLine(QObject *parent = 0);
public:
    QVariantMap encodeToVariantMap();
    int parseCommandLine(QStringList commandLine);
public:
    bool launchMode();
    QString launchID();
    QString launchCategory();
    QString launchName();
    bool    launchAutoinstall();
    bool    getStatus();
protected:
    bool bMode;
    QString _id;
    QString _category;
    QString _launchName;
    bool    _autoInstall;
    bool    _status;

signals:

public slots:
};

#endif // COMMANDLINE_H
