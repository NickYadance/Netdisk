#include "threadSend.h"
#include "define.h"
#include <QTextStream>
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
        if (!file.open(QIODevice::WriteOnly | QIODevice::Text))
        {
            s->closeSocket();
            return ;
        }
    }

    QTextStream stream(&file);
    int len = 0;

    if (cmd == 1)
    {
        s->sendMessage(QString::number(filesize), 10);
        while(len < filesize)
        {
            len += s->sendMessage(stream.read(BUFSIZE), BUFSIZE);
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
        QString buf;
        while(len < filesize)
        {
            len += s->recvMessage(buf, BUFSIZE);
            stream << QString(buf) ;

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
    file.close();
}
