#include "login.h"
#include "ui_login.h"

Login::Login(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Login)
{
    ui->setupUi(this);
}

Login::~Login()
{
    delete ui;
}

void Login::on_pushButton_clicked()//assure
{
    bool flagsuccess;
    if(  (ui->username->text()=="LPL")&&(ui->password->text()=="321" )  )
    {
        flagsuccess = 1;
        emit assure(flagsuccess);
    }
    else{
        flagsuccess = 0;
        emit assure(flagsuccess);
    }
}

void Login::on_pushButton_2_clicked()
{
    ui->password->clear();
    ui->username->clear();
}
