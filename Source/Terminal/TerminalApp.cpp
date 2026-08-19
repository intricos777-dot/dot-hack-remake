#include "TerminalApp.hpp"
#include <SDL.h>

namespace mine {

void TerminalApp::buildDesktop() {
    desktop_.icons.clear();
    desktop_.cols = 4;
    // These mirror the real .hack terminal apps from the DLL (IconMenu, IconFile)
    desktop_.add("MAP", "map", true);
    desktop_.add("QUEST", "quest", true);
    desktop_.add("MAIL", "mail", true);
    desktop_.add("BBS", "bbs", true);
    desktop_.add("NEWS", "news", true);
    desktop_.add("SHOP", "shop", true);
    desktop_.add("BANK", "bank", true);
    desktop_.add("STORAGE", "storage", true);
    desktop_.add("STATUS", "status", true);
    desktop_.add("SKILLS", "skills", true);
    desktop_.add("WEAPONS", "weapons", true);
    desktop_.add("OPTIONS", "options", true);
    desktop_.add("LOGOUT", "logout", true);
}

void TerminalApp::buildOptions() {
    options_.items.clear();
    options_.title = "OPTIONS";
    options_.addToggle("Auto-Run", &optAutorun, true);
    options_.addToggle("VSync", &optVsync, true);
    options_.addToggle("FullScreen", &optFullScreen, true);
    options_.addValue("Volume", &optVolume, 0, 100, 5);
    options_.addValue("Brightness", &optBrightness, 0, 100, 5);
    options_.addBack("Save & Close");
    options_.reset();
}

void TerminalApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
    (void)dt;
    if (desktop_.icons.empty()) buildDesktop();

    if (view_ == TermView::Desktop) {
        RenderTerminalMenu(r, body, in, desktop_);
        desktop_.handle(in);

        if (in.key == SDLK_RETURN || in.key == SDLK_SPACE) {
            std::string tag = desktop_.icons[desktop_.selected].tag;
            if (tag == "options") {
                buildOptions();
                view_ = TermView::Options;
            } else if (tag == "logout") {
                os_->overlay("logging out...");
                // session close logic
            } else if (tag == "map") {
                // open map terminal in separate window
                os_->openWindow(os_->mapTerminalApp());
            } else {
                os_->overlay("opening " + tag + "...");
            }
        }
    } else if (view_ == TermView::Options) {
        RenderOptionMenu(r, body, in, options_);
        std::string result = options_.handle(in);
        if (options_.done) view_ = TermView::Desktop;
    } else if (view_ == TermView::Map) {
        // map terminal window
    } else if (view_ == TermView::Dialog) {
        RenderSelectBox(r, body, dialog_);
        dialog_.handle(in);
        if (dialog_.done) view_ = TermView::Desktop;
    }
}

// ---- Map Terminal App ----

void MapTerminalApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
    (void)dt;
    if (mt_.areas.empty()) {
        // populate from extracted disc data
        mt_.add("macanu", "Mac Anu", "INFECTION");
        mt_.add("lataele", "Lata Ele", "INFECTION");
        mt_.add("basha", "Basha", "INFECTION");
        mt_.add("sundelto", "Sundelto", "INFECTION");
        mt_.add("delracsal", "Delracsal", "INFECTION");
        mt_.add("leviathan", "Leviathan", "INFECTION");
        mt_.add("dnavarath", "Dnavarath", "INFECTION");
        mt_.add("ranbabingo", "Ranbabingo", "INFECTION");
        mt_.add("rutilant", "Rutilant", "INFECTION");
        mt_.add("net_slum", "Net Slum", "INFECTION");
        mt_.add("snow_forest", "Snow Forest", "INFECTION");
        mt_.add("sea_of_death", "Sea of Death", "INFECTION");
        mt_.add("lost_water", "Lost Water", "INFECTION");
    }

    if (confirm_.done) {
        RenderMapTerminal(r, body, in, mt_);
    } else {
        RenderSelectBox(r, body, confirm_);
        confirm_.handle(in);
        if (confirm_.done && confirm_.accepted()) {
            // launch the area
            AreaNode& a = mt_.areas[mt_.selected];
            os_->overlay("entering " + a.name + "...");
            // TODO: launch world for this area
        }
    }

    if (in.key == SDLK_RETURN || in.key == SDLK_SPACE) {
        if (!confirm_.done) {
            confirm_.open(SelBoxKind::YesNo, "ENTER AREA", "Enter " + mt_.areas[mt_.selected].name + "?");
        }
    }
}

} // namespace mine
