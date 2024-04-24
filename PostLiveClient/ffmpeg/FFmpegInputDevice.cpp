#include "FFmpegInputDevice.h"

extern "C" {
#include <libavdevice/avdevice.h>
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
#include <libavutil/avutil.h>
#include <libavutil/imgutils.h>
}
#include <Windows.h>

void FFmpegInputDevice::updateInputDeviceList() {
#ifdef __linux__
    const std::vector<std::string> inputDeviceFormats = { "v4l2", "alsa" };
#elif defined(_WIN32)
    const std::vector<std::string> inputDeviceFormats = { "dshow" };
#elif defined(__APPLE__)
    const std::vector<std::string> inputDeviceFormats = { "avfoundation" };
#endif
    return updateInputDeviceList(inputDeviceFormats);
}


void FFmpegInputDevice::updateInputDeviceList(const std::vector<std::string>& formats) {

    std::call_once(registerFlag, avdevice_register_all);
    std::lock_guard lock{ devicesMutex };
    devices.clear();
    for (auto& format : formats) {
        auto inputDeviceFormat = av_find_input_format(format.c_str());
        if (inputDeviceFormat == nullptr) {
            continue;
        }
        AVDeviceInfoList* deviceList = nullptr;
        auto ret = avdevice_list_input_sources(inputDeviceFormat, nullptr, nullptr, &deviceList);
        if (ret < 0) {
            continue;
        }
        for (int i = 0; i < deviceList->nb_devices; i++) {
            FFmpegInputDevice device;
            device.input_format = inputDeviceFormat;
            device.device_name = deviceList->devices[i]->device_name;
            device.device_description = deviceList->devices[i]->device_description;
            for (int j = 0; j < deviceList->devices[i]->nb_media_types; j++) {
                auto media_type = deviceList->devices[i]->media_types[j];
                device.media_types.push_back(media_type);
                if (media_type < MediaTypeMax) {
                    if (media_type == AVMEDIA_TYPE_VIDEO) {
                        device.streamUrls[AVMEDIA_TYPE_VIDEO] = "video=" + device.device_name;
                    }
                    else if (deviceList->devices[i]->media_types[j] == AVMEDIA_TYPE_AUDIO) {
                        device.streamUrls[AVMEDIA_TYPE_AUDIO] = "audio=" + device.device_name;
                    }
                }
            }
            devices.push_back(std::move(device));
        }
        avdevice_free_list_devices(&deviceList);
    }
}

std::vector<const FFmpegInputDevice*> FFmpegInputDevice::filterDevices(const std::vector<enum AVMediaType>& media_types) {
    std::lock_guard<std::mutex> lock(devicesMutex);
    std::vector<const FFmpegInputDevice*> result;
    for (auto& device : devices) {
        for (auto& media_type : media_types) {
            if (std::find(device.media_types.begin(), device.media_types.end(), media_type) != device.media_types.end()) {
                result.push_back(&device);
                break;
            }
        }
    }
    return result;
}

std::vector<const FFmpegInputDevice*> FFmpegInputDevice::filterVideoDevices() {
    return filterDevices({ AVMEDIA_TYPE_VIDEO });
}

std::vector<const FFmpegInputDevice*> FFmpegInputDevice::filterAudioDevices() {
    return filterDevices({ AVMEDIA_TYPE_AUDIO });
}

std::vector<const FFmpegInputDevice*> FFmpegInputDevice::allDevices() {
    std::lock_guard<std::mutex> lock(devicesMutex);
    std::vector<const FFmpegInputDevice*> result;
    for (auto& device : devices) {
        result.push_back(&device);
    }
    return result;
}

FFmpegInputDevice FFmpegInputDevice::make_gdi_device(std::string title) {
    static_assert(MediaTypeMax > AVMEDIA_TYPE_VIDEO);
    FFmpegInputDevice device{};
    device.input_format = av_find_input_format("gdigrab");
    if (title.empty()) {
        device.device_name = "desktop";
        device.device_description = "GDI Grab - Desktop";
        device.media_types.push_back(AVMEDIA_TYPE_VIDEO);
        device.streamUrls[AVMEDIA_TYPE_VIDEO] = "desktop";
    }
    else {
        device.device_name = title;
        device.device_description = "GDI Grab - \"" + title + "\"";
        device.media_types.push_back(AVMEDIA_TYPE_VIDEO);
        device.streamUrls[AVMEDIA_TYPE_VIDEO] = "title=" + title;
    }
    return device;
}

