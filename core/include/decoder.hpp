#pragma once

#include "common.hpp"
#include "frame.hpp"
#include <memory>
#include <vector>

extern "C" {
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswscale/swscale.h>
#include <libswresample/swresample.h>
}

namespace obs {

class Decoder {
public:
    Decoder();
    ~Decoder();
    
    Decoder(const Decoder&) = delete;
    Decoder& operator=(const Decoder&) = delete;
    
    bool initialize(AVCodecParameters* codecParams, AVRational timeBase);
    void shutdown();
    
    bool decode(AVPacket* packet, FramePtr outputFrame);
    
    Resolution getResolution() const { return resolution_; }
    FrameRate getFrameRate() const { return frameRate_; }
    PixelFormat getPixelFormat() const { return pixelFormat_; }
    
    AVCodecContext* getCodecContext() const { return codecContext_; }
    
    static PixelFormat fromAVPixelFormat(AVPixelFormat avFormat);
    static AVPixelFormat toAVPixelFormat(PixelFormat format);
    
private:
    AVCodecContext* codecContext_ = nullptr;
    const AVCodec* codec_ = nullptr;
    SwsContext* swsContext_ = nullptr;
    
    Resolution resolution_;
    FrameRate frameRate_;
    PixelFormat pixelFormat_ = PixelFormat::RGBA32;
    
    AVRational timeBase_;
    
    bool needsConversion_ = true;
    
    bool initializeConverter();
    void shutdownConverter();
    
    bool convertFrame(AVFrame* avFrame, FramePtr outputFrame);
};

} // namespace obs