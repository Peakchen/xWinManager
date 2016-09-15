#ifndef CHARACTER_H
#define CHARACTER_H

#include <iostream>
#include <QString>
#include <QStringList>
#include "Windows.h"

extern std::string GBKToUTF8(const std::string& strGBK);
extern std::string UTF8ToGBK(const std::string& strUTF8);
extern bool IsDigitString(const QString &src);
extern QString DelEndDigit(const QString &src);
extern QString DelEndSpace(const QString &src);
extern int CompareVersion(QString version1, QString version2);
extern bool SeparateNameAndVersion(const QString &src, QString &name, QString &version);
extern QString UrlEncode(QString string);
extern void PrintTime(const char *function, const int line);
extern bool USleep(unsigned long usec);
extern bool IsFileExist(const QString &szPath);

#endif // CHARACTER_H
