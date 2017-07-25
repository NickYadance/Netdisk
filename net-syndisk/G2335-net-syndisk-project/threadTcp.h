#ifndef TCPTHREAD_H
#define TCPTHREAD_H

#include <QObject>
#include <QThread>
#include "socket.h"
class TcpThread : public QThread
{
public:
    TcpThread();
    ~TcpThread();
    TcpThread(Socket * s);
    void stop();
protected:
    void run();
private:
    Socket * s;
};

#endif // TCPTHREAD_H
