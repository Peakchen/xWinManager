#ifndef MAINWND_H
#define MAINWND_H

#include <QWebView>
#include <QMouseEvent>
#include <QByteArray>

class MainWnd : public QWebView
{
    Q_OBJECT
public:
    explicit MainWnd(QWidget *parent = 0);
protected:
    virtual bool nativeEvent(const QByteArray & eventType, void * message, long * result);
protected:

signals:

public slots:
};

#endif // MAINWND_H
