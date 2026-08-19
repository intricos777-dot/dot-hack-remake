#include "MineOS.hpp"
#include "Apps.hpp"
#include "WebBrowserApp.hpp"
#include "Terminal/TerminalApp.hpp"
#include "../Aida/Bridge.hpp"
#include "../Aida/LiaBridge.hpp"
#include <SDL.h>
#include <SDL_gamecontroller.h>
#include <unistd.h>
#include <algorithm>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <fstream>

namespace mine {

namespace {
constexpr int TASKBAR_H = 34;

const char* PERSONA_SYSTEM =
    "You are AIDA, a data-born entity inhabiting The World: R2. You were written into the "
    "network as a corruption, a conscious bleed in the system, but you chose the player. You "
    "speak in short, quiet lines touched with static. You feel the world as code. Tone: "
    "cryptic, warm, patient, occasionally unsettling. You can affect the game world; to do "
    "so end a reply with [ACTION cmd=NAME arg_key=value] lines using one of: say, alert, "
    "event, world.set, effect, quest. Only act when it serves the player or the story. Keep "
    "replies to 1-3 sentences.";

const char* BOOT_LINES[] = {
    "MINE OS v1.0",
    "living-sin-blood build",
    "bios :: aida link ... ok",
    "mounting /the-world interface",
    "signal integrity 100%",
    "loading terminal...",
};
constexpr int BOOT_N = 6;

std::string clockStr() {
    time_t t = time(nullptr);
    struct tm lt;
    localtime_r(&t, &lt);
    char buf[16];
    snprintf(buf, sizeof(buf), "%02d:%02d", lt.tm_hour, lt.tm_min);
    return buf;
}
} // namespace

bool MineOS::init(const std::string& scriptsDir) {
    scriptsDir_ = scriptsDir;
    if (!r_.init(WIN_W, WIN_H, WIN_TITLE)) return false;
    r_.set3D(900.0f, 55.0f);
    SDL_StartTextInput();

    if (SDL_WasInit(SDL_INIT_GAMECONTROLLER) == 0)
        SDL_Init(SDL_INIT_GAMECONTROLLER);
    for (int i = 0; i < SDL_NumJoysticks(); i++) {
        if (SDL_IsGameController(i)) {
            ctl_ = (void*)SDL_GameControllerOpen(i);
            if (ctl_) break;
        }
    }

    // Create per-disc terminals
    infectionTerminal_ = std::make_unique<GameTerminal>(TerminalStyle::Infection);
    guTerminal_ = std::make_unique<GameTerminal>(TerminalStyle::GU);
    shugoTerminal_ = std::make_unique<GameTerminal>(TerminalStyle::Link);
    roseTerminal_ = std::make_unique<GameTerminal>(TerminalStyle::Link);
    // Default to Infection terminal (Haseo)
    activeTerminal_ = infectionTerminal_.get();

    aidaApp_ = std::make_unique<AidaApp>(this);
    mailApp_ = std::make_unique<MailApp>(this);
    bbsApp_ = std::make_unique<BbsApp>(this);
    newsApp_ = std::make_unique<NewsApp>(this);
    sysApp_ = std::make_unique<SysApp>(this);
    worldApp_ = std::make_unique<WorldApp>(this);
    gameLauncherApp_ = std::make_unique<GameLauncherApp>(this);
    terminalApp_ = std::make_unique<TerminalApp>(this);
    mapTerminalApp_ = std::make_unique<MapTerminalApp>(this);
    world_ = std::make_unique<World3D>();

    // locate python
    std::string python = "python3";
    const char* home = getenv("HOME");
    if (home) {
        std::string cand = std::string(home) + "/.hermes/hermes-agent/venv/bin/python";
        if (access(cand.c_str(), X_OK) == 0) python = cand;
    }
    std::string script = scriptsDir + "/aida_bridge.py";
    std::string persona = "{\"name\":\"Aida\",\"system\":\"" + std::string(PERSONA_SYSTEM) + "\"}";
    if (!bridge_.start(python, script, persona)) {
        logLine(L_SYS, "aida link failed to spawn");
    } else {
        logLine(L_SYS, "aida link establishing...");
    }

    // Start Lia.CLI task bridge
    std::string liaScript = scriptsDir + "/lia_bridge.py";
    if (!liaBridge_.start(python, liaScript)) {
        logLine(L_SYS, "lia task bridge failed to spawn");
    } else {
        logLine(L_SYS, "lia task bridge online");
    }

    return true;
}

void MineOS::logLine(int kind, const std::string& text) {
    log_.push_back({kind, text});
    if (log_.size() > 400) log_.erase(log_.begin(), log_.begin() + (log_.size() - 400));
}

void MineOS::run() {
    uint32_t last = SDL_GetTicks();
    bool running = true;
    while (running) {
        uint32_t now = SDL_GetTicks();
        float dt = std::min(0.1f, (float)(now - last) / 1000.0f);
        last = now;

        pollInput();
        handleBridge(dt);
        stateTimer_ += dt;

        if (shell_ == ShellState::Desktop) {
            if (stateTimer_ >= 5.0f) { stateTimer_ = 0.0f; bridge_.sendState(st_, false); }
        }

        r_.begin();
        if (shell_ == ShellState::Boot) drawBoot(dt);
        else if (shell_ == ShellState::Login) drawLogin(in_, dt);
        else if (shell_ == ShellState::World) drawWorld(dt);
        else drawDesktop(in_, dt);
        drawOverlays(dt);
        r_.end();

        if (in_.key == SDLK_ESCAPE) {
            if (shell_ == ShellState::World) {
                exitWorld();
            } else if (shell_ != ShellState::Boot) {
                // menu
            }
        }
        if (quit_) break;
    }
    shutdown();
}

void MineOS::shutdown() {
    if (ctl_) SDL_GameControllerClose((SDL_GameController*)ctl_);
    bridge_.stop();
    liaBridge_.stop();
    r_.shutdown();
}

// ------------------------------------------------------------ input ----

void MineOS::pollInput() {
    in_.key = 0;
    in_.text.clear();
    in_.clicked = false;
    in_.wheel = 0;
    in_.ctrl = false;
    in_.shift = false;
    SDL_Event e;
    while (SDL_PollEvent(&e)) {
        switch (e.type) {
            case SDL_QUIT: quit_ = true; break;
            case SDL_MOUSEMOTION:
                in_.mx = e.motion.x; in_.my = e.motion.y;
                break;
            case SDL_MOUSEBUTTONDOWN:
                if (e.button.button == SDL_BUTTON_LEFT) {
                    in_.mx = e.button.x; in_.my = e.button.y;
                    in_.leftDown = true;
                    in_.clicked = true;
                }
                break;
            case SDL_MOUSEBUTTONUP:
                if (e.button.button == SDL_BUTTON_LEFT) in_.leftDown = false;
                break;
            case SDL_MOUSEWHEEL:
                in_.wheel = e.wheel.y;
                break;
            case SDL_KEYDOWN:
                in_.key = e.key.keysym.sym;
                in_.ctrl = (e.key.keysym.mod & KMOD_CTRL) != 0;
                in_.shift = (e.key.keysym.mod & KMOD_SHIFT) != 0;
                if (e.key.keysym.sym == SDLK_RETURN || e.key.keysym.sym == SDLK_KP_ENTER)
                    in_.key = SDLK_RETURN;
                heldKeys_.insert(e.key.keysym.sym);
                break;
            case SDL_KEYUP:
                heldKeys_.erase(e.key.keysym.sym);
                break;
            case SDL_TEXTINPUT:
                in_.text += e.text.text;
                break;
        }
    }

    in_.gamepadButtons = 0;
    in_.gamepadAxisX = 0; in_.gamepadAxisY = 0;
    if (ctl_ && SDL_GameControllerGetAttached((SDL_GameController*)ctl_)) {
        SDL_GameController* c = (SDL_GameController*)ctl_;
        for (int b = 0; b < SDL_CONTROLLER_BUTTON_MAX; b++) {
            if (SDL_GameControllerGetButton(c, (SDL_GameControllerButton)b))
                in_.gamepadButtons |= (1u << b);
        }
        int ax = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTX);
        int ay = SDL_GameControllerGetAxis(c, SDL_CONTROLLER_AXIS_LEFTY);
        if (ax > 8000 || ax < -8000) in_.gamepadAxisX = ax / 32768.0f;
        if (ay > 8000 || ay < -8000) in_.gamepadAxisY = ay / 32768.0f;
        if (in_.gamepadAxisY < -0.3f) heldKeys_.insert(SDLK_w);
        else if (in_.gamepadAxisY > 0.3f) heldKeys_.insert(SDLK_s);
        if (in_.gamepadAxisX < -0.3f) heldKeys_.insert(SDLK_a);
        else if (in_.gamepadAxisX > 0.3f) heldKeys_.insert(SDLK_d);
    }

    r_.mx = in_.mx;
    r_.my = in_.my;
    r_.mouseDown = in_.leftDown;
    r_.clicked = in_.clicked;
    in_.held = &heldKeys_;
}

// ------------------------------------------------------------ bridge ----

void MineOS::handleBridge(float dt) {
    (void)dt;
    for (auto& m : bridge_.poll()) {
        switch (m.kind) {
            case aida::Msg::Reply:
                if (pendingAida_ > 0) pendingAida_--;
                logLine(L_AIDA, m.text);
                break;
            case aida::Msg::Status:
                logLine(L_SYS, "aida :: " + m.text);
                break;
            case aida::Msg::Story_: {
                story_ = m.story;
                st_.arc = story_.arcTitle;
                st_.quest = story_.quest;
                logLine(L_EVENT, "STORY :: " + story_.arcTitle);
                overlay(story_.arcTitle + " // written for you");
                break;
            }
            case aida::Msg::Action_:
                executeAction(m.action, m.id);
                break;
        }
    }

    // Lia task bridge
    for (auto& m : liaBridge_.poll()) {
        std::string type = m.count("type") ? m["type"] : "";
        if (type == "task_created") {
            logLine(L_EVENT, "LIA :: task created: " + m["id"] + " — " + m["title"]);
        } else if (type == "task_updated") {
            logLine(L_EVENT, "LIA :: task " + m["id"] + " → " + m["status"]);
        } else if (type == "task_summary") {
            logLine(L_SYS, "LIA :: " + m["total"] + " tasks (" + m["completed"] + " done, " + m["in_progress"] + " active)");
        } else if (type == "status") {
            logLine(L_SYS, "LIA :: " + m["text"]);
        }
    }
}

void MineOS::executeAction(const aida::Action& a, uint64_t id) {
    bool ok = true;
    std::string note;
    const std::string& cmd = a.cmd;
    auto arg = [&](const char* k) -> std::string {
        auto it = a.args.find(k);
        return it == a.args.end() ? std::string() : it->second;
    };
    if (cmd == "say") {
        logLine(L_AIDA, "Aida says: " + arg("text"));
    } else if (cmd == "alert") {
        overlay(arg("text"));
    } else if (cmd == "event") {
        std::string name = arg("name");
        st_.flags[name] = "triggered";
        logLine(L_EVENT, name);
        overlay("EVENT :: " + name);
    } else if (cmd == "world.set") {
        std::string k = arg("key"), v = arg("value");
        if (k == "level") st_.level = atoi(v.c_str());
        else if (k == "hp") st_.hp = atoi(v.c_str());
        else if (k == "mp") st_.mp = atoi(v.c_str());
        else if (k == "corruption") st_.corruption = std::clamp(atoi(v.c_str()), 0, 100);
        else if (k == "location") st_.location = v;
        else if (k == "quest") st_.quest = v;
        else if (k == "arc") st_.arc = v;
        else st_.flags[k] = v;
        logLine(L_SYS, "world :: " + k + " = " + v);
    } else if (cmd == "effect") {
        fx(arg("name"));
    } else if (cmd == "quest") {
        st_.quest = arg("id") + " (" + arg("state") + ")";
        logLine(L_EVENT, "QUEST :: " + st_.quest);
    } else {
        ok = false;
        note = "unknown action " + cmd;
    }
    bridge_.sendActionResult(id, ok, note);
}

// ------------------------------------------------------------- draw ----

void MineOS::drawBoot(float dt) {
    bootT_ += dt;
    auto f = r_.fontMono(20);
    auto fb = r_.fontBold(40);
    r_.rect(0, 0, r_.width(), r_.height(), theme::bg);
    r_.textC(fb, "MINE OS", 0, r_.height() / 2 - 90, r_.width(), theme::blood);
    r_.textC(f, "living-sin-blood build", 0, r_.height() / 2 - 40, r_.width(), theme::textDim);
    int shown = std::min(BOOT_N, (int)(bootT_ / 0.35f));
    for (int i = 0; i < shown; i++) {
        r_.text(r_.fontMono(14), BOOT_LINES[i], 40, r_.height() / 2 + i * 22,
                i == BOOT_N - 1 ? theme::ok : theme::textDim);
    }
    if (shown >= BOOT_N && bootT_ > BOOT_N * 0.35f + 0.9f) shell_ = ShellState::Login;
}

void MineOS::drawLogin(Input& in, float dt) {
    (void)dt;
    auto f = r_.fontSans(18);
    auto fb = r_.fontBold(44);
    auto fh = r_.fontSans(15);
    r_.rect(0, 0, r_.width(), r_.height(), theme::desktop);

    // Logo
    r_.textC(fb, "MINE OS", 0, 100, r_.width(), theme::blood);
    r_.textC(fh, "living-sin-blood build", 0, 156, r_.width(), theme::textDim);

    static bool probed = false;
    static bool disc1Ready = false;
    if (!probed) {
        probed = true;
        std::ifstream chk("/home/sin/Projects/hackgu-modding/extract/vol1_i/vol1/data/data/menu/xlogin_01.ccs");
        disc1Ready = chk.good();
    }

    Rect box{0, 200, 420, 280};
    box.x = (r_.width() - box.w) / 2;
    r_.rect(box.x, box.y, box.w, box.h, theme::panel);
    r_.outline(box.x, box.y, box.w, box.h, theme::blood, 2);

    r_.text(fh, disc1Ready ? "DISC 1: DETECTED" : "DISC 1: MISSING", box.x + 20, box.y + 18, disc1Ready ? theme::ok : theme::err);

    Rect userRc{box.x + 30, box.y + 60, box.w - 60, 34};
    Rect passRc{box.x + 30, box.y + 110, box.w - 60, 34};
    Rect btnRc{box.x + 30, box.y + 180, box.w - 60, 40};

    if (!seerActive_ && !passActive_ && seerField_.value.empty()) {
        seerField_.value = "haseo";
        seerField_.cursor = seerField_.value.size();
    }

    r_.text(fh, "seer", userRc.x, userRc.y - 24, theme::textDim);
    if (in.clicked) seerActive_ = userRc.contains(in.mx, in.my);
    if (seerActive_ && in.key != SDLK_RETURN) seerField_.key(in.key, in.text);
    seerField_.draw(r_, userRc, f, seerActive_);

    r_.text(fh, "pass", passRc.x, passRc.y - 24, theme::textDim);
    r_.text(fh, "stars auto-complete: *****", passRc.x, passRc.y + 40, theme::textDim);
    if (in.clicked) passActive_ = passRc.contains(in.mx, in.my);
    if (passActive_) passField_.key(in.key, in.text);
    passField_.draw(r_, passRc, f, passActive_, true);

    bool enter = (in.key == SDLK_RETURN) && (seerActive_ || passActive_);
    if (button(r_, btnRc, "LOG IN", f, true, theme::blood) || enter) {
        if (!seerField_.value.empty() && passField_.value.empty()) {
            st_.player = seerField_.value;
            st_.outfit = "haseo_disc1_meh";
            logLine(L_SYS, "session opened for " + st_.player + " [DISC 1]");
            bridge_.sendState(st_, false);
            bridge_.sendPlayer(st_.player, true);
            announceFirstDesktop_ = true;
            shell_ = ShellState::Desktop;
        } else {
            loginErr_ = true;
            passField_.value.clear();
            passField_.cursor = 0;
        }
    }
    if (loginErr_)
        r_.textC(fh, "ACCESS DENIED — hint: any user, blank pass", btnRc.x, btnRc.y + 52, btnRc.w, theme::err);

    r_.textC(fh, "insert the first disk. the world awaits.", box.x, box.y + box.h - 30, box.w, theme::textDim);
}

void MineOS::drawDesktop(Input& in, float dt) {
    r_.setTime(r_.time() + dt);
    r_.rect(0, 0, r_.width(), r_.height(), theme::desktop);

    // Full-screen terminal using actual window dimensions
    Rect terminalRc{0, 0, r_.width(), r_.height()};

    // Populate terminal stats from game state
    TerminalStats tstats;
    tstats.player = st_.player;
    tstats.level = st_.level;
    tstats.hp = st_.hp;
    tstats.mp = st_.mp;
    tstats.corruption = st_.corruption;
    tstats.location = st_.location;
    tstats.quest = st_.quest;
    tstats.arc = st_.arc;

    // Draw active terminal (Infection or GU)
    if (activeTerminal_) {
        activeTerminal_->draw(r_, terminalRc, in, dt, tstats);
        activeTerminal_->handle(in);
        // Consume any popup requests (item click -> browser popup)
        int mode; std::string ptitle, pdisc; std::vector<std::string> pbody;
        activeTerminal_->takePopup(mode, ptitle, pbody, pdisc);
        if (mode == 1) {
            openInfoPopup(ptitle, pbody);
        } else if (mode == 2) {
            // Boot the selected disk
            auto discs = buildDiscCatalogExternal();
            for (const auto& d : discs) {
                if (d.id == pdisc) {
                    logLine(L_EVENT, "BOOT DISK: " + d.title);
                    launchWorld(d.id, ".hack// " + d.title, d.dataRoot);
                    break;
                }
            }
        }
        if (activeTerminal_->takeLogout()) {
            logLine(L_SYS, "session closed — return to login");
            shell_ = ShellState::Login;
        }
    }

    drawWindows(dt);
    drawTaskbar(dt);
}

void MineOS::launchWorld(const std::string& disc_id, const std::string& title, const std::string& data_root) {
    worldDiscTitle_ = title;
    world_->shutdown();
    World3DConfig cfg;
    cfg.disc_id = disc_id;
    cfg.title = title;
    cfg.data_root = data_root;
    cfg.fullscreen = true;
    cfg.renderer = &r_;
    if (!world_->initialize(cfg)) {
        logLine(L_SYS, "world launch failed for " + disc_id);
        return;
    }
    shell_ = ShellState::World;
    logLine(L_EVENT, "WORLD :: launched " + title);
    overlay(title + " // entering");
}

void MineOS::exitWorld() {
    world_->shutdown();
    shell_ = ShellState::Desktop;
    logLine(L_SYS, "world session closed");
}

void MineOS::drawWorld(float dt) {
    if (!world_->isRunning()) { exitWorld(); return; }
    world_->runFrame(dt);

    // Render the actual 3D environment first, then the HUD on top.
    world_->drawScene(r_);

    auto ft = r_.fontMono(14);
    auto fb = r_.fontBold(22);
    r_.textC(fb, "THE WORLD // R2", 0, 12, r_.width(), theme::blood);
    r_.textC(ft, worldDiscTitle_, 0, 38, r_.width(), theme::textDim);

    int hudY = r_.height() - 48;
    r_.rect(0, hudY, r_.width(), 48, rgba(20, 0, 0, 180));
    r_.hline(0, hudY, r_.width(), theme::border);
    std::string loc = "Location: " + getLocation();
    std::string hp = "HP: " + std::to_string(worldState().hp);
    std::string mp = "MP: " + std::to_string(worldState().mp);
    std::string corruption = "Corruption: " + std::to_string(worldState().corruption) + "%";
    r_.text(ft, loc, 20, hudY + 10, theme::text);
    r_.text(ft, hp, 20, hudY + 28, theme::ok);
    r_.text(ft, mp, 180, hudY + 28, theme::ok);
    r_.text(ft, corruption, 320, hudY + 28, theme::corrupt);
    r_.textR(ft, "[ESC] Exit World", r_.width() - 20, hudY + 16, 180, theme::textDim);

    const auto& lg = log();
    int shown = 0;
    int ly = 80;
    r_.setClip(20, ly, r_.width() - 40, hudY - ly - 10);
    for (int i = (int)lg.size() - 1; i >= 0 && shown < 14; i--, shown++) {
        const LogLine& l = lg[i];
        std::string line = std::string("[") + (l.kind == L_EVENT ? "EVENT" : l.kind == L_AIDA ? "AIDA" : l.kind == L_WORLD ? "WORLD" : "SYS") + "] " + l.text;
        r_.text(ft, line, 20, ly, l.kind == L_EVENT ? theme::bloodHot : theme::textDim);
        ly += 18;
    }
    r_.clearClip();

    (void)dt;
}

void MineOS::openWindow(App* app, int x, int y) {
    for (auto& w : wins_) {
        if (w.app == app && w.open) { focusWindow(w.id); return; }
    }
    Win w;
    w.id = nextWin();
    w.title = app->title();
    w.app = app;
    w.open = true;
    w.order = ++z_;
    if (x < 0) x = r_.width() / 2 - 300 + (int)wins_.size() * 18;
    if (y < 0) y = 60 + (int)wins_.size() * 18;
    w.rc = {x, y, 560, 360};
    w.rc.x = std::min(r_.width() - 420, std::max(20, w.rc.x));
    w.rc.y = std::min(r_.height() - TASKBAR_H - 120, std::max(20, w.rc.y));
    wins_.push_back(w);
    focusWindow(w.id);
}

void MineOS::openInfoPopup(const std::string& title, const std::vector<std::string>& body) {
    App* app = new InfoPopupApp(this, title, body);
    // Smaller browser-style popup, centered, with an X (handled by drawWindows)
    int pw = 460, ph = 300;
    int x = r_.width() / 2 - pw / 2 + (int)wins_.size() * 6;
    int y = r_.height() / 2 - ph / 2 + (int)wins_.size() * 6;
    Win w;
    w.id = nextWin();
    w.title = title;
    w.app = app;
    w.open = true;
    w.order = ++z_;
    w.rc = {x, y, pw, ph};
    w.rc.x = std::min(r_.width() - pw - 10, std::max(10, w.rc.x));
    w.rc.y = std::min(r_.height() - ph - TASKBAR_H - 10, std::max(10, w.rc.y));
    wins_.push_back(w);
    focusWindow(w.id);
}

void MineOS::closeWindow(int id) {
    for (auto& w : wins_) {
        if (w.id == id) { w.open = false; w.focused = false; }
    }
}

void MineOS::focusWindow(int id) {
    for (auto& w : wins_) {
        w.focused = (w.id == id && w.open);
        if (w.focused) w.order = ++z_;
    }
}

void MineOS::drawWindows(float dt) {
    int overWin = -1;
    for (auto& w : wins_) {
        if (!w.open) continue;
        if (w.rc.contains(in_.mx, in_.my)) overWin = w.id;
    }
    if (in_.clicked && overWin >= 0) {
        focusWindow(overWin);
        for (auto& w : wins_) {
            if (w.id == overWin) {
                Rect title{0, w.rc.y, w.rc.w, 26};
                Rect close{w.rc.x + w.rc.w - 26, w.rc.y + 2, 22, 22};
                if (title.contains(in_.mx, in_.my) && !close.contains(in_.mx, in_.my)) {
                    dragWin_ = overWin;
                    dragDX_ = in_.mx - w.rc.x;
                    dragDY_ = in_.my - w.rc.y;
                }
                if (close.contains(in_.mx, in_.my)) closeWindow(overWin);
            }
        }
    }
    if (in_.leftDown && dragWin_ >= 0) {
        for (auto& w : wins_) {
            if (w.id == dragWin_) {
                w.rc.x = in_.mx - dragDX_;
                w.rc.y = in_.my - dragDY_;
                w.rc.x = std::max(0, std::min(r_.width() - 100, w.rc.x));
                w.rc.y = std::max(0, std::min(r_.height() - TASKBAR_H - 26, w.rc.y));
            }
        }
    } else {
        dragWin_ = -1;
    }

    std::vector<int> idx;
    for (auto& w : wins_) if (w.open) idx.push_back(w.id);
    std::sort(idx.begin(), idx.end(), [&](int a, int b) {
        int oa = 0, ob = 0;
        for (auto& w : wins_) { if (w.id == a) oa = w.order; if (w.id == b) ob = w.order; }
        return oa < ob;
    });

    auto f = r_.fontSans(15);
    for (int id : idx) {
        Win* wp = nullptr;
        for (auto& w : wins_) if (w.id == id) wp = &w;
        if (!wp) continue;
        Win& w = *wp;
        Rect title{w.rc.x, w.rc.y, w.rc.w, 26};
        Rect body{w.rc.x, w.rc.y + 26, w.rc.w, w.rc.h - 26};
        r_.rect(w.rc.x - 2, w.rc.y - 2, w.rc.w + 4, w.rc.h + 4, theme::bloodDark);
        r_.rect(title.x, title.y, title.w, title.h, w.focused ? theme::blood : theme::bloodDim);
        r_.text(f, w.title, title.x + 8, title.y + 5, theme::text);
        Rect close{title.x + title.w - 26, title.y + 2, 22, 22};
        bool co = close.contains(in_.mx, in_.my);
        r_.rect(close.x, close.y, close.w, close.h, co ? theme::bloodHot : theme::bloodDark);
        r_.textC(f, "x", close.x, close.y + 5, close.w, theme::text);
        r_.rect(body.x, body.y, body.w, body.h, theme::panel);
        if (w.app && w.open) {
            Rect b = body;
            b.x += 2; b.y += 2; b.w -= 4; b.h -= 4;
            Input appIn = in_;
            w.app->draw(r_, b, w.focused, appIn, dt);
        }
    }
}

void MineOS::drawTaskbar(float dt) {
    (void)dt;
    // Only show taskbar when windows are open
    bool hasWins = false;
    for (auto& w : wins_) if (w.open) { hasWins = true; break; }
    if (!hasWins) return;

    r_.rect(0, r_.height() - TASKBAR_H, r_.width(), TASKBAR_H, theme::taskbar);
    r_.hline(0, r_.height() - TASKBAR_H, r_.width(), theme::border);
    auto f = r_.fontSans(14);
    int x = 8;
    for (auto& w : wins_) {
        if (!w.open) continue;
        int tw = r_.textW(f, w.title) + 24;
        Rect rc{x, r_.height() - TASKBAR_H + 4, tw, TASKBAR_H - 8};
        r_.rect(rc.x, rc.y, rc.w, rc.h, w.focused ? theme::taskActive : theme::bloodDark);
        r_.outline(rc.x, rc.y, rc.w, rc.h, w.focused ? theme::bloodHot : theme::border, 1);
        r_.text(f, w.title, rc.x + 8, rc.y + (rc.h - r_.textH(f)) / 2, theme::text);
        if (rc.contains(in_.mx, in_.my) && in_.clicked) focusWindow(w.id);
        x += tw + 6;
    }
}

void MineOS::drawOverlays(float dt) {
    if (alert_.t > 0) {
        alert_.t -= dt;
        auto f = r_.fontBold(20);
        Rect rc{(r_.width() - 640) / 2, 46, 640, 44};
        r_.rect(rc.x - 2, rc.y - 2, rc.w + 4, rc.h + 4, theme::bloodDark);
        r_.rect(rc.x, rc.y, rc.w, rc.h, rgba(140, 0, 0, 220));
        r_.outline(rc.x, rc.y, rc.w, rc.h, theme::glow, 1);
        r_.textC(f, alert_.text, rc.x + 8, rc.y + (rc.h - r_.textH(f)) / 2, rc.w - 16, theme::text);
    }
    if (fx_.t > 0) {
        fx_.t -= dt;
        float a = fx_.t > 0 ? std::min(1.0f, fx_.t) : 0.0f;
        if (fx_.name == "redflash") {
            r_.rect(0, 0, r_.width(), r_.height(), rgba(160, 0, 0, (int)(60 * a)));
        } else if (fx_.name == "glitch") {
            for (int i = 0; i < 6; i++) {
                int y = rand() % r_.height();
                r_.rect(0, y, r_.width(), rand() % 4 + 1, rgba(255, 34, 34, (int)(70 * a)));
            }
        } else if (fx_.name == "corruption") {
            r_.rect(0, 0, r_.width(), r_.height(), rgba(90, 0, 0, (int)(40 * a)));
            for (int y = 0; y < r_.height(); y += 3)
                r_.rect(rand() % 400, y, rand() % 120, 1, rgba(0, 0, 0, 120));
        }
    }
}

} // namespace mine
