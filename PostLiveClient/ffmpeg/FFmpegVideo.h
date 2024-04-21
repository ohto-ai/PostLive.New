#pragma once
#include <mutex>
#include <QThread>

struct AVDeviceInfoList;
struct AVInputFormat;
struct AVFormatContext;
struct AVCodecContext;
struct AVCodecParameters;
struct AVCodec;
struct SwsContext;

class FFmpegVideo : public QThread {
    Q_OBJECT
    friend class FFmpegAudio;
    friend class FFmpegWidget;
public:
    FFmpegVideo(QObject* parent = nullptr);
    ~FFmpegVideo();

    static AVDeviceInfoList* updateDeviceList();

    void setUrl(QString url);
    void run() override;
    void stop();
private:
    void clean();
    bool open();
signals:
    void error(int error, const QString& msg);
    void streamReady(const struct AVStream* stream);
    void frameReady(const QImage& image);

private:
    void postFFmpegError(int error);
    static inline std::once_flag registerFlag_;
    static inline AVDeviceInfoList* deviceList = nullptr;
    static inline const AVInputFormat* inputDeviceFormat = nullptr;

    QString url;

    AVFormatContext* formatContext = nullptr;
    AVCodecContext* inputVideoCodecContext = nullptr;
    int inputVideoStreamIndex = -1;
    AVCodecParameters* inputVideoCodecPara = nullptr;
    const AVCodec* inputVideoCodec = nullptr;

    SwsContext* img_ctx = nullptr;
};
