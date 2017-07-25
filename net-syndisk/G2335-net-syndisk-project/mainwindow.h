#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QWidget>
#include <QList>
#include <QDirModel>
#include <QStandardItemModel>
#include <QStandardItem>
#include <QStringList>
#include <QProgressBar>
#include <QFileSystemWatcher>
#include "socket.h"

namespace Ui {
class Mainwindow;
}

//网盘文件目录
class File{
public:
    File(QString Path, QString Isfile, QString Utime, QString Md5):path(Path), isFile(Isfile), updateTime(Utime), md5(Md5){}
    QString path;
    QString isFile;
    QString updateTime;
    QString md5;

    friend bool operator == (const File& file1, const File& file2);
};

typedef QList<File> FileList;

class Mainwindow : public QWidget
{
    Q_OBJECT

public:
    explicit Mainwindow(QWidget *parent = 0);
    ~Mainwindow();

    QFileSystemWatcher      *watcher;

    friend  class   threadLoop;

    void    GetServerMd5s();                                      //获得服务器真实文件md5列表
    void    GetFileMd5(QString filename, QString& target);        //获取文件md5值
    int     GetSize(const QString& path);                         //获取文件夹总大小
    void    GetFileList();                                        //获取网盘目录列表
    void    GetBindCatalog();                                     //获取绑定目录，包括本地绑定目录及服务器绑定目录

    void    SetFileList();                                        //重设网盘目录
    void    SetBindCatalog();                                     //重设网盘绑定目录，包括本地绑定目录及服务器绑定目录

    void    ShowFileListTree();                                    //输出网盘目录树视图

    QStandardItem*  FindItem(QStandardItem *parent, QString targetName);        //根据文件名在视图中查找部件
    void            FindCompleteCatalog(QStandardItem *item, QString& catalog); //根据指定的部件寻找完整的网盘目录名

    void    SynchronizeUpload(const QString& localBind, bool flags = true) ;     //上传文件夹和文件
    void    SynchronizeDownload();    //下载文件夹和文件

    void    InsertRow(const QString& path, int size, const QString& status, QProgressBar* bar) ;    //向表格中插入一行同步文件信息
    void    WriteSynLogFile();                          //创建同步日志
    void    AddWatcherPaths(const QString& path);                          //向监视器中加入监视目录

public slots:
    void    ChangeBarValue(QProgressBar *bar, int value);
    void    ChangeFileSize(int size);
    void    ChangeLabelValue(QString value);
    void    ChangeBrowserValue(QString value);
    void    ChangeFilelist();

private slots:
    void on_BindPushButton_clicked();   //同步按钮点击槽

    void on_UnbindPushButton_clicked();

    void on_logoutPushButton_clicked();

    void on_ChangeBindPushButton_clicked();

signals:
    void        ChangeListDone(int value);
    void        DirectoryChanged();

private:
    Ui::Mainwindow         *ui;

    QDirModel              *localModel ;

    QStandardItemModel     *serverModel;

    QStringList             serverMd5s;
    QStringList             localMd5s;

    QString                 localBindCatalog;
    QString                 serverBindCatalog;

    Socket                 *s;

    FileList                Filelist;               //网盘目录列表
    FileList                serverBindFilelist;     //网盘绑定目录列表
    FileList                localBindFilelist;      //本地绑定目录列表

    bool                    isMacBinded;            //本机是否进行过绑定
    bool                    isUserBinded;           //本用户是否进行过绑定

};

#endif // MAINWINDOW_H
