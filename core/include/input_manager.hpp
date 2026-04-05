#pragma once

#include "common.hpp"
#include "frame.hpp"
#include <memory>
#include <string>
#include <vector>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>

namespace obs {

struct InputDevice {
    std::string id;
    std::string name;
    InputType type;
    std::string path;
    
    bool isVideo = true;
    bool isAudio = true;
    
    std::string toString() const {
        return name + " (" + id + ")";
    }
};

struct InputConfig {
    InputType type = InputType::MediaFile;
    std::string path = "";
    std::string device = "";
    
    Resolution resolution{1920, 1080};
    FrameRate frameRate{30, 1};
    PixelFormat pixelFormat = PixelFormat::RGBA32;
    
    bool enableAudio = true;
    bool enableVideo = true;
    
    int bufferSize = 30;
    
    std::string url = "";
    int timeout = 5000;
    
    std::string toString() const {
        std::string result = "InputConfig[type=";
        switch (type) {
            case InputType::DeviceCapture: result += "Device"; break;
            case InputType::ScreenCapture: result += "Screen"; break;
            case InputType::WindowCapture: result += "Window"; break;
            case InputType::IPAddress: result += "IP"; break;
            case InputType::MediaFile: result += "File"; break;
        }
        result += ", path=" + path + "]";
        return result;
    }
};

class VideoSource;

class InputManager {
public:
    InputManager();
    ~InputManager();
    
    InputManager(const InputManager&) = delete;
    InputManager& operator=(const InputManager&) = delete;
    
    std::vector<InputDevice> enumerateDevices();
    
    SourceId createInput(const InputConfig& config);
    void destroyInput(SourceId id);
    
    std::shared_ptr<VideoSource> getInput(SourceId id) const;
    bool hasInput(SourceId id) const;
    
    std::vector<SourceId> getActiveInputs() const;
    size_t getActiveInputCount() const;
    
    void startAll();
    void stopAll();
    
    void setOnFrameCallback(SourceId id, Callback<FramePtr> callback);
    void setOnErrorCallback(SourceId id, ErrorCallback callback);
    
    void pauseInput(SourceId id);
    void resumeInput(SourceId id);
    bool isInputPaused(SourceId id) const;
    bool isInputActive(SourceId id) const;
    
private:
    mutable std::mutex mutex_;
    HashMap<SourceId, std::shared_ptr<VideoSource>> inputs_;
    SourceId nextSourceId_ = 1;
    
    std::vector<InputDevice> detectWebcams();
    std::vector<InputDevice> detectCaptureCards();
    std::vector<InputDevice> detectScreens();
};

} // namespace obs