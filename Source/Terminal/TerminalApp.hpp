#pragma once
#include "MineOS.hpp"
#include "Terminal/Terminal.hpp"
#include "Renderer.hpp"
#include <string>
#include <vector>

namespace mine {

// Terminal desktop — the main "Terminal" icon target
class TerminalApp : public App {
public:
    explicit TerminalApp(MineOS* os) : os_(os) {}
    const char* title() const override { return "TERMINAL"; }
    void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
    MineOS* os_;
    IconMenu desktop_;
    OptionMenu options_;
    SelectBox dialog_;
    MapTerminal mapTerm_;

    enum class TermView { Desktop, Options, Map, Dialog };
    TermView view_ = TermView::Desktop;

    bool optAutorun = false;
    bool optVsync = true;
    bool optFullScreen = false;
    int optVolume = 80;
    int optBrightness = 50;

    void buildDesktop();
    void buildOptions();
    void resetDialog() { dialog_.done = true; }
};

// Map Terminal window (separate app)
class MapTerminalApp : public App {
public:
    explicit MapTerminalApp(MineOS* os) : os_(os) {}
    const char* title() const override { return "MAP TERMINAL"; }
    void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
    MineOS* os_;
    MapTerminal mt_;
    SelectBox confirm_;
};

} // namespace mine
