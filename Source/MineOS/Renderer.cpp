#include "Renderer.hpp"
#include <SDL.h>
#include <SDL_ttf.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <numbers>

namespace mine {

class Font {
public:
  TTF_Font* f = nullptr;
  int size = 0;
  ~Font() { if (f) TTF_CloseFont(f); }
};

bool Renderer::init(int w, int h, const char* title) {
  if (SDL_Init(SDL_INIT_VIDEO) != 0) return false;
  if (TTF_Init() != 0) return false;
  win_ = SDL_CreateWindow(title, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h,
                          SDL_WINDOW_SHOWN | SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_FULLSCREEN_DESKTOP);
  if (!win_) return false;
  ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
  if (!ren_) ren_ = SDL_CreateRenderer(win_, -1, SDL_RENDERER_SOFTWARE);
  if (!ren_) return false;
  SDL_SetRenderDrawBlendMode(ren_, SDL_BLENDMODE_BLEND);
  // Auto-detect display resolution
  SDL_DisplayMode dm;
  if (SDL_GetDesktopDisplayMode(0, &dm) == 0) {
    w_ = dm.w;
    h_ = dm.h;
  } else {
    SDL_GetWindowSize(win_, &w_, &h_);
  }
  // Set logical size for consistent rendering coordinates
  SDL_RenderSetLogicalSize(ren_, w_, h_);
  return true;
}

void Renderer::begin() {
  SDL_SetRenderDrawColor(ren_, 0, 0, 0, 255);
  SDL_RenderClear(ren_);
}

void Renderer::end() { SDL_RenderPresent(ren_); }

void Renderer::shutdown() {
  cache_.clear();
  if (ren_) { SDL_DestroyRenderer(ren_); ren_ = nullptr; }
  if (win_) { SDL_DestroyWindow(win_); win_ = nullptr; }
  TTF_Quit();
  SDL_Quit();
}

FontRef Renderer::font(const std::string& face, int size) {
  for (auto& e : cache_)
    if (e.first == face + "#" + std::to_string(size)) return e.second;
  auto f = std::make_shared<Font>();
  f->size = size;
  f->f = TTF_OpenFont(face.c_str(), size);
  if (!f->f) f->f = TTF_OpenFont(FONT_SANS, size);
  cache_.push_back({face + "#" + std::to_string(size), f});
  return f;
}

int Renderer::textHeight(FontRef f) {
  if (!f || !f->f) return 12;
  return TTF_FontHeight(f->f);
}

int Renderer::textW(FontRef f, const std::string& s) {
  if (!f || !f->f) return 0;
  int w = 0;
  TTF_SizeUTF8(f->f, s.c_str(), &w, nullptr);
  return w;
}

void Renderer::drawSurface(SDL_Surface* s, int x, int y) {
  if (!s) return;
  SDL_Texture* tex = SDL_CreateTextureFromSurface(ren_, s);
  if (!tex) { SDL_FreeSurface(s); return; }
  SDL_Rect dst{x, y, s->w, s->h};
  SDL_RenderCopy(ren_, tex, nullptr, &dst);
  SDL_DestroyTexture(tex);
  SDL_FreeSurface(s);
}

void Renderer::setClip(int x, int y, int w, int h) {
  SDL_Rect r{x, y, w, h};
  SDL_RenderSetClipRect(ren_, &r);
}
void Renderer::clearClip() { SDL_RenderSetClipRect(ren_, nullptr); }

static void setc(SDL_Renderer* r, Color c) {
  SDL_SetRenderDrawColor(r, c.r, c.g, c.b, c.a);
}

void Renderer::rect(int x, int y, int w, int h, Color c) {
  if (w <= 0 || h <= 0) return;
  setc(ren_, c);
  SDL_Rect r{x, y, w, h};
  SDL_RenderFillRect(ren_, &r);
}

void Renderer::outline(int x, int y, int w, int h, Color c, int t) {
  setc(ren_, c);
  for (int i = 0; i < t; i++) {
    SDL_Rect r{x + i, y + i, w - 2 * i, h - 2 * i};
    SDL_RenderDrawRect(ren_, &r);
  }
}

void Renderer::hline(int x, int y, int w, Color c) { setc(ren_, c); SDL_RenderDrawLine(ren_, x, y, x + w - 1, y); }
void Renderer::vline(int x, int y, int h, Color c) { setc(ren_, c); SDL_RenderDrawLine(ren_, x, y, x, y + h - 1); }
void Renderer::line(int x0, int y0, int x1, int y1, Color c) { setc(ren_, c); SDL_RenderDrawLine(ren_, x0, y0, x1, y1); }

static SDL_Surface* renderText(TTF_Font* f, const std::string& s, Color c) {
  SDL_Color col{c.r, c.g, c.b, c.a};
  return TTF_RenderUTF8_Blended(f, s.c_str(), col);
}

void Renderer::text(FontRef f, const std::string& s, int x, int y, Color c) {
  if (!f || !f->f || s.empty()) return;
  drawSurface(renderText(f->f, s, c), x, y);
}

void Renderer::textR(FontRef f, const std::string& s, int x, int y, int w, Color c) {
  text(f, s, x + w - textW(f, s), y, c);
}

void Renderer::textC(FontRef f, const std::string& s, int x, int y, int w, Color c) {
  text(f, s, x + (w - textW(f, s)) / 2, y, c);
}

void Renderer::textWrap(FontRef f, const std::string& s, int x, int y, int w, int& outH, Color c) {
  if (!f || !f->f || s.empty()) { outH = 0; return; }
  int lineH = textHeight(f);
  int lx = x, ly = y;
  std::string cur;
  auto flush = [&]() {
    if (!cur.empty()) { text(f, cur, lx, ly, c); ly += lineH; }
    cur.clear();
  };
  size_t i = 0;
  while (i < s.size()) {
    size_t sp = s.find(' ', i);
    std::string word = (sp == std::string::npos) ? s.substr(i) : s.substr(i, sp - i + 1);
    if (!cur.empty() && textW(f, cur + word) > w) flush();
    cur += word;
    i += word.size();
  }
  flush();
  outH = (ly - y);
}

void Renderer::drawHexagon(int cx, int cy, int r, Color c) {
  setc(ren_, c);
  for (int i = 0; i < 6; i++) {
    double a1 = std::numbers::pi * 2.0 / 6.0 * i + std::numbers::pi / 6.0;
    double a2 = std::numbers::pi * 2.0 / 6.0 * (i + 1) + std::numbers::pi / 6.0;
    int x1 = cx + (int)(std::cos(a1) * r);
    int y1 = cy + (int)(std::sin(a1) * r);
    int x2 = cx + (int)(std::cos(a2) * r);
    int y2 = cy + (int)(std::sin(a2) * r);
    SDL_RenderDrawLine(ren_, x1, y1, x2, y2);
  }
}

void Renderer::bubble(int cx, int cy, int rx, int ry, Color fill, Color border, int border_t) {
  setc(ren_, fill);
  for (int y = -ry; y <= ry; y++) {
    int xr = (int)(rx * std::sqrt(1.0 - (double)y * y / (ry * ry)));
    SDL_RenderDrawLine(ren_, cx - xr, cy + y, cx + xr, cy + y);
  }
  for (int t = 0; t < border_t; t++) {
    setc(ren_, border);
    for (int y = -ry + t; y <= ry - t; y++) {
      int xr = (int)(rx * std::sqrt(1.0 - (double)y * y / (ry * ry)));
      SDL_RenderDrawLine(ren_, cx - xr + t, cy + y, cx - xr + t + 1, cy + y);
      SDL_RenderDrawLine(ren_, cx + xr - t, cy + y, cx + xr - t + 1, cy + y);
    }
  }
}

void Renderer::bubblePanel(int x, int y, int w, int h, Color fill, Color border) {
  int rx = w / 2;
  int ry = h / 2;
  int cx = x + rx;
  int cy = y + ry;
  bubble(cx, cy, rx, ry, fill, border, 2);
}

void Renderer::bubble3D(float x, float y, float z, float rx, float ry, Color fill, Color border, int border_t) {
  for (float wy = -ry; wy <= ry; wy += 2.0f) {
    float t2 = wy / ry;
    if (t2 * t2 > 1.0f) continue;
    float xr = rx * std::sqrt(1.0f - t2 * t2);
    float yw = y + wy;
    int sx, sy, cx, cy;
    if (!r3d_.project(x - xr, yw, z, sx, sy)) continue;
    if (!r3d_.project(x + xr, yw, z, cx, cy)) continue;
    setc(ren_, fill);
    SDL_RenderDrawLine(ren_, sx, sy, cx, cy);
  }
  for (int k = 0; k < 6; k++) {
    float ang = std::numbers::pi_v<float> * 2.0f * k / 6.0f;
    setc(ren_, border);
    for (float step = 0.05f; step <= 1.0f; step += 0.05f) {
      float a2 = step * std::numbers::pi_v<float>;
      float a1 = (step - 0.05f) * std::numbers::pi_v<float>;
      int px, py, px0, py0;
      if (!r3d_.project(x + std::cos(a2) * rx * std::cos(ang), y + std::sin(a2) * ry, z + std::sin(ang) * rx * 0.4f, px, py)) continue;
      if (!r3d_.project(x + std::cos(a1) * rx * std::cos(ang), y + std::sin(a1) * ry, z + std::sin(ang) * rx * 0.4f, px0, py0)) continue;
      SDL_RenderDrawLine(ren_, px0, py0, px, py);
    }
  }
}

void Renderer::bubbleHex3D(float x, float y, float z, float r, Color fill, Color border, int border_t) {
  for (int i = 0; i < 6; i++) {
    double a1 = M_PI / 3.0 * i + M_PI / 6.0;
    double a2 = M_PI / 3.0 * (i + 1) + M_PI / 6.0;
    int sx1, sy1, sx2, sy2, sx3, sy3;
    r3d_.project((float)(x + std::cos(a1) * r), (float)(y - r * 0.5f), z, sx1, sy1);
    r3d_.project((float)(x + std::cos(a2) * r), (float)(y - r * 0.5f), z, sx2, sy2);
    r3d_.project((float)(x + std::cos((a1 + a2) / 2.0) * r * 0.6f), (float)(y + r * 0.2f), z, sx3, sy3);
    setc(ren_, fill);
    SDL_RenderDrawLine(ren_, sx1, sy1, sx3, sy3);
    SDL_RenderDrawLine(ren_, sx3, sy3, sx2, sy2);
    SDL_RenderDrawLine(ren_, sx2, sy2, sx1, sy1);
  }
  for (int t = 0; t < border_t; t++) {
    setc(ren_, border);
    for (int i = 0; i < 6; i++) {
      double a1 = M_PI / 3.0 * i + M_PI / 6.0;
      double a2 = M_PI / 3.0 * (i + 1) + M_PI / 6.0;
      int sx1, sy1, sx2, sy2;
      r3d_.project((float)(x + std::cos(a1) * r), (float)(y - r * 0.5f), z, sx1, sy1);
      r3d_.project((float)(x + std::cos(a2) * r), (float)(y - r * 0.5f), z, sx2, sy2);
      SDL_RenderDrawLine(ren_, sx1, sy1, sx2, sy2);
    }
  }
}

void Renderer::glowRect(int x, int y, int w, int h, Color glowColor, int radius) {
  if (radius <= 0) { rect(x, y, w, h, glowColor); return; }
  rect(x, y, w, h, glowColor);
  for (int i = 1; i <= radius; i++) {
    Color c = glowColor;
    c.a = (uint8_t)(c.a * (1.0f - (float)i / (radius + 1)));
    rect(x - i, y - i, w + 2*i, h + 2*i, c);
  }
}

void Renderer::scanlineOverlay(int x, int y, int w, int h, uint8_t alpha) {
  setc(ren_, {0, 0, 0, alpha});
  for (int yy = y; yy < y + h; yy += 3) {
    SDL_RenderDrawLine(ren_, x, yy, x + w - 1, yy);
  }
}

// Fill a triangle using SDL2 (scanline)
void Renderer::fillTriangle(int x0, int y0, int x1, int y1, int x2, int y2, Color c) {
  setc(ren_, c);
  int minY = std::min({y0, y1, y2});
  int maxY = std::max({y0, y1, y2});
  for (int y = minY; y <= maxY; y++) {
    int xStart = w_ + 1, xEnd = -1;
    auto scanline = [&](int ya, int yb, int xa, int xb) {
      if ((y >= ya && y <= yb) || (y >= yb && y <= ya)) {
        if (ya != yb) {
          float t = (float)(y - ya) / (yb - ya);
          int x = (int)(xa + (xb - xa) * t);
          xStart = std::min(xStart, x);
          xEnd = std::max(xEnd, x);
        }
      }
    };
    scanline(y0, y1, x0, x1);
    scanline(y1, y2, x1, x2);
    scanline(y2, y0, x2, x0);
    if (xStart <= xEnd && xEnd >= 0 && xStart < w_) {
      xStart = std::max(0, xStart);
      xEnd = std::min(w_ - 1, xEnd);
      SDL_RenderDrawLine(ren_, xStart, y, xEnd, y);
    }
  }
}

// Fill a triangle with per-vertex colors (Gouraud shading)
void Renderer::fillTriangleGraded(int x0, int y0, int x1, int y1, int x2, int y2,
                                  Color c0, Color c1, Color c2) {
  int minY = std::min({y0, y1, y2});
  int maxY = std::max({y0, y1, y2});
  for (int y = minY; y <= maxY; y++) {
    struct Hit { int x; float r, g, b, a; };
    std::vector<Hit> hits;
    auto scanline = [&](int ya, int yb, int xa, int xb, Color ca, Color cb) {
      if ((y >= ya && y <= yb) || (y >= yb && y <= ya)) {
        if (ya != yb) {
          float t = (float)(y - ya) / (yb - ya);
          int x = (int)(xa + (xb - xa) * t);
          Hit h;
          h.x = x;
          h.r = ca.r + (cb.r - ca.r) * t;
          h.g = ca.g + (cb.g - ca.g) * t;
          h.b = ca.b + (cb.b - ca.b) * t;
          h.a = ca.a + (cb.a - ca.a) * t;
          hits.push_back(h);
        }
      }
    };
    scanline(y0, y1, x0, x1, c0, c1);
    scanline(y1, y2, x1, x2, c1, c2);
    scanline(y2, y0, x2, x0, c2, c0);
    if (hits.size() >= 2) {
      int xStart = hits[0].x, xEnd = hits[1].x;
      Color cStart = {hits[0].r, hits[0].g, hits[0].b, hits[0].a};
      Color cEnd = {hits[1].r, hits[1].g, hits[1].b, hits[1].a};
      if (xStart > xEnd) {
        std::swap(xStart, xEnd);
        std::swap(cStart, cEnd);
      }
      for (int x = xStart; x <= xEnd; x++) {
        float t = (xEnd > xStart) ? (float)(x - xStart) / (xEnd - xStart) : 0;
        Color c = {
          cStart.r + (cEnd.r - cStart.r) * t,
          cStart.g + (cEnd.g - cStart.g) * t,
          cStart.b + (cEnd.b - cStart.b) * t,
          cStart.a + (cEnd.a - cStart.a) * t
        };
        setc(ren_, c);
        SDL_RenderDrawPoint(ren_, x, y);
      }
    }
  }
}

} // namespace mine
