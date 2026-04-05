#pragma once

#include "common.hpp"
#include "frame.hpp"
#include <memory>
#include <string>
#include <nlohmann/json.hpp>

namespace obs {

class Overlay {
public:
    virtual ~Overlay() = default;
    
    OverlayId getId() const { return id_; }
    void setId(OverlayId id) { id_ = id; }
    
    virtual void render(FramePtr baseFrame, double timestamp) = 0;
    virtual void update(const nlohmann::json& params) = 0;
    
    virtual std::string getType() const = 0;
    
    bool isVisible() const { return visible_; }
    void setVisible(bool visible) { visible_ = visible; }
    
    Position getPosition() const { return position_; }
    void setPosition(const Position& pos) { position_ = pos; }
    
    Size getSize() const { return size_; }
    void setSize(const Size& s) { size_ = s; }
    
    float getOpacity() const { return opacity_; }
    void setOpacity(float opacity) { opacity_ = std::clamp(opacity, 0.0f, 1.0f); }
    
protected:
    OverlayId id_ = INVALID_OVERLAY_ID;
    std::string name_;
    bool visible_ = true;
    Position position_{0.0f, 0.0f};
    Size size_{100.0f, 100.0f};
    float opacity_ = 1.0f;
};

class FootballScoreboard : public Overlay {
public:
    struct MatchData {
        std::string team1Name;
        std::string team2Name;
        std::string team1LogoPath;
        std::string team2LogoPath;
        int team1Score = 0;
        int team2Score = 0;
        int minute = 0;
        std::string period = "1ST HALF";
    };
    
    FootballScoreboard();
    ~FootballScoreboard() override = default;
    
    void render(FramePtr baseFrame, double timestamp) override;
    void update(const nlohmann::json& params) override;
    
    std::string getType() const override { return "FootballScoreboard"; }
    
    void setMatchData(const MatchData& data) { matchData_ = data; }
    MatchData getMatchData() const { return matchData_; }
    
private:
    MatchData matchData_;
    
    void renderBackground(FramePtr frame);
    void renderTeamSection(FramePtr frame, int team, int x, int y);
    void renderScore(FramePtr frame, int x, int y);
    void renderTime(FramePtr frame, int x, int y);
    
    uint32_t getColor(const std::string& colorName);
};

class LowerThird : public Overlay {
public:
    struct Data {
        std::string playerName;
        std::string teamName;
        int playerNumber = 0;
        std::string position;
        std::string additionalInfo;
    };
    
    LowerThird();
    ~LowerThird() override = default;
    
    void render(FramePtr baseFrame, double timestamp) override;
    void update(const nlohmann::json& params) override;
    
    std::string getType() const override { return "LowerThird"; }
    
    void setData(const Data& data) { data_ = data; }
    Data getData() const { return data_; }
    
    void playAnimation();
    void stopAnimation();
    
private:
    Data data_;
    double animationProgress_ = 0.0;
    bool animating_ = false;
    
    void renderBackground(FramePtr frame, int y);
    void renderPlayerName(FramePtr frame, int x, int y);
    void renderTeamInfo(FramePtr frame, int x, int y);
};

class ImageOverlay : public Overlay {
public:
    ImageOverlay();
    ~ImageOverlay() override = default;
    
    void render(FramePtr baseFrame, double timestamp) override;
    void update(const nlohmann::json& params) override;
    
    std::string getType() const override { return "Image"; }
    
    bool loadImage(const std::string& path);
    
private:
    std::vector<uint8_t> imageData_;
    int imageWidth_ = 0;
    int imageHeight_ = 0;
    std::string imagePath_;
};

class TextOverlay : public Overlay {
public:
    TextOverlay();
    ~TextOverlay() override = default;
    
    void render(FramePtr baseFrame, double timestamp) override;
    void update(const nlohmann::json& params) override;
    
    std::string getType() const override { return "Text"; }
    
    void setText(const std::string& text) { text_ = text; }
    void setFontSize(int size) { fontSize_ = size; }
    void setColor(const Color& color) { color_ = color; }
    void setFontPath(const std::string& path) { fontPath_ = path; }
    
private:
    std::string text_;
    int fontSize_ = 24;
    Color color_{255, 255, 255, 255};
    std::string fontPath_;
};

} // namespace obs