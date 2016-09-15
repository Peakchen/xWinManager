#include "logwriter.h"
#include <QDir>
#include <QTextStream>
#include <QDateTime>
#pragma execution_character_set("utf-8")
LogWriter *LogWriter::getLogCenter()
{
    static LogWriter * _logCenter = new LogWriter;
    return _logCenter;
}

LogWriter::LogWriter()
{
    _logLevel = LOG_DEBUG;
    _logPath = QDir::currentPath();
}

LogWriter::~LogWriter()
{
//    if (_logCenter) {
//        delete _logCenter;
//        _logCenter = NULL;
//    }
}

void LogWriter::PrintLog(LOGLEVEL level, const char *format, ...)
{
    if (level < _logLevel)  return;     //low level
    char logBuffer[8192] = {0};
    va_list vl_fmt;                     //buffer
    va_start(vl_fmt, format);
    vsprintf(logBuffer, format, vl_fmt);
    va_end(vl_fmt);

    QString fileTime = "";
    QString logTime = "";
    QString logLevel = getLevelStr(level);
    fileTime = QDateTime::currentDateTime().toString("yyyyddMM");
    logTime = QDateTime::currentDateTime().toString("yyyy-dd-MM hh:mm:ss.zzz");
    qDebug("[%s] [%s] {%s}", logTime.toStdString().c_str(), logLevel.toStdString().c_str(), logBuffer);
}

void LogWriter::SaveFileLog(LOGLEVEL level, const char *format,  ...)
{
    if (level < _logLevel)  return;  //low level
    char logBuffer[8192] = {0};
    va_list vl_fmt;                  //buff
	va_start(vl_fmt, format);
	vsprintf(logBuffer, format, vl_fmt);
    va_end(vl_fmt);

    QString logTime = "";
    QString fileTime = "";
    fileTime = QDateTime::currentDateTime().toString("yyyyddMM");
    logTime = QDateTime::currentDateTime().toString("[yyyy-dd-MM hh:mm:ss.zzz]");
    QString logLevel = getLevelStr(level);
    QString logFile = _logPath;
    if (logFile.right(1) != "/") {
        logFile += "/";
    }
    QDir mDir(logFile);
    if (!mDir.exists()) {
        mDir.mkpath(logFile);
    }
    logFile += "isoft_";
    logFile += fileTime;
    logFile += ".log";

    QFile file(logFile);
    file.open(QIODevice::ReadWrite | QIODevice::Append | QIODevice::Text);
    QTextStream out(&file);
    out << logTime << " [" << logLevel << "] " << logBuffer << endl;
    file.close();
}

QString LogWriter::getLevelStr(LOGLEVEL level)
{
    switch(level) {
        case LOG_DEBUG: return "LOG_DEBUG"; break;
        case LOG_INFO: return "LOG_INFO"; break;
        case LOG_WARN: return "LOG_WARN"; break;
        case LOG_ERROR: return "LOG_ERROR"; break;
    }
}

void LogWriter::setLogPath(QString logPath)
{
    _logPath = logPath;
}

void LogWriter::setLogLevel(LOGLEVEL logLevel)
{
    _logLevel = logLevel;
}
