#ifndef SOCKET_H
#define SOCKET_H

#include <winsock2.h>
#include <QString>
#include <fcntl.h>
class Socket
{
public:
    Socket();

    ~Socket();
    friend class sendThread;

    bool    initSocket();
    int     setNonblock();
    int     connectServer();
    int     sendMessage(QString message);
    int     sendMessage(QString message, int len);
    int     recvMessage(QString &message, int messageLen);
    void    closeSocket();
    static void Sleep(const int SleepTime);
private:
    SOCKET          s;
    SOCKADDR_IN     serverAddr;
};

#endif // SOCKET_H
