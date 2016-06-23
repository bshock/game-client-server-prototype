#ifndef CONNECTIONDIALOG_H
#define CONNECTIONDIALOG_H

#include <QDialog>

namespace Ui {
class ConnectionDialog;
}

class ConnectionDialog : public QDialog
{
    Q_OBJECT
    Q_PROPERTY(QString ip READ
    ip WRITE setIP)
    Q_PROPERTY(int port READ
    port WRITE setPort)
public:
    explicit ConnectionDialog(QWidget *parent = 0);
    ~ConnectionDialog();

    virtual void showEvent(QShowEvent *) override;

    void setIP(const QString &ip);
    void setPort(int prt);
    QString ip() const;
    int port() const;

private:
    Ui::ConnectionDialog *ui;

    bool validateIP(const QString &iP) const;
};

#endif // CONNECTIONDIALOG_H
