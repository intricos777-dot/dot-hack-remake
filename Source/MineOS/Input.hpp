#pragma once
#include <set>
#include <string>

namespace mine {

struct Input {
    int mx = 0, my = 0;
    bool leftDown = false;
    bool clicked = false;
    int key = 0;
    std::string text;
    bool ctrl = false;
    bool shift = false;
    int wheel = 0;
    const std::set<int>* held = nullptr;
    uint32_t gamepadButtons = 0;
    float gamepadAxisX = 0, gamepadAxisY = 0;
};

} // namespace mine
