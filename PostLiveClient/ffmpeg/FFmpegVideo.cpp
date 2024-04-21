#include "FFmpegVideo.h"
#include <memory>
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

FFmpegVideo::FFmpegVideo(QObject* parent) : QThread{ parent } {
    std::call_once(registerFlag_, [this] {
        avdevice_register_all();
        avformat_network_init();
        });
#if defined(Q_OS_WINDOWS)
    inputDeviceFormat = av_find_input_format("dshow");
#elif defined(Q_OS_LINUX)
    inputDeviceFormat = av_find_input_format("video4linux2");
#elif defined(Q_OS_MAC)
    inputDeviceFormat = av_find_input_format("avfoundation");
#endif
}

FFmpegVideo::~FFmpegVideo() {
    //avdevice_free_list_devices(&deviceList);
    stop();
}

AVDeviceInfoList* FFmpegVideo::updateDeviceList() {
    if (deviceList) {
        avdevice_free_list_devices(&deviceList);
    }
    auto ret = avdevice_list_input_sources(inputDeviceFormat, nullptr, nullptr, &deviceList);
    if (ret < 0) {
        return nullptr;
    }
#ifdef _DEBUG
    for (int i = 0; i < deviceList->nb_devices; i++) {
        qDebug() << "Device: " << deviceList->devices[i]->device_name << " - " << deviceList->devices[i]->device_description;
    }
#endif // _DEBUG

    return deviceList;
}

void FFmpegVideo::setUrl(QString url) {
    this->url = url;
}

void FFmpegVideo::clean() {
    avcodec_free_context(&inputVideoCodecContext);
    avformat_close_input(&formatContext);
    avformat_free_context(formatContext);
    formatContext = nullptr;
    inputVideoCodecContext = nullptr;
    inputVideoStreamIndex = -1;
    inputVideoCodecPara = nullptr;
    inputVideoCodec = nullptr;
    img_ctx = nullptr;
}

bool FFmpegVideo::open() {
    do {
        int ec = 0;
        //=========================== 创建AVFormatContext结构体 ===============================//
        //分配一个AVFormatContext，FFMPEG所有的操作都要通过这个AVFormatContext来进行
        formatContext = avformat_alloc_context();
        //==================================== 打开文件 ======================================//
        ec = avformat_open_input(&formatContext, url.toUtf8().constData(), url.startsWith("video=") ? inputDeviceFormat : nullptr, nullptr);
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        //=================================== 获取视频流信息 ===================================//
        ec = avformat_find_stream_info(formatContext, nullptr);
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        //循环查找视频中包含的流信息，直到找到视频类型的流
        //便将其记录下来 保存到videoStreamIndex变量中
        inputVideoStreamIndex = av_find_best_stream(formatContext, AVMEDIA_TYPE_VIDEO, -1, -1, nullptr, 0);

        //如果videoStream为-1 说明没有找到视频流
        if (inputVideoStreamIndex < 0) {
            postFFmpegError(inputVideoStreamIndex);
            break;
        }

#ifdef _DEBUG
        //打印输入和输出信息：长度 比特率 流格式等
        av_dump_format(formatContext, 0, url.toUtf8().constData(), 0);
#endif // _DEBUG

        //=================================  查找解码器 ===================================//
        inputVideoCodecPara = formatContext->streams[inputVideoStreamIndex]->codecpar;
        inputVideoCodec = avcodec_find_decoder(inputVideoCodecPara->codec_id);
        if (inputVideoCodec == nullptr) {
            error(-1, "Cannot find decoder.");
            break;
        }
        //根据解码器参数来创建解码器内容
        inputVideoCodecContext = avcodec_alloc_context3(inputVideoCodec);
        ec = avcodec_parameters_to_context(inputVideoCodecContext, inputVideoCodecPara);
        if (inputVideoCodecContext == nullptr) {
            error(-1, "Cannot allocate codec context.");
            break;
        }
        if (ec < 0) {
            postFFmpegError(ec);
            break;
        }

        //================================  打开解码器 ===================================//
        if ((ec = avcodec_open2(inputVideoCodecContext, inputVideoCodec, NULL)) < 0) { // 具体采用什么解码器ffmpeg经过封装 我们无须知道
            postFFmpegError(ec);
            break;
        }

        int w = inputVideoCodecContext->width;//视频宽度
        int h = inputVideoCodecContext->height;//视频高度

        //================================ 设置数据转换参数 ================================//
        img_ctx = sws_getContext(
            w, h, inputVideoCodecContext->pix_fmt, //源地址长宽以及数据格式
            w, h, AV_PIX_FMT_RGB32,  //目的地址长宽以及数据格式
            SWS_BICUBIC, NULL, NULL, NULL);                       //算法类型  AV_PIX_FMT_YUVJ420P   AV_PIX_FMT_BGR24


        emit streamReady(formatContext->streams[inputVideoStreamIndex]);
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

        //=========================== 分配AVPacket结构体 ===============================//
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
        auto target_frame_duration = std::chrono::milliseconds(static_cast<long>(1000 / av_q2d(formatContext->streams[inputVideoStreamIndex]->r_frame_rate)));

        auto start_time = std::chrono::steady_clock::now();

        while (!isInterruptionRequested() && av_read_frame(formatContext, packet) >= 0) { //读取的是一帧视频  数据存入一个AVPacket的结构中
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

    clean();
}

void FFmpegVideo::postFFmpegError(int error) {
    char err_msg[512];
    av_strerror(error, err_msg, sizeof(err_msg));
    qWarning() << QString::asprintf("FFmpeg error: (%d) %s", error, err_msg);
    this->error(error, err_msg);
}
