#include "threadloop.h"
#include "define.h"
#include <QDebug>
#include <QDir>
#include <QDateTime>

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
    if ( !isChangeListDone )    return ;

    w->localModel->refresh();
    QDir        info(dir);
    QFileInfo   s ;

    foreach (s , info.entryInfoList(QDir::Dirs| QDir::Files| QDir::NoDotAndDotDot))
    {
        if (s.isDir() && !w->watcher->directories().contains(s.filePath()))
        {
            emit browserValueChanged("New dir: " + s.filePath());
            w->watcher->addPath(s.filePath());
            isDirChanged = true ;
            break ;
        }
        else
            if (s.isFile() && !w->watcher->files().contains(s.filePath()))
            {
                int size = s.size() ;
                emit browserValueChanged("New file: " + s.filePath() + ",Size: " + QString::number(size));
                w->watcher->addPath(s.filePath());
                if (size)
                {
                    isDirChanged = true;
                }
                break ;
            }
    }
}

void threadLoop::run()
{
    QString  filelistSize;
    int      size;

    while(1)
    {
        Socket::Sleep(5000);
        qDebug() << "waiting..." << endl;

        if (w->isMacBinded && isChangeListDone)     //isMacBinded可能在睡眠中被改变
        {
            if (thread_s->connectServer())
                emit labelValueChanged(Username);
            else
            {
                emit labelValueChanged("not login in");
                emit browserValueChanged("internet down");
                continue ;
            }
            thread_s->sendMessage(PROTOCOL_GET_FILELIST_SIZE);
            thread_s->sendMessage(Username, 16);
            thread_s->recvMessage(filelistSize, FILESIZE);
            thread_s->closeSocket();

            size = filelistSize.toInt() ;
            if ( size != w->Filelist.size() || isDirChanged )
            {
                emit listChanged();
            }
        }
    }

}
