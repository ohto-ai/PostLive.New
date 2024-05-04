#pragma once
#include <QList>

class CameraHelper {
public:
    QList<class QSize> getSupportedResolutions(const struct FFmpegInputDevice& device);

private:
    // singleton
    CameraHelper() = default;
    CameraHelper(const CameraHelper&) = delete;
    CameraHelper& operator=(const CameraHelper&) = delete;
    CameraHelper(CameraHelper&&) = delete;
    CameraHelper& operator=(CameraHelper&&) = delete;
    ~CameraHelper() = default;
public:
    static CameraHelper& getInstance();
};

