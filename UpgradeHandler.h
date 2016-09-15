#ifndef UPGRADEHANDLER_H
#define UPGRADEHANDLER_H

#include <QObject>
#include <QTimer>
#include <QDebug>
#include <QMutex>

#include "DataStruct.h"

class DataControl;

class UpgradeHandler : public QObject
{
    Q_OBJECT
public:
    explicit UpgradeHandler(QObject *parent = 0);
    ~UpgradeHandler();
    void Init(DataControl *pDataControl);

protected:
    void EncodeToVariantMap(LPUpgradeItemData itemData, QVariantMap& object);
    void EncodeToUpgradeData(QVariantMap object, LPUpgradeItemData &itemData);

    void SendSignal(const LPUpgradeItemData &pItemData);
    bool CompareDisplayVersion(QString szOldVersion, QString szNewVersion);

protected slots:
    void ReloadUpgradeData();
    void RequestUpgradeData();
    void SetUpgradeStatus(QVariantMap object);

signals:
    void sigUpdateOneUpgradeInfo(QVariantMap);

protected:
    QUpgradeItemDataMap m_mapUpgradeData;
};

#endif // UPGRADEHANDLER_H
