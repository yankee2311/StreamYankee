#pragma once

#include "common.hpp"
#include "frame.hpp"
#include "video_source.hpp"
#include <memory>
#include <vector>
#include <string>
#include <unordered_map>
#include <mutex>

namespace obs {

struct SceneLayer {
    int id = -1;
    int zIndex = 0;
    std::string name;
    bool visible = true;
    
    Position position{0.0f, 0.0f};
    Size size{100.0f, 100.0f};
    float opacity = 1.0f;
    float rotation = 0.0f;
};

struct Scene {
    SceneId id = INVALID_SCENE_ID;
    std::string name;
    std::vector<SceneLayer> layers;
    
    int activeSourceId = INVALID_SOURCE_ID;
    int previewSourceId = INVALID_SOURCE_ID;
    
    bool active = false;
    
    std::string toString() const {
        return "Scene[" + std::to_string(id) + "] " + name;
    }
};

class Overlay;

class SceneEngine {
public:
    SceneEngine();
    ~SceneEngine();
    
    SceneEngine(const SceneEngine&) = delete;
    SceneEngine& operator=(const SceneEngine&) = delete;
    
    SceneId createScene(const std::string& name);
    void destroyScene(SceneId id);
    
    Scene* getScene(SceneId id);
    const Scene* getScene(SceneId id) const;
    
    std::vector<SceneId> getScenes() const;
    size_t getSceneCount() const { return scenes_.size(); }
    
    bool setActiveScene(SceneId id);
    SceneId getActiveScene() const { return activeSceneId_; }
    
    bool setPreviewScene(SceneId id);
    SceneId getPreviewScene() const { return previewSceneId_; }
    
    void addSource(SceneId sceneId, SourceId sourceId, int zIndex = 0);
    void removeSource(SceneId sceneId, SourceId sourceId);
    
    void addOverlay(SceneId sceneId, std::shared_ptr<Overlay> overlay, int zIndex = 0);
    void removeOverlay(SceneId sceneId, OverlayId overlayId);
    void updateOverlay(SceneId sceneId, OverlayId overlayId, const std::string& params);
    
    FramePtr renderActiveScene(double timestamp);
    FramePtr renderPreviewScene(double timestamp);
    
    FramePtr renderScene(SceneId id, double timestamp);
    
    void setResolution(const Resolution& res) { resolution_ = res; }
    Resolution getResolution() const { return resolution_; }
    
    void setFrameRate(const FrameRate& fps) { frameRate_ = fps; }
    FrameRate getFrameRate() const { return frameRate_; }
    
    void clearAll();
    
private:
    std::mutex mutex_;
    HashMap<SceneId, Scene> scenes_;
    HashMap<SceneId, std::vector<std::shared_ptr<Overlay>>> overlays_;
    
    SceneId activeSceneId_ = INVALID_SCENE_ID;
    SceneId previewSceneId_ = INVALID_SCENE_ID;
    SceneId nextSceneId_ = 1;
    
    Resolution resolution_{1920, 1080};
    FrameRate frameRate_{30, 1};
    
    FramePool framePool_;
    
    FramePtr composeFrame(const Scene& scene, double timestamp);
    bool blendFrames(FramePtr base, FramePtr overlay, const SceneLayer& layer);
    
    SceneLayer* findLayer(Scene& scene, int layerId);
};

} // namespace obs