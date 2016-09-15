#include "character.h"
#include <QDateTime>
#include <QDebug>
#include <QDir>

std::string GBKToUTF8(const std::string& strGBK)
{
    if(strGBK.size() == 0)
    {
        return std::string("");
    }

    std::string strOutUTF8 = "";
    wchar_t * str1;
    int n = MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, NULL, 0);
    str1 = new wchar_t[n];
    MultiByteToWideChar(CP_ACP, 0, strGBK.c_str(), -1, str1, n);
    n = WideCharToMultiByte(CP_UTF8, 0, str1, -1, NULL, 0, NULL, NULL);
    char * str2 = new char[n];
    WideCharToMultiByte(CP_UTF8, 0, str1, -1, str2, n, NULL, NULL);
    strOutUTF8 = str2;
    delete[]str1;
    str1 = NULL;
    delete[]str2;
    str2 = NULL;
    return strOutUTF8;
}

std::string UTF8ToGBK(const std::string& strUTF8)
{
    if(strUTF8.size() == 0)
    {
        return std::string("");
    }

    int len = MultiByteToWideChar(CP_UTF8, 0, strUTF8.c_str(), -1, NULL, 0);
    unsigned short * wszGBK = new unsigned short[len + 1];
    memset(wszGBK, 0, len * 2 + 2);
    MultiByteToWideChar(CP_UTF8, 0, (LPCCH)strUTF8.c_str(), -1, (LPWSTR)wszGBK, len);

    len = WideCharToMultiByte(CP_ACP, 0, (LPCWCH)wszGBK, -1, NULL, 0, NULL, NULL);
    char *szGBK = new char[len + 1];
    memset(szGBK, 0, len + 1);
    WideCharToMultiByte(CP_ACP,0, (LPCWCH)wszGBK, -1, szGBK, len, NULL, NULL);
    std::string strTemp(szGBK);
    delete[]szGBK;
    delete[]wszGBK;
    return strTemp;
}

bool IsDigitString(const QString &src)
{
    QByteArray ba = src.toLatin1();
    const char *s = ba.data();

    while(*s && *s >= '0' && *s <= '9')
    {
        s++;
    }

    if (*s)
    {
        return false;
    }
    else
    {
        return true;
    }
}

QString DelEndDigit(const QString &src)
{
    const std::string origin = src.toStdString();
    const char* start = origin.data();
    const char* end = start + origin.length() - 1;
    const char* p = end;

    while(*p)
    {
        if(*p >= '0' && *p < '9')
        {
            p--;
        }
        else
        {
            break;
        }
    }
    if(p == end)
    {
        return src;
    }

    std::string temp;
    while(start <= p)
    {
        temp.push_back(*start++);
    }

    return QString::fromStdString(temp);
}

QString DelEndSpace(const QString &src)
{
    if(src.isEmpty())
    {
        return QString("");
    }

    const std::string origin = src.toStdString();
    const char* start = origin.data();
    const char* end = start + origin.length() - 1;
    const char* p = end;

    while(*p)
    {
        if(*p == ' ' || *p == '\t')
        {
            p--;
        }
        else
        {
            break;
        }
    }
    if(p == end)
    {
        return src;
    }

    std::string temp;
    while(start <= p)
    {
        temp.push_back(*start++);
    }

    return QString::fromStdString(temp);
}

int CompareVersion(QString version1, QString version2)
{
    if(version1.isEmpty() && !version2.isEmpty())
    {
        return -1;
    }
    else if(version1.isEmpty() && version2.isEmpty())
    {
        return 0;
    }
    else if(!version1.isEmpty() && version2.isEmpty())
    {
        return 1;
    }


    QStringList versionList1 = version1.split(QString("."), QString::SkipEmptyParts, Qt::CaseInsensitive);
    QStringList versionList2 = version2.split(QString("."), QString::SkipEmptyParts, Qt::CaseInsensitive);

    int size1 = versionList1.size();
    int size2 = versionList2.size();

    int min = size1 > size2 ? size2 : size1;

    int number1 = 0;
    int number2 = 0;
    for(int i = 0; i < min; i++)
    {
        number1 = versionList1.at(i).toInt();
        number2 = versionList2.at(i).toInt();
        if(number1 > number2)
        {
            return 1;
        }
        else if(number1 < number2)
        {
            return -1;
        }
    }
    if(size1 > size2)
    {
        for(int i = min; i < size1; i++)
        {
            number1 = versionList1.at(i).toInt();
            if(number1 != 0)
            {
                return 1;
            }
        }
        return 0;
    }
    else if(size1 < size2)
    {
        for(int i = min; i < size2; i++)
        {
            number2 = versionList2.at(i).toInt();
            if(number2 != 0)
            {
                return -1;
            }
        }
        return 0;
    }
    return 0;
}

// 从给定的字符串中分离出版本和软件名称
bool SeparateNameAndVersion(const QString &src, QString &name, QString &version)
{
    if(src.isEmpty())
    {
        return false;
    }

    QStringList stringList = src.split(QString(" "), QString::SkipEmptyParts, Qt::CaseInsensitive);
    for(int i = 0; i < stringList.size(); i++)
    {
        QString stringChip = stringList.at(i);

        if(IsDigitString(stringChip))       // 过滤纯数字字符串
            continue;

        if(stringChip.contains(".0") ||         // 取出版本
                stringChip.contains(".1") ||
                stringChip.contains(".2") ||
                stringChip.contains(".3") ||
                stringChip.contains(".4") ||
                stringChip.contains(".5") ||
                stringChip.contains(".6") ||
                stringChip.contains(".7") ||
                stringChip.contains(".8") ||
                stringChip.contains(".9"))
        {
            if(stringChip.at(0) >= '0' && stringChip.at(0) <= '9')
            {
                version = stringChip;
            }
            continue;
        }
        if(stringChip.contains("/") || stringChip.contains("\\"))
            continue;
        if((stringChip.contains("(") || stringChip.contains(")")) && (stringChip.contains("32") || stringChip.contains("64") || stringChip.contains("bit", Qt::CaseInsensitive)))
            continue;
        if(stringChip.contains("x86", Qt::CaseInsensitive) || stringChip.contains("x64", Qt::CaseInsensitive))
            continue;
        if(stringChip.length() == 1)
            continue;
        if(!stringChip.compare("build", Qt::CaseInsensitive) || !stringChip.compare("beta"))
            break;

        name += DelEndDigit(stringChip);
        if(i != stringList.size() - 1)
            name += " ";
    }

    name = DelEndSpace(name);

    return true;
}

QString UrlEncode(QString string)
{
    QByteArray ba;
    ba.append(string);
    return ba.toBase64().toPercentEncoding();
}

void PrintTime(const char *function, const int line)
{
    QTime curTime = QTime::currentTime();
    char msg[100] = {0x00};
    sprintf(msg, "[%s--%d] %d:%d:%d.%d", function, line, curTime.hour(), curTime.minute(), curTime.second(), curTime.msec());
    qDebug() << msg;
}

bool USleep(unsigned long usec)
{
    struct timeval tv;
    fd_set dummy;
    SOCKET s = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
    FD_ZERO(&dummy);
    FD_SET(s, &dummy);
    tv.tv_sec = usec / 1000000ul;
    tv.tv_usec = usec % 1000000ul;
    bool success = (0 == select(0, 0, 0, &dummy, &tv));
    closesocket(s);
    return success;
}

bool IsFileExist(const QString &szPath)
{
    QFile file(szPath);
    if(file.exists())
    {
        return true;
    }

    return false;
}
