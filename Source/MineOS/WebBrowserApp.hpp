#pragma once
#include "MineOS.hpp"
#include "Renderer.hpp"
#include "Input.hpp"
#include <string>
#include <vector>

namespace mine {

// Secondary browser for displaying full content: long text, images, videos
// from in-game CCS data. Opens as a child window from Mail/BBS/News.
class WebBrowserApp : public App {
public:
    // Content types
    enum class ContentType { Text, Image, Video, Forum, Mixed };

    WebBrowserApp(MineOS* os, const std::string& title, const std::string& content,
                  ContentType type = ContentType::Text);
    WebBrowserApp(MineOS* os, const std::string& title,
                  const std::vector<std::string>& paragraphs,
                  ContentType type = ContentType::Text);

    const char* title() const override { return title_.c_str(); }
    void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

    // Set image path (from CCS extraction)
    void setImagePath(const std::string& path) { imagePath_ = path; hasImage_ = true; }
    // Set video path (from CPK movies)
    void setVideoPath(const std::string& path) { videoPath_ = path; hasVideo_ = true; }
    // Add navigation links (forum-style)
    void addLink(const std::string& label, const std::string& content);

private:
    MineOS* os_;
    std::string title_;
    std::vector<std::string> paragraphs_;
    ContentType type_;
    int scroll_ = 0;
    bool hasImage_ = false;
    std::string imagePath_;
    bool hasVideo_ = false;
    std::string videoPath_;

    // Navigation links for forum-style browsing
    struct Link { std::string label; std::string content; };
    std::vector<Link> links_;
    int selectedLink_ = -1;

    void drawTextContent(Renderer& r, const Rect& body, Input& in);
    void drawImageContent(Renderer& r, const Rect& body, Input& in);
    void drawVideoContent(Renderer& r, const Rect& body, Input& in);
    void drawForumContent(Renderer& r, const Rect& body, Input& in);
};

} // namespace mine
