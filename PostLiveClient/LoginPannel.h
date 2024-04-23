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

public:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    Ui::LoginPannelClass *ui;
    QPoint m_dragPosition;
};
