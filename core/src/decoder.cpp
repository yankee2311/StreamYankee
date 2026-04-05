#include "decoder.hpp"
#include "logger.hpp"

namespace obs {

Decoder::Decoder() {
    LOG_INFO("Decoder created");
}

Decoder::~Decoder() {
    shutdown();
}

bool Decoder::initialize(AVCodecParameters* codecParams, AVRational timeBase) {
    if (!codecParams) {
        LOG_ERROR("Null codec parameters");
        return false;
    }
    
    codec_ = avcodec_find_decoder(codecParams->codec_id);
    if (!codec_) {
        LOG_ERROR("Failed to find decoder for codec ID: " + std::to_string(codecParams->codec_id));
        return false;
    }
    
    codecContext_ = avcodec_alloc_context3(codec_);
    if (!codecContext_) {
        LOG_ERROR("Failed to allocate codec context");
        return false;
    }
    
    int ret = avcodec_parameters_to_context(codecContext_, codecParams);
    if (ret < 0) {
        LOG_ERROR("Failed to copy codec parameters to context");
        avcodec_free_context(&codecContext_);
        return false;
    }
    
    timeBase_ = timeBase;
    codecContext_->time_base = timeBase;
    
    ret = avcodec_open2(codecContext_, codec_, nullptr);
    if (ret < 0) {
        LOG_ERROR("Failed to open codec");
        avcodec_free_context(&codecContext_);
        return false;
    }
    
    resolution_.width = codecContext_->width;
    resolution_.height = codecContext_->height;
    pixelFormat_ = fromAVPixelFormat(codecContext_->pix_fmt);
    
    LOG_INFO("Decoder initialized: " + std::to_string(resolution_.width) + "x" + 
             std::to_string(resolution_.height));
    
    return true;
}

void Decoder::shutdown() {
    if (swsContext_) {
        sws_freeContext(swsContext_);
        swsContext_ = nullptr;
    }
    
    if (codecContext_) {
        avcodec_free_context(&codecContext_);
        codecContext_ = nullptr;
    }
    
    codec_ = nullptr;
}

bool Decoder::decode(AVPacket* packet, FramePtr outputFrame) {
    if (!codecContext_ || !packet) {
        return false;
    }
    
    AVFrame* frame = av_frame_alloc();
    if (!frame) {
        return false;
    }
    
    int ret = avcodec_send_packet(codecContext_, packet);
    if (ret < 0) {
        av_frame_free(&frame);
        return false;
    }
    
    ret = avcodec_receive_frame(codecContext_, frame);
    if (ret < 0) {
        av_frame_free(&frame);
        return false;
    }
    
    bool success = convertFrame(frame, outputFrame);
    
    av_frame_free(&frame);
    return success;
}

Resolution Decoder::getResolution() const {
    return resolution_;
}

FrameRate Decoder::getFrameRate() const {
    return frameRate_;
}

PixelFormat Decoder::getPixelFormat() const {
    return pixelFormat_;
}

AVCodecContext* Decoder::getCodecContext() const {
    return codecContext_;
}

PixelFormat Decoder::fromAVPixelFormat(AVPixelFormat avFormat) {
    switch (avFormat) {
        case AV_PIX_FMT_RGB24:
            return PixelFormat::RGB24;
        case AV_PIX_FMT_RGBA:
            return PixelFormat::RGBA32;
        case AV_PIX_FMT_YUV420P:
            return PixelFormat::YUV420P;
        case AV_PIX_FMT_YUV422P:
            return PixelFormat::YUV422P;
        case AV_PIX_FMT_YUV444P:
            return PixelFormat::YUV444P;
        case AV_PIX_FMT_NV12:
            return PixelFormat::NV12;
        case AV_PIX_FMT_NV21:
            return PixelFormat::NV21;
        default:
            return PixelFormat::RGBA32;
    }
}

AVPixelFormat Decoder::toAVPixelFormat(PixelFormat format) {
    switch (format) {
        case PixelFormat::RGB24:
            return AV_PIX_FMT_RGB24;
        case PixelFormat::RGBA32:
            return AV_PIX_FMT_RGBA;
        case PixelFormat::YUV420P:
            return AV_PIX_FMT_YUV420P;
        case PixelFormat::YUV422P:
            return AV_PIX_FMT_YUV422P;
        case PixelFormat::YUV444P:
            return AV_PIX_FMT_YUV444P;
        case PixelFormat::NV12:
            return AV_PIX_FMT_NV12;
        case PixelFormat::NV21:
            return AV_PIX_FMT_NV21;
        default:
            return AV_PIX_FMT_RGBA;
    }
}

bool Decoder::initializeConverter() {
    if (swsContext_) {
        sws_freeContext(swsContext_);
    }
    
    AVPixelFormat dstFormat = toAVPixelFormat(pixelFormat_);
    
    swsContext_ = sws_getContext(
        codecContext_->width, codecContext_->height, codecContext_->pix_fmt,
        resolution_.width, resolution_.height, dstFormat,
        SWS_BILINEAR, nullptr, nullptr, nullptr
    );
    
    return swsContext_ != nullptr;
}

void Decoder::shutdownConverter() {
    if (swsContext_) {
        sws_freeContext(swsContext_);
        swsContext_ = nullptr;
    }
}

bool Decoder::convertFrame(AVFrame* avFrame, FramePtr outputFrame) {
    if (!swsContext_) {
        if (!initializeConverter()) {
            return false;
        }
    }
    
    outputFrame->allocateBuffer();
    
    uint8_t* dstData[4] = { outputFrame->data.data(), nullptr, nullptr, nullptr };
    int dstLinesize[4] = { outputFrame->linesize[0], 0, 0, 0 };
    
    sws_scale(swsContext_, avFrame->data, avFrame->linesize, 0,
              avFrame->height, dstData, dstLinesize);
    
    return true;
}

} // namespace obs