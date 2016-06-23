#include "logindialog.h"
#include "ui_logindialog.h"
#include <QPushButton>

LoginDialog::LoginDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::LoginDialog)
{
    ui->setupUi(this);
    updateOKButtonState();
}

LoginDialog::~LoginDialog()
{
    delete ui;
}

void LoginDialog::showEvent(QShowEvent *)
{
    ui->lineEdit->setFocus();
}

void LoginDialog::updateOKButtonState()
{
    bool UserNameEmpty = ui->lineEdit->text().isEmpty();
    bool PasswordEmpty = ui->lineEdit_2->text().isEmpty();
    QPushButton *okButton = ui->buttonBox->button(QDialogButtonBox::Ok);
    okButton->setDisabled(UserNameEmpty || PasswordEmpty);
}

void LoginDialog::setUserName(const QString &uname) {
ui->lineEdit->setText(uname);
}

void LoginDialog::setPassword(const QString &pword) {
ui->lineEdit_2->setText(pword);
}

QString LoginDialog::userName() const
{ return ui->lineEdit->text(); }

QString LoginDialog::password() const
{ return ui->lineEdit_2->text(); }
