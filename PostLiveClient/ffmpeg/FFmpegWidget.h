#pragma once

#include <QWidget>
#include <QImage>
#include <memory>

class FFmpegWidget : public QWidget
{
    Q_OBJECT

public:
    FFmpegWidget(QWidget *parent);
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
    void dragEnterEvent(QDragEnterEvent* event) override;
    //void dragMoveEvent(QDragMoveEvent* event) override;
    void dropEvent(QDropEvent* event) override;

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
};
