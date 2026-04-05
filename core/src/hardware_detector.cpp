#include "hardware_detector.hpp"
#include "logger.hpp"
#include <thread>
#include <sstream>

#if defined(_WIN32)
#include <windows.h>
#elif defined(__linux__)
#include <unistd.h>
#include <sys/sysinfo.h>
#elif defined(__APPLE__)
#include <sys/sysctl.h>
#include <unistd.h>
#endif

namespace obs {

HardwareDetector& HardwareDetector::instance() {
    static HardwareDetector instance;
    return instance;
}

HardwareDetector::Capabilities HardwareDetector::detect() {
    LOG_INFO("Detecting hardware capabilities...");
    
    Capabilities caps;
    
    detectCPU(caps);
    detectRAM(caps);
    
    detectNVIDIA(caps);
    detectAMD(caps);
    detectIntel(caps);
    
#if defined(_WIN32)
    detectDirectShow(caps);
    detectWindowsDevices(caps);
#elif defined(__linux__)
    detectV4L2(caps);
    detectLinuxDevices(caps);
#elif defined(__APPLE__)
    detectAVFoundation(caps);
    detectMacOSDevices(caps);
#endif
    
    adjustRecommendations(caps);
    
    LOG_INFO(caps.toString());
    
    return caps;
}

void HardwareDetector::detectCPU(Capabilities& caps) {
    caps.cpuThreads = std::thread::hardware_concurrency();
    caps.cpuCores = caps.cpuThreads;
    
#if defined(_WIN32)
    SYSTEM_INFO sysInfo;
    GetSystemInfo(&sysInfo);
    caps.cpuCores = sysInfo.dwNumberOfProcessors / 2;
#elif defined(__linux__)
    caps.cpuCores = sysconf(_SC_NPROCESSORS_ONLN) / 2;
#elif defined(__APPLE__)
    int cores = 0;
    size_t size = sizeof(cores);
    sysctlbyname("hw.physicalcpu", &cores, &size, nullptr, 0);
    caps.cpuCores = cores;
#endif
    
    LOG_INFO("Detected CPU: " + std::to_string(caps.cpuCores) + " cores, " + 
             std::to_string(caps.cpuThreads) + " threads");
}

void HardwareDetector::detectRAM(Capabilities& caps) {
#if defined(_WIN32)
    MEMORYSTATUSEX status;
    status.dwLength = sizeof(status);
    GlobalMemoryStatusEx(&status);
    caps.totalRAM = status.ullTotalPhys;
    caps.availableRAM = status.ullAvailPhys;
#elif defined(__linux__)
    struct sysinfo info;
    sysinfo(&info);
    caps.totalRAM = info.totalram;
    caps.availableRAM = info.freeram;
#elif defined(__APPLE__)
    int64_t memory;
    size_t size = sizeof(memory);
    sysctlbyname("hw.memsize", &memory, &size, nullptr, 0);
    caps.totalRAM = memory;
    vm_statistics64_data_t vmstat;
    mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
    host_statistics64(mach_host_self(), HOST_VM_INFO64, (host_info64_t)&vmstat, &count);
    caps.availableRAM = vmstat.free_count * vm_page_size;
#else
    caps.totalRAM = 8ULL * 1024 * 1024 * 1024;
    caps.availableRAM = caps.totalRAM / 2;
#endif
    
    LOG_INFO("Detected RAM: " + std::to_string(caps.totalRAM / (1024 * 1024)) + " MB total, " +
             std::to_string(caps.availableRAM / (1024 * 1024)) + " MB available");
}

void HardwareDetector::detectNVIDIA(Capabilities& caps) {
#if defined(_WIN32)
    caps.hasNVENC = checkNVENCSupport();
    if (caps.hasNVENC) {
        caps.nvidiaGPUs = 1;
        LOG_INFO("NVIDIA GPU with NVENC detected");
    }
#else
    caps.hasNVENC = false;
    caps.nvidiaGPUs = 0;
#endif
}

void HardwareDetector::detectAMD(Capabilities& caps) {
    caps.hasVCE = checkVCESupport();
    if (caps.hasVCE) {
        caps.amdGPUs = 1;
        LOG_INFO("AMD GPU with VCE detected");
    }
}

void HardwareDetector::detectIntel(Capabilities& caps) {
    caps.hasQuickSync = checkQuickSyncSupport();
    if (caps.hasQuickSync) {
        caps.intelGPUs = 1;
        LOG_INFO("Intel GPU with QuickSync detected");
    }
}

bool HardwareDetector::checkNVENCSupport() {
#if defined(_WIN32)
    return false; // TODO: Implement NVENC detection
#else
    return false;
#endif
}

bool HardwareDetector::checkQuickSyncSupport() {
#if defined(_WIN32)
    return false; // TODO: Implement QuickSync detection
#else
    return false;
#endif
}

bool HardwareDetector::checkVCESupport() {
    return false; // TODO: Implement VCE detection
}

void HardwareDetector::adjustRecommendations(Capabilities& caps) {
    const size_t minRAMFor1080p = 4ULL * 1024 * 1024 * 1024;
    const size_t minRAMFor720p = 2ULL * 1024 * 1024 * 1024;
    
    caps.recommendedEncoder = static_cast<int>(selectBestEncoder(caps));
    
    if (caps.totalRAM >= minRAMFor1080p && caps.cpuCores >= 4) {
        caps.recommendedResolution = 1080;
        caps.recommendedFPS = 30;
        caps.recommendedBitrate = 4500;
    } else if (caps.totalRAM >= minRAMFor720p && caps.cpuCores >= 2) {
        caps.recommendedResolution = 720;
        caps.recommendedFPS = 30;
        caps.recommendedBitrate = 2500;
    } else {
        caps.recommendedResolution = 480;
        caps.recommendedFPS = 24;
        caps.recommendedBitrate = 1500;
    }
    
    if (caps.hasNVENC || caps.hasQuickSync || caps.hasVCE) {
        caps.recommendedResolution = 1080;
        caps.recommendedFPS = 60;
        caps.recommendedBitrate = 6000;
    }
}

EncoderType HardwareDetector::selectBestEncoder(const Capabilities& caps, EncoderType preferred) {
    if (preferred != EncoderType::x264) {
        if (preferred == EncoderType::NVENC && caps.hasNVENC) {
            return EncoderType::NVENC;
        }
        if (preferred == EncoderType::QuickSync && caps.hasQuickSync) {
            return EncoderType::QuickSync;
        }
        if (preferred == EncoderType::VCE && caps.hasVCE) {
            return EncoderType::VCE;
        }
    }
    
    if (caps.hasNVENC) return EncoderType::NVENC;
    if (caps.hasQuickSync) return EncoderType::QuickSync;
    if (caps.hasVCE) return EncoderType::VCE;
    
    return EncoderType::x264;
}

bool HardwareDetector::testEncoderSupport(EncoderType encoder) {
    switch (encoder) {
        case EncoderType::NVENC:
            return checkNVENCSupport();
        case EncoderType::QuickSync:
            return checkQuickSyncSupport();
        case EncoderType::VCE:
            return checkVCESupport();
        case EncoderType::x264:
        case EncoderType::AV1:
            return true;
        default:
            return false;
    }
}

std::string HardwareDetector::getEncoderName(EncoderType encoder) {
    switch (encoder) {
        case EncoderType::x264: return "x264";
        case EncoderType::NVENC: return "NVENC";
        case EncoderType::QuickSync: return "QuickSync";
        case EncoderType::VCE: return "VCE";
        case EncoderType::AV1: return "AV1";
        default: return "Unknown";
    }
}

std::vector<std::string> HardwareDetector::listVideoDevices() {
    std::vector<std::string> devices;
    
#if defined(_WIN32)
    // TODO: Implement DirectShow device enumeration
    devices.push_back("dshow:video=Integrated Camera");
#elif defined(__linux__)
    // TODO: Implement V4L2 device enumeration
    devices.push_back("/dev/video0");
#elif defined(__APPLE__)
    // TODO: Implement AVFoundation device enumeration
    devices.push_back("avfoundation:0");
#endif
    
    return devices;
}

std::vector<std::string> HardwareDetector::listAudioDevices() {
    std::vector<std::string> devices;
    
#if defined(_WIN32)
    devices.push_back("dshow:audio=Microphone");
#elif defined(__linux__)
    devices.push_back("pulse");
#elif defined(__APPLE__)
    devices.push_back("avfoundation:0");
#endif
    
    return devices;
}

#if defined(_WIN32)
void HardwareDetector::detectDirectShow(Capabilities& caps) {
    caps.supportsDirectShow = true;
}

void HardwareDetector::detectWindowsDevices(Capabilities& caps) {
    // TODO: Implement actual device enumeration
    caps.videoDevices = {"Integrated Camera", "USB Camera"};
    caps.audioDevices = {"Microphone", "Stereo Mix"};
}
#elif defined(__linux__)
void HardwareDetector::detectV4L2(Capabilities& caps) {
    caps.supportsV4L2 = true;
}

void HardwareDetector::detectLinuxDevices(Capabilities& caps) {
    caps.videoDevices = {"/dev/video0", "/dev/video1"};
    caps.audioDevices = {"pulse", "alsa"};
}
#elif defined(__APPLE__)
void HardwareDetector::detectAVFoundation(Capabilities& caps) {
    caps.supportsAVFoundation = true;
}

void HardwareDetector::detectMacOSDevices(Capabilities& caps) {
    caps.videoDevices = {"FaceTime HD Camera"};
    caps.audioDevices = {"Built-in Microphone"};
}
#endif

} // namespace obs