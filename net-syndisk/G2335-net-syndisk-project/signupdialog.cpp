#include "signupdialog.h"
#include "ui_signupdialog.h"
#include "define.h"
#include <QRegExp>
#include <QMessageBox>
#include <QFile>
#include <QTextStream>
#include <QDebug>

SignupDialog::SignupDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::SignupDialog)
{
    ui->setupUi(this);
    s = new Socket();
    setWindowTitle("signup");
}

SignupDialog::~SignupDialog()
{
    delete ui;
    delete s ;
}

bool SignupDialog::AuthUserInfo(QString username, QString password, QString repassword)
{
    ui->pwdAuthLabel->clear();

    if (username.isEmpty() )
    {
        ui->usernameAuthLabel->setText(QString::fromLocal8Bit("�û�������Ϊ�գ�"));
        return false;
    }
    if (password.isEmpty())
    {
        ui->pwdAuthLabel->setText(QString::fromLocal8Bit("���벻��Ϊ�գ�"));
        return false;
    }
    if (username.length()<6 || username.length()>16)
    {
        ui->usernameAuthLabel->setText(QString::fromLocal8Bit("�û������ȱ���6λ����,16λ����"));
        return false;
    }
    if (password.length()<6 || password.length()>16)
    {
        ui->pwdAuthLabel->setText(QString::fromLocal8Bit("���볤�ȱ���6λ����,16λ����"));
        return false;
    }
    else
    {
        QRegExp rx ;
        rx.setPatternSyntax(QRegExp::RegExp);
        rx.setCaseSensitivity(Qt::CaseSensitive);               //�Դ�Сд��ĸ���У������ִ�Сд

        rx.setPattern("^[A-Za-z][A-Za-z0-9]+$");       //ƥ���ʽΪ��ĸ��ͷ����������ĸ�����ֵ��ַ�����λ������
        if (!rx.exactMatch(username))
        {
            ui->usernameAuthLabel->setText(QString::fromLocal8Bit("��������ĸ��ͷ�����������֡���ĸ"));
            return false;
        }

        rx.setPattern("^(?![A-Z]+$)(?![a-z]+$)(?!\\d+$)(?![\\W_]+$)\\S{6,16}$");     //ƥ���ʽΪ��ĸ��ͷ����������ĸ�����ֵ��ַ�����λ������
        if (!rx.exactMatch(password))
        {
            ui->pwdAuthLabel->setText(QString::fromLocal8Bit("���д�Сд��ĸ,����,����������ֻ�����"));
            return false;
        }
    }
    if (password != repassword)
    {
        ui->pwdreAuthLabel->setText(QString::fromLocal8Bit("���벻һ��"));
        return false;
    }
    return true;
}


void SignupDialog::on_okPushButton_clicked()
{
    ui->usernameAuthLabel->clear();
    ui->pwdAuthLabel->clear();
    ui->pwdreAuthLabel->clear();
    ui->infoLabel->clear();
    QString infoFromServer ;
    if (AuthUserInfo(ui->usernameLineEdit->text(), ui->pwdLineEdit->text(), ui->pwdreLineEdit->text()))
    {
        ui->infoLabel->clear();
        ui->infoLabel->setText("login...");

        if (s->connectServer())
            ui->infoLabel->setText("connected to the server successfuly");
        else
        {
            ui->infoLabel->setText("signup failed,check your internet");
            return ;
        }
        s->sendMessage(PROTOCOL_SIGNUP);
        s->sendMessage(ui->usernameLineEdit->text(), 16);
        s->sendMessage(ui->pwdLineEdit->text(), 16);
        s->recvMessage(infoFromServer, BUFSIZE);

        ui->infoLabel->setText(infoFromServer);
        s->closeSocket();
    }
}
