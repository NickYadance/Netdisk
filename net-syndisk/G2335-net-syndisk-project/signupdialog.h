#ifndef SIGNUPDIALOG_H
#define SIGNUPDIALOG_H

#include <QDialog>
#include "socket.h"
namespace Ui {
class SignupDialog;
}

class SignupDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SignupDialog(QWidget *parent = 0);

    ~SignupDialog();

    bool AuthUserInfo(QString username, QString password, QString repassword);

    void CreateConfFile(QString username);
private slots:
    void on_okPushButton_clicked();

private:
    Ui::SignupDialog *ui;

     Socket * s;
};

#endif // SIGNUPDIALOG_H
