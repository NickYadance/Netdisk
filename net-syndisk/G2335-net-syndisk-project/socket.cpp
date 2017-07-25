#include "socket.h"
#include "define.h"
#include "threadTcp.h"
#include <QByteArray>
#include <QDebug>
#include <QElapsedTimer>
#include <QCoreApplication>

Socket::Socket()
{

}

Socket::~Socket()
{

}

bool Socket::initSocket()
{
    WSADATA         wsaData;

    WSAStartup(MAKEWORD(2, 2), &wsaData);

    if ( (s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP)) == (unsigned)SOCKET_ERROR )
    {
        qDebug() << "socket error!";
        return false;
    }

    if ( (signed)s == -1)    return 0;

    serverAddr.sin_family       = AF_INET;
    serverAddr.sin_port         = htons(SERVER_PORT);
    serverAddr.sin_addr.s_addr  = inet_addr(SERVER_IP);

    while (1)
    {
        if (::connect(s, (SOCKADDR*)&serverAddr, sizeof(serverAddr))==-1)
            continue;
        break ;
    }
    return 1;
}

int Socket::setNonblock()
{
    ULONG NonBlock = 1;
    if(ioctlsocket(s, FIONBIO, &NonBlock) == SOCKET_ERROR)
    {
        printf("ioctlsocket() failed with error %d\n", WSAGetLastError());
        return 0;
    }
    return 1;
}

int Socket::connectServer()
{
    TcpThread * tcpThread = new TcpThread(this);
    tcpThread->start();

    for (int i=0; i<10; i++)        //尝试10次查询连接，间隔1秒钟
    {
        Socket::Sleep(1000);
        if (!tcpThread->isRunning())
            return 1;
    }
    tcpThread->terminate();

    delete tcpThread ;
    this->closeSocket();
    return 0;
}

int Socket::sendMessage(QString message)
{
    QByteArray msgByteArray = message.toLatin1();
    char * msg = msgByteArray.data();
    return send(s, msg, message.length(), 0);
}

int Socket::sendMessage(QString message, int len)
{
    QByteArray msgByteArray = message.toLatin1();
    char * msg = msgByteArray.data();
    return send(s, msg, len, 0);
}

int Socket::sendBinaryMessage(char *buffer, int len)
{
    return send(s, buffer, len, 0);
}

int Socket::recvMessage(QString &content, int contentlen)
{
    content.clear();

    char buf[contentlen + 1] = {'\0'};
    int len = 0;

    memset(buf, 0, contentlen);
    while (len < contentlen)
    {
        len += recv(s, buf + len, contentlen - len, 0);
    }
    content = QString(buf);
    return contentlen;
}

int Socket::recvBinaryMessage(char *buffer, int len)
{
    int recvlen = 0;

    while (recvlen < len)
    {
        recvlen += recv(s, buffer + recvlen, len - recvlen, 0);
    }
    return len ;
}



void Socket::closeSocket()
{
    closesocket(s);
    WSACleanup();
}

void Socket::Sleep(const int SleepTime)
{
    QElapsedTimer t;
    t.start();
    while(t.elapsed()<SleepTime)
        QCoreApplication::processEvents();
}
