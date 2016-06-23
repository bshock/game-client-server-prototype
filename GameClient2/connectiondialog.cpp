#include "connectiondialog.h"
#include "ui_connectiondialog.h"
#include <QHostAddress>

ConnectionDialog::ConnectionDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::ConnectionDialog)
{
    ui->setupUi(this);
}

ConnectionDialog::~ConnectionDialog()
{
    delete ui;
}

void ConnectionDialog::showEvent(QShowEvent *)
{
    ui->lineEdit->setFocus();
}

void ConnectionDialog::setIP(const QString &ip) {
ui->lineEdit->setText(ip);
}

void ConnectionDialog::setPort(int prt) {
    ui->spinBox->setValue(prt);
}

QString ConnectionDialog::ip() const
{ return ui->lineEdit->text(); }

int ConnectionDialog::port() const
{ return ui->spinBox->value(); }

bool ConnectionDialog::validateIP(const QString &iP) const
{
    QHostAddress address(iP);
    if (address.isNull()) {
        return false;
    }
    return true;
    //You could try a regex like ^(?:(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\.){3}(?:25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)$
    //  -- but what if somebody had a more complex address or a full url?
}

