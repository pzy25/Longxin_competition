#include "dialog1.h"
#include "ui_dialog1.h"

Dialog1::Dialog1(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::Dialog1)
{
    ui->setupUi(this);
}

Dialog1::~Dialog1()
{
    delete ui;
}

void Dialog1::on_assure_clicked()
{
    bool flagsuccess;
    if(  (ui->username->text()=="LPL")&&(ui->password->text()=="321" )  )
    {
        flagsuccess = 1;
        emit assure1(flagsuccess);
    }
    else{
        flagsuccess = 0;
        emit assure1(flagsuccess);
    }
}

void Dialog1::on_quxiao_clicked()
{
    ui->password->clear();
    ui->username->clear();

}
