#include <napi.h>
#include <unordered_map>
#include "../core/include/input_manager.hpp"
#include "../core/include/scene_engine.hpp"
#include "../core/include/output_engine.hpp"
#include "../core/include/replay_engine.hpp"
#include "../core/include/hardware_detector.hpp"
#include "../core/include/logger.hpp"

namespace obs {

class OBSCore : public Napi::ObjectWrap<OBSCore> {
public:
    static Napi::Object Initialize(Napi::Env env, Napi::Object exports) {
        Napi::HandleScope scope(env);
        
        Napi::Function func = DefineClass(env, "OBSCore", {
            InstanceMethod("initialize", &OBSCore::Initialize),
            InstanceMethod("shutdown", &OBSCore::Shutdown),
            
            InstanceMethod("createInput", &OBSCore::CreateInput),
            InstanceMethod("destroyInput", &OBSCore::DestroyInput),
            InstanceMethod("getInputs", &OBSCore::GetInputs),
            InstanceMethod("startInput", &OBSCore::StartInput),
            InstanceMethod("stopInput", &OBSCore::StopInput),
            
            InstanceMethod("createScene", &OBSCore::CreateScene),
            InstanceMethod("destroyScene", &OBSCore::DestroyScene),
            InstanceMethod("getScenes", &OBSCore::GetScenes),
            InstanceMethod("setActiveScene", &OBSCore::SetActiveScene),
            InstanceMethod("getActiveScene", &OBSCore::GetActiveScene),
            
            InstanceMethod("startStreaming", &OBSCore::StartStreaming),
            InstanceMethod("stopStreaming", &OBSCore::StopStreaming),
            InstanceMethod("isStreaming", &OBSCore::IsStreaming),
            
            InstanceMethod("startRecording", &OBSCore::StartRecording),
            InstanceMethod("stopRecording", &OBSCore::StopRecording),
            InstanceMethod("isRecording", &OBSCore::IsRecording),
            
            InstanceMethod("startReplayBuffer", &OBSCore::StartReplayBuffer),
            InstanceMethod("stopReplayBuffer", &OBSCore::StopReplayBuffer),
            InstanceMethod("saveReplay", &OBSCore::SaveReplay),
            
            InstanceMethod("detectHardware", &OBSCore::DetectHardware),
            
            InstanceMethod("getFrame", &OBSCore::GetFrame),
            
            InstanceAccessor("version", &OBSCore::GetVersion, nullptr)
        });
        
        exports.Set("OBSCore", func);
        return exports;
    }
    
    OBSCore(const Napi::CallbackInfo& info) 
        : Napi::ObjectWrap<OBSCore>(info) {
        inputManager_ = std::make_unique<InputManager>();
        sceneEngine_ = std::make_unique<SceneEngine>();
        outputEngine_ = std::make_unique<OutputEngine>();
        initialized_ = false;
    }
    
    ~OBSCore() {
        if (initialized_) {
            shutdown();
        }
    }
    
private:
    Napi::Value Initialize(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Object expected for options").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Object options = info[0].As<Napi::Object>();
        
        std::string logPath = "";
        if (options.Has("logPath")) {
            logPath = options.Get("logPath").As<Napi::String>().Utf8Value();
        }
        
        bool consoleLog = true;
        if (options.Has("consoleLog")) {
            consoleLog = options.Get("consoleLog").As<Napi::Boolean>().Value();
        }
        
        try {
            Logger::instance().initialize(logPath, consoleLog);
            
            auto caps = HardwareDetector::instance().detect();
            
            if (!inputManager_ || !sceneEngine_ || !outputEngine_) {
                throw std::runtime_error("Failed to initialize core components");
            }
            
            initialized_ = true;
            LOG_INFO("OBS Core initialized successfully");
            
            return env.True();
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    
    void Shutdown(const Napi::CallbackInfo& info) {
        shutdown();
    }
    
    void shutdown() {
        if (!initialized_) return;
        
        LOG_INFO("Shutting down OBS Core");
        
        if (outputEngine_) {
            if (outputEngine_->isStreaming()) {
                outputEngine_->stopStreaming();
            }
            if (outputEngine_->isRecording()) {
                outputEngine_->stopRecording();
            }
        }
        
        inputManager_.reset();
        sceneEngine_.reset();
        outputEngine_.reset();
        replayEngine_.reset();
        
        initialized_ = false;
        LOG_INFO("OBS Core shutdown complete");
    }
    
    Napi::Value CreateInput(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Object expected for input config").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Object configObj = info[0].As<Napi::Object>();
        InputConfig config;
        
        if (configObj.Has("type")) {
            std::string typeStr = configObj.Get("type").As<Napi::String>().Utf8Value();
            if (typeStr == "device") config.type = InputType::DeviceCapture;
            else if (typeStr == "screen") config.type = InputType::ScreenCapture;
            else if (typeStr == "window") config.type = InputType::WindowCapture;
            else if (typeStr == "network") config.type = InputType::IPAddress;
            else config.type = InputType::MediaFile;
        }
        
        if (configObj.Has("path")) {
            config.path = configObj.Get("path").As<Napi::String>().Utf8Value();
        }
        
        if (configObj.Has("device")) {
            config.device = configObj.Get("device").As<Napi::String>().Utf8Value();
        }
        
        if (configObj.Has("url")) {
            config.url = configObj.Get("url").As<Napi::String>().Utf8Value();
        }
        
        try {
            SourceId id = inputManager_->createInput(config);
            return Napi::Number::New(env, static_cast<double>(id));
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    
    Napi::Value DestroyInput(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Number expected for input id").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        SourceId id = static_cast<SourceId>(info[0].As<Napi::Number>().Int32Value());
        inputManager_->destroyInput(id);
        
        return env.Undefined();
    }
    
    Napi::Value GetInputs(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        auto inputs = inputManager_->getActiveInputs();
        Napi::Array result = Napi::Array::New(env, inputs.size());
        
        for (size_t i = 0; i < inputs.size(); ++i) {
            result.Set(static_cast<uint32_t>(i), Napi::Number::New(env, static_cast<double>(inputs[i])));
        }
        
        return result;
    }
    
    Napi::Value StartInput(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Number expected for input id").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        SourceId id = static_cast<SourceId>(info[0].As<Napi::Number>().Int32Value());
        auto input = inputManager_->getInput(id);
        
        if (input && !input->isRunning()) {
            input->start();
            return env.True();
        }
        
        return env.False();
    }
    
    Napi::Value StopInput(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Number expected for input id").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        SourceId id = static_cast<SourceId>(info[0].As<Napi::Number>().Int32Value());
        auto input = inputManager_->getInput(id);
        
        if (input && input->isRunning()) {
            input->stop();
            return env.True();
        }
        
        return env.False();
    }
    
    Napi::Value CreateScene(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsString()) {
            Napi::TypeError::New(env, "String expected for scene name").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        std::string name = info[0].As<Napi::String>().Utf8Value();
        SceneId id = sceneEngine_->createScene(name);
        
        return Napi::Number::New(env, static_cast<double>(id));
    }
    
    Napi::Value DestroyScene(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Number expected for scene id").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        SceneId id = static_cast<SceneId>(info[0].As<Napi::Number>().Int32Value());
        sceneEngine_->destroyScene(id);
        
        return env.Undefined();
    }
    
    Napi::Value GetScenes(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        auto scenes = sceneEngine_->getScenes();
        Napi::Array result = Napi::Array::New(env, scenes.size());
        
        for (size_t i = 0; i < scenes.size(); ++i) {
            result.Set(static_cast<uint32_t>(i), Napi::Number::New(env, static_cast<double>(scenes[i])));
        }
        
        return result;
    }
    
    Napi::Value SetActiveScene(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsNumber()) {
            Napi::TypeError::New(env, "Number expected for scene id").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        SceneId id = static_cast<SceneId>(info[0].As<Napi::Number>().Int32Value());
        bool success = sceneEngine_->setActiveScene(id);
        
        return Napi::Boolean::New(env, success);
    }
    
    Napi::Value GetActiveScene(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        SceneId id = sceneEngine_->getActiveScene();
        return Napi::Number::New(env, static_cast<double>(id));
    }
    
    Napi::Value StartStreaming(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Object expected for stream config").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Object configObj = info[0].As<Napi::Object>();
        StreamConfig config;
        
        if (configObj.Has("rtmpUrl")) {
            config.rtmpUrl = configObj.Get("rtmpUrl").As<Napi::String>().Utf8Value();
        }
        
        if (configObj.Has("streamKey")) {
            config.streamKey = configObj.Get("streamKey").As<Napi::String>().Utf8Value();
        }
        
        if (configObj.Has("bitrate")) {
            config.bitrate = configObj.Get("bitrate").As<Napi::Number>().Int32Value();
        }
        
        if (configObj.Has("fps")) {
            config.fps = configObj.Get("fps").As<Napi::Number>().Int32Value();
        }
        
        try {
            HardwareDetector::Capabilities caps = HardwareDetector::instance().detect();
            if (!outputEngine_->initialize(caps)) {
                throw std::runtime_error("Failed to initialize output engine");
            }
            
            bool success = outputEngine_->startStreaming(config);
            return Napi::Boolean::New(env, success);
        } catch (const std::exception& e) {
            Napi::Error::New(env, e.what()).ThrowAsJavaScriptException();
            return env.Null();
        }
    }
    
    Napi::Value StopStreaming(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        outputEngine_->stopStreaming();
        return env.Undefined();
    }
    
    Napi::Value IsStreaming(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            return Napi::Boolean::New(env, false);
        }
        
        return Napi::Boolean::New(env, outputEngine_->isStreaming());
    }
    
    Napi::Value StartRecording(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (info.Length() < 1 || !info[0].IsObject()) {
            Napi::TypeError::New(env, "Object expected for recording config").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        Napi::Object configObj = info[0].As<Napi::Object>();
        RecordingConfig config;
        
        if (configObj.Has("filePath")) {
            config.filePath = configObj.Get("filePath").As<Napi::String>().Utf8Value();
        }
        
        if (configObj.Has("format")) {
            config.format = configObj.Get("format").As<Napi::String>().Utf8Value();
        }
        
        if (configObj.Has("bitrate")) {
            config.bitrate = configObj.Get("bitrate").As<Napi::Number>().Int32Value();
        }
        
        bool success = outputEngine_->startRecording(config);
        return Napi::Boolean::New(env, success);
    }
    
    Napi::Value StopRecording(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        outputEngine_->stopRecording();
        return env.Undefined();
    }
    
    Napi::Value IsRecording(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            return Napi::Boolean::New(env, false);
        }
        
        return Napi::Boolean::New(env, outputEngine_->isRecording());
    }
    
    Napi::Value StartReplayBuffer(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        if (!replayEngine_) {
            ReplayConfig config;
            config.durationSeconds = 300;
            replayEngine_ = std::make_unique<ReplayEngine>(config);
        }
        
        replayEngine_->startBuffering();
        return env.True();
    }
    
    Napi::Value StopReplayBuffer(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            return env.Undefined();
        }
        
        if (replayEngine_) {
            replayEngine_->stopBuffering();
        }
        
        return env.Undefined();
    }
    
    Napi::Value SaveReplay(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_ || !replayEngine_) {
            Napi::Error::New(env, "OBSCore not initialized or replay not active").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        int seconds = 30;
        if (info.Length() > 0 && info[0].IsNumber()) {
            seconds = info[0].As<Napi::Number>().Int32Value();
        }
        
        auto segment = replayEngine_->extractLastSeconds(seconds);
        if (segment) {
            Napi::Object result = Napi::Object::New(env);
            result.Set("startTime", Napi::Number::New(env, segment->startTime));
            result.Set("endTime", Napi::Number::New(env, segment->endTime));
            result.Set("duration", Napi::Number::New(env, segment->duration));
            result.Set("id", Napi::String::New(env, segment->id));
            return result;
        }
        
        return env.Null();
    }
    
    Napi::Value DetectHardware(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        auto caps = HardwareDetector::instance().detect();
        
        Napi::Object result = Napi::Object::New(env);
        result.Set("hasNVENC", Napi::Boolean::New(env, caps.hasNVENC));
        result.Set("hasQuickSync", Napi::Boolean::New(env, caps.hasQuickSync));
        result.Set("hasVCE", Napi::Boolean::New(env, caps.hasVCE));
        result.Set("cpuCores", Napi::Number::New(env, caps.cpuCores));
        result.Set("cpuThreads", Napi::Number::New(env, caps.cpuThreads));
        result.Set("totalRAM", Napi::Number::New(env, static_cast<double>(caps.totalRAM)));
        result.Set("availableRAM", Napi::Number::New(env, static_cast<double>(caps.availableRAM)));
        result.Set("recommendedResolution", Napi::Number::New(env, caps.recommendedResolution));
        result.Set("recommendedFPS", Napi::Number::New(env, caps.recommendedFPS));
        result.Set("recommendedBitrate", Napi::Number::New(env, caps.recommendedBitrate));
        
        Napi::Array videoDevices = Napi::Array::New(env, caps.videoDevices.size());
        for (size_t i = 0; i < caps.videoDevices.size(); ++i) {
            videoDevices.Set(static_cast<uint32_t>(i), Napi::String::New(env, caps.videoDevices[i]));
        }
        result.Set("videoDevices", videoDevices);
        
        Napi::Array audioDevices = Napi::Array::New(env, caps.audioDevices.size());
        for (size_t i = 0; i < caps.audioDevices.size(); ++i) {
            audioDevices.Set(static_cast<uint32_t>(i), Napi::String::New(env, caps.audioDevices[i]));
        }
        result.Set("audioDevices", audioDevices);
        
        return result;
    }
    
    Napi::Value GetFrame(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        
        if (!initialized_) {
            Napi::Error::New(env, "OBSCore not initialized").ThrowAsJavaScriptException();
            return env.Null();
        }
        
        double timestamp = 0.0;
        if (info.Length() > 0 && info[0].IsNumber()) {
            timestamp = info[0].As<Napi::Number>().DoubleValue();
        }
        
        FramePtr frame = sceneEngine_->renderActiveScene(timestamp);
        
        if (!frame) {
            return env.Null();
        }
        
        Napi::Object result = Napi::Object::New(env);
        result.Set("width", Napi::Number::New(env, frame->resolution.width));
        result.Set("height", Napi::Number::New(env, frame->resolution.height));
        result.Set("format", Napi::Number::New(env, static_cast<int>(frame->pixelFormat)));
        result.Set("data", Napi::Buffer<uint8_t>::Copy(env, frame->data.data(), frame->data.size()));
        
        return result;
    }
    
    Napi::Value GetVersion(const Napi::CallbackInfo& info) {
        Napi::Env env = info.Env();
        return Napi::String::New(env, "1.0.0");
    }
    
    std::unique_ptr<InputManager> inputManager_;
    std::unique_ptr<SceneEngine> sceneEngine_;
    std::unique_ptr<OutputEngine> outputEngine_;
    std::unique_ptr<ReplayEngine> replayEngine_;
    bool initialized_ = false;
};

Napi::Object Init(Napi::Env env, Napi::Object exports) {
    return OBSCore::Initialize(env, exports);
}

NODE_API_MODULE(obs_addon, Init)

} // namespace obs