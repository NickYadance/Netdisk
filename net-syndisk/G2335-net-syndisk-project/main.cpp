#include "mainwindow.h"
#include "logindialog.h"
#include "threadloop.h"
#include <QApplication>
#include <QDebug>
#include <QFile>
#include <QObject>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);
    Mainwindow w;
    LoginDialog dialog;

    if (dialog.exec() == QDialog::Accepted)
    {
        w.show();
        w.GetBindCatalog();
        w.GetFileList();
        w.GetServerMd5s();
        w.ShowFileListTree();

        threadLoop * loopthread = new threadLoop(&w);
        loopthread->start();

        QObject::connect(loopthread, SIGNAL(labelValueChanged(QString)), &w, SLOT(ChangeLabelValue(QString)));
        QObject::connect(loopthread, SIGNAL(browserValueChanged(QString)), &w, SLOT(ChangeBrowserValue(QString)));
        QObject::connect(loopthread, SIGNAL(listChanged()), &w, SLOT(ChangeFilelist()));
        QObject::connect(&w, SIGNAL(ChangeListDone(int )), loopthread, SLOT(changeListDone(int )));
        QObject::connect(w.watcher, SIGNAL(directoryChanged(QString)), loopthread, SLOT(ChangeDirectory(QString )));
        return a.exec();
    }
    else
        return 0;
}

