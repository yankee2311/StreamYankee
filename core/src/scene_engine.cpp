#include "scene_engine.hpp"
#include "logger.hpp"

namespace obs {

SceneEngine::SceneEngine() : framePool_(30, Resolution{1920, 1080}, PixelFormat::RGBA32) {
    LOG_INFO("SceneEngine created");
}

SceneEngine::~SceneEngine() {
    clearAll();
}

SceneId SceneEngine::createScene(const std::string& name) {
    std::lock_guard<std::mutex> lock(mutex_);
    SceneId id = nextSceneId_++;
    scenes_[id] = Scene{id, name, {}, INVALID_SOURCE_ID, INVALID_SOURCE_ID, false};
    overlays_[id] = {};
    LOG_INFO("Created scene: " + name);
    return id;
}

void SceneEngine::destroyScene(SceneId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    scenes_.erase(id);
    overlays_.erase(id);
    LOG_INFO("Destroyed scene: " + std::to_string(id));
}

Scene* SceneEngine::getScene(SceneId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    auto it = scenes_.find(id);
    return it != scenes_.end() ? &it->second : nullptr;
}

bool SceneEngine::setActiveScene(SceneId id) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (scenes_.find(id) == scenes_.end()) return false;
    
    if (activeSceneId_ != INVALID_SCENE_ID && scenes_.count(activeSceneId_)) {
        scenes_[activeSceneId_].active = false;
    }
    
    scenes_[id].active = true;
    activeSceneId_ = id;
    LOG_INFO("Set active scene: " + std::to_string(id));
    return true;
}

FramePtr SceneEngine::renderActiveScene(double timestamp) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (activeSceneId_ == INVALID_SCENE_ID) return nullptr;
    return renderScene(activeSceneId_, timestamp);
}

FramePtr SceneEngine::renderScene(SceneId id, double timestamp) {
    auto it = scenes_.find(id);
    if (it == scenes_.end()) return nullptr;
    
    // TODO: Implement actual rendering
    FramePtr frame = framePool_.acquire();
    return frame;
}

void SceneEngine::clearAll() {
    std::lock_guard<std::mutex> lock(mutex_);
    scenes_.clear();
    overlays_.clear();
}

} // namespace obs
