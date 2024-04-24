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

public:
    void mousePressEvent(QMouseEvent* event) override;
    void mouseMoveEvent(QMouseEvent* event) override;
    void mouseReleaseEvent(QMouseEvent* event) override;

private:
    Ui::PostLiveClientClass *ui;
    QPoint m_dragPosition;
    std::vector<struct FFmpegInputDevice> m_gdiDevices;
};
