#include "tcpthread.h"

TcpThread::TcpThread()
{

}

TcpThread::~TcpThread()
{
    //·ÀÖ¹Ö¸Õë±»Ïú»Ù
}

TcpThread::TcpThread(Socket *sockfd)
{
    s = sockfd ;
}

void TcpThread::run()
{
    s->initSocket();
}
