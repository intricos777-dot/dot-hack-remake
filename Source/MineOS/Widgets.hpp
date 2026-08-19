#pragma once
#include "Renderer.hpp"
#include <string>
#include <vector>

namespace mine {

// ---- simple immediate-mode widgets -------------------------------------

// returns true when clicked this frame
inline bool button(Renderer& r, const Rect& rc, const std::string& label, FontRef f,
                   bool filled = true, Color c = theme::blood) {
  bool over = rc.contains(r.mx, r.my);
  Color bg = over ? theme::bloodHot : c;
  if (filled) {
    r.rect(rc.x, rc.y, rc.w, rc.h, bg);
    r.outline(rc.x, rc.y, rc.w, rc.h, theme::textDim, 1);
  } else {
    r.outline(rc.x, rc.y, rc.w, rc.h, over ? theme::bloodHot : theme::textDim, 1);
  }
  r.textC(f, label, rc.x, rc.y + (rc.h - r.textH(f)) / 2, rc.w, theme::text);
  return over && r.clicked;
}

struct TextField {
  std::string value;
  bool active = false;
  size_t cursor = 0;

  void key(int sym, const std::string& text);
  void draw(Renderer& r, const Rect& rc, FontRef f, bool focused, bool password = false);
};

// selectable list (scroll handled by caller)
struct ListBox {
  std::vector<std::string> items;
  int selected = -1;
  int hover = -1;
  int offset = 0; // scroll offset in items

  bool draw(Renderer& r, const Rect& rc, FontRef f, int rowH, bool& outClicked);
};

} // namespace mine
