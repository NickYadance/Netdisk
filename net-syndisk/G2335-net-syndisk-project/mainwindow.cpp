#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "define.h"
#include <sys/utime.h>
#include <time.h>
#include <windows.h>
#include <QDebug>
#include <QDir>
#include <QStringList>
#include <QByteArray>
#include <QIcon>
#include <QModelIndex>
#include <QMessageBox>
#include <QCryptographicHash>
#include <QFileInfoList>
#include <QProgressBar>
#include <QDateTime>
#include <QTableWidget>
#include <QTableView>
#include <QTextStream>
#include <QScrollBar>
#include <QMetaType>
#include "threadSend.h"

extern QString Username;

Mainwindow::Mainwindow(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Mainwindow)
{
    ui->setupUi(this);
    localModel  = new QDirModel();
    serverModel = new QStandardItemModel(ui->serverTreeView);
    s           = new Socket();
    watcher     = new QFileSystemWatcher();

    localModel->setSorting(QDir::DirsLast | QDir::IgnoreCase | QDir::Name);

    ui->localTreeView->setModel(localModel);
    ui->localTreeView->setRootIndex(localModel->index("D:\\"));
    ui->localTreeView->header()->hideSection(1);
    ui->localTreeView->header()->resizeSection(0, 200);
    ui->localTreeView->header()->setStretchLastSection(true);

    ui->serverTreeView->setModel(serverModel);
    ui->serverTreeView->header()->setStretchLastSection(true);

    ui->tableWidget->setColumnCount(5);
    ui->tableWidget->setHorizontalHeaderLabels(QStringList()<<QString("Name")
                                               <<QString("Synsize")
                                               <<QString("Totalsize")
                                               <<QString("SynType")
                                               <<QString("Progress"));

    QScrollBar * tableBar = ui->tableWidget->verticalScrollBar();
    tableBar->setRange(0, 200);
    tableBar->setValue(40);

    ui->tableWidget->horizontalHeader()->resizeSection(0, 250);
    ui->tableWidget->horizontalHeader()->resizeSection(1, 82);
    ui->tableWidget->horizontalHeader()->resizeSection(2, 82);
    ui->tableWidget->horizontalHeader()->resizeSection(3, 80);
    ui->tableWidget->horizontalHeader()->resizeSection(4, 150);
    ui->tableWidget->setAutoScroll(true);

    if (localBindCatalog == MARK_NULL_CATALOG || serverBindCatalog == MARK_NULL_CATALOG)
    {
        ui->BindPushButton->setEnabled(false);
        ui->UnbindPushButton->setEnabled(true);
    }
    else
    {
        ui->BindPushButton->setEnabled(true);
        ui->UnbindPushButton->setEnabled(false);
    }
}

Mainwindow::~Mainwindow()
{
    delete ui;
}

void Mainwindow::GetServerMd5s()
{
    serverMd5s.clear();

    if (s->connectServer())
        ui->infoLabel->setText(Username);
    else
    {
        ui->infoLabel->setText("not login in");
        return ;
    }

    s->sendMessage(PROTOCOL_GET_MD5);
    while(1)
    {
        QString md5 ;
        s->recvMessage(md5, 32);
        if (md5 == PROTOCOL_END)
            break;
        serverMd5s.push_back(md5);
    }
    s->closeSocket();
}

void Mainwindow::GetFileList()
{
    Filelist.clear();

    if (s->connectServer())
        ui->infoLabel->setText(Username);
    else
    {
        ui->infoLabel->setText("not login in");
        return ;
    }
    s->sendMessage(PROTOCOL_GET_FILELIST);
    s->sendMessage(Username, 16);

    QString path, isFile, updateTime, md5;
    while(1)
    {
        s->recvMessage(updateTime, 19);
        if (updateTime == PROTOCOL_END)
            break ;
        s->recvMessage(isFile, 1);
        s->recvMessage(md5, 32);
        s->recvMessage(path, 128);
        Filelist.push_back(File(path, isFile, updateTime, md5));
    }
    s->closeSocket();
}

void Mainwindow::GetFileMd5(QString filename, QString &target)
{
    QFile file(filename);
    if (!file.open(QIODevice::ReadOnly))
        return ;
    QByteArray ba = QCryptographicHash::hash(file.readAll(),QCryptographicHash::Md5);
    file.close();
    target = ba.toHex().data() ;
    return ;
}

int Mainwindow::GetSize(const QString &path)
{
    QDir dir(path);
    quint64 size = 0;

    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
    {
        size += fileInfo.size();
    }
    foreach(QString subDir, dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        size += GetSize(path + QDir::separator() + subDir);
    }

    return size;
}

void Mainwindow::GetBindCatalog()
{
    if (s->connectServer())
        ui->infoLabel->setText(Username);
    else
    {
        ui->infoLabel->setText("not login in");
        return ;
    }
    s->sendMessage(PROTOCOL_GET_BINDCATALOG);
    s->sendMessage(Username, 16);
    s->recvMessage(serverBindCatalog, 128);
    s->closeSocket();

    QFile * confile = new QFile(PATH_CONF+  Username + ".txt");
    if (!confile->open(QIODevice::ReadOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, "warning", "cannot open the operation file",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        this->close();
        return ;
    }

    QTextStream out(confile);
    QString text = out.readAll();
//    localBindCatalog = MARK_NULL_CATALOG;
    localBindCatalog = QString(text.split(';').at(1)).section('=', -1, -1).simplified();
    confile->close();

    ui->LBindTextEdit->setText(localBindCatalog);
    ui->SBindTextEdit->setText(serverBindCatalog);

    isMacBinded = (localBindCatalog != MARK_NULL_CATALOG) ;          //本机是否进行过绑定
    isUserBinded = (serverBindCatalog != MARK_NULL_CATALOG) ;        //本用户是否进行过绑定

    if (isMacBinded)
    {
        AddWatcherPaths(localBindCatalog);
    }

    ui->BindPushButton->setEnabled(!isMacBinded);
    ui->UnbindPushButton->setEnabled(isMacBinded);
}

void Mainwindow::SetFileList()
{
    if (s->connectServer())
        ui->infoLabel->setText(Username);
    else
    {
        ui->infoLabel->setText("not login in");
        return ;
    }

    s->sendMessage(PROTOCOL_SET_FILELIST) ;
    s->sendMessage(Username, 16);
    foreach(File file, serverBindFilelist)
    {
        s->sendMessage(file.path, 128);
        s->sendMessage(file.isFile, 1);
        s->sendMessage(file.updateTime, 19);
        s->sendMessage(file.md5, 32);
    }
    s->sendMessage(PROTOCOL_END, 128) ;
    s->closeSocket();
}

void Mainwindow::SetBindCatalog()
{
    if (s->connectServer())
        ui->infoLabel->setText(Username);
    else
    {
        ui->infoLabel->setText("not login in");
        return ;
    }
    s->sendMessage(PROTOCOL_SET_BINDCATALOG);
    s->sendMessage(Username, 16);
    s->sendMessage(serverBindCatalog, 128);
    s->closeSocket();

    QString confpath =  QString(PATH_CONF)+ Username + ".txt";

    QFile * writeFile = new QFile(confpath);
    if (!writeFile->open(QIODevice::WriteOnly|QIODevice::Text))
    {
        QMessageBox::warning(NULL, "warning", "cannot open the operation file",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        this->close();
        return ;
    }

    QTextStream in(writeFile);
    in<<"Username = "<<Username<<";"<<endl;
    in<<"Local bind catalog = "<<localBindCatalog<<";"<<endl;
    writeFile->close();

    ui->BindPushButton->setEnabled(false);
    ui->UnbindPushButton->setEnabled(true);
    ui->LBindTextEdit->setText(localBindCatalog);
    ui->SBindTextEdit->setText(serverBindCatalog);

    isMacBinded = (localBindCatalog != MARK_NULL_CATALOG) ;          //本机是否进行过绑定
    isUserBinded = (serverBindCatalog != MARK_NULL_CATALOG) ;        //本用户是否进行过绑定

    if (isMacBinded)
    {
        AddWatcherPaths(localBindCatalog);
    }
}

void Mainwindow::ShowFileListTree()
{
    serverModel->clear();
    serverModel->setHorizontalHeaderLabels(QStringList()<<QString("Name")
                                           <<QString("Type")
                                           <<QString("Date Modified"));
    ui->serverTreeView->header()->resizeSection(0, 200);
    localModel->refresh();

    QString path ;
    QString isFile;
    QString updateTime;

    foreach (File file, Filelist)
    {
        path = file.path;
        isFile = file.isFile;
        updateTime = file.updateTime;

        int depth = path.count('/') - 1 ;

        //根目录
        if (depth < 0)
        {
            QStandardItem   *newItem = new QStandardItem(ICON_DIR, path);
            serverModel->appendRow(newItem);
            continue ;
        }

        //子目录
        QStandardItem   *newItem;
        QStandardItem   *parentItem = serverModel->item(0);

        int flag = isFile.toInt();
        if (flag)
            newItem = new QStandardItem(ICON_FILE, path.section('/', -1, -1));
        else
            newItem = new QStandardItem(ICON_DIR, path.section('/', -1, -1));

        for (int i=0; i<depth; i++)
        {
            parentItem = FindItem(parentItem, path.section('/', i+1, i+1));
        }
        parentItem->appendRow(newItem);

        if (flag)
            parentItem->setChild(newItem->index().row(), 1, new QStandardItem("File"));
        else
            parentItem->setChild(newItem->index().row(), 1, new QStandardItem("Dir"));

        parentItem->setChild(newItem->index().row(), 2, new QStandardItem(updateTime));
    }
}

void Mainwindow::FindCompleteCatalog(QStandardItem *item, QString &catalog)
{
    if (item)
    {
        FindCompleteCatalog(item->parent(), catalog);

        if (item->parent())
            catalog += "/" + item->data(0).toString();
        else
            catalog += item->data(0).toString();
    }
    else
    {
        return ;
    }
}

QStandardItem *Mainwindow::FindItem(QStandardItem *parent, QString targetName)
{
    QStandardItem * item ;
    for (int i=0; i<parent->rowCount(); i++)
    {
        item = parent->child(i);
        if (item->data(0).toString() == targetName)
        {
            return item ;
        }
    }
    return NULL;
}

void Mainwindow::SynchronizeUpload(const QString& localBind, bool flags)
{
    QDir    dir(localBind);
    QString md5 ;
    QString filepath;
    QString path ;
    QString isFile;
    QString updateTime;
    int     filesize;
    /**************对文件的操作**************/
    foreach(QFileInfo fileInfo, dir.entryInfoList(QDir::Files))
    {
        filepath    = fileInfo.filePath();
        path        = filepath;
        isFile      = "1";
        updateTime  = fileInfo.lastModified().toString("yyyy-MM-dd hh:mm:ss");
        filesize    = fileInfo.size();

        if (filesize == 0)  continue ;

        QProgressBar *bar = new QProgressBar;

        GetFileMd5(filepath, md5);

        bar->setRange(0, filesize);
        bar->setValue(0);
        path.replace(localBindCatalog, serverBindCatalog) ;           //本地路径转换为服务器路径

        QStringList pathList;               //相同md5值的文件的路径列表
        bool hasMd5;                        //服务器有无对应真实文件
        bool hasPath = false;                       //同步目录下有无对应同名文件

        hasMd5 = serverMd5s.contains(md5);
        qDebug() << serverMd5s << endl;

        /**************真实文件存在**************/
        if (hasMd5)
        {
            bool equalMd5 = false;

            foreach(File file, serverBindFilelist)
            {
                if (file.path == path)  hasPath = true;
                if (file.md5 == md5)    equalMd5 = true;
                if (file.md5 == md5)
                    pathList.push_back(file.path);
            }

            /**************真实文件存在,同名文件存在**************/
            if (hasPath)
            {
                if (equalMd5)
                {
                    if (flags)
                    {
                        InsertRow(filepath, filesize, "FileExits", bar);
                        bar->setValue(filesize);
                        ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(filesize) + "Byte");
                    }
                }
                else
                {
                    int i = 1;
                    int n = filepath.count('/');
                    QString newname ;

                    do
                    {
                        newname = filepath.section('/', 0, n-1) + "/" + QString("(%1)").arg(i++) +filepath.section('/', -1, -1);
                    }
                    while(!QFile::rename(filepath, newname));

                    path = filepath = newname;
                    path.replace(localBindCatalog, serverBindCatalog) ;

                    InsertRow(filepath, filesize, "NameChanged&&FastUpload", bar);

                    Filelist.push_back(File(path, isFile, updateTime, md5));
                    serverBindFilelist.push_back(File(path, isFile, updateTime, md5));

                    bar->setValue(filesize);
                    ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(filesize) + "Byte");
                }

            }

            /**************真实文件存在,同名文件不存在**************/
            else
            {
                if (equalMd5)
                {
                    bool flag = false;
                    foreach(QString pathtemp, pathList)
                    {
                        QString newname ;

                        if (path.section('/', 0, -2) == pathtemp.section('/', 0, -2))
                        {

                            int i = 0;
                            newname = pathtemp ;
                            newname.replace(serverBindCatalog, localBindCatalog) ;

                            /*
                             * 这里本机按照服务器上的标准改名字，如果改名字之后的文件已经存在，则会加上依次尝试在文件名前加上(0),(1),(2)...的前缀
                             */
                            while(!QFile::rename(filepath, newname))
                            {
                                newname = filepath.section('/', 0, -2) + QString("/(%1)").arg(i++) + pathtemp.section('/', -1 ,-1);
                            }

                            path = filepath = newname ;
                            path.replace(localBindCatalog, serverBindCatalog) ;
                            flag = true;
                            break ;
                        }
                    }
                    if (flag)
                    {
                        InsertRow(filepath, filesize, "NamedChangedByServer", bar);
                        localModel->refresh();
                    }
                    else
                    {
                        InsertRow(filepath, filesize, "FastUpload", bar);
                        Filelist.push_back(File(path, isFile, updateTime, md5));
                        serverBindFilelist.push_back(File(path, isFile, updateTime, md5));
                        bar->setValue(filesize);
                        ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(filesize) + "Byte");
                    }
                }
                else
                {
                    InsertRow(filepath, filesize, "FastUpload", bar);
                    Filelist.push_back(File(path, isFile, updateTime, md5));
                    serverBindFilelist.push_back(File(path, isFile, updateTime, md5));
                    bar->setValue(filesize);
                    ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(filesize) + "Byte");
                }
            }
        }

        /**************真实文件不存在**************/
        else
        {
            foreach(File file, serverBindFilelist)
            {
                if (file.path == path)
                {
                    hasPath = true;
                    //break;
                }
            }

            /**************真实文件不存在,同名文件存在**************/
            if (hasPath)
            {
                int i = 1;
                QString newname ;

                do
                {
                    newname = filepath.section('/', 0, -2) + "/" + QString("(%1)").arg(i++) +filepath.section('/', -1, -1);
                    //should name it better
                    //newname = filepath.section('/', 0, -2) + "/" + Username + QString("(%1)").arg(i++) +filepath.section('/', -1, -1);
                }
                while(!QFile::rename(filepath, newname));

                path = filepath = newname;
                path.replace(localBindCatalog, serverBindCatalog) ;

                InsertRow(filepath, filesize, "NameChanged&&Upload", bar);
            }

            /**************真实文件不存在,同名文件不存在**************/
            else
            {
                InsertRow(filepath, filesize, "Upload", bar);
            }

            /**************开新线程上传文件**************/
            s->sendMessage(path, 128);
            s->sendMessage("1");
            s->sendMessage(updateTime);
            s->sendMessage(md5);

            sendThread * pthread = new sendThread(s, filepath, bar, filesize, 1);
            QObject::connect(pthread, SIGNAL(barValueChanged(QProgressBar*,int)), this, SLOT(ChangeBarValue(QProgressBar*,int)));
            QObject::connect(pthread, SIGNAL(filesizeChanged(int)), this, SLOT(ChangeFileSize(int)));
            pthread->start();
            while(1)
            {
                QCoreApplication::processEvents();
                if (pthread->isFinished())
                    break;
            }

            ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(filesize) + "Byte");

            Filelist.push_back(File(path, isFile, updateTime, md5));
            serverBindFilelist.push_back(File(path, isFile, updateTime, md5));
            serverMd5s.push_back(md5);
        }

        localBindFilelist.push_back(File(path, isFile, updateTime, md5));
        localMd5s.push_back(md5);
    }

    /**************对子文件夹的操作**************/
    foreach(QFileInfo subDir, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot))
    {
        QString subDirFilepath = subDir.filePath() ;
        QString subDirTime = subDir.lastModified().toString("yyyy-MM-dd hh:mm:ss");
        QString subDirPath = subDirFilepath;

        subDirPath.replace(localBindCatalog, serverBindCatalog);

        File newdir(subDirPath, "0", subDirTime, "00000000000000000000000000000000");

        if (!Filelist.contains(newdir))
            Filelist.push_back(File(subDirPath, "0", subDirTime, "00000000000000000000000000000000"));

        if (!serverBindFilelist.contains(newdir))
            serverBindFilelist.push_back(File(subDirPath, "0", subDirTime, "00000000000000000000000000000000"));

        if (!localBindFilelist.contains(newdir))
            localBindFilelist.push_back(File(subDirPath, "0", subDirTime, "00000000000000000000000000000000"));

        SynchronizeUpload(subDirFilepath, flags);
    }
}

void Mainwindow::SynchronizeDownload()
{
    /**************遍历服务器绑定目录**************/
    foreach(File serverFile, serverBindFilelist)
    {
        QString pathtemp ;
        bool hasPath = false;

        foreach(File localFile, localBindFilelist)
        {
            if (localFile.md5 == serverFile.md5)    pathtemp = localFile.path;

            if (localFile.path == serverFile.path)
            {
                hasPath = true;
                break;
            }
        }

        /**************有同名文件**************/
        if (hasPath)
        {

        }

        /**************无同名文件**************/
        else
        {
            QString filepath = serverFile.path;

            filepath.replace(serverBindCatalog, localBindCatalog);

            /**************文件夹**************/
            if (serverFile.isFile == "0")
            {
                qDebug() << "new dir: " << filepath << endl;
                QDir dir(filepath);

                if (!dir.mkpath(filepath))
                {
                    perror("mkpath error");
                }
            }

            /**************文件**************/
            else
            {
                bool        hasMd5 = localMd5s.contains(serverFile.md5);
                int         filesize = 0;
                QProgressBar *bar = new QProgressBar();

                /**************无同名文件,真实文件已经存在**************/
                if (hasMd5)
                {
                    pathtemp.replace(serverBindCatalog, localBindCatalog);
                    qDebug() << "pathtemp" << pathtemp << endl;
                    qDebug() << "filepath" <<filepath<< endl;
                    QFile::copy(pathtemp, filepath);

                    filesize = QFile(filepath).size();
                    InsertRow(filepath, filesize, "FastDownload", bar );

                    bar->setRange(0, filesize);
                    bar->setValue(0);

                    ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(filesize) + "Byte");
                    bar->setValue(filesize);
                }

                /**************无同名文件，真实文件不存在**************/
                else
                {
                    QString size;

                    s->sendMessage(serverFile.md5, 32);
                    s->recvMessage(size, FILESIZE);

                    filesize = size.toInt();
                    bar->setRange(0, filesize);
                    bar->setValue(0);

                    InsertRow(filepath, filesize, "Download", bar);

                    sendThread * pthread = new sendThread(s, filepath, bar, filesize, 0);
                    QObject::connect(pthread, SIGNAL(barValueChanged(QProgressBar*,int)), this, SLOT(ChangeBarValue(QProgressBar*,int)));
                    QObject::connect(pthread, SIGNAL(filesizeChanged(int)), this, SLOT(ChangeFileSize(int)));
                    pthread->start();
                    while(1)
                    {
                        QCoreApplication::processEvents();
                        if (pthread->isFinished())
                            break;
                    }
                }
            }
            localBindFilelist.push_back(serverFile);
        }
    }
}
void Mainwindow::InsertRow(const QString &path, int size, const QString &status, QProgressBar *bar)
{
    int irow = ui->tableWidget->rowCount() ;
    ui->tableWidget->insertRow( irow );
    ui->tableWidget->setItem(irow, 0, new QTableWidgetItem(path));
    ui->tableWidget->setItem(irow, 1, new QTableWidgetItem(QString::number(0) + "Byte"));
    ui->tableWidget->setItem(irow, 2, new QTableWidgetItem(QString("%1Byte").arg(size)));
    ui->tableWidget->setItem(irow, 3, new QTableWidgetItem(status));
    ui->tableWidget->setCurrentCell(irow, 0);
    ui->tableWidget->verticalScrollBar()->setValue(ui->tableWidget->verticalScrollBar()->maximum() - 1);
    ui->tableWidget->setCellWidget( irow, 4, bar);
}

void Mainwindow::WriteSynLogFile()
{
    QFile log(QString(PATH_LOG+ Username + ".txt"));
    if (!log.open(QIODevice::WriteOnly|QIODevice::Text|QIODevice::Append))
    {
        QMessageBox::warning(NULL, "warning", "cannot make log file",
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
        return ;
    }
    QTextStream in(&log);
    QStringList info;
    info <<QString("Name")
        <<QString("Synsize")
       <<QString("Totalsize")
      <<QString("SynType");

    in << "Username: " <<Username<<", Synchorize time: "<<QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")<<endl;

    //遍历table取得同步文件的信息，写入日志
    for (int i = 0; i < ui->tableWidget->rowCount(); i++)
    {
        for (int j = 0; j< ui->tableWidget->columnCount() - 1; j++)
        {
            in << info.at(j) <<": "<<ui->tableWidget->item(i, j)->data(0).toString()<<"    ";
        }
        in << endl ;
    }
    in << endl;
    log.close();
}

void Mainwindow::AddWatcherPaths(const QString &path)
{
    QDir            dir(path);
    QFileInfoList   filelist = dir.entryInfoList(QDir::Files) ;

    watcher->addPath(path);
    foreach (QFileInfo file, filelist)
    {
        watcher->addPath(file.filePath());
    }

    foreach (QFileInfo subdir, dir.entryInfoList(QDir::Dirs | QDir::NoDotAndDotDot)) {
        AddWatcherPaths(subdir.filePath());
    }
}

void Mainwindow::ChangeBarValue(QProgressBar *bar, int value)
{
    bar->setValue(value);
}

void Mainwindow::ChangeFileSize(int size)
{
    ui->tableWidget->item(ui->tableWidget->rowCount() - 1, 1)->setText(QString::number(size) + "Byte");
}

void Mainwindow::ChangeLabelValue(QString value)
{
    ui->infoLabel->setText(value);
}

void Mainwindow::ChangeBrowserValue(QString value)
{
    ui->infoTextBrowser->append(value);
}

void Mainwindow::ChangeFilelist()
{
    if (localBindCatalog == MARK_NULL_CATALOG)
        return ;

    emit    ChangeListDone(false);

    if (s->connectServer())
        ui->infoLabel->setText(Username);
    else
    {
        ui->infoLabel->setText("not login in");
        return ;
    }

    GetFileList();
    GetServerMd5s();

    serverBindFilelist.clear();
    localBindFilelist.clear();
    localMd5s.clear();

    foreach(File file, Filelist)
    {        if (file.path.contains(serverBindCatalog))
            serverBindFilelist.push_back(file);
    }

    s->connectServer();
    s->sendMessage(PROTOCOL_UPLOAD);
    SynchronizeUpload(localBindCatalog, false);
    s->sendMessage(PROTOCOL_END, 128);   //a special one
    s->closeSocket();

    s->connectServer();
    s->sendMessage(PROTOCOL_DOWNLOAD);
    SynchronizeDownload();
    s->sendMessage(PROTOCOL_END, 32);
    s->closeSocket();

    SetFileList();

    emit    ChangeListDone(true);

    WriteSynLogFile();

    ShowFileListTree();
}

void Mainwindow::on_BindPushButton_clicked()
{
    QModelIndex localIndex = ui->localTreeView->currentIndex();
    QModelIndex serverIndex = ui->serverTreeView->currentIndex();
    if (!localIndex.isValid())
    {
        ui->infoTextBrowser->append("Choose the local catalog to be binded");
        return ;
    }
    if (!serverIndex.isValid() && !isUserBinded)
    {
        ui->infoTextBrowser->append("Choose the server catalog to be binded");
        return ;
    }

    QStandardItem *item = serverModel->itemFromIndex(serverIndex);

    if (localModel->isDir(localIndex))
    {
        emit   ChangeListDone(false);

        localBindCatalog = localModel->filePath(localIndex) ;
        if (!isUserBinded)
        {
            serverBindCatalog.clear();
            FindCompleteCatalog(item, serverBindCatalog);
        }

        if (s->connectServer())
            ui->infoLabel->setText(Username);
        else
        {
            ui->infoLabel->setText("not login in");
            return ;
        }

        serverBindFilelist.clear();
        localBindFilelist.clear();
        localMd5s.clear();

        foreach(File file, Filelist)
        {
            if (file.path.contains(serverBindCatalog))
                serverBindFilelist.push_back(file);
        }

        s->sendMessage(PROTOCOL_UPLOAD);
        SynchronizeUpload(localBindCatalog);
        s->sendMessage(PROTOCOL_END, 128);   //a special one
        s->closeSocket();

        if (isUserBinded)
        {
            s->connectServer();
            s->sendMessage(PROTOCOL_DOWNLOAD);
            SynchronizeDownload();
        }

        s->sendMessage(PROTOCOL_END, 32);
        s->closeSocket();

        SetBindCatalog();
        SetFileList();

        WriteSynLogFile();
        serverModel->clear();
        localModel->refresh();
        ShowFileListTree();

        emit   ChangeListDone(true);
    }
    else
    {
        ui->infoTextBrowser->append("cannot synchronize file, choose a dir");
    }
}

void Mainwindow::on_UnbindPushButton_clicked()
{
    if (QMessageBox::Yes ==
            QMessageBox::information(this, "Infomation", "Are you sure to unbind ?", QMessageBox::Yes|QMessageBox::No))
    {
        localBindCatalog = MARK_NULL_CATALOG ;

        QString confpath =  QString(PATH_CONF)+ Username + ".txt";
        QFile * writeFile = new QFile(confpath);
        if (!writeFile->open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::warning(NULL, "warning", "cannot open the operation file",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            this->close();
            return ;
        }

        QTextStream in(writeFile);
        in<<"Username = "<<Username<<";"<<endl;
        in<<"Local bind catalog = "<<localBindCatalog<<";"<<endl;
        writeFile->close();

        ui->BindPushButton->setEnabled(true);
        ui->UnbindPushButton->setEnabled(false);
        ui->LBindTextEdit->setText(localBindCatalog);

        isMacBinded = (localBindCatalog != MARK_NULL_CATALOG) ;          //本机是否进行过绑定
    }
}

void Mainwindow::on_logoutPushButton_clicked()
{
    localModel->refresh();
}

void Mainwindow::on_ChangeBindPushButton_clicked()
{
    if (QMessageBox::Yes == QMessageBox::information(this, "Infomation",
                                                    "Change the local catalog to the current choosen one?",
                                                    QMessageBox::Yes|QMessageBox::No))
    {
        QModelIndex localIndex = ui->localTreeView->currentIndex();

        if (localModel->isDir(localIndex))
        {
            localBindCatalog = localModel->filePath(localIndex) ;
            ChangeFilelist();
            SetBindCatalog();
        }
        else
        {
            ui->infoTextBrowser->append("You should choose a local dir rather than a file");
        }
    }
}

bool operator ==(const File &file1, const File &file2)
{
    return ( (file1.md5 == file2.md5)&&
             (file1.path == file2.path)&&
             (file1.isFile == file2.isFile));
}
