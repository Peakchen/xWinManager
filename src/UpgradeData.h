#ifndef UPGRADEDATA_H
#define UPGRADEDATA_H

#include <QObject>
#include <QDebug>
#include <QTimer>

#include "DataStruct.h"

class DataControl;

class UpgradeData : public QObject
{
    Q_OBJECT
public:
    explicit UpgradeData(QObject *parent = 0);
    ~UpgradeData();
    void Init(DataControl *pDataControl);
    void StartTimer();
    void StopTimer();

protected slots:
    void PollUpgradeData();
    void AddUpgradeItemData(const QVariant &package, const QString &softName, const QString &displayName, const QString &displayVersion);
    void DelUpgradeItemData(LPUpgradeItemData &pItemData);

protected:
    void SendUpgradeCountSignal();

signals:
    void sigReloadUpgradeData();
    void sigUpdateUpgradeCount(int);

protected:
    bool m_bPolling;
    bool m_bQuit;
    QTimer m_timer;
    DataControl *m_pDataControl;
    QUpgradeItemDataMap m_mapUpgradeData;
};

#endif // UPGRADEDATA_H
