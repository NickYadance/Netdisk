#include "tcpthread.h"

TcpThread::TcpThread()
{

}

TcpThread::~TcpThread()
{
    //��ָֹ�뱻����
}

TcpThread::TcpThread(Socket *sockfd)
{
    s = sockfd ;
}

void TcpThread::run()
{
    s->initSocket();
}
