#pragma once

#include "common.hpp"
#include <string>
#include <vector>

namespace obs {

class HardwareDetector {
public:
    struct Capabilities {
        bool hasNVENC = false;
        bool hasQuickSync = false;
        bool hasVCE = false;
        bool hasAV1 = false;
        
        int nvidiaGPUs = 0;
        int amdGPUs = 0;
        int intelGPUs = 0;
        
        int cpuCores = 0;
        int cpuThreads = 0;
        std::string cpuVendor;
        std::string cpuModel;
        
        size_t totalRAM = 0;
        size_t availableRAM = 0;
        
        bool supportsDirectShow = false;
        bool supportsV4L2 = false;
        bool supportsAVFoundation = false;
        
        std::vector<std::string> videoDevices;
        std::vector<std::string> audioDevices;
        
        int recommendedEncoder = 0;
        int recommendedResolution = 1080;
        int recommendedFPS = 30;
        int recommendedBitrate = 2500;
        
        std::string toString() const {
            std::string result = "Hardware Capabilities:\n";
            result += "  NVENC: " + std::string(hasNVENC ? "Yes" : "No") + "\n";
            result += "  QuickSync: " + std::string(hasQuickSync ? "Yes" : "No") + "\n";
            result += "  VCE: " + std::string(hasVCE ? "Yes" : "No") + "\n";
            result += "  CPU Cores: " + std::to_string(cpuCores) + "\n";
            result += "  CPU Threads: " + std::to_string(cpuThreads) + "\n";
            result += "  RAM: " + std::to_string(totalRAM / (1024 * 1024)) + " MB\n";
            result += "  Recommended Resolution: " + std::to_string(recommendedResolution) + "p\n";
            result += "  Recommended FPS: " + std::to_string(recommendedFPS) + "\n";
            result += "  Recommended Bitrate: " + std::to_string(recommendedBitrate) + " kbps\n";
            return result;
        }
    };
    
    static HardwareDetector& instance();
    
    Capabilities detect();
    
    static EncoderType selectBestEncoder(const Capabilities& caps, EncoderType preferred = EncoderType::x264);
    
    static bool testEncoderSupport(EncoderType encoder);
    
    static std::string getEncoderName(EncoderType encoder);
    
    static std::vector<std::string> listVideoDevices();
    static std::vector<std::string> listAudioDevices();
    
private:
    HardwareDetector() = default;
    ~HardwareDetector() = default;
    
    HardwareDetector(const HardwareDetector&) = delete;
    HardwareDetector& operator=(const HardwareDetector&) = delete;
    
    void detectNVIDIA(Capabilities& caps);
    void detectAMD(Capabilities& caps);
    void detectIntel(Capabilities& caps);
    
    void detectCPU(Capabilities& caps);
    void detectRAM(Capabilities& caps);
    
    void adjustRecommendations(Capabilities& caps);
    
    bool checkNVENCSupport();
    bool checkQuickSyncSupport();
    bool checkVCESupport();
    
#if defined(_WIN32)
    void detectDirectShow(Capabilities& caps);
    void detectWindowsDevices(Capabilities& caps);
#elif defined(__linux__)
    void detectV4L2(Capabilities& caps);
    void detectLinuxDevices(Capabilities& caps);
#elif defined(__APPLE__)
    void detectAVFoundation(Capabilities& caps);
    void detectMacOSDevices(Capabilities& caps);
#endif
};

} // namespace obs