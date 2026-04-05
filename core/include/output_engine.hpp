#pragma once

#include "common.hpp"
#include "frame.hpp"
#include "hardware_detector.hpp"
#include <memory>
#include <string>
#include <functional>
#include <mutex>
#include <thread>
#include <atomic>
#include <queue>
#include <condition_variable>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
}

namespace obs {

struct StreamConfig {
    std::string rtmpUrl;
    std::string streamKey;
    
    int bitrate = 2500;
    int fps = 30;
    Resolution resolution{1280, 720};
    PixelFormat pixelFormat = PixelFormat::YUV420P;
    
    EncoderType encoder = EncoderType::x264;
    std::string preset = "medium";
    std::string profile = "high";
    
    int keyframeInterval = 2;
    
    bool enableAudio = true;
    int audioBitrate = 128;
    int audioSampleRate = 44100;
    int audioChannels = 2;
};

struct RecordingConfig {
    std::string filePath;
    std::string format = "mp4";
    
    int bitrate = 8000;
    int fps = 30;
    Resolution resolution{1920, 1080};
    PixelFormat pixelFormat = PixelFormat::YUV420P;
    
    EncoderType encoder = EncoderType::x264;
    std::string preset = "medium";
    std::string profile = "high";
    
    bool enableAudio = true;
    int audioBitrate = 192;
};

class OutputEngine {
public:
    using ErrorCallback = std::function<void(const std::string&)>;
    using FrameCallback = std::function<void(FramePtr)>;
    
    OutputEngine();
    ~OutputEngine();
    
    OutputEngine(const OutputEngine&) = delete;
    OutputEngine& operator=(const OutputEngine&) = delete;
    
    bool initialize(const HardwareDetector::Capabilities& caps);
    void shutdown();
    
    bool startStreaming(const StreamConfig& config);
    void stopStreaming();
    bool isStreaming() const { return streaming_.load(); }
    
    bool startRecording(const RecordingConfig& config);
    void stopRecording();
    bool isRecording() const { return recording_.load(); }
    
    void pushFrame(FramePtr frame);
    
    void setStreamConfig(const StreamConfig& config) { streamConfig_ = config; }
    StreamConfig getStreamConfig() const { return streamConfig_; }
    
    void setRecordingConfig(const RecordingConfig& config) { recordingConfig_ = config; }
    RecordingConfig getRecordingConfig() const { return recordingConfig_; }
    
    void setErrorCallback(ErrorCallback callback) { errorCallback_ = callback; }
    
    struct Stats {
        uint64_t framesEncoded = 0;
        uint64_t bytesWritten = 0;
        double currentBitrate = 0.0;
        double averageFps = 0.0;
        double elapsedSeconds = 0.0;
        uint64_t droppedFrames = 0;
    };
    
    Stats getStats() const;
    void resetStats();
    
private:
    std::mutex mutex_;
    std::thread encodeThread_;
    std::thread streamThread_;
    
    std::atomic<bool> running_{false};
    std::atomic<bool> streaming_{false};
    std::atomic<bool> recording_{false};
    
    std::queue<FramePtr> frameQueue_;
    std::mutex queueMutex_;
    std::condition_variable queueCV_;
    size_t maxQueueSize_ = 60;
    
    StreamConfig streamConfig_;
    RecordingConfig recordingConfig_;
    ErrorCallback errorCallback_;
    
    AVFormatContext* outputFormatContext_ = nullptr;
    AVCodecContext* videoCodecContext_ = nullptr;
    AVCodecContext* audioCodecContext_ = nullptr;
    AVStream* videoStream_ = nullptr;
    AVStream* audioStream_ = nullptr;
    
    SwsContext* swsContext_ = nullptr;
    
    AVFrame* yuvFrame_ = nullptr;
    AVFrame* audioFrame_ = nullptr;
    AVPacket* packet_ = nullptr;
    
    HardwareDetector::Capabilities caps_;
    
    Stats stats_;
    std::chrono::steady_clock::time_point startTime_;
    
    bool initializeEncoder(const StreamConfig& config);
    bool initializeEncoder(const RecordingConfig& config);
    void shutdownEncoder();
    
    bool initializeCodecContext(const StreamConfig& config);
    bool initializeCodecContext(const RecordingConfig& config);
    
    bool openOutputStream();
    bool openOutputFile();
    
    void encodeLoop();
    bool encodeFrame(FramePtr frame);
    
    bool initFFmpeg();
    void cleanupFFmpeg();
    
    EncoderType selectBestEncoder(EncoderType preferred);
    const AVCodec* findCodec(EncoderType type);
    std::string getEncoderName(EncoderType type);
    
    bool convertFrameFormat(FramePtr input, AVFrame* output);
    
    void updateStats(uint64_t bytesWritten);
};

} // namespace obs