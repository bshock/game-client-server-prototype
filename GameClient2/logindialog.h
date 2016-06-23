#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>

namespace Ui {
class LoginDialog;
}

class LoginDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString userName READ
    userName WRITE setUserName)
    Q_PROPERTY(QString password READ
    password WRITE setPassword)

public:
    explicit LoginDialog(QWidget *parent = 0);
    ~LoginDialog();

    virtual void showEvent(QShowEvent *) override;

    void setUserName(const QString &uname);
    void setPassword(const QString &pword);
    QString userName() const;
    QString password() const;
public slots:
    void updateOKButtonState();

private:
    Ui::LoginDialog *ui;
};

#endif // LOGINDIALOG_H
