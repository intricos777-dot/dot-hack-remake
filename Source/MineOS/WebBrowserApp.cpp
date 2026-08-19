#include "WebBrowserApp.hpp"
#include <SDL.h>
#include <algorithm>
#include <cmath>

namespace mine {

WebBrowserApp::WebBrowserApp(MineOS* os, const std::string& title,
                             const std::string& content, ContentType type)
    : os_(os), title_(title), type_(type) {
    paragraphs_.push_back(content);
}

WebBrowserApp::WebBrowserApp(MineOS* os, const std::string& title,
                             const std::vector<std::string>& paragraphs,
                             ContentType type)
    : os_(os), title_(title), paragraphs_(paragraphs), type_(type) {}

void WebBrowserApp::addLink(const std::string& label, const std::string& content) {
    links_.push_back({label, content});
    type_ = ContentType::Forum;
}

void WebBrowserApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
    (void)dt;
    auto fb = r.fontBold(17);
    auto f = r.fontSans(14);
    auto ft = r.fontMono(11);

    // Background
    r.rect(body.x, body.y, body.w, body.h, theme::panel);
    r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);

    // Header strip
    r.rect(body.x, body.y, body.w, 30, theme::bloodDark);
    r.text(fb, title_, body.x + 10, body.y + 7, theme::blood);

    // Content area
    Rect contentRc{body.x + 4, body.y + 34, body.w - 8, body.h - 40};

    switch (type_) {
        case ContentType::Image:
            drawImageContent(r, contentRc, in);
            break;
        case ContentType::Video:
            drawVideoContent(r, contentRc, in);
            break;
        case ContentType::Forum:
            drawForumContent(r, contentRc, in);
            break;
        default:
            drawTextContent(r, contentRc, in);
            break;
    }

    // Footer hint
    r.rect(body.x, body.y + body.h - 26, body.w, 26, theme::bloodDark);
    r.text(ft, "ESC: close  |  UP/DOWN: scroll  |  ENTER: open link", body.x + 8, body.y + body.h - 19, theme::textDim);
}

void WebBrowserApp::drawTextContent(Renderer& r, const Rect& body, Input& in) {
    auto f = r.fontSans(14);

    // Handle scrolling
    if (in.key == SDLK_UP || in.wheel > 0) scroll_ = std::max(0, scroll_ - 1);
    if (in.key == SDLK_DOWN || in.wheel < 0) scroll_ = std::min(100, scroll_ + 1);

    int lineH = r.textH(f) + 4;
    int y = body.y - scroll_ * lineH;
    r.setClip(body.x, body.y, body.w, body.h);

    for (const auto& para : paragraphs_) {
        if (para.empty()) { y += lineH; continue; }
        if (y + lineH > body.y && y < body.y + body.h) {
            int wrapH = 0;
            r.textWrap(f, para, body.x + 8, y, body.w - 16, wrapH, theme::text);
            y += wrapH + 8;
        } else {
            int wrapH = 0;
            r.textWrap(f, para, body.x + 8, y, body.w - 16, wrapH, theme::text);
            y += wrapH + 8;
        }
    }
    r.clearClip();
}

void WebBrowserApp::drawImageContent(Renderer& r, const Rect& body, Input& in) {
    auto f = r.fontSans(12);

    // Image area (placeholder for CCS-parsed images)
    Rect imgRc{body.x + 10, body.y + 10, body.w - 20, body.h - 40};

    // Background for image
    r.rect(imgRc.x, imgRc.y, imgRc.w, imgRc.h, theme::bg);
    r.outline(imgRc.x, imgRc.y, imgRc.w, imgRc.h, theme::border, 1);

    // Draw image path or placeholder
    if (hasImage_) {
        // Show image placeholder
        r.textC(f, "[Image: " + imagePath_ + "]", imgRc.x + imgRc.w / 2, imgRc.y + imgRc.h / 2 - 8, imgRc.w, theme::textDim);
        r.textC(f, "(Click to view full image)", imgRc.x + imgRc.w / 2, imgRc.y + imgRc.h / 2 + 10, imgRc.w, theme::textDim);
    } else {
        r.textC(f, "[No image loaded]", imgRc.x + imgRc.w / 2, imgRc.y + imgRc.h / 2, imgRc.w, theme::textDim);
    }

    // Caption below
    if (!paragraphs_.empty()) {
        r.textC(f, paragraphs_[0], body.x + body.w / 2, body.y + body.h - 20, body.w, theme::textDim);
    }

    if (in.key == SDLK_UP || in.wheel > 0) scroll_ = std::max(0, scroll_ - 1);
    if (in.key == SDLK_DOWN || in.wheel < 0) scroll_ = std::min(100, scroll_ + 1);
}

void WebBrowserApp::drawVideoContent(Renderer& r, const Rect& body, Input& in) {
    auto f = r.fontSans(12);

    // Video area (placeholder for movie playback)
    Rect vidRc{body.x + 10, body.y + 10, body.w - 20, body.h - 40};

    // Black background for video
    r.rect(vidRc.x, vidRc.y, vidRc.w, vidRc.h, rgb(0, 0, 0));
    r.outline(vidRc.x, vidRc.y, vidRc.w, vidRc.h, theme::border, 1);

    // Play button (centered triangle)
    int cx = vidRc.x + vidRc.w / 2;
    int cy = vidRc.y + vidRc.h / 2;
    int sz = 20;
    for (int dy = -sz; dy <= sz; dy++) {
        int w = (int)(sz * (1.0f - (float)std::abs(dy) / sz));
        r.line(cx - sz/2 + w/2, cy + dy, cx + sz/2 + w/2, cy + dy, rgb(255, 60, 60));
    }

    if (hasVideo_) {
        r.textC(f, "[Video: " + videoPath_ + "]", vidRc.x + vidRc.w / 2, vidRc.y + vidRc.h - 20, vidRc.w, theme::textDim);
    } else {
        r.textC(f, "[No video loaded]", vidRc.x + vidRc.w / 2, vidRc.y + vidRc.h - 20, vidRc.w, theme::textDim);
    }

    // Caption
    if (!paragraphs_.empty()) {
        r.textC(f, paragraphs_[0], body.x + body.w / 2, body.y + body.h - 20, body.w, theme::textDim);
    }

    if (in.key == SDLK_UP || in.wheel > 0) scroll_ = std::max(0, scroll_ - 1);
    if (in.key == SDLK_DOWN || in.wheel < 0) scroll_ = std::min(100, scroll_ + 1);
}

void WebBrowserApp::drawForumContent(Renderer& r, const Rect& body, Input& in) {
    auto f = r.fontSans(13);
    auto fb = r.fontBold(14);

    // Handle scrolling
    if (in.key == SDLK_UP || in.wheel > 0) scroll_ = std::max(0, scroll_ - 1);
    if (in.key == SDLK_DOWN || in.wheel < 0) scroll_ = std::min(100, scroll_ + 1);

    int itemH = 40;
    int startY = body.y + 8 - scroll_ * itemH;

    r.setClip(body.x, body.y, body.w, body.h);

    for (int i = 0; i < (int)links_.size(); i++) {
        int y = startY + i * itemH;
        if (y + itemH < body.y || y > body.y + body.h) continue;

        // Link item background
        bool sel = (i == selectedLink_);
        bool hover = (in.mx >= body.x && in.mx <= body.x + body.w && in.my >= y && in.my < y + itemH - 4);

        if (sel) {
            r.rect(body.x + 4, y, body.w - 8, itemH - 4, theme::bloodDark);
        } else if (hover) {
            r.rect(body.x + 4, y, body.w - 8, itemH - 4, theme::panelAlt);
        }

        // Link label
        r.text(fb, links_[i].label, body.x + 12, y + 4, sel ? theme::blood : (hover ? theme::bone : theme::text));
        r.text(f, "Click to read...", body.x + 12, y + 22, theme::textDim);

        // Selection indicator
        if (sel) {
            r.rect(body.x + 4, y, 3, itemH - 4, theme::blood);
        }

        // Click to open
        if (hover && in.clicked) {
            selectedLink_ = i;
        }
        if (sel && (in.key == SDLK_RETURN || in.key == SDLK_SPACE)) {
            // Open content in new browser
            auto* browser = new WebBrowserApp(os_, links_[i].label, links_[i].content, ContentType::Text);
            os_->openWindow(browser);
        }
    }

    r.clearClip();
}

} // namespace mine
