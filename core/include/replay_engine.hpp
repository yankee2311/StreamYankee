#pragma once

#include "common.hpp"
#include "frame.hpp"
#include <memory>
#include <string>
#include <vector>
#include <deque>
#include <mutex>
#include <thread>
#include <atomic>
#include <condition_variable>
#include <functional>

namespace obs {

struct ReplayConfig {
    int durationSeconds = 300;
    Resolution resolution{1280, 720};
    FrameRate frameRate{30, 1};
    PixelFormat pixelFormat = PixelFormat::YUV420P;
    
    std::string storagePath = "/tmp/obs_replay_buffer";
    bool useMemoryMapping = true;
    size_t maxMemoryMB = 1024;
    
    int speedLevels[10] = {25, 33, 50, 67, 75, 100, 125, 150, 200, 300};
};

struct ReplaySegment {
    std::string id;
    double startTime = 0.0;
    double endTime = 0.0;
    double duration = 0.0;
    
    std::vector<FramePtr> frames;
    
    std::string toString() const {
        return "ReplaySegment[" + id + "] " + 
               std::to_string(startTime) + "s - " + 
               std::to_string(endTime) + "s";
    }
};

class MappedCircularBuffer {
public:
    MappedCircularBuffer(size_t maxSize, const std::string& basePath);
    ~MappedCircularBuffer();
    
    bool initialize();
    void shutdown();
    
    void append(const uint8_t* data, size_t size);
    bool readFrame(double timestamp, uint8_t* buffer, size_t* size);
    
    size_t getCapacity() const { return capacity_; }
    size_t getUsedSize() const { return usedSize_; }
    size_t getAvailableSize() const { return capacity_ - usedSize_; }
    
    void clear();
    
private:
    std::string basePath_;
    int fd_ = -1;
    uint8_t* mappedData_ = nullptr;
    size_t capacity_;
    size_t usedSize_ = 0;
    size_t writeOffset_ = 0;
    
    std::mutex mutex_;
    bool initialized_ = false;
    
    std::string getFilePath() const;
    bool createFile();
    bool mapFile();
    void unmapFile();
};

class ReplayEngine {
public:
    using SegmentCallback = std::function<void(std::shared_ptr<ReplaySegment>)>;
    
    ReplayEngine(const ReplayConfig& config);
    ~ReplayEngine();
    
    ReplayEngine(const ReplayEngine&) = delete;
    ReplayEngine& operator=(const ReplayEngine&) = delete;
    
    bool initialize();
    void shutdown();
    
    void startBuffering();
    void stopBuffering();
    bool isBuffering() const { return buffering_.load(); }
    
    void pushFrame(FramePtr frame);
    
    std::shared_ptr<ReplaySegment> extractSegment(double startTime, double endTime);
    std::shared_ptr<ReplaySegment> extractLastSeconds(int seconds);
    
    void playSegment(std::shared_ptr<ReplaySegment> segment, double speed = 1.0);
    void pause();
    void resume();
    void stop();
    
    bool isPlaying() const { return playing_.load(); }
    bool isPaused() const { return paused_.load(); }
    
    void setSpeed(double speed);
    double getSpeed() const { return currentSpeed_.load(); }
    
    double getCurrentTime() const { return currentTime_.load(); }
    double getDuration() const;
    
    void seek(double timestamp);
    
    void setSegmentCallback(SegmentCallback callback) {
        segmentCallback_ = callback;
    }
    
    size_t getBufferSize() const;
    int getBufferDuration() const { return config_.durationSeconds; }
    
    void clearBuffer();
    
private:
    ReplayConfig config_;
    
    std::atomic<bool> buffering_{false};
    std::atomic<bool> playing_{false};
    std::atomic<bool> paused_{false};
    
    std::atomic<double> currentSpeed_{1.0};
    std::atomic<double> currentTime_{0.0};
    
    std::deque<FramePtr> frameBuffer_;
    std::deque<double> timestamps_;
    std::mutex bufferMutex_;
    
    size_t maxFrames_;
    
    std::unique_ptr<MappedCircularBuffer> circularBuffer_;
    
    std::thread bufferThread_;
    std::thread playThread_;
    std::mutex mutex_;
    std::condition_variable cv_;
    
    FramePool framePool_;
    
    SegmentCallback segmentCallback_;
    
    std::shared_ptr<ReplaySegment> currentSegment_;
    
    void bufferLoop();
    void playLoop();
    
    void dropOldestFrames();
    void saveFrameToBuffer(FramePtr frame);
    bool loadFrameFromBuffer(double timestamp, FramePtr frame);
};

} // namespace obs