#pragma once

#include "common.hpp"
#include <vector>
#include <memory>
#include <chrono>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavutil/frame.h>
}

namespace obs {

struct Frame {
    FrameId id = 0;
    Timestamp timestamp;
    Duration duration{0};
    
    Resolution resolution;
    FrameRate frameRate;
    PixelFormat pixelFormat = PixelFormat::RGB24;
    
    std::vector<uint8_t> data;
    int linesize[8] = {0};
    
    SourceId sourceId = INVALID_SOURCE_ID;
    SceneId sceneId = INVALID_SCENE_ID;
    
    double pts = 0.0;
    
    Frame() {
        timestamp = std::chrono::steady_clock::now();
    }
    
    Frame(const Resolution& res, PixelFormat fmt)
        : resolution(res), pixelFormat(fmt) {
        timestamp = std::chrono::steady_clock::now();
        allocateBuffer();
    }
    
    Frame(const Frame&) = delete;
    Frame& operator=(const Frame&) = delete;
    
    Frame(Frame&& other) noexcept
        : id(other.id)
        , timestamp(other.timestamp)
        , duration(other.duration)
        , resolution(other.resolution)
        , frameRate(other.frameRate)
        , pixelFormat(other.pixelFormat)
        , data(std::move(other.data))
        , sourceId(other.sourceId)
        , sceneId(other.sceneId)
        , pts(other.pts) {
        for (int i = 0; i < 8; ++i) {
            linesize[i] = other.linesize[i];
        }
    }
    
    Frame& operator=(Frame&& other) noexcept {
        if (this != &other) {
            id = other.id;
            timestamp = other.timestamp;
            duration = other.duration;
            resolution = other.resolution;
            frameRate = other.frameRate;
            pixelFormat = other.pixelFormat;
            data = std::move(other.data);
            sourceId = other.sourceId;
            sceneId = other.sceneId;
            pts = other.pts;
            for (int i = 0; i < 8; ++i) {
                linesize[i] = other.linesize[i];
            }
        }
        return *this;
    }
    
    void allocateBuffer() {
        const size_t size = calculateBufferSize();
        data.resize(size);
        calculateLinesize();
    }
    
    size_t calculateBufferSize() const {
        const int width = resolution.width;
        const int height = resolution.height;
        
        switch (pixelFormat) {
            case PixelFormat::RGB24:
                return static_cast<size_t>(width * height * 3);
            case PixelFormat::RGBA32:
                return static_cast<size_t>(width * height * 4);
            case PixelFormat::YUV420P:
                return static_cast<size_t>(width * height * 3 / 2);
            case PixelFormat::YUV422P:
                return static_cast<size_t>(width * height * 2);
            case PixelFormat::YUV444P:
                return static_cast<size_t>(width * height * 3);
            case PixelFormat::NV12:
            case PixelFormat::NV21:
                return static_cast<size_t>(width * height * 3 / 2);
            default:
                return static_cast<size_t>(width * height * 4);
        }
    }
    
    void calculateLinesize() {
        const int width = resolution.width;
        
        switch (pixelFormat) {
            case PixelFormat::RGB24:
                linesize[0] = width * 3;
                break;
            case PixelFormat::RGBA32:
                linesize[0] = width * 4;
                break;
            case PixelFormat::YUV420P:
                linesize[0] = width;
                linesize[1] = width / 2;
                linesize[2] = width / 2;
                break;
            case PixelFormat::YUV422P:
                linesize[0] = width;
                linesize[1] = width / 2;
                linesize[2] = width / 2;
                break;
            case PixelFormat::YUV444P:
                linesize[0] = width;
                linesize[1] = width;
                linesize[2] = width;
                break;
            case PixelFormat::NV12:
            case PixelFormat::NV21:
                linesize[0] = width;
                linesize[1] = width;
                break;
            default:
                linesize[0] = width * 4;
                break;
        }
    }
    
    static std::shared_ptr<Frame> create() {
        return std::make_shared<Frame>();
    }
    
    static std::shared_ptr<Frame> create(const Resolution& res, PixelFormat fmt) {
        return std::make_shared<Frame>(res, fmt);
    }
};

using FramePtr = std::shared_ptr<Frame>;
using FrameQueue = std::vector<FramePtr>;

class FramePool {
public:
    FramePool(size_t poolSize = 30, const Resolution& res = Resolution{1920, 1080}, 
              PixelFormat fmt = PixelFormat::RGBA32)
        : resolution_(res), pixelFormat_(fmt), maxPoolSize_(poolSize) {
        for (size_t i = 0; i < poolSize; ++i) {
            pool_.push_back(Frame::create(res, fmt));
        }
    }
    
    FramePtr acquire() {
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (!pool_.empty()) {
            FramePtr frame = pool_.back();
            pool_.pop_back();
            return frame;
        }
        
        return Frame::create(resolution_, pixelFormat_);
    }
    
    void release(FramePtr frame) {
        if (!frame) return;
        
        std::lock_guard<std::mutex> lock(mutex_);
        
        if (pool_.size() < maxPoolSize_) {
            frame->data.clear();
            pool_.push_back(frame);
        }
    }
    
    void resize(size_t newSize) {
        std::lock_guard<std::mutex> lock(mutex_);
        
        while (pool_.size() > newSize) {
            pool_.pop_back();
        }
        
        while (pool_.size() < newSize) {
            pool_.push_back(Frame::create(resolution_, pixelFormat_));
        }
        
        maxPoolSize_ = newSize;
    }
    
    size_t size() const {
        std::lock_guard<std::mutex> lock(mutex_);
        return pool_.size();
    }
    
private:
    Resolution resolution_;
    PixelFormat pixelFormat_;
    size_t maxPoolSize_;
    std::vector<FramePtr> pool_;
    mutable std::mutex mutex_;
};

} // namespace obs