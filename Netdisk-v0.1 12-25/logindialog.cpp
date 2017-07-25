#include "ui_logindialog.h"
#include "logindialog.h"
#include "signupdialog.h"
#include "define.h"
#include <QDebug>
#include <QFile>
#include <QDir>
#include <QMessageBox>

QString Username;

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    s = new Socket();
    setWindowTitle("Login");
}

LoginDialog::~LoginDialog()
{
    delete ui;
    delete s;
}

bool LoginDialog::AuthUserInfo(QString username, QString password)
{
    if (username.isEmpty())
    {
        ui->usernameAuthLabel->setText("username cannot be empty");
        return false;
    }
    if (password.isEmpty())
    {
        ui->pwdAuthLabel->setText("password cannot be empty");
        return false;
    }
    return true;
}

void LoginDialog::CreateConfFile(QString username)
{
    QDir dir(PATH_CONF);
    if (!dir.exists())
        if (!dir.mkpath(PATH_CONF))
        {
            QMessageBox::warning(NULL, "warning", "cannot make config path",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            this->close();
        }
    QDir dir1(PATH_LOG);
    if (!dir1.exists())
        if (!dir1.mkpath(PATH_LOG))
        {
            QMessageBox::warning(NULL, "warning", "cannot make log path",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            this->close();
        }

    QFile * logfile = new QFile(QString(PATH_LOG+ username + ".txt"));
    if (!logfile->exists())
    {
        if (!logfile->open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::warning(NULL, "warning", "cannot make log file",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            return ;
        }
        logfile->close();
    }

    QFile * confile = new QFile(QString(PATH_CONF+ username + ".txt"));
    if (!confile->exists())
    {
        if (!confile->open(QIODevice::WriteOnly|QIODevice::Text))
        {
            QMessageBox::warning(NULL, "warning", "cannot make config file",
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::Yes);
            return ;
        }
        QTextStream in(confile);
        in<<"Username = "<<username<<";"<<endl;
        in<<"Local bind catalog = "<<"NOT BINDED;"<<endl;
        confile->close();
    }
}


void LoginDialog::on_LoginPushButton_clicked()
{    
    if (!AuthUserInfo(ui->usernameLineEdit->text(), ui->passwordLineEdit->text()))
        return ;

    QString infoFromServer ;
    ui->infoLabel->setText("login...");

    if (s->connectServer())
        ui->infoLabel->setText("login successfuly!");
    else
    {
        ui->infoLabel->setText("login failed,check your internet");
        return ;
    }

    s->sendMessage(PROTOCOL_LOGIN);
    s->sendMessage(ui->usernameLineEdit->text(), 16);
    s->sendMessage(ui->passwordLineEdit->text(), 16);
    s->recvMessage(infoFromServer, BUFSIZE);
    ui->infoLabel->setText(infoFromServer);
    if (infoFromServer == QString("Login Success"))
    {
        s->closeSocket();

        /********************¸øUsername¸³Öµ******************/
        Username = ui->usernameLineEdit->text();

        CreateConfFile(Username);
        accept();
    }
    else
        s->closeSocket();
}

void LoginDialog::on_SignPushButton_clicked()
{
    SignupDialog * signDialog = new SignupDialog();
    signDialog->setModal(true);
    signDialog->show();
}
