#include "overlay_engine.hpp"
#include "logger.hpp"

namespace obs {

FootballScoreboard::FootballScoreboard() {
    name_ = "Football Scoreboard";
}

void FootballScoreboard::render(FramePtr baseFrame, double timestamp) {
    if (!visible_) return;
    
    renderBackground(baseFrame);
    renderTeamSection(baseFrame, 0, position_.x, position_.y);
    renderTeamSection(baseFrame, 1, position_.x + 200, position_.y);
    renderScore(baseFrame, position_.x + 100, position_.y);
    renderTime(baseFrame, position_.x + 100, position_.y + 50);
}

void FootballScoreboard::update(const nlohmann::json& params) {
    if (params.contains("team1Name")) {
        matchData_.team1Name = params["team1Name"];
    }
    if (params.contains("team2Name")) {
        matchData_.team2Name = params["team2Name"];
    }
    if (params.contains("team1Score")) {
        matchData_.team1Score = params["team1Score"];
    }
    if (params.contains("team2Score")) {
        matchData_.team2Score = params["team2Score"];
    }
    if (params.contains("minute")) {
        matchData_.minute = params["minute"];
    }
    if (params.contains("period")) {
        matchData_.period = params["period"];
    }
}

void FootballScoreboard::renderBackground(FramePtr frame) {
    // TODO: Render semi-transparent background
}

void FootballScoreboard::renderTeamSection(FramePtr frame, int team, int x, int y) {
    // TODO: Render team logo and name
}

void FootballScoreboard::renderScore(FramePtr frame, int x, int y) {
    // TODO: Render score
}

void FootballScoreboard::renderTime(FramePtr frame, int x, int y) {
    // TODO: Render time
}

uint32_t FootballScoreboard::getColor(const std::string& colorName) {
    // TODO: Implement color mapping
    return 0xFFFFFFFF;
}

LowerThird::LowerThird() {
    name_ = "Lower Third";
}

void LowerThird::render(FramePtr baseFrame, double timestamp) {
    if (!visible_) return;
    
    renderBackground(baseFrame, position_.y);
    renderPlayerName(baseFrame, position_.x, position_.y);
    renderTeamInfo(baseFrame, position_.x, position_.y + 30);
}

void LowerThird::update(const nlohmann::json& params) {
    if (params.contains("playerName")) {
        data_.playerName = params["playerName"];
    }
    if (params.contains("teamName")) {
        data_.teamName = params["teamName"];
    }
    if (params.contains("playerNumber")) {
        data_.playerNumber = params["playerNumber"];
    }
    if (params.contains("position")) {
        data_.position = params["position"];
    }
}

void LowerThird::playAnimation() {
    animating_ = true;
    animationProgress_ = 0.0;
}

void LowerThird::stopAnimation() {
    animating_ = false;
}

void LowerThird::renderBackground(FramePtr frame, int y) {
    // TODO: Render background
}

void LowerThird::renderPlayerName(FramePtr frame, int x, int y) {
    // TODO: Render player name
}

void LowerThird::renderTeamInfo(FramePtr frame, int x, int y) {
    // TODO: Render team info
}

ImageOverlay::ImageOverlay() {
    name_ = "Image Overlay";
}

void ImageOverlay::render(FramePtr baseFrame, double timestamp) {
    if (!visible_ || imageData_.empty()) return;
    // TODO: Render image
}

void ImageOverlay::update(const nlohmann::json& params) {
    // TODO: Update image parameters
}

bool ImageOverlay::loadImage(const std::string& path) {
    // TODO: Load image from file
    imagePath_ = path;
    return true;
}

TextOverlay::TextOverlay() {
    name_ = "Text Overlay";
}

void TextOverlay::render(FramePtr baseFrame, double timestamp) {
    if (!visible_) return;
    // TODO: Render text
}

void TextOverlay::update(const nlohmann::json& params) {
    if (params.contains("text")) {
        text_ = params["text"];
    }
}

} // namespace obs
