#pragma once
#include <vector>
#include <string>
#include <mutex>
#include <array>

struct FFmpegInputDevice {
    constexpr static size_t MediaTypeMax = 5;
    const struct AVInputFormat* input_format = nullptr;
    std::string device_name{};
    std::string device_description{};
    std::vector<enum AVMediaType> media_types{};
    std::array<std::string, MediaTypeMax> streamUrls{};

    static void updateInputDeviceList();
    static void updateInputDeviceList(const std::vector<std::string>& formats);
    static std::vector<const FFmpegInputDevice*> filterDevices(const std::vector<enum AVMediaType>& media_types);
    static std::vector<const FFmpegInputDevice*> filterVideoDevices();
    static std::vector<const FFmpegInputDevice*> filterAudioDevices();
    static std::vector<const FFmpegInputDevice*> allDevices();

    static FFmpegInputDevice make_gdi_device(std::string title = "");

private:
    static inline std::vector<FFmpegInputDevice> devices;
    static inline std::mutex devicesMutex;
    static inline std::once_flag registerFlag;
};
