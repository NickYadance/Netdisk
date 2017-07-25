#include "threadloop.h"
#include "define.h"
#include <QDebug>
#include <QDir>

extern QString Username;

threadLoop::threadLoop(Mainwindow *window)
{
    w = window ;
    watcher = new QFileSystemWatcher();
    thread_s = new Socket();

    isChangeListDone = true ;
    isDirChanged = false;
}

threadLoop::threadLoop()
{

}

threadLoop::~threadLoop()
{

}

void threadLoop::stopOnQuit()
{
    thread_s->closeSocket();
    delete thread_s;

    this->exit();
}

void threadLoop::changeListDone(int value)
{
    isChangeListDone = value ;
    isDirChanged = false ;
}

void threadLoop::ChangeDirectory(QString dir)
{
    isDirChanged = true ;

    QDir dirnew(dir);
    dirnew.setSorting(QDir::Time);
    emit browserValueChanged(dirnew.entryInfoList(QDir::Files).at(0).filePath());
}

void threadLoop::ChangeFile()
{

}

void threadLoop::run()
{
    while(1)
    {
        if ( w->isMacBinded && isChangeListDone )
        {
            Socket::Sleep(10000);
            qDebug() << "i am waiting for someone!" << endl;

            if (!isChangeListDone)   continue;

            if (thread_s->connectServer())
                emit labelValueChanged(Username);
            else
            {
                emit labelValueChanged("not login in");
                emit browserValueChanged("internet down");
                continue ;
            }

            QString  filelistSize;
            int      size;

            thread_s->sendMessage(PROTOCOL_GET_FILELIST_SIZE);
            thread_s->sendMessage(Username, 16);
            thread_s->recvMessage(filelistSize, FILESIZE);
            thread_s->closeSocket();

            qDebug() << "file listsize: " << filelistSize << endl;
            size = filelistSize.toInt() ;

            if ( (w->isUserBinded && size != w->Filelist.size()) || (isDirChanged == true) )
            {
                if (isDirChanged == true)
                {
                    qDebug() << "local changed!" << endl;
                    w->localModel->refresh();
                }
                isChangeListDone = false ;
                emit listChanged();
            }
        }
    }
}
