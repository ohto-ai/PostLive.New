#pragma once

#include "ffmpeg/FFmpegVideo.h"
#include <QImage>
#include <QtWidgets/QMainWindow>
#include "ui_PostLiveClient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class PostLiveClientClass; };
QT_END_NAMESPACE

class PostLiveClient : public QMainWindow
{
    Q_OBJECT

public:
    PostLiveClient(QWidget *parent = nullptr);
    ~PostLiveClient();
private:
    Ui::PostLiveClientClass *ui;
};
