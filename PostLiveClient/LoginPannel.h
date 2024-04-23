#pragma once

#include <QDialog>
#include "ui_LoginPannel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginPannelClass; };
QT_END_NAMESPACE

class LoginPannel : public QDialog
{
    Q_OBJECT

public:
    LoginPannel(QWidget *parent = nullptr);
    ~LoginPannel();

    void login();

private:
    Ui::LoginPannelClass *ui;
};
