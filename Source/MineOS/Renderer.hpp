#pragma once
#include <string>
#include <vector>
#include <memory>
#include <cmath>
#include <numbers>
#include "Theme.hpp"

struct SDL_Window;
struct SDL_Renderer;
struct SDL_Surface;
struct SDL_Texture;

namespace mine {

class Font;
using FontRef = std::shared_ptr<Font>;

struct Render3D {
    float eyeZ = 800.0f;
    float fovDeg = 55.0f;
    float worldW = 1280.0f;
    float worldH = 800.0f;
    float t = 0.0f;

    bool project(float wx, float wy, float wz, int& sx, int& sy) const {
        float fovRad = fovDeg * (float)std::numbers::pi / 180.0f;
        float d = eyeZ / (eyeZ - wz);
        float sx_f = (wx - worldW * 0.5f) * d + worldW * 0.5f;
        float sy_f = (wy - worldH * 0.5f) * d + worldH * 0.5f;
        sx = (int)sx_f;
        sy = (int)sy_f;
        return wz >= -eyeZ + 1.0f && wz <= eyeZ - 1.0f;
    }
};

class Renderer {
public:
    bool init(int w, int h, const char* title);
    void begin();
    void end();
    void shutdown();
    int width() const { return w_; }
    int height() const { return h_; }

    FontRef font(const std::string& face, int size);
    FontRef fontMono(int size) { return font(theme_face_mono_, size); }
    FontRef fontSans(int size) { return font(theme_face_sans_, size); }
    FontRef fontBold(int size) { return font(theme_face_sansb_, size); }

    // 2D primitives
    void rect(int x, int y, int w, int h, Color c);
    void outline(int x, int y, int w, int h, Color c, int t = 1);
    void hline(int x, int y, int w, Color c);
    void vline(int x, int y, int h, Color c);
    void line(int x0, int y0, int x1, int y1, Color c);

    // Bubble UI primitives
    void bubble(int cx, int cy, int rx, int ry, Color fill, Color border, int border_t = 2);
    void bubbleHex(int cx, int cy, int r, Color fill, Color border, int border_t = 2);
    void bubblePanel(int x, int y, int w, int h, Color fill, Color border);
    void scanlineOverlay(int x, int y, int w, int h, uint8_t alpha = 40);
    void glowRect(int x, int y, int w, int h, Color glowColor, int radius = 12);

    // 3D bubble primitives
    void bubble3D(float x, float y, float z, float rx, float ry, Color fill, Color border, int border_t = 2);
    void bubbleHex3D(float x, float y, float z, float r, Color fill, Color border, int border_t = 2);
    void set3D(float eyeZ, float fovDeg) { r3d_.eyeZ = eyeZ; r3d_.fovDeg = fovDeg; }
    const Render3D& r3d() const { return r3d_; }
    Render3D& r3d() { return r3d_; }

    // Triangle filling for 3D meshes
    void fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color c);
    void fillTriangleGraded(int x0, int y0, int x1, int y1, int x2, int y2, Color c0, Color c1, Color c2);

    // text
    void text(FontRef f, const std::string& s, int x, int y, Color c);
    void textR(FontRef f, const std::string& s, int x, int y, int w, Color c);
    void textC(FontRef f, const std::string& s, int x, int y, int w, Color c);
    void textWrap(FontRef f, const std::string& s, int x, int y, int w, int& outH, Color c);

    int textW(FontRef f, const std::string& s);
    int textH(FontRef f) { return textHeight(f); }

    void setClip(int x, int y, int w, int h);
    void clearClip();

    // animation/time
    void setTime(float t) { time_ = t; }
    float time() const { return time_; }

    // input state updated by the shell each frame
    int mx = 0, my = 0;
    bool mouseDown = false;
    bool clicked = false;

private:
    int textHeight(FontRef f);
    void drawSurface(SDL_Surface* s, int x, int y);
    SDL_Texture* loadTexture(SDL_Surface* surf);
    void drawHexagon(int cx, int cy, int r, Color c);

    SDL_Window* win_ = nullptr;
    SDL_Renderer* ren_ = nullptr;
    int w_ = 0, h_ = 0;
    float time_ = 0.f;
    std::string theme_face_sans_ = FONT_SANS;
    std::string theme_face_sansb_ = FONT_SANS_BOLD;
    std::string theme_face_mono_ = FONT_MONO;
    std::vector<std::pair<std::string, FontRef>> cache_;
    Render3D r3d_;
};

struct Rect { int x = 0, y = 0, w = 0, h = 0; bool contains(int px, int py) const { return px >= x && py >= y && px < x + w && py < y + h; } };

} // namespace mine
