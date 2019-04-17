#ifndef LOGWRITER_H
#define LOGWRITER_H

#include <QString>

enum LOGLEVEL{
    LOG_DEBUG = 0,      /**< Debug >**/
    LOG_INFO,           /**< Info >**/
    LOG_WARN,           /**< Warn >**/
    LOG_ERROR           /**< Error >**/
};

class LogWriter{
public:
    static LogWriter* getLogCenter();
    void PrintLog(LOGLEVEL level, const char* format, ...);
    void SaveFileLog(LOGLEVEL level, const char* format, ...);
    void setLogPath(QString logPath);         //defalut: current path
    void setLogLevel(LOGLEVEL logLevel);      //defalut: LOG_DEBUG
private:
    explicit LogWriter();
    ~LogWriter();
private:
    QString _logPath;
    LOGLEVEL _logLevel;
private:
    QString getLevelStr(LOGLEVEL level);
};

#endif // LOGWRITER_H
