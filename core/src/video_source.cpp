#include "video_source.hpp"
#include "decoder.hpp"
#include "logger.hpp"

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
}

namespace obs {

VideoSource::VideoSource(SourceId id, const InputConfig& config)
    : id_(id)
    , config_(config)
    , framePool_(30, Resolution{config.resolution.width, config.resolution.height}, config.pixelFormat) {
    LOG_INFO("VideoSource created: " + std::to_string(id));
}

VideoSource::~VideoSource() {
    shutdown();
}

bool VideoSource::initialize() {
    return openInput() && initializeDecoder();
}

void VideoSource::shutdown() {
    stop();
    closeInput();
    LOG_INFO("VideoSource shutdown: " + std::to_string(id_));
}

bool VideoSource::start() {
    if (running_.load()) {
        return true;
    }
    
    running_.store(true);
    paused_.store(false);
    eos_.store(false);
    
    decodeThread_ = std::thread(&VideoSource::decodeLoop, this);
    
    LOG_INFO("VideoSource started: " + std::to_string(id_));
    return true;
}

void VideoSource::stop() {
    if (!running_.load()) {
        return;
    }
    
    running_.store(false);
    cv_.notify_all();
    
    if (decodeThread_.joinable()) {
        decodeThread_.join();
    }
    
    LOG_INFO("VideoSource stopped: " + std::to_string(id_));
}

void VideoSource::pause() {
    paused_.store(true);
}

void VideoSource::resume() {
    paused_.store(false);
}

FramePtr VideoSource::getNextFrame() {
    std::lock_guard<std::mutex> lock(mutex_);
    
    if (frameQueue_.empty()) {
        return nullptr;
    }
    
    FramePtr frame = frameQueue_.front();
    frameQueue_.pop();
    return frame;
}

bool VideoSource::hasFrameAvailable() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return !frameQueue_.empty();
}

size_t VideoSource::getQueueSize() const {
    std::lock_guard<std::mutex> lock(mutex_);
    return frameQueue_.size();
}

void VideoSource::seek(double timestamp) {
    if (isFile() && formatContext_) {
        currentTimestamp_.store(timestamp);
        av_seek_frame(formatContext_, -1, static_cast<int64_t>(timestamp * AV_TIME_BASE), 0);
    }
}

bool VideoSource::openInput() {
    int ret = 0;
    
    if (isFile()) {
        ret = avformat_open_input(&formatContext_, config_.path.c_str(), nullptr, nullptr);
        if (ret < 0) {
            LOG_ERROR("Failed to open file: " + config_.path);
            return false;
        }
        
        ret = avformat_find_stream_info(formatContext_, nullptr);
        if (ret < 0) {
            LOG_ERROR("Failed to find stream info");
            avformat_close_input(&formatContext_);
            return false;
        }
        
        for (unsigned int i = 0; i < formatContext_->nb_streams; ++i) {
            if (formatContext_->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO) {
                videoStreamIndex_ = i;
                break;
            }
        }
        
        if (videoStreamIndex_ == -1) {
            LOG_ERROR("No video stream found");
            avformat_close_input(&formatContext_);
            return false;
        }
        
        duration_ = static_cast<double>(formatContext_->duration) / AV_TIME_BASE;
        
        LOG_INFO("Opened file: " + config_.path + ", duration: " + std::to_string(duration_));
        return true;
    }
    
    // TODO: Implement other input types (device, screen, network)
    LOG_ERROR("Input type not implemented yet");
    return false;
}

void VideoSource::closeInput() {
    if (formatContext_) {
        avformat_close_input(&formatContext_);
        formatContext_ = nullptr;
    }
    
    if (codecContext_) {
        avcodec_free_context(&codecContext_);
        codecContext_ = nullptr;
    }
    
    if (swsContext_) {
        sws_freeContext(swsContext_);
        swsContext_ = nullptr;
    }
}

bool VideoSource::initializeDecoder() {
    if (videoStreamIndex_ == -1 || !formatContext_) {
        return false;
    }
    
    AVCodecParameters* codecParams = formatContext_->streams[videoStreamIndex_]->codecpar;
    codec_ = avcodec_find_decoder(codecParams->codec_id);
    
    if (!codec_) {
        LOG_ERROR("Failed to find decoder");
        return false;
    }
    
    codecContext_ = avcodec_alloc_context3(codec_);
    if (!codecContext_) {
        LOG_ERROR("Failed to allocate codec context");
        return false;
    }
    
    int ret = avcodec_parameters_to_context(codecContext_, codecParams);
    if (ret < 0) {
        LOG_ERROR("Failed to copy codec parameters");
        return false;
    }
    
    ret = avcodec_open2(codecContext_, codec_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to open codec");
        return false;
    }
    
    resolution_.width = codecContext_->width;
    resolution_.height = codecContext_->height;
    
    AVRational frameRate = formatContext_->streams[videoStreamIndex_]->r_frame_rate;
    frameRate_.numerator = frameRate.num;
    frameRate_.denominator = frameRate.den;
    
    LOG_INFO("Decoder initialized: " + std::to_string(resolution_.width) + "x" + 
             std::to_string(resolution_.height) + " @ " + std::to_string(frameRate_.toFloat()) + " fps");
    
    return true;
}

void VideoSource::decodeLoop() {
    AVPacket* packet = av_packet_alloc();
    AVFrame* frame = av_frame_alloc();
    
    while (running_.load()) {
        if (paused_.load()) {
            std::unique_lock<std::mutex> lock(mutex_);
            cv_.wait(lock, [this] { return !paused_.load() || !running_.load(); });
            continue;
        }
        
        if (decodeFrame(frame)) {
            FramePtr outputFrame = framePool_.acquire();
            if (outputFrame) {
                convertFrame(frame, outputFrame);
                
                std::lock_guard<std::mutex> lock(mutex_);
                if (frameQueue_.size() < maxQueueSize_) {
                    frameQueue_.push(outputFrame);
                }
                
                if (frameCallback_) {
                    frameCallback_(outputFrame);
                }
            }
        }
        
        if (eos_.load()) {
            break;
        }
    }
    
    av_frame_free(&frame);
    av_packet_free(&packet);
}

bool VideoSource::decodeFrame(AVFrame* frame) {
    if (!formatContext_ || !codecContext_) {
        return false;
    }
    
    AVPacket* packet = av_packet_alloc();
    
    while (true) {
        int ret = av_read_frame(formatContext_, packet);
        
        if (ret < 0) {
            eos_.store(true);
            av_packet_free(&packet);
            return false;
        }
        
        if (packet->stream_index != videoStreamIndex_) {
            av_packet_unref(packet);
            continue;
        }
        
        ret = avcodec_send_packet(codecContext_, packet);
        if (ret < 0) {
            av_packet_unref(packet);
            continue;
        }
        
        ret = avcodec_receive_frame(codecContext_, frame);
        av_packet_unref(packet);
        
        if (ret == 0) {
            av_packet_free(&packet);
            return true;
        }
    }
    
    av_packet_free(&packet);
    return false;
}

void VideoSource::convertFrame(AVFrame* avFrame, FramePtr outputFrame) {
    if (!swsContext_) {
        swsContext_ = sws_getContext(
            avFrame->width, avFrame->height, avFrame->format,
            outputFrame->resolution.width, outputFrame->resolution.height, AV_PIX_FMT_RGBA,
            SWS_BILINEAR, nullptr, nullptr, nullptr
        );
    }
    
    outputFrame->sourceId = id_;
    outputFrame->pts = avFrame->pts * av_q2d(formatContext_->streams[videoStreamIndex_]->time_base);
    
    // TODO: Implement actual frame conversion using sws_scale
}

void VideoSource::handleDecodingError(const std::string& error) {
    LOG_ERROR("Decoding error: " + error);
    if (errorCallback_) {
        errorCallback_(error);
    }
}

} // namespace obs