#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QString>
#include "socket.h"
namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT

public:
    explicit LoginDialog(QWidget *parent = 0);

    ~LoginDialog();

    bool AuthUserInfo(QString username, QString password);

    void CreateConfFile(QString username);
private slots:

    void on_LoginPushButton_clicked();

    void on_SignPushButton_clicked();

private:
    Ui::LoginDialog *ui;

    Socket * s;
};

#endif // LOGINDIALOG_H
