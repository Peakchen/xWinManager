#ifndef DATASTRUCT_H
#define DATASTRUCT_H

#include <QString>
#include <QVariant>
#include <QVariantList>
#include <QVariantMap>
#include "DownWrapper.h"

enum{           // 软件所有数据的状态说明
    PACKAGE_STATUS_ORIGIN = 0,              // list文件中的原始包: 用于大全模块
    PACKAGE_STATUS_CAN_UPGRADE,             // 可升级的包: 用于升级模块和大全
    PACKAGE_STATUS_NEW_DOWNLOAD,            // 新的下载项
    PACKAGE_STATUS_DOWNLOAD_CANCEL,         // 下载取消
    PACKAGE_STATUS_DOWNLOAD_START,          // 开始或正在下载
    PACKAGE_STATUS_DOWNLOAD_START_ERR,      // 开始或正在下载异常
    PACKAGE_STATUS_DOWNLOAD_PAUSE,          // 暂停下载
    PACKAGE_STATUS_DOWNLOAD_FINISH,         // 下载完成, 未安装
    PACKAGE_STATUS_INSTALL_START,           // 启动安装或正在安装
    PACKAGE_STATUS_INSTALL_START_ERR,       // 启动安装失败
    PACKAGE_STATUS_INSTALL_CANCEL,          // 取消安装
    PACKAGE_STATUS_INSTALL_FINISH,          // 安装成功: 说明注册表可以找到该软件, 否则当作取消安装
    PACKAGE_STATUS_REMOVE,                  // 移除
};

/*************************** 下载数据结构 ***************************/
typedef struct tagDownloadItemData {
    QString id;                 // 软件ID
    QString category;           // 软件分类ID
    QString name;               // 软件名
    QString largeIcon;          // 软件大图标
    QString brief;              // 软件简介
    qint64 size;                // 软件大小
    QString downloadUrl;        // 软件下载URL
    int rating;                 // 评分
    bool isAd;                  // 软件是否是存在广告
    float priceInfo;            // 软件价格
    QString updateTime;         // 更新时间
    QString versionName;        // 软件版本
    float percent;              // 软件下载百分比
    qint64 speed;               // 软件下载速度
    QString packageName;        // 软件安装包名
    QString downloadPath;       // 软件安装包所在目录或下载目录
    int status;                 // 当前下载状态
    HANDLE hTaskHandle;         // 下载句柄
    DownTaskParam downTaskParam;// 下载参数
    unsigned int unfinishedCount;   // 未下载完成的数量
    unsigned int finishedCount;     // 下载完成的数量
}DownloadItemData, *LPDownloadItemData;

typedef QMap<QString, LPDownloadItemData> QDownloadItemDataMap;
typedef QDownloadItemDataMap::iterator DownloadIterator;

/*************************** 升级数据结构 ***************************/
typedef struct tagUpgradeItemData {
    QString id;                 // 软件ID
    QString category;           // 软件分类ID
    QString softName;           // 精简的displayName
    QString displayName;        // 当前已安装的软件名称
    QString displayVersion;     // 当前已安装的版本名称
    QString name;               // 更新软件名
    QString versionName;        // 更新版本
    QString updateTime;         // 更新时间
    QString largeIcon;          // 软件大图标
    QString brief;              // 软件简介
    qint64 size;                // 软件大小
    int arch;                   // 软件位数
    int status;                 // 升级状态
}UpgradeItemData, *LPUpgradeItemData;

typedef QMap<QString, LPUpgradeItemData> QUpgradeItemDataMap;
typedef QUpgradeItemDataMap::iterator UpgradeIterator;

/*************************** 软件大全数据结构 ***************************/
typedef QMap<QString, QVariantList> QPackageItemData;
typedef QPackageItemData::iterator PackageIterator;

/*************************** 系统备份-还原 数据结构****************************/
typedef struct tagBackupData {
    QString id;                 // 软件ID
    QString filename;           // 软件分类ID
}BackupDataItem, *LPBackupData;

typedef QMap<QString, LPBackupData> QBackupDataMap;
typedef QBackupDataMap::iterator QBackupIterator;

#endif // DATASTRUCT_H

