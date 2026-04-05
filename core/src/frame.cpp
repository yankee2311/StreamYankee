#include "frame.hpp"

namespace obs {

Frame::Frame(const Resolution& res, PixelFormat fmt)
    : resolution(res), pixelFormat(fmt) {
    timestamp = std::chrono::steady_clock::now();
    allocateBuffer();
}

void Frame::allocateBuffer() {
    const size_t size = calculateBufferSize();
    data.resize(size);
    calculateLinesize();
}

size_t Frame::calculateBufferSize() const {
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

void Frame::calculateLinesize() {
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

FramePtr Frame::create() {
    return std::make_shared<Frame>();
}

FramePtr Frame::create(const Resolution& res, PixelFormat fmt) {
    return std::make_shared<Frame>(res, fmt);
}

} // namespace obs