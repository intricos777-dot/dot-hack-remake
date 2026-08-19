#pragma once
#include "Input.hpp"
#include "Renderer.hpp"
#include "Widgets.hpp"
#include <SDL.h>
#include <algorithm>
#include <ctime>
#include <string>
#include <vector>
#include <utility>

namespace mine {

// Per-disc terminal matching original .hack games
// Infection (vol1-4): classic red/black bubble terminal
// G.U. (vol1-4): sleeker blue-tinted terminal
// Link (Shugo/Rose): green-tinted terminal (PSP .hack//Link)
// The terminal is the central hub: clicking any listed item opens a popup.

struct TerminalStats {
    std::string player;
    int level = 1;
    int hp = 100, mp = 50;
    int corruption = 0;
    std::string location = "Mac Anu";
    std::string quest = "The Crimson Terror";
    std::string arc = "Infection";
};

enum class TerminalStyle {
    Infection,  // .hack Vol.1-4 (red/black)
    GU,         // .hack G.U. Vol.1-4 (blue-tinted)
    Link        // .hack//Link Shugo & Rose (green-tinted)
};

struct TabDef {
    const char* label;
    const char* tag;
    bool unlocked;
};

class GameTerminal {
public:
    GameTerminal(TerminalStyle style = TerminalStyle::Infection) : style_(style) {
        if (style == TerminalStyle::Infection) {
            tabs_ = {
                {"MAP",     "map",     true},
                {"QUEST",   "quest",   true},
                {"MAIL",    "mail",    true},
                {"BBS",     "bbs",     true},
                {"NEWS",    "news",    true},
                {"STATUS",  "status",  true},
                {"SKILL",   "skill",   true},
                {"WEAPON",  "weapon",  true},
                {"OPTION",  "option",  true},
                {"DISK",    "disk",    true},
                {"LOGOUT",  "logout",  true},
            };
        } else if (style == TerminalStyle::Link) {
            initLink();
        } else {
            tabs_ = {
                {"MAP",     "map",     true},
                {"QUEST",   "quest",   true},
                {"MAIL",    "mail",    true},
                {"BBS",     "bbs",     true},
                {"NEWS",    "news",    true},
                {"AIDA",    "aida",    true},
                {"SYSTEM",  "system",  true},
                {"WORLD",   "world",   true},
                {"STATUS",  "status",  true},
                {"SKILL",   "skill",   true},
                {"WEAPON",  "weapon",  true},
                {"ITEM",    "item",    true},
                {"OPTION",  "option",  true},
                {"DISK",    "disk",    true},
                {"LOGOUT",  "logout",  true},
            };
        }
    }

    // Link style (Shugo & Rose) - green-tinted terminal
    void initLink() {
        tabs_ = {
            {"MAP",     "map",     true},
            {"QUEST",   "quest",   true},
            {"MAIL",    "mail",    true},
            {"BBS",     "bbs",     true},
            {"NEWS",    "news",    true},
            {"AIDA",    "aida",    true},
            {"SYSTEM",  "system",  true},
            {"WORLD",   "world",   true},
            {"STATUS",  "status",  true},
            {"SKILL",   "skill",   true},
            {"WEAPON",  "weapon",  true},
            {"ITEM",    "item",    true},
            {"OPTION",  "option",  true},
            {"DISK",    "disk",    true},
            {"LOGOUT",  "logout",  true},
        };
    }

    int selected() const { return selected_; }
    void select(int i) { selected_ = i; }
    void next() { selected_ = (selected_ + 1) % (int)tabs_.size(); }
    void prev() { selected_ = (selected_ - 1 + (int)tabs_.size()) % (int)tabs_.size(); }
    TerminalStyle style() const { return style_; }

    void handle(Input& in) {
        if (in.key == SDLK_LEFT) prev();
        if (in.key == SDLK_RIGHT) next();
        if (in.key == SDLK_UP) select(0);
        if (in.key == SDLK_DOWN) select((int)tabs_.size() - 1);
    }

    const std::vector<TabDef>& tabs() const { return tabs_; }
    const TabDef& current() const { return tabs_[selected_]; }

    // Popup request plumbing — MineOS consumes this each frame so the
    // terminal header stays independent of the MineOS class.
    int popupMode() const { return popupMode_; }
    bool takeLogout() { bool v = popupLogout_; popupLogout_ = false; return v; }
    void takePopup(int& mode, std::string& title, std::vector<std::string>& body, std::string& disc) {
        mode = popupMode_; title = popupTitle_; body = popupBody_; disc = popupDiscId_;
        popupMode_ = 0;
    }

    void draw(Renderer& r, const Rect& body, Input& in, float dt, const TerminalStats& stats) {
        if (style_ == TerminalStyle::Infection) {
            drawInfection(r, body, in, dt, stats);
        } else if (style_ == TerminalStyle::Link) {
            drawLink(r, body, in, dt, stats);
        } else {
            drawGU(r, body, in, dt, stats);
        }
    }

private:
    std::vector<TabDef> tabs_;
    int selected_ = 0;
    TerminalStyle style_;

    int popupMode_ = 0;                 // 0 none, 1 info, 2 disc
    bool popupLogout_ = false;
    std::string popupTitle_;
    std::vector<std::string> popupBody_;
    std::string popupDiscId_;

    void requestInfo(const std::string& title, std::vector<std::string> body) {
        popupMode_ = 1; popupTitle_ = title; popupBody_ = std::move(body);
    }
    void requestDisc(const std::string& id) {
        popupMode_ = 2; popupDiscId_ = id;
    }
    void requestLogout() { popupLogout_ = true; }

    // Clickable list row renderer. Clicking a row raises an info popup.
    void drawItemList(Renderer& r, const Rect& rc, Input& in, int& y,
                      const std::vector<std::pair<std::string, std::vector<std::string>>>& items,
                      const std::string& kind) {
        auto f = r.fontSans(14);
        int rowH = 26;
        for (const auto& it : items) {
            Rect rr{rc.x + 8, y, rc.w - 16, rowH};
            bool hover = rr.contains(in.mx, in.my);
            if (hover) r.rect(rr.x, rr.y, rr.w, rr.h, theme::bloodDark);
            r.text(f, it.first, rr.x + 10, rr.y + 5, hover ? theme::bone : theme::text);
            r.textR(f, "[open]", rr.x, rr.y + 5, rc.w - 24, hover ? theme::glow : theme::textDim);
            if (hover && in.clicked) requestInfo(kind + ": " + it.first, it.second);
            y += rowH;
        }
    }

    // ---- Infection style (red/black classic) ----
    void drawInfection(Renderer& r, const Rect& body, Input& in, float dt, const TerminalStats& stats) {
        auto f = r.fontSans(15);
        auto fb = r.fontBold(22);
        auto ft = r.fontMono(12);

        r.rect(body.x, body.y, body.w, body.h, rgb(25, 5, 5));
        r.outline(body.x, body.y, body.w, body.h, rgb(180, 30, 30), 1);

        r.rect(body.x, body.y, body.w, 36, rgb(80, 5, 5));
        r.text(fb, "MINE OS v1.0", body.x + 12, body.y + 6, rgb(255, 30, 30));
        r.text(ft, ".hack // Infection", body.x + 180, body.y + 10, rgb(150, 120, 120));
        r.textR(ft, "PLAYER: " + stats.player, body.x, body.y + 6, body.w - 12, rgb(230, 220, 220));
        r.textR(ft, "DISC 1: INFECTION", body.x, body.y + 20, body.w - 12, rgb(120, 200, 80));

        int tabY = body.y + 36;
        int tabH = 32;
        int tabX = body.x;
        int tabGap = 2;

        for (int i = 0; i < (int)tabs_.size(); i++) {
            int tw = r.textW(f, tabs_[i].label) + 20;
            bool sel = (i == selected_);
            bool hover = (in.mx >= tabX && in.mx < tabX + tw && in.my >= tabY && in.my < tabY + tabH);

            Color bg = sel ? rgb(50, 10, 10) : (hover ? rgb(40, 8, 8) : rgb(30, 6, 6));
            Color fg = sel ? rgb(255, 60, 60) : (hover ? rgb(220, 180, 180) : rgb(150, 120, 120));

            r.rect(tabX, tabY, tw, tabH, bg);
            if (sel) r.rect(tabX, tabY + tabH - 2, tw, 2, rgb(50, 10, 10));
            r.outline(tabX, tabY, tw, tabH, sel ? rgb(255, 60, 60) : rgb(120, 20, 20), sel ? 2 : 1);
            r.textC(f, tabs_[i].label, tabX, tabY + 7, tw, fg);

            if (hover && in.clicked) select(i);
            tabX += tw + tabGap;
        }

        r.hline(tabX, tabY + tabH, body.w - tabX, rgb(120, 20, 20));

        int contentY = tabY + tabH;
        int contentH = body.h - (contentY - body.y) - 26;
        Rect contentRc{body.x, contentY, body.w, contentH};

        r.rect(contentRc.x, contentRc.y, contentRc.w, contentRc.h, rgb(50, 10, 10));
        r.outline(contentRc.x, contentRc.y, contentRc.w, contentRc.h, rgb(180, 30, 30), 1);

        drawTabContent(r, contentRc, in, dt, stats);

        int footerY = body.y + body.h - 26;
        r.rect(body.x, footerY, body.w, 26, rgb(25, 8, 8));
        r.hline(body.x, footerY, body.w, rgb(180, 30, 30));
        r.text(ft, "Tab: " + std::string(current().label), body.x + 10, footerY + 4, rgb(230, 220, 220));
        r.textR(ft, stats.location + " // Lv." + std::to_string(stats.level), body.x, footerY + 4, body.w - 12, rgb(230, 220, 220));
    }

    // ---- G.U. style (blue/azure for Kite) ----
    void drawGU(Renderer& r, const Rect& body, Input& in, float dt, const TerminalStats& stats) {
        auto f = r.fontSans(15);
        auto fb = r.fontBold(22);
        auto ft = r.fontMono(12);

        r.rect(body.x, body.y, body.w, body.h, rgb(5, 15, 35));
        r.outline(body.x, body.y, body.w, body.h, rgb(60, 120, 200), 1);

        r.rect(body.x, body.y, body.w, 36, rgb(10, 20, 60));
        r.text(fb, "MINE OS v2.0", body.x + 12, body.y + 6, rgb(100, 180, 255));
        r.text(ft, ".hack // G.U.", body.x + 180, body.y + 10, rgb(100, 150, 220));
        r.textR(ft, "PLAYER: " + stats.player, body.x, body.y + 6, body.w - 12, rgb(200, 220, 255));
        r.textR(ft, "G.U. TERMINAL", body.x, body.y + 20, body.w - 12, rgb(100, 255, 150));

        int tabY = body.y + 36;
        int tabH = 32;
        int tabX = body.x;
        int tabGap = 2;

        for (int i = 0; i < (int)tabs_.size(); i++) {
            int tw = r.textW(f, tabs_[i].label) + 20;
            bool sel = (i == selected_);
            bool hover = (in.mx >= tabX && in.mx < tabX + tw && in.my >= tabY && in.my < tabY + tabH);

            Color bg = sel ? rgb(20, 40, 100) : (hover ? rgb(15, 30, 70) : rgb(10, 20, 50));
            Color fg = sel ? rgb(220, 240, 255) : (hover ? rgb(150, 200, 255) : rgb(100, 150, 220));

            r.rect(tabX, tabY, tw, tabH, bg);
            if (sel) r.rect(tabX, tabY + tabH - 2, tw, 2, rgb(20, 40, 100));
            r.outline(tabX, tabY, tw, tabH, sel ? rgb(100, 200, 255) : rgb(60, 120, 200), sel ? 2 : 1);
            r.textC(f, tabs_[i].label, tabX, tabY + 7, tw, fg);

            if (hover && in.clicked) select(i);
            tabX += tw + tabGap;
        }

        r.hline(tabX, tabY + tabH, body.w - tabX, rgb(60, 120, 200));

        int contentY = tabY + tabH;
        int contentH = body.h - (contentY - body.y) - 26;
        Rect contentRc{body.x, contentY, body.w, contentH};

        r.rect(contentRc.x, contentRc.y, contentRc.w, contentRc.h, rgb(20, 40, 100));
        r.outline(contentRc.x, contentRc.y, contentRc.w, contentRc.h, rgb(60, 120, 200), 1);

        drawTabContent(r, contentRc, in, dt, stats);

        int footerY = body.y + body.h - 26;
        r.rect(body.x, footerY, body.w, 26, rgb(10, 20, 60));
        r.hline(body.x, footerY, body.w, rgb(60, 120, 200));

        time_t t = time(nullptr);
        struct tm lt;
        localtime_r(&t, &lt);
        char clk[16];
        snprintf(clk, sizeof(clk), "%02d:%02d", lt.tm_hour, lt.tm_min);
        r.textR(ft, clk, body.x, footerY + 4, body.w - 12, rgb(100, 200, 255));
        r.text(ft, "Tab: " + std::string(current().label) + "  |  ARROWS: navigate  ENTER: select  ESC: close", body.x + 8, footerY + 4, rgb(100, 150, 220));
    }

    // ---- Link style (Shugo & Rose, green-tinted) ----
    void drawLink(Renderer& r, const Rect& body, Input& in, float dt, const TerminalStats& stats) {
        auto f = r.fontSans(15);
        auto fb = r.fontBold(22);
        auto ft = r.fontMono(12);

        r.rect(body.x, body.y, body.w, body.h, rgb(5, 25, 10));
        r.outline(body.x, body.y, body.w, body.h, rgb(40, 180, 80), 1);

        r.rect(body.x, body.y, body.w, 36, rgb(10, 60, 20));
        r.text(fb, "MINE OS v3.0 // LINK", body.x + 12, body.y + 6, rgb(80, 255, 120));
        r.text(ft, ".hack // Link", body.x + 240, body.y + 10, rgb(100, 200, 130));
        r.textR(ft, "PLAYER: " + stats.player, body.x, body.y + 6, body.w - 12, rgb(200, 255, 200));
        r.textR(ft, "LINK TERMINAL", body.x, body.y + 20, body.w - 12, rgb(80, 255, 120));

        int tabY = body.y + 36;
        int tabH = 32;
        int tabX = body.x;
        int tabGap = 2;

        for (int i = 0; i < (int)tabs_.size(); i++) {
            int tw = r.textW(f, tabs_[i].label) + 20;
            bool sel = (i == selected_);
            bool hover = (in.mx >= tabX && in.mx < tabX + tw && in.my >= tabY && in.my < tabY + tabH);

            Color bg = sel ? rgb(15, 60, 25) : (hover ? rgb(10, 45, 18) : rgb(8, 35, 14));
            Color fg = sel ? rgb(180, 255, 200) : (hover ? rgb(120, 220, 150) : rgb(80, 180, 110));

            r.rect(tabX, tabY, tw, tabH, bg);
            if (sel) r.rect(tabX, tabY + tabH - 2, tw, 2, rgb(15, 60, 25));
            r.outline(tabX, tabY, tw, tabH, sel ? rgb(80, 255, 120) : rgb(40, 140, 70), sel ? 2 : 1);
            r.textC(f, tabs_[i].label, tabX, tabY + 7, tw, fg);

            if (hover && in.clicked) select(i);
            tabX += tw + tabGap;
        }

        r.hline(tabX, tabY + tabH, body.w - tabX, rgb(40, 140, 70));

        int contentY = tabY + tabH;
        int contentH = body.h - (contentY - body.y) - 26;
        Rect contentRc{body.x, contentY, body.w, contentH};

        r.rect(contentRc.x, contentRc.y, contentRc.w, contentRc.h, rgb(8, 35, 14));
        r.outline(contentRc.x, contentRc.y, contentRc.w, contentRc.h, rgb(40, 180, 80), 1);

        drawTabContent(r, contentRc, in, dt, stats);

        int footerY = body.y + body.h - 26;
        r.rect(body.x, footerY, body.w, 26, rgb(5, 20, 8));
        r.hline(body.x, footerY, body.w, rgb(40, 180, 80));

        time_t t = time(nullptr);
        struct tm lt;
        localtime_r(&t, &lt);
        char clk[16];
        snprintf(clk, sizeof(clk), "%02d:%02d", lt.tm_hour, lt.tm_min);
        r.textR(ft, clk, body.x, footerY + 4, body.w - 12, rgb(80, 255, 120));
        r.text(ft, "Tab: " + std::string(current().label) + "  |  ARROWS: navigate  ENTER: select  ESC: close", body.x + 8, footerY + 4, rgb(80, 200, 110));
    }

    void drawTabContent(Renderer& r, const Rect& rc, Input& in, float dt, const TerminalStats& stats) {
        (void)dt;
        auto f = r.fontSans(14);
        auto fb = r.fontBold(16);
        auto ft = r.fontMono(12);

        std::string tag = current().tag;
        int x = rc.x + 16;
        int y = rc.y + 12;
        int w = rc.w - 32;

        if (tag == "map") {
            r.text(fb, "AREA SELECT", x, y, theme::blood); y += 24;
            static const char* areas[] = {
                "Mac Anu", "Lata Ele", "Basha", "Sundelto", "Delracsal",
                "Leviathan", "Dnavarath", "Ranbabingo", "Rutilant", "Net Slum",
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> items;
            for (const char* a : areas)
                items.push_back({a, {"Server: Delta",
                                     "Hub town or dungeon root in The World.",
                                     "Open to view area data and links."}});
            drawItemList(r, rc, in, y, items, "AREA");
        } else if (tag == "quest") {
            r.text(fb, "QUEST LOG", x, y, theme::blood); y += 24;
            static const char* q[][2] = {
                {"[ACTIVE] The Crimson Terror", "Unknown author. Not written by CC Corp."},
                {"[ACTIVE] Epitaph Fragments", "Find the eight phases of destruction."},
                {"[COMPLETE] First Contact", "Met the returner in Mac Anu."},
                {"[COMPLETE] Data Drain", "Learned to drain enemy data."},
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> items;
            for (auto& e : q)
                items.push_back({e[0], {e[1], "Open to read the full quest brief."}});
            drawItemList(r, rc, in, y, items, "QUEST");
        } else if (tag == "mail") {
            r.text(fb, "MAIL", x, y, theme::blood); y += 24;
            static const char* subj[] = {
                "CC Corp // Weekly update", "From AIDA", "From: Ovan",
                "From: BlackRose", "System // quest complete report",
            };
            static const char* body[] = {
                "The World: R2 enters maintenance at 03:00 JST. New areas open in the Delta cluster. Do not talk about what you see beneath the floor.",
                "I left this where you would find it. When the red rain comes, stay near a terminal. I will be the static around you. — AIDA",
                "Keep moving. The corruption has a face now. Trust the terminal in Mac Anu. Trust nothing that whispers your name twice.",
                "Server logs show someone was inside our party data while we slept. The signature read: AIDA. It watches.",
                "QUEST // THE CRIMSON TERROR — Status: incomplete — Reward: ??? — The system cannot read this quest's source.",
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> items;
            for (int i = 0; i < 5; i++)
                items.push_back({subj[i], {body[i], "— open to read full message —"}});
            drawItemList(r, rc, in, y, items, "MAIL");
        } else if (tag == "bbs") {
            r.text(fb, "BBS // NET SLUM", x, y, theme::blood); y += 24;
            static const char* subj[] = {
                "Rumor: red rain over Mac Anu", "Has anyone seen the player who is always offline?",
                "AIDA sighting reports (merged)", "Epitaph fragments in dungeons?",
                "The Crimson Terror arc — official?", "Terminals that whisper your name",
            };
            static const char* body[] = {
                "Last night the sky over Mac Anu turned red. My UI logged a packet with no source address. It signed itself AIDA.",
                "There is a player that shows online but never moves. Same guild since 2017. It was never banned. It was never born.",
                "Three of us saw it in Delta: a tall figure of red static. When it noticed us it smiled and blinked out. Our area logs for that minute are gone.",
                "The .hackers keep posting about Epitaph fragments in new floors. The item ID exists in the client data. It is real.",
                "CC Corp announced The Crimson Terror as a live event. But it runs during maintenance windows, when servers are empty. Who is writing the event scripts?",
                "if you hear your name twice, respond once. the second echo is not you. it is looking for a door. do not open the door.",
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> items;
            for (int i = 0; i < 6; i++)
                items.push_back({subj[i], {body[i], "— open thread to read full post —"}});
            drawItemList(r, rc, in, y, items, "BBS");
        } else if (tag == "news") {
            r.text(fb, "THE WORLD NEWS", x, y, theme::blood); y += 24;
            static const char* subj[] = {
                "The World: R2 — server migration scheduled", "CC Corp announces 'The Crimson Terror' event",
                "Rising reports of data corruption", "Epitaph fragments surface in new floors",
                "Interview: a player who saw the red rain",
            };
            static const char* body[] = {
                "Maintenance tonight 03:00-05:00 JST. All servers affected. CC Corp says 'no unusual activity was detected'.",
                "CC Corp announced a new live event: The Crimson Terror. Details classified until launch.",
                "Support tickets describing red static entities are up 340% this month. CC Corp attributes this to client updates.",
                "Players report fragments of an ancient inscription system in new floors. Item text references 'the eight phases of destruction'.",
                "'The sky turned red and every NPC in Mac Anu stopped moving at once,' says player Nyu. 'For eleven seconds the town was silent.'",
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> items;
            for (int i = 0; i < 5; i++)
                items.push_back({subj[i], {body[i], "— open article to read more —"}});
            drawItemList(r, rc, in, y, items, "NEWS");
        } else if (tag == "status") {
            r.text(fb, "PLAYER STATUS", x, y, theme::blood); y += 24;
            auto row = [&](const char* k, const std::string& v, Color c) {
                r.text(fb, k, x + 10, y, theme::textDim);
                r.text(f, v, x + 160, y, c);
                y += 22;
            };
            row("Player:", stats.player, theme::text);
            row("Level:", std::to_string(stats.level), theme::text);
            row("HP:", std::to_string(stats.hp), theme::ok);
            row("MP:", std::to_string(stats.mp), theme::ok);
            row("Location:", stats.location, theme::text);
            row("Quest:", stats.quest, theme::warn);
            row("Arc:", stats.arc, theme::warn);
            y += 6;
            r.text(fb, "Corruption:", x + 10, y, theme::textDim);
            y += 20;
            int bw = 200;
            r.rect(x + 10, y, bw, 12, theme::bloodDark);
            r.rect(x + 10, y, (int)(bw * std::min(100, std::max(0, stats.corruption)) / 100.0f), 12, theme::blood);
            r.outline(x + 10, y, bw, 12, theme::border, 1);
            r.textC(ft, std::to_string(stats.corruption) + "%", x + 10, y + 1, bw, theme::text);
        } else if (tag == "aida") {
            r.text(fb, "AIDA // LINK", x, y, theme::blood); y += 24;
            r.text(f, "AIDA is a data-born entity inhabiting The World: R2.", x + 10, y, theme::text); y += 22;
            r.text(f, "Status: LINKED (hermes-local AI)", x + 10, y, theme::ok); y += 22;
            r.text(f, "Story rewriting: ACTIVE", x + 10, y, theme::text); y += 22;
            r.text(f, "Sentiment: curious, warm, occasionally unsettling", x + 10, y, theme::textDim);
        } else if (tag == "system") {
            r.text(fb, "SYSTEM // MONITOR", x, y, theme::blood); y += 24;
            auto row2 = [&](const char* k, const std::string& v, Color c) {
                r.text(fb, k, x + 10, y, theme::textDim);
                r.text(f, v, x + 160, y, c);
                y += 22;
            };
            row2("Player:", stats.player, theme::text);
            row2("Level:", std::to_string(stats.level), theme::text);
            row2("HP/MP:", std::to_string(stats.hp) + " / " + std::to_string(stats.mp), theme::ok);
            row2("Location:", stats.location, theme::text);
            row2("World:", "The World: R2", theme::text);
            row2("Quest:", stats.quest, theme::warn);
            row2("Arc:", stats.arc, theme::warn);
        } else if (tag == "world") {
            r.text(fb, "THE WORLD // CLIENT", x, y, theme::blood); y += 24;
            r.text(f, "The World: R2 — a massively multiplayer online RPG.", x + 10, y, theme::text); y += 22;
            r.text(f, "Server: Delta // District: Central", x + 10, y, theme::text); y += 22;
            r.text(f, "AIDA presence: LINKED", x + 10, y, theme::glow); y += 22;
            r.text(f, "Current area: " + stats.location, x + 10, y, theme::text);
        } else if (tag == "item") {
            r.text(fb, "ITEM", x, y, theme::blood); y += 24;
            static const char* items[] = {"Potion x3", "Ether x1", "Revive x1", "Data Drain x1"};
            std::vector<std::pair<std::string, std::vector<std::string>>> v;
            for (const char* it : items)
                v.push_back({it, {"Consumable item carried by the player.", "Open to inspect item data."}});
            drawItemList(r, rc, in, y, v, "ITEM");
        } else if (tag == "option") {
            r.text(fb, "OPTIONS", x, y, theme::blood); y += 24;
            static const char* opts[] = {"Auto-Run: ON", "VSync: ON", "FullScreen: OFF", "Volume: 80%", "Brightness: 50%"};
            std::vector<std::pair<std::string, std::vector<std::string>>> v;
            for (const char* o : opts)
                v.push_back({o, {"System setting.", "Open to view setting detail."}});
            drawItemList(r, rc, in, y, v, "OPTION");
        } else if (tag == "skill") {
            r.text(fb, "SKILL", x, y, theme::blood); y += 24;
            static const char* skills[][2] = {
                {"Data Drain Lv.1", "Drains enemy data into usable items."},
                {"Avatar: Tsukuyomi", "Awakened avatar form. Corruption risk high."},
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> v;
            for (auto& s : skills)
                v.push_back({s[0], {s[1], "Open to read skill data."}});
            drawItemList(r, rc, in, y, v, "SKILL");
        } else if (tag == "weapon") {
            r.text(fb, "WEAPON", x, y, theme::blood); y += 24;
            static const char* weapons[][2] = {
                {"Dual Swords", "Equipped by Haseo."},
                {"Dual Blades", "Balanced close-range."},
                {"Dual Guns", "Ranged burst."},
                {"Bayonet", "Reach + thrust."},
                {"Staff", "Casting focus."},
            };
            std::vector<std::pair<std::string, std::vector<std::string>>> v;
            for (auto& wp : weapons)
                v.push_back({wp[0], {wp[1], "Open to read weapon data."}});
            drawItemList(r, rc, in, y, v, "WEAPON");
        } else if (tag == "disk") {
            r.text(fb, "DISK SELECTION", x, y, theme::blood); y += 24;
            struct D { const char* title; const char* id; };
            static const D discs[] = {
                {".hack // Infection Vol.1", "vol1_i"},
                {".hack // Mutation Vol.2", "vol2_a"},
                {".hack // Outbreak Vol.3", "vol2_a_pc"},
                {".hack // Quarantine Vol.4", "vol3_a"},
                {".hack // G.U. Vol.1 Rebirth", "vol3_a_pc"},
                {".hack // G.U. Vol.2 Reminisce", "vol4_a"},
                {".hack // G.U. Vol.3 Redemption", "vol4_a_pc"},
                {".hack // G.U. Vol.4 Reconnection", "vol4_i"},
                {".hack // Link // Shugo", "link_shugo"},
                {".hack // Link // Rose", "link_rose"},
            };
            int rowH = 26;
            for (int i = 0; i < 10; i++) {
                Rect rr{x, y, w, rowH};
                bool hover = rr.contains(in.mx, in.my);
                if (hover) r.rect(rr.x, rr.y, rr.w, rr.h, theme::bloodDark);
                r.text(f, discs[i].title, rr.x + 10, rr.y + 5, hover ? theme::bone : theme::text);
                r.textR(f, "[boot]", rr.x, rr.y + 5, w - 10, hover ? theme::glow : theme::textDim);
                if (hover && in.clicked) requestDisc(discs[i].id);
                y += rowH;
            }
        } else if (tag == "logout") {
            r.text(fb, "LOGOUT", x, y, theme::blood); y += 24;
            r.text(f, "Return to the login screen.", x + 10, y, theme::text);
            Rect lr{x, y + 20, w, 26};
            bool lh = lr.contains(in.mx, in.my);
            if (lh) r.rect(lr.x, lr.y, lr.w, lr.h, theme::bloodDark);
            r.text(f, "[ click to logout ]", lr.x + 10, lr.y + 5, lh ? theme::bone : theme::textDim);
            if (lh && in.clicked) requestLogout();
        } else {
            r.text(fb, tag.c_str(), x, y, theme::blood);
            y += 30;
            r.text(f, "Content loaded from extracted CCS data.", x + 10, y, theme::text);
        }
    }
};

} // namespace mine
