#include "LoginPanel.h"
#include <QMessageBox>
#include <QStyle>
#include <QMouseEvent>

LoginPanel::LoginPanel(QWidget* parent)
    : QDialog(parent)
    , ui(new Ui::LoginPanelClass()) {
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);

    ui->closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));
    connect(ui->loginButton, &QPushButton::clicked, this, &LoginPanel::login);

#ifdef _DEBUG
    ui->inputAccount->setText("username");
    ui->inputPassword->setText("password");
#endif
}

LoginPanel::~LoginPanel() {
    delete ui;
}

void LoginPanel::login() {
    QString username = ui->inputAccount->text();
    QString password = ui->inputPassword->text();
    if (username == "username" && password == "password") {
        accept();
    }
    else {
        QMessageBox::warning(this, "警告", "用户名或密码错误");
    }
}
