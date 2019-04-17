#ifndef STORAGE_H
#define STORAGE_H

#include <QObject>
#include "DataStruct.h"

class Storage : public QObject
{
    Q_OBJECT
public:
    explicit Storage(QObject *parent = 0);
public:
    // for programe setting
    static void getSettingFromFile(QString, QVariantMap&);
    static void setSettingToFile(QString, QVariantMap&);

    static bool LoadSoftwareCategory(QString, QVariantList &);

    // for package list
    static bool LoadCategorySoftwareList(QString, QString, QMap<QString,QVariantList> &);
    static bool LoadArrayOfSoftwareList(QString, QVariantList &);

    static void LoadFromConfArray(QString , QVariantList &);
    static void SaveToConfArray(QString , QVariantList &);

    // for download
    static void LoadDownloadData(const bool bFirst, const QString &, QDownloadItemDataMap &);
    static void SaveDownloadData(const QString &, QDownloadItemDataMap &);

    // for upgrade
    static void LoadUpgradeData(const bool bFirst, const QString &, QUpgradeItemDataMap &);
    static void SaveUpgradeData(const QString &, QUpgradeItemDataMap &);
};

#endif // STORAGE_H
