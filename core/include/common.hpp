#pragma once

#include <cstdint>
#include <string>
#include <memory>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>
#include <functional>
#include <optional>
#include <iostream>
#include <algorithm>

namespace obs {

using TimeStamp = std::chrono::steady_clock::time_point;
using Duration = std::chrono::microseconds;
using FrameId = uint64_t;
using SourceId = int;
using SceneId = int;
using OverlayId = int;

constexpr SourceId INVALID_SOURCE_ID = -1;
constexpr SceneId INVALID_SCENE_ID = -1;
constexpr OverlayId INVALID_OVERLAY_ID = -1;

enum class PixelFormat {
    RGB24,
    RGBA32,
    YUV420P,
    YUV422P,
    YUV444P,
    NV12,
    NV21
};

enum class InputType {
    DeviceCapture,
    ScreenCapture,
    WindowCapture,
    IPAddress,
    MediaFile
};

enum class EncoderType {
    x264,
    NVENC,
    QuickSync,
    VCE,
    AV1
};

struct Resolution {
    int width = 1920;
    int height = 1080;
    
    bool operator==(const Resolution& other) const {
        return width == other.width && height == other.height;
    }
    
    bool operator!=(const Resolution& other) const {
        return !(*this == other);
    }
    
    std::string toString() const {
        return std::to_string(width) + "x" + std::to_string(height);
    }
    
    float aspectRatio() const {
        return static_cast<float>(width) / height;
    }
    
    bool isHD() const { return width >= 1280; }
    bool isFullHD() const { return width >= 1920; }
    bool is4K() const { return width >= 3840; }
};

struct FrameRate {
    int numerator = 30;
    int denominator = 1;
    
    float toFloat() const {
        return static_cast<float>(numerator) / denominator;
    }
    
    std::string toString() const {
        return std::to_string(toFloat()) + " fps";
    }
    
    bool operator==(const FrameRate& other) const {
        return toFloat() == other.toFloat();
    }
};

struct Position {
    float x = 0.0f;
    float y = 0.0f;
    
    Position(float x_ = 0.0f, float y_ = 0.0f) : x(x_), y(y_) {}
    
    bool operator==(const Position& other) const {
        return x == other.x && y == other.y;
    }
};

struct Size {
    float width = 100.0f;
    float height = 100.0f;
    
    Size(float w = 100.0f, float h = 100.0f) : width(w), height(h) {}
    
    bool operator==(const Size& other) const {
        return width == other.width && height == other.height;
    }
};

struct Color {
    uint8_t r = 0;
    uint8_t g = 0;
    uint8_t b = 0;
    uint8_t a = 255;
    
    Color(uint8_t r_ = 0, uint8_t g_ = 0, uint8_t b_ = 0, uint8_t a_ = 255)
        : r(r_), g(g_), b(b_), a(a_) {}
    
    static Color Black() { return Color(0, 0, 0, 255); }
    static Color White() { return Color(255, 255, 255, 255); }
    static Color Transparent() { return Color(0, 0, 0, 0); }
    static Color Red() { return Color(255, 0, 0, 255); }
    static Color Green() { return Color(0, 255, 0, 255); }
    static Color Blue() { return Color(0, 0, 255, 255); }
};

struct Rectangle {
    Position position;
    Size size;
    
    Rectangle() = default;
    Rectangle(float x, float y, float w, float h)
        : position(x, y), size(w, h) {}
    
    bool contains(float x, float y) const {
        return x >= position.x && x <= position.x + size.width &&
               y >= position.y && y <= position.y + size.height;
    }
    
    bool intersects(const Rectangle& other) const {
        return !(position.x + size.width < other.position.x ||
                other.position.x + other.size.width < position.x ||
                position.y + size.height < other.position.y ||
                other.position.y + other.size.height < position.y);
    }
};

template<typename T>
using VecPtr = std::vector<std::shared_ptr<T>>;

template<typename K, typename V>
using HashMap = std::unordered_map<K, V>;

template<typename T>
using Callback = std::function<void(T)>;

using ErrorCallback = std::function<void(const std::string&)>;
using VoidCallback = std::function<void()>;

} // namespace obs