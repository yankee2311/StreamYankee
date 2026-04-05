#include "output_engine.hpp"
#include "logger.hpp"

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
}

namespace obs {

OutputEngine::OutputEngine() {
    LOG_INFO("OutputEngine created");
}

OutputEngine::~OutputEngine() {
    shutdown();
}

bool OutputEngine::initialize(const HardwareDetector::Capabilities& caps) {
    caps_ = caps;
    LOG_INFO("OutputEngine initialized");
    return true;
}

void OutputEngine::shutdown() {
    stopStreaming();
    stopRecording();
    cleanupFFmpeg();
    LOG_INFO("OutputEngine shutdown");
}

bool OutputEngine::startStreaming(const StreamConfig& config) {
    if (streaming_.load()) return false;
    
    streamConfig_ = config;
    
    if (!initFFmpeg()) {
        LOG_ERROR("Failed to initialize FFmpeg");
        return false;
    }
    
    streaming_.store(true);
    startTime_ = std::chrono::steady_clock::now();
    encodeThread_ = std::thread(&OutputEngine::encodeLoop, this);
    
    LOG_INFO("Started streaming to: " + config.rtmpUrl);
    return true;
}

void OutputEngine::stopStreaming() {
    if (!streaming_.load()) return;
    
    streaming_.store(false);
    if (encodeThread_.joinable()) {
        encodeThread_.join();
    }
    
    cleanupFFmpeg();
    LOG_INFO("Stopped streaming");
}

bool OutputEngine::startRecording(const RecordingConfig& config) {
    if (recording_.load()) return false;
    
    recordingConfig_ = config;
    recording_.store(true);
    
    LOG_INFO("Started recording to: " + config.filePath);
    return true;
}

void OutputEngine::stopRecording() {
    if (!recording_.load()) return;
    
    recording_.store(false);
    LOG_INFO("Stopped recording");
}

void OutputEngine::pushFrame(FramePtr frame) {
    if (!streaming_.load() && !recording_.load()) return;
    
    std::lock_guard<std::mutex> lock(queueMutex_);
    if (frameQueue_.size() < maxQueueSize_) {
        frameQueue_.push(frame);
        queueCV_.notify_one();
    }
}

OutputEngine::Stats OutputEngine::getStats() const {
    return stats_;
}

void OutputEngine::resetStats() {
    stats_.framesEncoded = 0;
    stats_.bytesWritten = 0;
    stats_.currentBitrate = 0.0;
    stats_.averageFps = 0.0;
    stats_.elapsedSeconds = 0.0;
    stats_.droppedFrames = 0;
}

void OutputEngine::encodeLoop() {
    while (streaming_.load() || recording_.load()) {
        FramePtr frame;
        {
            std::unique_lock<std::mutex> lock(queueMutex_);
            queueCV_.wait_for(lock, std::chrono::milliseconds(10), [this] {
                return !frameQueue_.empty() || !streaming_.load();
            });
            
            if (frameQueue_.empty()) continue;
            
            frame = frameQueue_.front();
            frameQueue_.pop();
        }
        
        if (frame) {
            encodeFrame(frame);
            framePool_.release(frame);
        }
    }
}

bool OutputEngine::encodeFrame(FramePtr frame) {
    // TODO: Implement actual encoding
    stats_.framesEncoded++;
    return true;
}

bool OutputEngine::initFFmpeg() {
    avformat_network_init();
    return true;
}

void OutputEngine::cleanupFFmpeg() {
    if (outputFormatContext_) {
        avformat_free_context(outputFormatContext_);
        outputFormatContext_ = nullptr;
    }
}

} // namespace obs
