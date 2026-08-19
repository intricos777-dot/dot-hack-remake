#pragma once
#include <memory>
#include <set>
#include <string>
#include <vector>
#include "Theme.hpp"
#include "Renderer.hpp"
#include "Widgets.hpp"
#include "Input.hpp"
#include "Terminal.hpp"
#include "../Aida/Bridge.hpp"
#include "../Aida/LiaBridge.hpp"
#include "../Aida/GameState.hpp"
#include "../World3D/World3D.hpp"

namespace mine {

enum class ShellState { Boot, Login, Desktop, World };

enum LogKind { L_SYS = 0, L_AIDA, L_USER, L_WORLD, L_EVENT };
struct LogLine { int kind = L_SYS; std::string text; };

class App {
public:
    virtual ~App() = default;
    virtual const char* title() const = 0;
    virtual void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) = 0;
};

class MineOS {
public:
    bool init(const std::string& scriptsDir);
    void run();
    void shutdown();

    aida::Bridge& bridge() { return bridge_; }
    aida::LiaBridge& liaBridge() { return liaBridge_; }
    aida::GameState& worldState() { return st_; }
    std::vector<LogLine>& log() { return log_; }
    void logLine(int kind, const std::string& text);
    void overlay(const std::string& text) { alert_ = {text, 6.0f}; }
    void fx(const std::string& name) { fx_ = {name, 1.4f}; }
    void sendUser(const std::string& text) { pendingAida_++; bridge_.sendUser(text); }
    int pendingAida() const { return pendingAida_; }
    void requestStory() { bridge_.sendStoryRequest(); }
    aida::Story& story() { return story_; }
    void announcePlayer(bool ask) { bridge_.sendPlayer(st_.player, ask); }
    bool firstDesktop = false;

    void launchWorld(const std::string& disc_id, const std::string& title, const std::string& data_root);
    void exitWorld();

    std::string getUser() const { return st_.player; }
    std::string getLocation() const { return st_.location; }
    aida::GameState& getState() { return st_; }

    // Terminal apps
    App* terminalApp() { return terminalApp_.get(); }
    App* mapTerminalApp() { return mapTerminalApp_.get(); }
    App* aidaApp() { return aidaApp_.get(); }
    void openWindow(App* app, int x = -1, int y = -1);
    void openInfoPopup(const std::string& title, const std::vector<std::string>& body);

private:
    struct Win {
        int id = 0;
        std::string title;
        Rect rc;
        App* app = nullptr;
        bool open = false;
        bool focused = false;
        int order = 0;
    };
    struct Icon { std::string label; Rect rc; App* app; };

    void handleBridge(float dt);
    void executeAction(const aida::Action& a, uint64_t id);
    void pollInput();
    void closeWindow(int id);
    void focusWindow(int id);
    void drawBoot(float dt);
    void drawLogin(Input& in, float dt);
    void drawDesktop(Input& in, float dt);
    void drawWorld(float dt);
    void drawWindows(float dt);
    void drawTaskbar(float dt);
    void drawOverlays(float dt);
    int nextWin() { return nextWinId_++; }

    Renderer r_;
    aida::Bridge bridge_;
    aida::LiaBridge liaBridge_;
    aida::GameState st_;
    aida::Story story_;
    std::vector<LogLine> log_;
    std::vector<Win> wins_;
    std::vector<Icon> icons_;
    int nextWinId_ = 1;
    int z_ = 0;
    int iconRowY_ = 90;

    ShellState shell_ = ShellState::Boot;
    float bootT_ = 0.0f;
    int bootLine_ = 0;
    struct { std::string text; float t; } alert_ = {"", 0.0f};
    struct { std::string name; float t; } fx_ = {"", 0.0f};
    float stateTimer_ = 0.0f;

    std::string seer_ = "sin";
    std::string pass_;
    bool loginErr_ = false;
    bool quit_ = false;
    bool announceFirstDesktop_ = false;
    bool seerActive_ = false;
    bool passActive_ = false;
    TextField seerField_, passField_;

    int dragWin_ = -1, dragDX_ = 0, dragDY_ = 0;
    Input in_;
    std::set<int> heldKeys_;

    // Per-disc terminals
    std::unique_ptr<GameTerminal> infectionTerminal_;
    std::unique_ptr<GameTerminal> guTerminal_;
    std::unique_ptr<GameTerminal> shugoTerminal_;
    std::unique_ptr<GameTerminal> roseTerminal_;
    GameTerminal* activeTerminal_ = nullptr;

    std::unique_ptr<App> aidaApp_, mailApp_, bbsApp_, newsApp_, sysApp_, worldApp_, gameLauncherApp_, terminalApp_, mapTerminalApp_;
    std::unique_ptr<World3D> world_;
    bool worldLaunched_ = false;
    std::string worldDiscTitle_;

public:
    std::string scriptsDir_ = "/home/sin/Projects/dot-hack-remake/scripts";
    uint64_t actionSeq_ = 0;
    int pendingAida_ = 0;
    void* ctl_ = nullptr;

    // Delegated world operations
    bool loadWorldSeele(const std::string& path, const std::string& disc_id) { return world_->loadSeeleWorlds(path, disc_id); }
};

} // namespace mine
