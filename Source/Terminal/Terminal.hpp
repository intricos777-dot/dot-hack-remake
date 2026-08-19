#include "MineOS.hpp"
#include "Widgets.hpp"
#include <SDL.h>
#include <algorithm>
#include <cmath>
#include <cstdio>
#include <cstring>
#include <ctime>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>

// ---- Map Terminal ----
// scTerminal (scene) — area list with map data from CCS

namespace mine {

// adapted from ccStfRollControl@terminal@gu
struct RollControl {
    int itemH = 24;
    int visible = 0;
    int scroll = 0;
    int maxScroll = 0;

    void setCount(int n, int areaH) {
        visible = (areaH) / itemH;
        maxScroll = std::max(0, n - visible);
        scroll = std::clamp(scroll, 0, maxScroll);
    }
    void wheel(int dir) {
        if (dir > 0) scroll = std::max(0, scroll - 1);
        else if (dir < 0) scroll = std::min(maxScroll, scroll + 1);
    }
    void move(int delta) {
        scroll = std::clamp(scroll + delta, 0, maxScroll);
    }
    int start() const { return scroll; }
    int end(int total) const { return std::min(total, scroll + visible); }
};

} // namespace mine

// ---- Select Box ----
// ccSelectBox + ccMessageBox — modal dialog (ok / cancel / yes / no)

namespace mine {

enum class SelBoxKind { Ok, OkCancel, YesNo, Message };

struct SelectBox {
    SelBoxKind kind = SelBoxKind::Ok;
    std::string title;
    std::string message;
    int choice = 0; // 0 = left/ok/yes, 1 = right/cancel/no
    bool done = false;

    void open(SelBoxKind k, const std::string& t, const std::string& m) {
        kind = k; title = t; message = m; choice = 0; done = false;
    }

    void handle(Input& in) {
        if (done) return;
        if (in.key == SDLK_LEFT || in.key == SDLK_RIGHT) {
            choice = 1 - choice;
        }
        if (in.key == SDLK_RETURN || in.key == SDLK_SPACE) {
            done = true;
        }
        if (in.key == SDLK_ESCAPE) {
            choice = 1; done = true; // cancel
        }
    }

    bool accepted() const {
        if (!done) return false;
        if (kind == SelBoxKind::YesNo) return choice == 0; // yes
        return choice == 0; // ok
    }
};

} // namespace mine

// ---- Option Menu ----
// OptionMenu + ccSysMenuItem + ccSysMenuItemOnOff

namespace mine {

enum class OptKind { Toggle, Value, Back, Action };

struct OptItem {
    OptKind kind = OptKind::Action;
    std::string label;
    bool* bVal = nullptr; // for Toggle
    int* iVal = nullptr;  // for Value
    int iMin = 0, iMax = 100, iStep = 1;
    std::string(*onAction)() = nullptr;
    bool onOffVisual = false; // ccSysMenuItemOnOff
};

struct OptionMenu {
    std::string title;
    std::vector<OptItem> items;
    int selected = 0;
    bool done = false;

    void reset() { selected = 0; done = false; }
    void addToggle(const std::string& l, bool* b, bool onOff = false) {
        items.push_back({OptKind::Toggle, l, b, nullptr, 0, 0, 0, nullptr, onOff});
    }
    void addValue(const std::string& l, int* v, int mn, int mx, int step = 1) {
        items.push_back({OptKind::Value, l, nullptr, v, mn, mx, step, nullptr, false});
    }
    void addAction(const std::string& l, std::string(*fn)()) {
        items.push_back({OptKind::Action, l, nullptr, nullptr, 0, 0, 0, fn, false});
    }
    void addBack(const std::string& l = "Back") {
        items.push_back({OptKind::Back, l, nullptr, nullptr, 0, 0, 0, nullptr, false});
    }

    std::string handle(Input& in) {
        if (done) return "";
        if (in.key == SDLK_UP) selected = std::max(0, selected - 1);
        if (in.key == SDLK_DOWN) selected = std::min((int)items.size() - 1, selected + 1);

        OptItem& it = items[selected];
        if (it.kind == OptKind::Value) {
            if (in.key == SDLK_LEFT || in.key == SDLK_a)
                *it.iVal = std::max(it.iMin, *it.iVal - it.iStep);
            if (in.key == SDLK_RIGHT || in.key == SDLK_d)
                *it.iVal = std::min(it.iMax, *it.iVal + it.iStep);
        }
        if (in.key == SDLK_RETURN || in.key == SDLK_SPACE) {
            if (it.kind == OptKind::Toggle && it.bVal) *it.bVal = !*it.bVal;
            if (it.kind == OptKind::Action && it.onAction) return it.onAction();
            if (it.kind == OptKind::Back) done = true;
        }
        if (in.key == SDLK_ESCAPE) { done = true; }
        return "";
    }
};

} // namespace mine

// ---- Icon Menu (Terminal Desktop) ----
// IconBase + IconFile + IconMenu

namespace mine {

struct TerminalIcon {
    std::string label;
    std::string tag;
    bool enabled = true;
};

struct IconMenu {
    std::vector<TerminalIcon> icons;
    int cols = 4;
    int selected = 0;

    void add(const std::string& label, const std::string& tag, bool en = true) {
        icons.push_back({label, tag, en});
    }

    void handle(Input& in) {
        int n = (int)icons.size();
        if (in.key == SDLK_RIGHT) selected = (selected + 1) % n;
        if (in.key == SDLK_LEFT) selected = (selected - 1 + n) % n;
        if (in.key == SDLK_DOWN) {
            int r = selected / cols, c = selected % cols;
            int nr = (r + 1);
            int ni = nr * cols + c;
            if (ni < n) selected = ni;
        }
        if (in.key == SDLK_UP) {
            int r = selected / cols, c = selected % cols;
            if (r > 0) selected = (r - 1) * cols + c;
        }
        if (in.key == SDLK_RETURN || in.key == SDLK_SPACE) {
            // handled by caller
        }
    }
};

} // namespace mine

// ---- Terminal Sub-View ----
// scTerminal (scene) — Map Terminal area list

namespace mine {

struct AreaNode {
    std::string id;
    std::string name;
    std::string series;
    bool unlocked = true;
    bool instanced = false;
    int levelReq = 1;
};

struct MapTerminal {
    std::vector<AreaNode> areas;
    int selected = 0;
    RollControl roll;
    bool atMacAnu = false;

    void clear() { areas.clear(); selected = 0; }
    void add(const std::string& id, const std::string& name, const std::string& series, bool unlocked = true) {
        areas.push_back({id, name, series, unlocked, false, 1});
    }

    void handle(Input& in, int areaH) {
        roll.setCount((int)areas.size(), areaH);
        if (in.key == SDLK_UP) selected = std::max(0, selected - 1);
        if (in.key == SDLK_DOWN) selected = std::min((int)areas.size() - 1, selected + 1);
        roll.wheel(in.wheel);
        // keep selection visible
        if (selected < roll.start()) roll.move(selected - roll.start());
        if (selected >= roll.end((int)areas.size())) roll.move(selected - roll.end((int)areas.size()) + 1);
    }
};

void RenderTerminalMenu(Renderer& r, const Rect& body, Input& in, IconMenu& menu);
void RenderOptionMenu(Renderer& r, const Rect& body, Input& in, OptionMenu& opt);
void RenderSelectBox(Renderer& r, const Rect& body, SelectBox& box);
void RenderMapTerminal(Renderer& r, const Rect& body, Input& in, MapTerminal& mt);

} // namespace mine
