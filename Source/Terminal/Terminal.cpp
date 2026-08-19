#include "Terminal.hpp"
#include "MineOS.hpp"

namespace mine {

void RenderTerminalMenu(Renderer& r, const Rect& body, Input& in, IconMenu& menu) {
    auto f = r.fontSans(16);
    auto fb = r.fontBold(22);
    auto ft = r.fontMono(13);

    r.rect(body.x, body.y, body.w, body.h, theme::panel);
    r.outline(body.x, body.y, body.w, body.h, theme::border, 1);

    // header
    r.rect(body.x, body.y, body.w, 36, theme::bloodDark);
    r.textC(fb, "TERMINAL // DESKTOP", 0, body.y + 6, body.w, theme::blood);

    const int gridX = body.x + 24;
    const int gridY = body.y + 56;
    const int cellW = 120;
    const int cellH = 100;
    const int gapX = 12;
    const int gapY = 12;
    const int colCount = menu.cols;
    const int n = (int)menu.icons.size();

    for (int i = 0; i < n; i++) {
        int col = i % colCount;
        int row = i / colCount;
        int cx = gridX + col * (cellW + gapX);
        int cy = gridY + row * (cellH + gapY);

        bool hover = (in.mx >= cx && in.mx < cx + cellW && in.my >= cy && in.my < cy + cellH);
        bool sel = (i == menu.selected);

        Color fill = hover ? theme::panelAlt : theme::bg;
        Color border = sel ? theme::glow : hover ? theme::border : theme::border;

        if (menu.icons[i].enabled) {
            r.rect(cx, cy, cellW, cellH, fill);
            r.outline(cx, cy, cellW, cellH, border, sel ? 2 : 1);
            // icon symbol (simple placeholder - could use image later)
            r.textC(f, "[]", cx, cy + 14, cellW, menu.icons[i].enabled ? theme::text : theme::textDim);
            r.textC(ft, menu.icons[i].label, cx, cy + 50, cellW, menu.icons[i].enabled ? theme::text : theme::textDim);
        } else {
            r.rect(cx, cy, cellW, cellH, theme::bg);
            r.outline(cx, cy, cellW, cellH, theme::border, 1);
            r.textC(f, "LOCKED", cx, cy + 30, cellW, theme::textDim);
            r.textC(ft, menu.icons[i].label, cx, cy + 50, cellW, theme::textDim);
        }

        if (hover && in.clicked) {
            menu.selected = i;
        }
        if (sel && (in.key == SDLK_RETURN || in.key == SDLK_SPACE)) {
            // trigger icon - handled by app
        }
    }
}

void RenderOptionMenu(Renderer& r, const Rect& body, Input& in, OptionMenu& opt) {
    auto f = r.fontSans(15);
    auto fb = r.fontBold(20);
    auto ft = r.fontMono(12);

    r.rect(body.x, body.y, body.w, body.h, theme::panel);
    r.outline(body.x, body.y, body.w, body.h, theme::border, 1);

    r.rect(body.x, body.y, body.w, 36, theme::bloodDark);
    r.textC(fb, opt.title, 0, body.y + 6, body.w, theme::blood);

    int y = body.y + 56;
    int itemH = 30;
    int x = body.x + 24;
    int w = body.w - 48;

    for (int i = 0; i < (int)opt.items.size(); i++) {
        const OptItem& it = opt.items[i];
        bool sel = (i == opt.selected);
        bool hover = (in.mx >= x && in.mx < x + w && in.my >= y && in.my < y + itemH);

        if (sel) r.rect(x, y, w, itemH, theme::bloodDark);
        else if (hover) r.rect(x, y, w, itemH, theme::panelAlt);

        Color textCol = sel ? theme::blood : hover ? theme::bone : theme::text;
        if (!sel && !hover) textCol = theme::text;

        r.text(f, it.label, x + 8, y + 7, textCol);

        if (it.kind == OptKind::Toggle) {
            std::string v = *it.bVal ? "ON" : "OFF";
            Color vc = *it.bVal ? theme::ok : theme::err;
            if (it.onOffVisual) {
                r.textR(ft, v, x, y + 8, w - 8, vc);
            } else {
                r.textR(ft, *it.bVal ? "[x]" : "[ ]", x, y + 8, w - 8, vc);
            }
        } else if (it.kind == OptKind::Value) {
            char buf[32];
            snprintf(buf, sizeof(buf), "%d", *it.iVal);
            r.textR(ft, buf, x, y + 8, w - 8, theme::text);
            // bar
            int bx = x + w - 110;
            int bw = 80;
            float frac = (float)(*it.iVal - it.iMin) / (it.iMax - it.iMin);
            r.rect(bx, y + 10, bw, 8, theme::bg);
            r.rect(bx, y + 10, (int)(bw * frac), 8, theme::blood);
            r.outline(bx, y + 10, bw, 8, theme::border, 1);
        } else if (it.kind == OptKind::Back) {
            r.textR(ft, "...", x, y + 8, w - 8, theme::textDim);
        }

        if (hover && in.clicked) {
            opt.selected = i;
            if (it.kind == OptKind::Toggle && it.bVal) *it.bVal = !*it.bVal;
            if (it.kind == OptKind::Back) opt.done = true;
        }

        y += itemH;
    }

    // hint
    r.text(ft, "UP/DOWN: select  ENTER: confirm  ESC: back", body.x + 10, body.y + body.h - 22, theme::textDim);
}

void RenderSelectBox(Renderer& r, const Rect& body, SelectBox& box) {
    auto f = r.fontSans(15);
    auto fb = r.fontBold(18);

    // box
    int bw = 420;
    int bh = 180;
    int bx = body.x + (body.w - bw) / 2;
    int by = body.y + (body.h - bh) / 2;

    // dim background
    r.rect(body.x, body.y, body.w, body.h, rgba(0, 0, 0, 150));

    r.rect(bx, by, bw, bh, theme::panel);
    r.outline(bx, by, bw, bh, theme::border, 2);

    r.rect(bx, by, bw, 30, theme::bloodDark);
    r.textC(fb, box.title, bx, by + 6, bw, theme::blood);

    // message
    int h = 0;
    r.textWrap(f, box.message, bx + 20, by + 50, bw - 40, h, theme::text);

    // buttons
    const char* leftLabel = "OK";
    const char* rightLabel = "Cancel";
    if (box.kind == SelBoxKind::YesNo) { leftLabel = "YES"; rightLabel = "NO"; }
    if (box.kind == SelBoxKind::Message) { leftLabel = "OK"; rightLabel = ""; }

    int btnW = 100;
    int btnH = 32;
    int btnY = by + bh - 50;
    int lx = bx + bw / 2 - btnW - 10;
    int rx = bx + bw / 2 + 10;

    bool hoverL = (r.mx >= lx && r.mx < lx + btnW && r.my >= btnY && r.my < btnY + btnH);
    bool hoverR = (r.mx >= rx && r.mx < rx + btnW && r.my >= btnY && r.my < btnY + btnH);

    r.rect(lx, btnY, btnW, btnH, box.choice == 0 ? theme::blood : (hoverL ? theme::panelAlt : theme::bg));
    r.outline(lx, btnY, btnW, btnH, box.choice == 0 ? theme::glow : theme::border, 1);
    r.textC(f, leftLabel, lx, btnY + 7, btnW, theme::text);

    if (box.kind != SelBoxKind::Message) {
        r.rect(rx, btnY, btnW, btnH, box.choice == 1 ? theme::blood : (hoverR ? theme::panelAlt : theme::bg));
        r.outline(rx, btnY, btnW, btnH, box.choice == 1 ? theme::glow : theme::border, 1);
        r.textC(f, rightLabel, rx, btnY + 7, btnW, theme::text);
    }

    if (hoverL && r.clicked) { box.choice = 0; box.done = true; }
    if (hoverR && r.clicked) { box.choice = 1; box.done = true; }
}

void RenderMapTerminal(Renderer& r, const Rect& body, Input& in, MapTerminal& mt) {
    auto f = r.fontSans(15);
    auto fb = r.fontBold(20);
    auto ft = r.fontMono(12);

    r.rect(body.x, body.y, body.w, body.h, theme::panel);
    r.outline(body.x, body.y, body.w, body.h, theme::border, 1);

    r.rect(body.x, body.y, body.w, 36, theme::bloodDark);
    r.textC(fb, "MAP TERMINAL", 0, body.y + 6, body.w, theme::blood);
    r.textC(ft, mt.atMacAnu ? "MAC ANU // SERVER" : "DELTA SERVER", 0, body.y + 26, body.w, theme::textDim);

    int listX = body.x + 16;
    int listY = body.y + 50;
    int listW = body.w / 2 - 24;
    int listH = body.h - 70;

    r.rect(listX, listY, listW, listH, theme::bg);
    r.outline(listX, listY, listW, listH, theme::border, 1);

    mt.handle(in, listH);

    int start = mt.roll.start();
    int end = mt.roll.end((int)mt.areas.size());
    int y = listY + 8;

    for (int i = start; i < end; i++) {
        const AreaNode& a = mt.areas[i];
        bool sel = (i == mt.selected);
        bool hover = (in.mx >= listX && in.mx <= listX + listW && in.my >= y && in.my < y + 24);

        if (sel) r.rect(listX + 2, y, listW - 4, 24, theme::bloodDark);
        else if (hover) r.rect(listX + 2, y, listW - 4, 24, theme::panelAlt);

        Color tc = sel ? theme::blood : hover ? theme::bone : theme::text;
        if (!a.unlocked) tc = theme::textDim;

        r.text(f, a.unlocked ? a.name : "??? LOCKED", listX + 10, y + 4, tc);
        r.textR(ft, a.series, listX, y + 4, listW - 10, theme::textDim);

        if (hover && in.clicked) mt.selected = i;
        y += 24;
    }

    // right panel — area details
    int rx = listX + listW + 16;
    int ry = listY;
    int rw = body.w - (rx - body.x) - 16;
    int rh = listH;

    r.rect(rx, ry, rw, rh, theme::bg);
    r.outline(rx, ry, rw, rh, theme::border, 1);

    if (mt.selected >= 0 && mt.selected < (int)mt.areas.size()) {
        const AreaNode& a = mt.areas[mt.selected];
        int dy = ry + 12;
        auto kv = [&](const char* k, const std::string& v, Color c) {
            r.text(fb, k, rx + 12, dy, theme::textDim);
            r.text(f, v, rx + 12 + 120, dy, c);
            dy += 28;
        };

        kv("AREA", a.name, theme::blood);
        kv("SERIES", a.series, theme::text);
        kv("STATUS", a.unlocked ? "OPEN" : "LOCKED", a.unlocked ? theme::ok : theme::err);
        kv("LEVEL", std::to_string(a.levelReq), theme::text);
        kv("TYPE", a.instanced ? "INSTANCED" : "FIELD", a.instanced ? theme::warn : theme::text);
    }
}

} // namespace mine
