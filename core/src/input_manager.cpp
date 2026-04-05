#include "input_manager.hpp"
#include "video_source.hpp"
#include "logger.hpp"
#include <algorithm>

namespace obs {

InputManager::InputManager() {
    LOG_INFO("InputManager initialized");
}

InputManager::~InputManager() {
    stopAll();
    LOG_INFO("InputManager destroyed");
}

std::vector<InputDevice> InputManager::enumerateDevices() {
    std::vector<InputDevice> devices;
    
    auto webcams = detectWebcams();
    devices.insert(devices.end(), webcams.begin(), webcams.end());
    
    auto captureCards = detectCaptureCards();
    devices.insert(devices.end(), captureCards.begin(), captureCards.end());
    
    auto screens = detectScreens();
    devices.insert(devices.end(), screens.begin(), screens.end());
    
    LOG_INFO("Detected " + std::to_string(devices.size()) + " devices");
    return devices;
}

SourceId InputManager::createInput(const InputConfig& config) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto source = std::make_shared<VideoSource>(nextSourceId_, config);
    
    if (!source->initialize()) {
        LOG_ERROR("Failed to initialize input source");
        return INVALID_SOURCE_ID;
    }
    
    inputs_[nextSourceId_] = source;
    LOG_INFO("Created input source: " + std::to_string(nextSourceId_));
    
    return nextSourceId_++;
}

void InputManager::destroyInput(SourceId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    
    auto it = inputs_.find(id);
    if (it != inputs_.end()) {
        it->second->shutdown();
        inputs_.erase(it);
        LOG_INFO("Destroyed input source: " + std::to_string(id));
    }
}

std::shared_ptr<VideoSource> InputManager::getInput(SourceId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    return it != inputs_.end() ? it->second : nullptr;
}

bool InputManager::hasInput(SourceId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return inputs_.find(id) != inputs_.end();
}

std::vector<SourceId> InputManager::getActiveInputs() const {
    std::lock_guard<std::mutex> lock(mutex_);
    std::vector<SourceId> ids;
    for (const auto& pair : inputs_) {
        ids.push_back(pair.first);
    }
    return ids;
}

size_t InputManager::getActiveInputCount() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return inputs_.size();
}

void InputManager::startAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : inputs_) {
        pair.second->start();
    }
    LOG_INFO("Started all inputs");
}

void InputManager::stopAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    for (auto& pair : inputs_) {
        pair.second->stop();
    }
    LOG_INFO("Stopped all inputs");
}

void InputManager::setOnFrameCallback(SourceId id, Callback<FramePtr> callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    if (it != inputs_.end()) {
        it->second->setFrameCallback(callback);
    }
}

void InputManager::setOnErrorCallback(SourceId id, ErrorCallback callback) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    if (it != inputs_.end()) {
        it->second->setErrorCallback(callback);
    }
}

void InputManager::pauseInput(SourceId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    if (it != inputs_.end()) {
        it->second->pause();
    }
}

void InputManager::resumeInput(SourceId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    if (it != inputs_.end()) {
        it->second->resume();
    }
}

bool InputManager::isInputPaused(SourceId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    return it != inputs_.end() ? it->second->isPaused() : false;
}

bool InputManager::isInputActive(SourceId id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = inputs_.find(id);
    return it != inputs_.end() ? it->second->isRunning() : false;
}

std::vector<InputDevice> InputManager::detectWebcams() {
    std::vector<InputDevice> devices;
    
#if defined(_WIN32)
    devices.push_back({"0", "Integrated Camera", InputType::DeviceCapture, "video=Integrated Camera"});
    devices.push_back({"1", "USB Camera", InputType::DeviceCapture, "video=USB Camera"});
#elif defined(__linux__)
    devices.push_back({"/dev/video0", "Camera 0", InputType::DeviceCapture, "/dev/video0"});
    devices.push_back({"/dev/video1", "Camera 1", InputType::DeviceCapture, "/dev/video1"});
#elif defined(__APPLE__)
    devices.push_back({"0", "FaceTime HD Camera", InputType::DeviceCapture, "avfoundation:0"});
#endif
    
    return devices;
}

std::vector<InputDevice> InputManager::detectCaptureCards() {
    std::vector<InputDevice> devices;
    // TODO: Implement capture card detection
    return devices;
}

std::vector<InputDevice> InputManager::detectScreens() {
    std::vector<InputDevice> devices;
    
#if defined(_WIN32)
    devices.push_back({"screen0", "Screen 1", InputType::ScreenCapture, "screen"});
#elif defined(__linux__)
    devices.push_back({":0.0", "Screen", InputType::ScreenCapture, ":0.0"});
#elif defined(__APPLE__)
    devices.push_back({"0", "Screen", InputType::ScreenCapture, "avfoundation:screen"});
#endif
    
    return devices;
}

} // namespace obs