#pragma once

#include <QDialog>
#include "ui_LoginPanel.h"

QT_BEGIN_NAMESPACE
namespace Ui { class LoginPanelClass; };
QT_END_NAMESPACE

class LoginPanel : public QDialog
{
    Q_OBJECT

public:
    LoginPanel(QWidget *parent = nullptr);
    ~LoginPanel();

    void login();

private:
    Ui::LoginPanelClass *ui;
};
