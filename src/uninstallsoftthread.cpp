#include <QObject>
#include <QDebug>
#include "uninstallsoftthread.h"
#include "Uninstallsoftware.h"
#include "logwriter.h"
#pragma execution_character_set("utf-8")

//卸载线程，在其中调用卸载程序，并等待卸载程序结束，向卸载类发送更新信号。
UninstallSoftThread::UninstallSoftThread()
{

}
UninstallSoftThread::~UninstallSoftThread()
{

}
void UninstallSoftThread::Init(QString name, QStringList param, QString strInstallLocation, QString strDisplayName)
{
    //QObject::connect(this,SIGNAL(update(UninstallSoftThread*)),uninstall,SLOT(UpdateSoftwareListForThread(UninstallSoftThread*)),Qt::QueuedConnection);
    m_name=name;
    m_param=param;
    m_strInstallLocation = strInstallLocation;
    m_strDisPlayName = strDisplayName;
}

void UninstallSoftThread::run()
{
    QProcess *Uninst= new QProcess();
    Uninst->start(m_name,m_param);
    Uninst->waitForFinished();
    delete Uninst;
    //LogWriter::getLogCenter()->SaveFileLog(LOG_INFO, "CMDname:%s,softName:%s,file:%s,line:%d\n",m_name.toStdString().data(),m_strDisPlayName.toStdString().data(),__FILE__, __LINE__);
    //QString asd = "1234566612222222222";
    //qDebug()<<asd<<endl;//qDebug()在输出的时候，如果不是直接输出的字符串，而是通过字符串变量进行输出，则输出结果会加上双引号。"1234566612222222222"
    emit update(this,m_strInstallLocation,m_strDisPlayName);
}

