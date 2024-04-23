#include "LoginPannel.h"
#include <QMessageBox>
#include <QStyle>
#include <QMouseEvent>

LoginPannel::LoginPannel(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginPannelClass())
{
    ui->setupUi(this);
    setWindowFlag(Qt::FramelessWindowHint);

    ui->closeButton->setIcon(QApplication::style()->standardIcon(QStyle::SP_TitleBarCloseButton));
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

void LoginPannel::mousePressEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = event->globalPos() - this->pos();
        event->accept();
    }
    else {
        QDialog::mousePressEvent(event);
    }
}

void LoginPannel::mouseMoveEvent(QMouseEvent* event) {
    if (event->buttons() & Qt::LeftButton) {
        if (!m_dragPosition.isNull()) {
            this->move(event->globalPos() - m_dragPosition);
            event->accept();
        }
    }
    else {
        QDialog::mouseMoveEvent(event);
    }
}

void LoginPannel::mouseReleaseEvent(QMouseEvent* event) {
    if (event->button() == Qt::LeftButton) {
        m_dragPosition = QPoint();
        event->accept();
    }
    else {
        QDialog::mouseReleaseEvent(event);
    }
}
