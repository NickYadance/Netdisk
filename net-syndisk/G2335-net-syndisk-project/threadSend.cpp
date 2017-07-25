#include "threadSend.h"
#include "define.h"
#include <QDataStream>
#include <QDebug>

sendThread::sendThread()
{

}

void sendThread::run()
{
    QFile file(filepath) ;

    if (cmd == 1)
    {
        if (!file.open(QIODevice::ReadOnly))
        {
            s->closeSocket();
            return ;
        }
    }
    else
    {
        if (!file.open(QIODevice::WriteOnly))
        {
            s->closeSocket();
            return ;
        }
    }

    QDataStream stream(&file);
    char        buffer[BUFSIZE+1] = {'\0'};
    int len = 0;

    if (cmd == 1)
    {
        s->sendMessage(QString::number(filesize), 10);
        while(len < filesize)
        {
            stream.readRawData(buffer, BUFSIZE);
            len += s->sendBinaryMessage(buffer, BUFSIZE);
            if (len > filesize)
            {
                emit barValueChanged(bar, filesize);
                emit filesizeChanged(filesize);
            }
            else
            {
                emit barValueChanged(bar, len);
                emit filesizeChanged(len);
            }
        }
    }
    else
    {
        char buf[BUFSIZE + 1] = {'\0'};;

        while(len < filesize)
        {
            len += s->recvBinaryMessage(buf, BUFSIZE);
            if (len > filesize)
            {
                stream.writeRawData(buf, filesize % BUFSIZE) ;
                emit barValueChanged(bar, filesize);
                emit filesizeChanged(filesize);
            }
            else
            {
                stream.writeRawData(buf, BUFSIZE);
                emit barValueChanged(bar, len);
                emit filesizeChanged(len);
            }
        }
    }
    file.close();
}
