#include "FFmpegVideo.h"
#include "ffmpeg/FFmpegInputDevice.h"
#include <memory>
#include <mutex>
#include <utils/scope_guard.hpp>

//#ifdef _DEBUG
#include <QDebug>
//#endif // _DEBUG
#include <QImage>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

static AVPixelFormat ConvertDeprecatedFormat(enum AVPixelFormat format) {
    switch (format) {
    case AV_PIX_FMT_YUVJ411P:
        return AV_PIX_FMT_YUV411P;
        break;
    case AV_PIX_FMT_YUVJ420P:
        return AV_PIX_FMT_YUV420P;
        break;
    case AV_PIX_FMT_YUVJ422P:
        return AV_PIX_FMT_YUV422P;
        break;
    case AV_PIX_FMT_YUVJ444P:
        return AV_PIX_FMT_YUV444P;
        break;
    case AV_PIX_FMT_YUVJ440P:
        return AV_PIX_FMT_YUV440P;
        break;
    default:
        return format;
        break;
    }
}

FFmpegVideo::FFmpegVideo(QObject* parent) : QThread{ parent } {}

FFmpegVideo::~FFmpegVideo() {
    stop();
}

void FFmpegVideo::setUrl(QString url) {
    std::lock_guard lock{ inputDeviceMutex };
    this->inputUrl = url;
    this->inputDevice = nullptr;
}

bool FFmpegVideo::setInputDevice(const FFmpegInputDevice* device) {
    std::lock_guard lock{ inputDeviceMutex };
    if (device == nullptr || std::find(device->media_types.begin(), device->media_types.end(), AVMEDIA_TYPE_VIDEO) == device->media_types.end()) {
        return false;
    }
    this->inputDevice = device;
    this->inputUrl = device->streamUrls[AVMEDIA_TYPE_VIDEO].c_str();
    return true;
}

void FFmpegVideo::clean() {
    avcodec_free_context(&inputVideoCodecContext);
    avformat_close_input(&inputFormatContext);
    avformat_free_context(inputFormatContext);
    av_dict_free(&inputOptions);
    inputFormatContext = nullptr;
    inputVideoCodecContext = nullptr;
    inputVideoStreamIndex = -1;
    inputVideoCodec = nullptr;
    img_ctx = nullptr;
}

bool FFmpegVideo::open() {
    do {
        if (inputUrl.isEmpty()) {
            break;
        }
        int ec = 0;
        inputFormatContext = avformat_alloc_context();
        if (inputDevice != nullptr && inputDevice->input_format != nullptr && !strcmp(inputDevice->input_format->name, "gdigrab")) {
            av_dict_set(&inputOptions, "probesize", "50000000", 0);
        }

        ec = avformat_open_input(&inputFormatContext, inputUrl.toUtf8().constData(), inputDevice != nullptr ? inputDevice->input_format : nullptr, &inputOptions);
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        //=================================== 获取视频流信息 ===================================//
        ec = avformat_find_stream_info(inputFormatContext, &inputOptions);
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        inputVideoStreamIndex = av_find_best_stream(inputFormatContext, AVMEDIA_TYPE_VIDEO, -1, -1, &inputVideoCodec, 0);
        if (inputVideoStreamIndex < 0) {
            postFFmpegError(inputVideoStreamIndex);
            break;
        }

#ifdef _DEBUG
        //打印输入和输出信息：长度 比特率 流格式等
        av_dump_format(inputFormatContext, 0, inputUrl.toUtf8().constData(), 0);
#endif // _DEBUG

        if (inputVideoCodec == nullptr) {
            postError(-1, "Cannot find decoder.");
            break;
        }

        // allocate codec context
        inputVideoCodecContext = avcodec_alloc_context3(inputVideoCodec);
        if (inputVideoCodecContext == nullptr) {
            postError(-1, "Cannot allocate codec context.");
            break;
        }
        ec = avcodec_parameters_to_context(inputVideoCodecContext, inputFormatContext->streams[inputVideoStreamIndex]->codecpar);
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        // open codec
        ec = avcodec_open2(inputVideoCodecContext, inputVideoCodec, nullptr);
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        // set up sws context
        int w = inputVideoCodecContext->width;
        int h = inputVideoCodecContext->height;
        img_ctx = sws_getContext(
            w, h, ConvertDeprecatedFormat(inputVideoCodecContext->pix_fmt), // 源地址长宽以及数据格式
            w, h, AV_PIX_FMT_RGB32, // 目的地址长宽以及数据格式
            SWS_BICUBIC, nullptr, nullptr, nullptr); // 算法类型  AV_PIX_FMT_YUVJ420P   AV_PIX_FMT_BGR24
        if (img_ctx == nullptr) {
            postError(-1, "Cannot create sws context.");
            break;
        }

        emit streamReady(inputFormatContext->streams[inputVideoStreamIndex]);
        return true;
    } while (false);
    return false;
}

void FFmpegVideo::run() {
    ohtoai::scope_guard sg([&] {
        clean();
        });
    do {
        if (!open()) {
            break;
        }

        int w = inputVideoCodecContext->width;//视频宽度
        int h = inputVideoCodecContext->height;//视频高度

        AVPacket* packet = nullptr;
        uint8_t* out_buffer = nullptr;
        AVFrame* yuvFrame = nullptr;
        AVFrame* rgbFrame = nullptr;

        ohtoai::scope_guard sg([&] {
            av_packet_free(&packet);
            av_frame_free(&yuvFrame);
            av_frame_free(&rgbFrame);
            av_free(out_buffer);
            });

        packet = av_packet_alloc();             // 分配一个packet
        av_new_packet(packet, w * h);   // 调整packet的数据

        int numBytes = av_image_get_buffer_size(AV_PIX_FMT_RGB32, w, h, 1);
        out_buffer = (unsigned char*)av_malloc(numBytes * sizeof(unsigned char));

        yuvFrame = av_frame_alloc();
        rgbFrame = av_frame_alloc();

        av_image_fill_arrays(rgbFrame->data, rgbFrame->linesize, out_buffer, AV_PIX_FMT_RGB32,
            w, h, 1);

        // 根据视频帧率计算每帧间隔时间
        auto target_frame_duration = std::chrono::milliseconds(static_cast<long>(1000 / av_q2d(inputFormatContext->streams[inputVideoStreamIndex]->r_frame_rate)));

        auto start_time = std::chrono::steady_clock::now();

        while (!isInterruptionRequested() && av_read_frame(inputFormatContext, packet) >= 0) { //读取的是一帧视频  数据存入一个AVPacket的结构中
            if (packet->stream_index == inputVideoStreamIndex) {
                if (avcodec_send_packet(inputVideoCodecContext, packet) >= 0) {
                    while (avcodec_receive_frame(inputVideoCodecContext, yuvFrame) >= 0) {
                        sws_scale(img_ctx,
                            yuvFrame->data,
                            yuvFrame->linesize,
                            0,
                            h,
                            rgbFrame->data,
                            rgbFrame->linesize);

                        QImage img(out_buffer, w, h, QImage::Format_RGB32);
                        emit frameReady(img.copy());

                        auto end_time = std::chrono::steady_clock::now();
                        auto elapsed_time = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time);
                        auto sleep_duration = target_frame_duration - elapsed_time;

                        if (sleep_duration.count() > 0) {
                            std::this_thread::sleep_for(sleep_duration);
                        }
                        else {
                            // 如果执行时间超过了目标帧率，可以根据需要执行一些适当的操作
                            // 这里我们简单地输出警告
                            qWarning() << "Frame rate is too low: " << elapsed_time.count() << "ms";
                        }
                        start_time = std::chrono::steady_clock::now();
                    }
                }
            }
            av_packet_unref(packet);//重置pkt的内容
        }
    } while (false);
}

void FFmpegVideo::stop() {
    requestInterruption();
    wait();
}

void FFmpegVideo::addOption(const QString& key, const QString& value) {
    std::lock_guard lock{ inputDeviceMutex };
    av_dict_set(&inputOptions, key.toUtf8().constData(), value.toUtf8().constData(), 0);
}

void FFmpegVideo::postFFmpegError(int error) {
    char err_msg[512];
    av_strerror(error, err_msg, sizeof(err_msg));
    qWarning() << QString::asprintf("FFmpeg error: (%d) %s", error, err_msg);
    emit postError(error, err_msg);
}
