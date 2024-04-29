#pragma once

#include "components/FileDropZone.h"
#include <QWidget>
#include <QImage>
#include <memory>
#include <QMovie>

class FFmpegWidget : public FileDropZone {
    Q_OBJECT

public:
    FFmpegWidget(QWidget* parent = nullptr);
    ~FFmpegWidget();

public:
    void setUrl(const QString& url);
    bool setInputDevice(const struct FFmpegInputDevice* device);
    void play();
    void stop();

protected slots:
    void setFrame(const QImage& frame);
    void clearFrame();
    void updateStatusBar();

public:
    void paintEvent(QPaintEvent* event) override;

private:
    bool option_displayFPS = false;
    bool option_changeSizeFromVideo = false;

private:
    int ffmpegVideoErrorCode = 0;
    QString ffmpegVideoErrorString;

private:
    class QStatusBar* parentStatusBar = nullptr;
    class FFmpegVideo* ffmpegVideo = nullptr;
    const struct AVStream* videoStream = nullptr;
    QImage frameImage;
    qreal decodeFPS = 0.0;

    bool frameAvailable = false;
    QMovie movie{ ":/main/res/splash.gif" };
};
