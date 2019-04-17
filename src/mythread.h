#ifndef MYTHREAD_H
#define MYTHREAD_H
#include <QThread>
class MyThread : public QThread
{
    Q_OBJECT
public:
    MyThread();
    ~MyThread();
protected:
    void run();
signals:
    void SendUpdateMsgToGUI(QString str);
public:
    bool isExit;
};
#endif // MYTHREAD_H

