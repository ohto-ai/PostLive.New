#include "FFmpegVideo.h"
#include "FFmpegWidget.h"
#include <QPainter>
#include <QTimer>
#include <QDragEnterEvent>
#include <QMimeData>
#include <QDateTime>
#include <QStatusBar>
#include <QMainWindow>

extern "C" {
#include <libavformat/avformat.h>
}

FFmpegWidget::FFmpegWidget(QWidget *parent)
    : FileDropZone(parent)
{
    ffmpegVideo = new FFmpegVideo(this);
#ifdef _DEBUG
    option_displayFPS= true;
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

    connect(ffmpegVideo, &FFmpegVideo::error, this, [this](int ec, const QString& error) {
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
            setUrl(paths[0]);
            play();
        }
        });
}

FFmpegWidget::~FFmpegWidget() {
    delete ffmpegVideo;
}

void FFmpegWidget::setUrl(const QString & url) {
    ffmpegVideo->setUrl(url);
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

void FFmpegWidget::setFrame(const QImage & frame) {
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

    frameImage = frame;
    updateStatusBar();
    update();
}

void FFmpegWidget::clearFrame() {
    frameImage = QImage();
    updateStatusBar();
    update();
}

void FFmpegWidget::updateStatusBar() {

    if (parentStatusBar) {
        // FPS and error
        QString status;
        if (option_displayFPS) {
            status += QString::asprintf("Decode FPS: %4.1f ", decodeFPS);
        }

        if (ffmpegVideo != nullptr && ffmpegVideo->isRunning()) {
            if (videoStream != nullptr) {
                status += QString(" | Size: %1x%2").arg(frameImage.width()).arg(frameImage.height());
            }
            if (!ffmpegVideo->url.isEmpty()) {
                constexpr size_t MaxUrlLength = 50;
                constexpr size_t RightReserved = 8;
                if (ffmpegVideo->url.size() > MaxUrlLength) {
                    status += QString(" | URL: %1...%2").arg(ffmpegVideo->url.left(MaxUrlLength - RightReserved - 3), ffmpegVideo->url.right(RightReserved));
                }
                else {
                    status += QString(" | URL: %1").arg(ffmpegVideo->url);
                }
            }
        }
        if (ffmpegVideoErrorCode != 0) {
            status += QString(" | Error: %1").arg(ffmpegVideoErrorString);
        }
        parentStatusBar->showMessage(status);
    }
}

void FFmpegWidget::paintEvent(QPaintEvent * event) {
    QPainter painter(this);

    if (frameImage.isNull()) {
        // 中央绘制Logo: OhtoAi Player
        QFont font = painter.font();
        font.setPointSize(20);
        painter.setFont(font);
        painter.setPen(Qt::white);
        painter.drawText(rect(), Qt::AlignCenter, "OhtoAi Player");
        return;
    }

    // 居中绘制
    if (option_changeSizeFromVideo) {
        int x = (width() - frameImage.width()) / 2;
        int y = (height() - frameImage.height()) / 2;
        painter.drawImage(x, y, frameImage);
    }
    else {
        // 按窗口大小 保持比例绘制
        auto frame = frameImage.scaled(width(), height(), Qt::KeepAspectRatio);
        int x = (width() - frame.width()) / 2;
        int y = (height() - frame.height()) / 2;
        painter.drawImage(x, y, frame);
    }
}
