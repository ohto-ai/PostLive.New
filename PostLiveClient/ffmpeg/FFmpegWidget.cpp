#include "FFmpegVideo.h"
#include "FFmpegWidget.h"
#include <QPainter>
#include <QTimer>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDateTime>
#include <QStatusBar>
#include <QMainWindow>
#include "ffmpeg/FFmpegInputDevice.h"

extern "C" {
#include <libavformat/avformat.h>
}

FFmpegWidget::FFmpegWidget(QWidget* parent)
    : FileDropZone(parent) {
    ffmpegVideo = new FFmpegVideo(this);
#ifdef _DEBUG
    option_displayFPS = true;
#endif

    // 设置黑色背景
    QPalette pal = palette();
    pal.setColor(QPalette::Background, Qt::black);
    setAutoFillBackground(true);
    setPalette(pal);

    QTimer::singleShot(0, this, [this] {
        auto widget = parentWidget();
        QMainWindow* mainWindow = nullptr;
        while (widget) {
            mainWindow = qobject_cast<QMainWindow*>(widget);
            if (mainWindow) {
                break;
            }
            widget = widget->parentWidget();
        }
        if (mainWindow) {
            parentStatusBar = mainWindow->statusBar();
        }
        if (frameImage.isNull()) {
            clearFrame();
        }
        });

    connect(ffmpegVideo, &FFmpegVideo::streamReady, this, [this](const AVStream* stream) {
        if (stream->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
            videoStream = stream;
            if (option_changeSizeFromVideo) {
                setFixedSize(videoStream->codecpar->width, videoStream->codecpar->height);
            }
        }
        }, Qt::QueuedConnection);

    connect(ffmpegVideo, &FFmpegVideo::postError, this, [this](int ec, const QString& error) {
        ffmpegVideoErrorCode = ec;
        ffmpegVideoErrorString = error;
        clearFrame();
        updateStatusBar();
        update();
        QTimer::singleShot(3000, this, [this] {
            ffmpegVideoErrorCode = 0;
            ffmpegVideoErrorString.clear();
            update();
            });
        }, Qt::QueuedConnection);

    connect(ffmpegVideo, &FFmpegVideo::frameReady, this, &FFmpegWidget::setFrame, Qt::QueuedConnection);

    connect(this, &FileDropZone::onFileDropped, this, [this](const QStringList& paths) {
        if (paths.size() > 0) {
            static FFmpegInputDevice device;
            device = FFmpegInputDevice::make_local_file_device(paths[0].toStdString());
            stop();
            setInputDevice(&device);
            play();
        }
        });

    connect(&movie, &QMovie::frameChanged, this, [this]() {
        frameImage = movie.currentImage().scaled(width(), height(), Qt::KeepAspectRatio);
        update();
        });
}

FFmpegWidget::~FFmpegWidget() {
    delete ffmpegVideo;
}

bool FFmpegWidget::setInputDevice(const FFmpegInputDevice* device) {
    return ffmpegVideo->setInputDevice(device);
}

void FFmpegWidget::play() {
    ffmpegVideo->start();
}

void FFmpegWidget::stop() {
    ffmpegVideo->stop();
    clearFrame();
}

void FFmpegWidget::addOption(const QString& key, const QString& value) {
    ffmpegVideo->addOption(key, value);
}

void FFmpegWidget::setFrame(const QImage& frame) {
    // 计算发送FPS
    static int frameCount = 0;
    static qint64 lastTime = QDateTime::currentMSecsSinceEpoch();
    constexpr size_t CountDuration = 1000;
    frameCount++;
    qint64 currentTime = QDateTime::currentMSecsSinceEpoch();
    if (currentTime - lastTime >= CountDuration) {
        decodeFPS = static_cast<decltype(decodeFPS)>(frameCount * CountDuration) / (currentTime - lastTime);
        frameCount = 0;
        lastTime = currentTime;
    }

    frameAvailable = true;

    if (option_changeSizeFromVideo) {
        frameImage = frame;
    }
    else {
        frameImage = frame.scaled(width(), height(), Qt::KeepAspectRatio);
    }

    movie.stop();
    updateStatusBar();
    update();
}

void FFmpegWidget::clearFrame() {
    frameAvailable = false;
    movie.start();
    updateStatusBar();
    update();
}

void FFmpegWidget::updateStatusBar() {
    if (parentStatusBar) {
        // FPS and error
        QString status;

        if (ffmpegVideo != nullptr && frameAvailable) {
            if (option_displayFPS) {
                status += QString::asprintf("Decode FPS: %4.1f ", decodeFPS);
            }
            if (videoStream != nullptr) {
                status += QString(" | Size: %1x%2").arg(frameImage.width()).arg(frameImage.height());
            }
            if (ffmpegVideo->inputDevice != nullptr) {
                constexpr size_t MaxUrlLength = 50;
                constexpr size_t RightReserved = 8;
                auto url = QString::fromStdString(ffmpegVideo->inputDevice->streamUrls[AVMEDIA_TYPE_VIDEO]);
                if (url.size() > MaxUrlLength) {
                    status += QString(" | URL: %1...%2").arg(url.left(MaxUrlLength - RightReserved - 3), url.right(RightReserved));
                }
                else {
                    status += QString(" | URL: %1").arg(url);
                }
            }
        }
        if (ffmpegVideoErrorCode != 0) {
            status += QString(" | Error: %1").arg(ffmpegVideoErrorString);
        }
        parentStatusBar->showMessage(status);
    }
}

void FFmpegWidget::paintEvent(QPaintEvent* event) {
    QPainter painter(this);

    int x = (width() - frameImage.width()) / 2;
    int y = (height() - frameImage.height()) / 2;
    painter.drawImage(x, y, frameImage);

    if (!frameAvailable) {
        QFont font = painter.font();
        font.setPointSize(20);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignTop | Qt::AlignHCenter, "OhtoAi Player");
    }
}
