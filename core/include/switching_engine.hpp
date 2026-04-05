#pragma once

#include "common.hpp"
#include "scene_engine.hpp"
#include <memory>
#include <string>
#include <functional>
#include <chrono>

namespace obs {

enum class TransitionType {
    Cut,
    Fade,
    wipeLeft,
    wipeRight,
    wipeUp,
    wipeDown,
    SlideLeft,
    SlideRight,
    Zoom,
    Dissolve
};

struct Transition {
    TransitionType type = TransitionType::Cut;
    double duration = 1.0;
    
    double progress = 0.0;
    bool active = false;
    TimeStamp startTime;
    
    std::string toString() const {
        return "Transition[type=" + std::to_string(static_cast<int>(type)) + 
               ", duration=" + std::to_string(duration) + "]";
    }
};

class SwitchingEngine {
public:
    using TransitionCompleteCallback = std::function<void(SceneId, SceneId)>;
    
    SwitchingEngine();
    ~SwitchingEngine();
    
    SwitchingEngine(const SwitchingEngine&) = delete;
    SwitchingEngine& operator=(const SwitchingEngine&) = delete;
    
    bool switchToScene(SceneId sceneId, TransitionType type = TransitionType::Cut, double duration = 1.0);
    
    bool isTransitioning() const { return transition_.active; }
    Transition getCurrentTransition() const { return transition_; }
    
    SceneId getCurrentScene() const { return currentSceneId_; }
    SceneId getPreviousScene() const { return previousSceneId_; }
    
    void setTransitionCompleteCallback(TransitionCompleteCallback callback) {
        transitionCompleteCallback_ = callback;
    }
    
    void updateTransition();
    
    FramePtr renderTransitionFrame(FramePtr fromFrame, FramePtr toFrame);
    
private:
    SceneId currentSceneId_ = INVALID_SCENE_ID;
    SceneId previousSceneId_ = INVALID_SCENE_ID;
    SceneId targetSceneId_ = INVALID_SCENE_ID;
    
    Transition transition_;
    TransitionCompleteCallback transitionCompleteCallback_;
    
    bool transitioningFrameA_ = false;
    bool transitioningFrameB_ = false;
    
    FramePtr applyCutTransition(FramePtr from, FramePtr to);
    FramePtr applyFadeTransition(FramePtr from, FramePtr to);
    FramePtr applyWipeTransition(FramePtr from, FramePtr to, TransitionType direction);
    FramePtr applySlideTransition(FramePtr from, FramePtr to, TransitionType direction);
    FramePtr applyZoomTransition(FramePtr from, FramePtr to);
    FramePtr applyDissolveTransition(FramePtr from, FramePtr to);
    
    double easeInOut(double t);
};

} // namespaceobs