#pragma once

#include "common.hpp"
#include "frame.hpp"
#include "input_manager.hpp"
#include <memory>
#include <string>
#include <thread>
#include <atomic>
#include <mutex>
#include <condition_variable>
#include <queue>
#include <functional>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libswscale/swscale.h>
}

namespace obs {

class VideoSource : public std::enable_shared_from_this<VideoSource> {
public:
    using FrameCallback = std::function<void(FramePtr)>;
    using ErrorCallback = std::function<void(const std::string&)>;
    
    VideoSource(SourceId id, const InputConfig& config);
    ~VideoSource();
    
    VideoSource(const VideoSource&) = delete;
    VideoSource& operator=(const VideoSource&) = delete;
    VideoSource(VideoSource&&) = delete;
    VideoSource& operator=(VideoSource&&) = delete;
    
    bool initialize();
    void shutdown();
    
    bool start();
    void stop();
    void pause();
    void resume();
    
    bool isRunning() const { return running_.load(); }
    bool isPaused() const { return paused_.load(); }
    bool isEOS() const { return eos_.load(); }
    
    SourceId getId() const { return id_; }
    const InputConfig& getConfig() const { return config_; }
    
    Resolution getResolution() const { return resolution_; }
    FrameRate getFrameRate() const { return frameRate_; }
    PixelFormat getPixelFormat() const { return pixelFormat_; }
    
    void setFrameCallback(FrameCallback callback) { frameCallback_ = callback; }
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }
    
    FramePtr getNextFrame();
    bool hasFrameAvailable() const;
    size_t getQueueSize() const;
    
    void seek(double timestamp);
    double getCurrentTimestamp() const { return currentTimestamp_.load(); }
    double getDuration() const { return duration_; }
    
private:
    SourceId id_;
    InputConfig config_;
    
    Resolution resolution_;
    FrameRate frameRate_;
    PixelFormat pixelFormat_;
    double duration_ = 0.0;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> paused_{false};
    std::atomic<bool> eos_{false};
    std::atomic<double> currentTimestamp_{0.0};
    
    std::thread decodeThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
    std::queue<FramePtr> frameQueue_;
    size_t maxQueueSize_ = 30;
    
    FrameCallback frameCallback_;
    ErrorCallback errorCallback_;
    
    AVFormatContext* formatContext_ = nullptr;
    AVCodecContext* codecContext_ = nullptr;
    const AVCodec* codec_ = nullptr;
    SwsContext* swsContext_ = nullptr;
    
    int videoStreamIndex_ = -1;
    int audioStreamIndex_ = -1;
    
    FramePool framePool_;
    
    bool openInput();
    void closeInput();
    
    bool initializeDecoder();
    
    void decodeLoop();
    bool decodeFrame(FramePtr frame);
    
    void convertFrame(AVFrame* avFrame, FramePtr frame);
    
    void handleDecodingError(const std::string& error);
    
    bool isCaptureDevice() const {
        return config_.type == InputType::DeviceCapture ||
               config_.type == InputType::ScreenCapture ||
               config_.type == InputType::WindowCapture;
    }
    
    bool isNetworkStream() const {
        return config_.type == InputType::IPAddress;
    }
    
    bool isFile() const {
        return config_.type == InputType::MediaFile;
    }
};

} // namespace obs