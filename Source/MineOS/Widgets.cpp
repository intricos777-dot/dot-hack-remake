#include "Widgets.hpp"
#include <SDL.h>
#include <algorithm>

namespace mine {

void TextField::key(int sym, const std::string& text) {
  if (!active) return;
  if (sym == 8) { // backspace
    if (cursor > 0) { value.erase(cursor - 1, 1); cursor--; }
  } else if (sym == 127) { // delete
    if (cursor < value.size()) value.erase(cursor, 1);
  } else if (sym == SDLK_LEFT) {
    if (cursor > 0) cursor--;
  } else if (sym == SDLK_RIGHT) {
    if (cursor < value.size()) cursor++;
  } else if (sym == SDLK_HOME) {
    cursor = 0;
  } else if (sym == SDLK_END) {
    cursor = value.size();
  } else if (!text.empty()) {
    value.insert(cursor, text);
    cursor += text.size();
  }
  if (cursor > value.size()) cursor = value.size();
}

void TextField::draw(Renderer& r, const Rect& rc, FontRef f, bool focused, bool password) {
  r.rect(rc.x, rc.y, rc.w, rc.h, theme::inputBg);
  r.outline(rc.x, rc.y, rc.w, rc.h, (focused || rc.contains(r.mx, r.my)) ? theme::blood : theme::border, 1);
  std::string shown = password ? std::string(value.size(), '*') : value;
  std::string prefix = shown.substr(0, cursor);
  int tx = rc.x + 6;
  int ty = rc.y + (rc.h - r.textH(f)) / 2;
  r.text(f, shown, tx, ty, theme::text);
  if (focused) {
    int cx = tx + r.textW(f, prefix);
    r.vline(cx, ty, r.textH(f), theme::glow);
  }
}

bool ListBox::draw(Renderer& r, const Rect& rc, FontRef f, int rowH, bool& outClicked) {
  outClicked = false;
  r.rect(rc.x, rc.y, rc.w, rc.h, theme::inputBg);
  r.outline(rc.x, rc.y, rc.w, rc.h, theme::border, 1);
  int vis = std::max(1, rc.h / rowH);
  if (offset > (int)items.size() - vis) offset = std::max(0, (int)items.size() - vis);
  if (offset < 0) offset = 0;
  for (int i = 0; i < vis && offset + i < (int)items.size(); i++) {
    int idx = offset + i;
    int y = rc.y + i * rowH;
    Rect row{rc.x, y, rc.w, rowH};
    bool over = row.contains(r.mx, r.my);
    if (over) hover = idx;
    if (idx == selected) {
      r.rect(rc.x, y, rc.w, rowH, theme::sel);
    } else if (over) {
      r.rect(rc.x, y, rc.w, rowH, theme::panelAlt);
    }
    r.text(f, items[idx], rc.x + 6, y + (rowH - r.textH(f)) / 2,
           idx == selected ? theme::text : theme::bone);
    if (over && r.clicked) { selected = idx; outClicked = true; }
  }
  // scrollbar
  if ((int)items.size() > vis) {
    int sbX = rc.x + rc.w - 8;
    float thumbH = (float)rc.h * (float)vis / (float)items.size();
    float thumbY = (float)rc.y + ((float)offset / (float)items.size()) * (float)rc.h;
    r.rect(sbX, rc.y, 8, rc.h, theme::bloodDark);
    r.rect(sbX, (int)thumbY, 8, (int)thumbH, theme::blood);
  }
  return outClicked;
}

} // namespace mine
