#ifndef SENDTHREAD_H
#define SENDTHREAD_H

#include <QObject>
#include <QThread>
#include <QFile>
#include <QProgressBar>
#include "socket.h"

class sendThread : public QThread
{
    Q_OBJECT
public:
    sendThread();
    sendThread(Socket* cs, QString cfilepath, QProgressBar *cbar, int cfilesize, int ccmd):
        s(cs), filepath(cfilepath), bar(cbar), cmd(ccmd), filesize(cfilesize){}
    ~sendThread(){}
protected:
    void    run();

signals:
    void    barValueChanged(QProgressBar *, int );
    void    filesizeChanged(int);

private:
    Socket          *s;
    QString         filepath;
    QProgressBar    *bar;
    int             cmd;
    int             filesize;
};

#endif // SENDTHREAD_H
