#include "LoginPannel.h"
#include <QMessageBox>

LoginPannel::LoginPannel(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginPannelClass())
{
    ui->setupUi(this);
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginPannel::login);
}

LoginPannel::~LoginPannel()
{
    delete ui;
}

void LoginPannel::login() {
    QString username = ui->inputAccount->text();
    QString password = ui->inputPassword->text();
    if (username == "username" && password == "password") {
        accept();
    } else {
        QMessageBox::warning(this, "警告", "用户名或密码错误");
    }
}
