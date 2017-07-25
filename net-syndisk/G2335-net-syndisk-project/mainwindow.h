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

//�����ļ�Ŀ¼
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

    void    GetServerMd5s();                                      //��÷�������ʵ�ļ�md5�б�
    void    GetFileMd5(QString filename, QString& target);        //��ȡ�ļ�md5ֵ
    int     GetSize(const QString& path);                         //��ȡ�ļ����ܴ�С
    void    GetFileList();                                        //��ȡ����Ŀ¼�б�
    void    GetBindCatalog();                                     //��ȡ��Ŀ¼���������ذ�Ŀ¼����������Ŀ¼

    void    SetFileList();                                        //��������Ŀ¼
    void    SetBindCatalog();                                     //�������̰�Ŀ¼���������ذ�Ŀ¼����������Ŀ¼

    void    ShowFileListTree();                                    //�������Ŀ¼����ͼ

    QStandardItem*  FindItem(QStandardItem *parent, QString targetName);        //�����ļ�������ͼ�в��Ҳ���
    void            FindCompleteCatalog(QStandardItem *item, QString& catalog); //����ָ���Ĳ���Ѱ������������Ŀ¼��

    void    SynchronizeUpload(const QString& localBind, bool flags = true) ;     //�ϴ��ļ��к��ļ�
    void    SynchronizeDownload();    //�����ļ��к��ļ�

    void    InsertRow(const QString& path, int size, const QString& status, QProgressBar* bar) ;    //�����в���һ��ͬ���ļ���Ϣ
    void    WriteSynLogFile();                          //����ͬ����־
    void    AddWatcherPaths(const QString& path);                          //��������м������Ŀ¼

public slots:
    void    ChangeBarValue(QProgressBar *bar, int value);
    void    ChangeFileSize(int size);
    void    ChangeLabelValue(QString value);
    void    ChangeBrowserValue(QString value);
    void    ChangeFilelist();

private slots:
    void on_BindPushButton_clicked();   //ͬ����ť�����

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

    FileList                Filelist;               //����Ŀ¼�б�
    FileList                serverBindFilelist;     //���̰�Ŀ¼�б�
    FileList                localBindFilelist;      //���ذ�Ŀ¼�б�

    bool                    isMacBinded;            //�����Ƿ���й���
    bool                    isUserBinded;           //���û��Ƿ���й���

};

#endif // MAINWINDOW_H
