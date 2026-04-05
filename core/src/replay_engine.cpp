#include "replay_engine.hpp"
#include "logger.hpp"

namespace obs {

ReplayEngine::ReplayEngine(const ReplayConfig& config)
    : config_(config)
    , framePool_(config.durationSeconds * 30, config.resolution, config.pixelFormat) {
    maxFrames_ = static_cast<size_t>(config.durationSeconds * config.frameRate.toFloat());
    LOG_INFO("ReplayEngine created with buffer: " + std::to_string(config.durationSeconds) + "s");
}

ReplayEngine::~ReplayEngine() {
    stopBuffering();
}

bool ReplayEngine::initialize() {
    if (config_.useMemoryMapping) {
        circularBuffer_ = std::make_unique<MappedCircularBuffer>(config_.maxMemoryMB * 1024 * 1024, config_.storagePath);
        if (!circularBuffer_->initialize()) {
            LOG_ERROR("Failed to initialize circular buffer");
            return false;
        }
    }
    
    LOG_INFO("ReplayEngine initialized");
    return true;
}

void ReplayEngine::shutdown() {
    stopBuffering();
    if (circularBuffer_) {
        circularBuffer_->shutdown();
    }
    LOG_INFO("ReplayEngine shutdown");
}

void ReplayEngine::startBuffering() {
    if (buffering_.load()) return;
    
    buffering_.store(true);
    bufferThread_ = std::thread(&ReplayEngine::bufferLoop, this);
    
    LOG_INFO("Replay buffering started");
}

void ReplayEngine::stopBuffering() {
    if (!buffering_.load()) return;
    
    buffering_.store(false);
    if (bufferThread_.joinable()) {
        bufferThread_.join();
    }
    
    LOG_INFO("Replay buffering stopped");
}

void ReplayEngine::pushFrame(FramePtr frame) {
    if (!buffering_.load()) return;
    
    std::lock_guard<std::mutex> lock(bufferMutex_);
    
    if (frameBuffer_.size() >= maxFrames_) {
        dropOldestFrames();
    }
    
    frameBuffer_.push_back(frame);
    timestamps_.push_back(frame->pts);
}

std::shared_ptr<ReplaySegment> ReplayEngine::extractSegment(double startTime, double endTime) {
    auto segment = std::make_shared<ReplaySegment>();
    segment->startTime = startTime;
    segment->endTime = endTime;
    segment->duration = endTime - startTime;
    segment->id = std::to_string(startTime) + "_" + std::to_string(endTime);
    
    // TODO: Extract frames from buffer
    
    LOG_INFO("Extracted replay segment: " + segment->id);
    return segment;
}

std::shared_ptr<ReplaySegment> ReplayEngine::extractLastSeconds(int seconds) {
    double endTime = timestamps_.empty() ? 0.0 : timestamps_.back();
    double startTime = std::max(0.0, endTime - seconds);
    return extractSegment(startTime, endTime);
}

void ReplayEngine::playSegment(std::shared_ptr<ReplaySegment> segment, double speed) {
    currentSegment_ = segment;
    currentSpeed_.store(speed);
    playing_.store(true);
    
    playThread_ = std::thread(&ReplayEngine::playLoop, this);
}

void ReplayEngine::pause() {
    paused_.store(true);
}

void ReplayEngine::resume() {
    paused_.store(false);
}

void ReplayEngine::stop() {
    playing_.store(false);
    paused_.store(false);
    if (playThread_.joinable()) {
        playThread_.join();
    }
}

void ReplayEngine::setSpeed(double speed) {
    currentSpeed_.store(std::clamp(speed, 0.25, 3.0));
}

double ReplayEngine::getDuration() const {
    return static_cast<double>(config_.durationSeconds);
}

void ReplayEngine::seek(double timestamp) {
    currentTime_.store(std::clamp(timestamp, 0.0, getDuration()));
}

size_t ReplayEngine::getBufferSize() const {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    return frameBuffer_.size();
}

void ReplayEngine::clearBuffer() {
    std::lock_guard<std::mutex> lock(bufferMutex_);
    frameBuffer_.clear();
    timestamps_.clear();
}

void ReplayEngine::bufferLoop() {
    while (buffering_.load()) {
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
        // Frame buffering happens via pushFrame()
    }
}

void ReplayEngine::playLoop() {
    while (playing_.load() && currentSegment_) {
        if (!paused_.load()) {
            // TODO: Advance playback
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(33));
    }
}

void ReplayEngine::dropOldestFrames() {
    while (frameBuffer_.size() >= maxFrames_) {
        frameBuffer_.pop_front();
        timestamps_.pop_front();
    }
}

// MappedCircularBuffer implementation

MappedCircularBuffer::MappedCircularBuffer(size_t maxSize, const std::string& basePath)
    : basePath_(basePath), capacity_(maxSize) {
}

MappedCircularBuffer::~MappedCircularBuffer() {
    unmapFile();
    if (fd_ != -1) {
        close(fd_);
    }
}

bool MappedCircularBuffer::initialize() {
    return createFile() && mapFile();
}

void MappedCircularBuffer::shutdown() {
    unmapFile();
}

void MappedCircularBuffer::append(const uint8_t* data, size_t size) {
    std::lock_guard<std::mutex> lock(mutex_);
    // TODO: Implement circular buffer append
}

bool MappedCircularBuffer::readFrame(double timestamp, uint8_t* buffer, size_t* size) {
    std::lock_guard<std::mutex> lock(mutex_);
    // TODO: Implement frame read
    return false;
}

void MappedCircularBuffer::clear() {
    std::lock_guard<std::mutex> lock(mutex_);
    usedSize_ = 0;
    writeOffset_ = 0;
}

bool MappedCircularBuffer::createFile() {
    // TODO: Implement file creation
    return true;
}

bool MappedCircularBuffer::mapFile() {
    // TODO: Implement mmap
    return true;
}

void MappedCircularBuffer::unmapFile() {
    if (mappedData_) {
        // TODO: Implement unmap
        mappedData_ = nullptr;
    }
}

std::string MappedCircularBuffer::getFilePath() const {
    return basePath_ + "/replay_buffer.bin";
}

} // namespace obs
