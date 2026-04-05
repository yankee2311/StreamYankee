#include "switching_engine.hpp"
#include "logger.hpp"
#include <algorithm>

namespace obs {

SwitchingEngine::SwitchingEngine() {
    LOG_INFO("SwitchingEngine created");
}

SwitchingEngine::~SwitchingEngine() {
    LOG_INFO("SwitchingEngine destroyed");
}

bool SwitchingEngine::switchToScene(SceneId sceneId, TransitionType type, double duration) {
    if (transition_.active) return false;
    
    previousSceneId_ = currentSceneId_;
    targetSceneId_ = sceneId;
    
    transition_.type = type;
    transition_.duration = duration;
    transition_.progress = 0.0;
    transition_.active = true;
    transition_.startTime = std::chrono::steady_clock::now();
    
    currentSceneId_ = sceneId;
    LOG_INFO("Switching to scene: " + std::to_string(sceneId));
    return true;
}

void SwitchingEngine::updateTransition() {
    if (!transition_.active) return;
    
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration<double>(now - transition_.startTime).count();
    
    transition_.progress = std::min(1.0, elapsed / transition_.duration);
    
    if (transition_.progress >= 1.0) {
        transition_.active = false;
        if (transitionCompleteCallback_) {
            transitionCompleteCallback_(previousSceneId_, targetSceneId_);
        }
    }
}

FramePtr SwitchingEngine::renderTransitionFrame(FramePtr fromFrame, FramePtr toFrame) {
    switch (transition_.type) {
        case TransitionType::Cut:
            return applyCutTransition(fromFrame, toFrame);
        case TransitionType::Fade:
            return applyFadeTransition(fromFrame, toFrame);
        case TransitionType::Dissolve:
            return applyDissolveTransition(fromFrame, toFrame);
        default:
            return toFrame;
    }
}

FramePtr SwitchingEngine::applyCutTransition(FramePtr from, FramePtr to) {
    return to;
}

FramePtr SwitchingEngine::applyFadeTransition(FramePtr from, FramePtr to) {
    return to;
}

FramePtr SwitchingEngine::applyDissolveTransition(FramePtr from, FramePtr to) {
    return to;
}

double SwitchingEngine::easeInOut(double t) {
    return t < 0.5 ? 2.0 * t * t : -1.0 + (4.0 - 2.0 * t) * t;
}

} // namespace obs
