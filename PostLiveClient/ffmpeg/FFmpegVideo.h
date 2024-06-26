#pragma once
#include <QThread>

class FFmpegVideo : public QThread {
    Q_OBJECT
        friend class FFmpegAudio;
    friend class FFmpegWidget;
public:
    FFmpegVideo(QObject* parent = nullptr);
    ~FFmpegVideo();

    bool setInputDevice(const struct FFmpegInputDevice* device);
    void run() override;
    void stop();
    void addOption(const QString& key, const QString& value);
private:
    void clean();
    bool open();
signals:
    void postError(int error, const QString& msg);
    void streamReady(const struct AVStream* stream);
    void frameReady(const QImage& image);

private:
    void postFFmpegError(int error);

    const struct FFmpegInputDevice* inputDevice = nullptr;
    struct AVFormatContext* inputFormatContext = nullptr;
    struct AVCodecContext* inputVideoCodecContext = nullptr;
    struct AVDictionary* inputOptions = nullptr;
    const struct AVCodec* inputVideoCodec = nullptr;

    int inputVideoStreamIndex = -1;
    std::mutex inputDeviceMutex;
    struct SwsContext* img_ctx = nullptr;
};
