#pragma once
#include <cstdint>

namespace mine {

// Mine OS theme: .hack//GU red ("living-sin-blood" build).
struct Color { uint8_t r, g, b, a; };

constexpr Color rgb(uint8_t r, uint8_t g, uint8_t b) { return Color{r, g, b, 255}; }
constexpr Color rgba(uint8_t r, uint8_t g, uint8_t b, uint8_t a) { return Color{r, g, b, a}; }

namespace theme {
inline constexpr Color bg         = rgb(10, 2, 2);
inline constexpr Color desktop    = rgb(18, 5, 5);
inline constexpr Color panel      = rgb(35, 8, 8);
inline constexpr Color panelAlt   = rgb(45, 10, 10);
inline constexpr Color primary    = rgb(200, 50, 50);
inline constexpr Color primaryDim = rgb(150, 30, 30);
inline constexpr Color accent     = rgb(255, 80, 80);
inline constexpr Color text       = rgb(230, 220, 220);
inline constexpr Color textDim    = rgb(150, 120, 120);
inline constexpr Color border     = rgb(180, 30, 30);
inline constexpr Color sel        = rgb(255, 40, 40);
inline constexpr Color bone       = rgb(220, 200, 200);
inline constexpr Color ok         = rgb(120, 200, 80);
inline constexpr Color warn       = rgb(255, 200, 50);
inline constexpr Color err        = rgb(255, 80, 80);
inline constexpr Color blood      = rgb(255, 30, 30);
inline constexpr Color bloodHot   = rgb(255, 100, 80);
inline constexpr Color bloodDim   = rgb(150, 15, 15);
inline constexpr Color bloodDark  = rgb(80, 5, 5);
inline constexpr Color inputBg    = rgb(15, 5, 5);
inline constexpr Color corrupt    = rgb(200, 40, 40);
inline constexpr Color taskbar    = rgb(22, 6, 6);
inline constexpr Color taskActive = rgb(100, 15, 15);
inline constexpr Color glow       = rgb(255, 60, 60);
inline constexpr Color windowEdge = rgb(200, 30, 30);
inline constexpr Color locked     = rgb(60, 15, 15);
}

constexpr int WIN_W = 1280;
constexpr int WIN_H = 800;
constexpr const char* WIN_TITLE = "MINE OS // living-sin-blood build";

inline constexpr const char* FONT_SANS     = "/usr/share/fonts/TTF/DejaVuSans.ttf";
inline constexpr const char* FONT_SANS_BOLD = "/usr/share/fonts/TTF/DejaVuSans-Bold.ttf";
inline constexpr const char* FONT_MONO     = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";

} // namespace mine
