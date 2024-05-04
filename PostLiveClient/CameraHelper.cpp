#include "utils/scope_guard.hpp"
#include "ffmpeg/FFmpegInputDevice.h"
#include "CameraHelper.h"
#include <QCamera>
#include <QCameraInfo>
#include <QCameraImageCapture>

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}

QList<class QSize> CameraHelper::getSupportedResolutions(const FFmpegInputDevice& device) {
    auto cameraInfos = QCameraInfo::availableCameras();
    auto deviceName = QString::fromStdString(device.device_name);
    qWarning() << device.device_name.c_str();
    auto cameraInfoIter = std::find_if(cameraInfos.begin(), cameraInfos.end(), [&](const QCameraInfo& info) {
        return info.deviceName().replace(":", "_") == deviceName;
        });

    if (cameraInfoIter == cameraInfos.end()) {
        qWarning() << "Camera not found: " << deviceName;
        return {};
    }

    QCamera camera(*cameraInfoIter);
    camera.load();
    QList<QSize> resolutions = camera.supportedViewfinderResolutions();
    camera.unload();
    return resolutions;
}

CameraHelper& CameraHelper::getInstance() {
    static CameraHelper instance;
    return instance;
}
