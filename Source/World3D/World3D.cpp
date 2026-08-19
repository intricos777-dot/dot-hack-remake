#include "World3D.hpp"
#include "WorldRenderer.hpp"
#include "../MineOS/Renderer.hpp"
#include "../MineOS/Theme.hpp"
#include <cmath>
#include <cstdio>
#include <algorithm>
#include <fstream>
#include <sstream>

namespace mine {

World3D::~World3D() = default;

bool World3D::initialize(const World3DConfig& cfg) {
    cfg_ = cfg;
    running_ = true;
    camZ_ = 0.0f;
    t_ = 0.0f;
    bobPhase_ = 0.0f;
    currentZoneIdx_ = 0;

    // Load world definition
    if (!loadWorldDef(cfg.disc_id)) {
        std::printf("[World3D] warning: no world def for %s\n", cfg.disc_id.c_str());
    }

    // Load zones from JSON
    if (!loadZonesForDisc(cfg.disc_id)) {
        std::printf("[World3D] warning: no zone data loaded for %s\n", cfg.disc_id.c_str());
        Zone z;
        z.id = "default";
        z.name = "The World";
        z.type = "field";
        z.sky[0] = 20; z.sky[1] = 0; z.sky[2] = 4;
        z.ground[0] = 140; z.ground[1] = 20; z.ground[2] = 30;
        z.fog = 60;
        z.structures = {"monolith", "tower"};
        z.npcs = {"Kite"};
        z.monsters = {"Skeleton"};
        z.items = {"Potion"};
        zones_.push_back(z);
    }

    if (!zones_.empty()) {
        currentZone_ = std::make_unique<Zone>(zones_[0]);
    }

    std::printf("[World3D] disc=%s era=%s world=%s\n", cfg.disc_id.c_str(), worldEra_.c_str(), worldDef_.name.c_str());
    return true;
}

void World3D::shutdown() {
    if (!running_) return;
    running_ = false;
    zones_.clear();
    currentZone_.reset();
    std::printf("[World3D] shutdown\n");
}

bool World3D::loadWorldDef(const std::string& disc_id) {
    // Determine era from disk ID
    if (disc_id == "vol1_i" || disc_id == "vol2_a" || disc_id == "vol2_a_pc" || disc_id == "vol3_a") {
        worldEra_ = "R1";
        worldDef_.name = "The World R:1";
        worldDef_.era = "Infection";
        worldDef_.skyTop[0] = 25; worldDef_.skyTop[1] = 5; worldDef_.skyTop[2] = 10;
        worldDef_.skyBottom[0] = 140; worldDef_.skyBottom[1] = 20; worldDef_.skyBottom[2] = 30;
        worldDef_.groundPrimary[0] = 140; worldDef_.groundPrimary[1] = 20; worldDef_.groundPrimary[2] = 30;
        worldDef_.groundGrid[0] = 180; worldDef_.groundGrid[1] = 30; worldDef_.groundGrid[2] = 40;
        worldDef_.fogDensity = 60;
        return true;
    } else if (disc_id == "vol3_a_pc" || disc_id == "vol4_a" || disc_id == "vol4_a_pc" || disc_id == "vol4_i") {
        worldEra_ = "R2";
        worldDef_.name = "The World R:2";
        worldDef_.era = "G.U.";
        worldDef_.skyTop[0] = 15; worldDef_.skyTop[1] = 0; worldDef_.skyTop[2] = 5;
        worldDef_.skyBottom[0] = 100; worldDef_.skyBottom[1] = 15; worldDef_.skyBottom[2] = 20;
        worldDef_.groundPrimary[0] = 100; worldDef_.groundPrimary[1] = 15; worldDef_.groundPrimary[2] = 20;
        worldDef_.groundGrid[0] = 140; worldDef_.groundGrid[1] = 25; worldDef_.groundGrid[2] = 35;
        worldDef_.fogDensity = 70;
        return true;
    } else if (disc_id == "link_shugo" || disc_id == "link_rose") {
        worldEra_ = "Link";
        worldDef_.name = "The World R:2 // Link";
        worldDef_.era = "Link";
        worldDef_.skyTop[0] = 20; worldDef_.skyTop[1] = 30; worldDef_.skyTop[2] = 15;
        worldDef_.skyBottom[0] = 80; worldDef_.skyBottom[1] = 100; worldDef_.skyBottom[2] = 35;
        worldDef_.groundPrimary[0] = 50; worldDef_.groundPrimary[1] = 100; worldDef_.groundPrimary[2] = 35;
        worldDef_.groundGrid[0] = 70; worldDef_.groundGrid[1] = 140; worldDef_.groundGrid[2] = 50;
        worldDef_.fogDensity = 55;
        return true;
    }
    return false;
}

void World3D::enterZone(int index) {
    if (index < 0 || index >= (int)zones_.size()) return;
    currentZoneIdx_ = index;
    currentZone_ = std::make_unique<Zone>(zones_[index]);
    std::printf("[World3D] entered zone: %s\n", zones_[index].name.c_str());
}

bool World3D::loadZonesForDisc(const std::string& disc_id) {
    std::string base = "/home/sin/Projects/dot-hack-remake/Content/Worlds/";
    std::string file;

    if (disc_id == "vol1_i" || disc_id == "vol2_a" || disc_id == "vol2_a_pc" || disc_id == "vol3_a") {
        file = base + "zones_infection.json";
    } else if (disc_id == "vol3_a_pc" || disc_id == "vol4_a" || disc_id == "vol4_a_pc" || disc_id == "vol4_i") {
        file = base + "zones_gu.json";
    } else if (disc_id == "link_shugo") {
        file = base + "zones_shugo.json";
    } else if (disc_id == "link_rose") {
        file = base + "zones_rose.json";
    }

    // Prefer Seele-generated arc worlds for this disc when available.
    std::string seele_file = base + "seele_" + disc_id + ".json";
    if (std::ifstream(seele_file).good()) {
        if (loadSeeleWorlds(seele_file, disc_id)) {
            return true;
        }
    }

    if (file.empty()) return false;
    return loadZoneFile(file);
}

// Minimal JSON parser helpers
static std::string extractString(const std::string& s, size_t& pos) {
    while (pos < s.size() && s[pos] != '"') pos++;
    if (pos >= s.size()) return "";
    pos++;
    size_t start = pos;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\') pos++;
        pos++;
    }
    std::string result = s.substr(start, pos - start);
    if (pos < s.size()) pos++;
    return result;
}

static int extractInt(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] < '0' || s[pos] > '9') && s[pos] != '-') pos++;
    int sign = 1;
    if (pos < s.size() && s[pos] == '-') { sign = -1; pos++; }
    int result = 0;
    while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
        result = result * 10 + (s[pos] - '0');
        pos++;
    }
    return result * sign;
}

static void skipWhitespace(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\n' || s[pos] == '\r' || s[pos] == '\t')) pos++;
}

bool World3D::loadZoneFile(const std::string& path) {
    std::ifstream f(path);
    if (!f.good()) return false;

    std::stringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();

    size_t zonesStart = json.find("\"zones\"", 0);
    if (zonesStart == std::string::npos) return false;

    size_t arrStart = json.find('[', zonesStart);
    if (arrStart == std::string::npos) return false;
    arrStart++;

    while (arrStart < json.size()) {
        skipWhitespace(json, arrStart);
        if (arrStart >= json.size() || json[arrStart] == ']') break;
        if (json[arrStart] != '{') { arrStart++; continue; }

        Zone z;
        arrStart++;

        while (arrStart < json.size()) {
            skipWhitespace(json, arrStart);
            if (arrStart >= json.size()) break;
            if (json[arrStart] == '}') { arrStart++; break; }
            if (json[arrStart] != '"') { arrStart++; continue; }

            std::string key = extractString(json, arrStart);
            skipWhitespace(json, arrStart);
            if (arrStart < json.size() && json[arrStart] == ':') arrStart++;
            skipWhitespace(json, arrStart);

            if (arrStart >= json.size()) break;

            if (json[arrStart] == '"') {
                std::string val = extractString(json, arrStart);
                if (key == "id") z.id = val;
                else if (key == "name") z.name = val;
                else if (key == "type") z.type = val;
            } else if (json[arrStart] == '[') {
                arrStart++;
                if (key == "sky" || key == "ground") {
                    int idx = 0;
                    while (arrStart < json.size() && json[arrStart] != ']') {
                        skipWhitespace(json, arrStart);
                        if (json[arrStart] == ',') { arrStart++; continue; }
                        if (json[arrStart] >= '0' && json[arrStart] <= '9') {
                            int v = extractInt(json, arrStart);
                            if (key == "sky" && idx < 3) z.sky[idx] = v;
                            if (key == "ground" && idx < 3) z.ground[idx] = v;
                            idx++;
                        } else arrStart++;
                    }
                } else {
                    std::vector<std::string>* target = nullptr;
                    if (key == "structures") target = &z.structures;
                    else if (key == "npcs") target = &z.npcs;
                    else if (key == "monsters") target = &z.monsters;
                    else if (key == "items") target = &z.items;
                    if (target) {
                        while (arrStart < json.size() && json[arrStart] != ']') {
                            skipWhitespace(json, arrStart);
                            if (json[arrStart] == ',') { arrStart++; continue; }
                            if (json[arrStart] == '"') {
                                target->push_back(extractString(json, arrStart));
                            } else arrStart++;
                        }
                    }
                }
                if (arrStart < json.size() && json[arrStart] == ']') arrStart++;
            } else if (json[arrStart] >= '0' && json[arrStart] <= '9') {
                int v = extractInt(json, arrStart);
                if (key == "fog") z.fog = v;
            }

            skipWhitespace(json, arrStart);
            if (arrStart < json.size() && json[arrStart] == ',') arrStart++;
        }

        if (!z.id.empty()) {
            zones_.push_back(z);
        }

        skipWhitespace(json, arrStart);
        if (arrStart < json.size() && json[arrStart] == ',') arrStart++;
    }

    std::printf("[World3D] loaded %zu zones from %s\n", zones_.size(), path.c_str());
    return !zones_.empty();
}

void World3D::runFrame(float dt) {
    if (!running_) return;
    t_ += dt;
    bobPhase_ += dt * 2.0f;
    camZ_ -= dt * 40.0f;
    if (camZ_ < -400.0f) camZ_ += 400.0f;
}

void World3D::drawScene(Renderer& r) {
    if (!running_) return;
    if (!currentZone_) return;

    Zone& z = *currentZone_;

    // Draw based on era
    if (worldEra_ == "R1") {
        drawR1World(r, z);
    } else if (worldEra_ == "R2") {
        drawR2World(r, z);
    } else if (worldEra_ == "Link") {
        drawLinkWorld(r, z);
    } else {
        drawR1World(r, z);  // Default
    }
}

void World3D::drawR1World(Renderer& r, const Zone& z) {
    Render3D& proj = r.r3d();
    r.set3D(900.0f, 55.0f);

    auto fb = r.fontBold(16);
    auto ft = r.fontMono(13);

    // R1 Infection sky - red/black
    r.rect(0, 0, r.width(), r.height(), rgb(25, 5, 10));
    r.rect(0, 0, r.width(), r.height() / 2, rgb(35, 8, 15));

    // Ground grid
    const int W = 1600, H = 1000;
    float horizon = (float)H * 0.42f;

    for (int c = -24; c <= 24; c++) {
        float wx = (float)c / 24 * (W * 0.9f);
        int sx0, sy0, sx1, sy1;
        proj.project(wx, horizon + 2, 0.0f, sx0, sy0);
        proj.project(wx, H + 200, 600.0f, sx1, sy1);
        r.line(std::max(0, std::min(r.width(), sx0)), std::max(0, std::min(r.height(), sy0)),
               std::max(0, std::min(r.width(), sx1)), std::max(0, std::min(r.height(), sy1)),
               rgba(140, 20, 30, 90));
    }

    // Monoliths (root towns)
    for (size_t i = 0; i < z.structures.size() && i < 5; i++) {
        float wz = 200.0f + (float)i * 90.0f + fmod(camZ_, 400.0f) * 0.5f;
        if (wz > 600) continue;
        float wx = ((i % 2 == 0) ? -1 : 1) * (180.0f + i * 60.0f);
        float scale = 0.6f + i * 0.12f;

        int bx, by, tx, ty;
        proj.project(wx, horizon, wz, bx, by);
        proj.project(wx, horizon - 220 * scale, wz, tx, ty);
        int bw = (int)(40 * scale);
        r.rect(bx - bw / 2, ty, bw, by - ty, rgba(60, 8, 12, 200));
        r.outline(bx - bw / 2, ty, bw, by - ty, rgba(255, 80, 80, 120), 1);
    }

    // Data motes
    for (int i = 0; i < 40; i++) {
        float a = (float)i / 40.0f * 6.2831853f + t_ * 0.3f;
        float rad = 120.0f + (i % 7) * 30.0f;
        float wx = std::cos(a) * rad;
        float wy = horizon - 40.0f + std::sin(a * 1.7f + t_) * 60.0f;
        float wz = 120.0f + (float)(i % 13) * 30.0f;
        int sx, sy;
        if (proj.project(wx, wy, wz, sx, sy) && sx > 0 && sx < r.width() && sy > 0 && sy < r.height()) {
            int sz = (int)(3 + 2 * std::sin(t_ * 3 + i));
            r.rect(sx, sy, sz, sz, rgba(255, 60, 70, 160));
        }
    }

    drawAvatar(r, z);
    drawZoneLabel(r, z);
    r.scanlineOverlay(0, 0, r.width(), r.height(), 28);
}

void World3D::drawR2World(Renderer& r, const Zone& z) {
    Render3D& proj = r.r3d();
    r.set3D(900.0f, 55.0f);

    auto fb = r.fontBold(16);
    auto ft = r.fontMono(13);

    // R2 G.U. sky - darker, more corrupted
    r.rect(0, 0, r.width(), r.height(), rgb(15, 0, 5));
    r.rect(0, 0, r.width(), r.height() / 2, rgb(25, 5, 10));

    // Ground grid - darker red
    const int W = 1600, H = 1000;
    float horizon = (float)H * 0.42f;

    for (int c = -24; c <= 24; c++) {
        float wx = (float)c / 24 * (W * 0.9f);
        int sx0, sy0, sx1, sy1;
        proj.project(wx, horizon + 2, 0.0f, sx0, sy0);
        proj.project(wx, H + 200, 600.0f, sx1, sy1);
        r.line(std::max(0, std::min(r.width(), sx0)), std::max(0, std::min(r.height(), sy0)),
               std::max(0, std::min(r.width(), sx1)), std::max(0, std::min(r.height(), sy1)),
               rgba(100, 15, 20, 90));
    }

    // AIDA corruption structures
    for (size_t i = 0; i < z.structures.size() && i < 5; i++) {
        float wz = 200.0f + (float)i * 90.0f + fmod(camZ_, 400.0f) * 0.5f;
        if (wz > 600) continue;
        float wx = ((i % 2 == 0) ? -1 : 1) * (180.0f + i * 60.0f);
        float scale = 0.6f + i * 0.12f;

        int bx, by, tx, ty;
        proj.project(wx, horizon, wz, bx, by);
        proj.project(wx, horizon - 220 * scale, wz, tx, ty);
        int bw = (int)(40 * scale);
        r.rect(bx - bw / 2, ty, bw, by - ty, rgba(40, 5, 10, 210));
        r.outline(bx - bw / 2, ty, bw, by - ty, rgba(255, 40, 40, 150), 1);
    }

    // AIDA data motes (darker, more corrupted)
    for (int i = 0; i < 40; i++) {
        float a = (float)i / 40.0f * 6.2831853f + t_ * 0.3f;
        float rad = 120.0f + (i % 7) * 30.0f;
        float wx = std::cos(a) * rad;
        float wy = horizon - 40.0f + std::sin(a * 1.7f + t_) * 60.0f;
        float wz = 120.0f + (float)(i % 13) * 30.0f;
        int sx, sy;
        if (proj.project(wx, wy, wz, sx, sy) && sx > 0 && sx < r.width() && sy > 0 && sy < r.height()) {
            int sz = (int)(3 + 2 * std::sin(t_ * 3 + i));
            r.rect(sx, sy, sz, sz, rgba(200, 30, 30, 180));
        }
    }

    drawAvatar(r, z);
    drawZoneLabel(r, z);
    r.scanlineOverlay(0, 0, r.width(), r.height(), 28);
}

void World3D::drawLinkWorld(Renderer& r, const Zone& z) {
    Render3D& proj = r.r3d();
    r.set3D(900.0f, 55.0f);

    auto fb = r.fontBold(16);
    auto ft = r.fontMono(13);

    // Link era sky - green-tinted (PSP)
    r.rect(0, 0, r.width(), r.height(), rgb(20, 30, 15));
    r.rect(0, 0, r.width(), r.height() / 2, rgb(25, 40, 20));

    // Ground grid - green
    const int W = 1600, H = 1000;
    float horizon = (float)H * 0.42f;

    for (int c = -24; c <= 24; c++) {
        float wx = (float)c / 24 * (W * 0.9f);
        int sx0, sy0, sx1, sy1;
        proj.project(wx, horizon + 2, 0.0f, sx0, sy0);
        proj.project(wx, H + 200, 600.0f, sx1, sy1);
        r.line(std::max(0, std::min(r.width(), sx0)), std::max(0, std::min(r.height(), sy0)),
               std::max(0, std::min(r.width(), sx1)), std::max(0, std::min(r.height(), sy1)),
               rgba(50, 100, 35, 90));
    }

    // Nature structures
    for (size_t i = 0; i < z.structures.size() && i < 5; i++) {
        float wz = 200.0f + (float)i * 90.0f + fmod(camZ_, 400.0f) * 0.5f;
        if (wz > 600) continue;
        float wx = ((i % 2 == 0) ? -1 : 1) * (180.0f + i * 60.0f);
        float scale = 0.6f + i * 0.12f;

        int bx, by, tx, ty;
        proj.project(wx, horizon, wz, bx, by);
        proj.project(wx, horizon - 220 * scale, wz, tx, ty);
        int bw = (int)(40 * scale);
        r.rect(bx - bw / 2, ty, bw, by - ty, rgba(30, 60, 20, 200));
        r.outline(bx - bw / 2, ty, bw, by - ty, rgba(80, 200, 80, 120), 1);
    }

    // Nature motes (green)
    for (int i = 0; i < 40; i++) {
        float a = (float)i / 40.0f * 6.2831853f + t_ * 0.3f;
        float rad = 120.0f + (i % 7) * 30.0f;
        float wx = std::cos(a) * rad;
        float wy = horizon - 40.0f + std::sin(a * 1.7f + t_) * 60.0f;
        float wz = 120.0f + (float)(i % 13) * 30.0f;
        int sx, sy;
        if (proj.project(wx, wy, wz, sx, sy) && sx > 0 && sx < r.width() && sy > 0 && sy < r.height()) {
            int sz = (int)(3 + 2 * std::sin(t_ * 3 + i));
            r.rect(sx, sy, sz, sz, rgba(80, 200, 80, 160));
        }
    }

    drawAvatar(r, z);
    drawZoneLabel(r, z);
    r.scanlineOverlay(0, 0, r.width(), r.height(), 28);
}

void World3D::drawAvatar(Renderer& r, const Zone& z) {
    auto ft = r.fontMono(13);
    auto fb = r.fontBold(16);

    int pax = r.width() / 2;
    int pay = (int)(r.height() * 0.72f);

    int bob = (int)(4 * std::sin(bobPhase_));
    pay += bob;

    // Determine character based on disc
    std::string charName = "HASEO";
    Color bodyColor = rgb(20, 0, 0);
    Color accentColor = rgb(255, 40, 40);
    Color visorColor = rgb(255, 60, 60);
    std::string title = "Terror of Death";

    if (cfg_.disc_id.find("vol1_i") != std::string::npos || cfg_.disc_id.find("vol2_a") != std::string::npos) {
        charName = "KITE";
        title = "Twilight Wanderer";
        bodyColor = rgb(30, 60, 120);
        accentColor = rgb(80, 160, 255);
        visorColor = rgb(100, 200, 255);
    } else if (cfg_.disc_id.find("vol3_a") != std::string::npos || cfg_.disc_id.find("vol4") != std::string::npos) {
        charName = "HASEO";
        title = "Terror of Death";
        bodyColor = rgb(20, 0, 0);
        accentColor = rgb(255, 40, 40);
        visorColor = rgb(255, 60, 60);
    } else if (cfg_.disc_id.find("shugo") != std::string::npos) {
        charName = "SHUGO";
        title = "Contest Winner";
        bodyColor = rgb(30, 60, 30);
        accentColor = rgb(80, 200, 80);
        visorColor = rgb(100, 255, 100);
    } else if (cfg_.disc_id.find("rose") != std::string::npos) {
        charName = "ROSE";
        title = "Heavy Blade";
        bodyColor = rgb(60, 30, 60);
        accentColor = rgb(160, 80, 200);
        visorColor = rgb(200, 100, 255);
    } else if (cfg_.disc_id.find("shino") != std::string::npos) {
        charName = "SHINO";
        title = "Wavemaster";
        bodyColor = rgb(40, 20, 50);
        accentColor = rgb(120, 60, 180);
        visorColor = rgb(160, 80, 255);
    }

    // Draw character body
    r.bubble(pax, pay, 50, 70, rgba(bodyColor.r, bodyColor.g, bodyColor.b, 230), accentColor, 2);
    r.bubble(pax, pay - 60, 30, 30, rgba(bodyColor.r + 20, bodyColor.g + 20, bodyColor.b + 20, 240), accentColor, 2);
    r.rect(pax - 22, pay - 68, 44, 16, rgba(visorColor.r, visorColor.g, visorColor.b, 200));
    r.bubble(pax - 55, pay - 20, 18, 25, rgba(accentColor.r, accentColor.g, accentColor.b, 200), accentColor, 1);
    r.bubble(pax + 55, pay - 20, 18, 25, rgba(accentColor.r, accentColor.g, accentColor.b, 200), accentColor, 1);
    r.rect(pax + 60, pay - 40, 6, 80, rgba(accentColor.r, accentColor.g, accentColor.b, 180));

    r.textC(fb, charName, pax, pay + 85, 120, accentColor);
    r.textC(ft, title, pax, pay + 105, 120, theme::textDim);
    r.textC(ft, "LV.1", pax - 40, pay + 65, 80, rgba(255, 60, 60, 150));
}

void World3D::drawZoneLabel(Renderer& r, const Zone& z) {
    auto fb = r.fontBold(20);
    auto ft = r.fontMono(12);

    r.textC(fb, z.name, 0, 12, r.width(), rgb(255, 60, 60));
    r.textC(ft, z.type + " // " + cfg_.disc_id, 0, 38, r.width(), rgb(180, 120, 120));
    r.textC(ft, "AIDA // LINKED", 0, r.height() - 30, r.width(), rgb(80, 255, 120));
}

void World3D::drawNpcMarkers(Renderer& r, const Zone& z) {
    Render3D& proj = r.r3d();
    const int H = 1000;
    float horizon = (float)H * 0.42f;
    auto ft = r.fontMono(11);

    for (size_t i = 0; i < z.npcs.size() && i < 3; i++) {
        float wx = -200.0f + (float)i * 200.0f;
        float wz = 150.0f + (float)i * 60.0f;
        float wy = horizon - 10.0f;
        int sx, sy;
        if (proj.project(wx, wy, wz, sx, sy)) {
            r.bubbleHex3D((float)sx, (float)sy, 0.0f, 12.0f, rgba(80, 200, 120, 200), rgba(120, 255, 160, 255), 1);
            r.textC(ft, z.npcs[i].c_str(), sx - 30, sy + 14, 60, rgb(180, 255, 200));
        }
    }
}

void World3D::drawMonsterEncounters(Renderer& r, const Zone& z) {
    Render3D& proj = r.r3d();
    const int H = 1000;
    float horizon = (float)H * 0.42f;
    auto ft = r.fontMono(10);

    for (size_t i = 0; i < z.monsters.size() && i < 3; i++) {
        float wx = -150.0f + (float)i * 150.0f;
        float wz = 250.0f + (float)i * 80.0f;
        float wy = horizon - 5.0f;
        int sx, sy;
        if (proj.project(wx, wy, wz, sx, sy)) {
            float pulse = 1.0f + 0.3f * std::sin(t_ * 4.0f + (float)i);
            int rad = (int)(10 * pulse);
            r.bubble(sx, sy, rad, (int)(rad * 1.3f), rgba(200, 40, 40, 180), rgba(255, 80, 80, 220), 1);
            r.textC(ft, z.monsters[i].c_str(), sx - 35, sy + 12, 70, rgb(255, 150, 150));
        }
    }
}

void World3D::drawHUD(Renderer& r) {
    auto ft = r.fontMono(14);
    auto fb = r.fontBold(22);
    r.textC(fb, "THE WORLD // R2", 0, 12, r.width(), theme::blood);
    r.textC(ft, "World", 0, 38, r.width(), theme::textDim);
}

bool World3D::loadSeeleWorlds(const std::string& path, const std::string& disc_id) {
    std::ifstream f(path);
    if (!f.good()) return false;

    std::stringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();

    size_t zonesStart = json.find("\"zones\"", 0);
    if (zonesStart == std::string::npos) return false;

    size_t arrStart = json.find('[', zonesStart);
    if (arrStart == std::string::npos) return false;
    arrStart++;

    int loaded = 0;
    while (arrStart < json.size()) {
        skipWhitespace(json, arrStart);
        if (arrStart >= json.size() || json[arrStart] == ']') break;
        if (json[arrStart] != '{') { arrStart++; continue; }

        Zone z;
        arrStart++;

        while (arrStart < json.size()) {
            skipWhitespace(json, arrStart);
            if (arrStart >= json.size()) break;
            if (json[arrStart] == '}') { arrStart++; break; }
            if (json[arrStart] != '"') { arrStart++; continue; }

            std::string key = extractString(json, arrStart);
            skipWhitespace(json, arrStart);
            if (arrStart < json.size() && json[arrStart] == ':') arrStart++;
            skipWhitespace(json, arrStart);
            if (arrStart >= json.size()) break;

            if (json[arrStart] == '"') {
                std::string val = extractString(json, arrStart);
                if (key == "id") z.id = val;
                else if (key == "name") z.name = val;
                else if (key == "type") z.type = val;
            } else if (json[arrStart] == '[') {
                arrStart++;
                if (key == "sky" || key == "ground") {
                    int idx = 0;
                    while (arrStart < json.size() && json[arrStart] != ']') {
                        skipWhitespace(json, arrStart);
                        if (json[arrStart] == ',') { arrStart++; continue; }
                        if (json[arrStart] >= '0' && json[arrStart] <= '9') {
                            int v = extractInt(json, arrStart);
                            if (key == "sky" && idx < 3) z.sky[idx] = v;
                            if (key == "ground" && idx < 3) z.ground[idx] = v;
                            idx++;
                        } else arrStart++;
                    }
                } else {
                    std::vector<std::string>* target = nullptr;
                    if (key == "structures") target = &z.structures;
                    else if (key == "npcs") target = &z.npcs;
                    else if (key == "monsters") target = &z.monsters;
                    else if (key == "items") target = &z.items;
                    if (target) {
                        while (arrStart < json.size() && json[arrStart] != ']') {
                            skipWhitespace(json, arrStart);
                            if (json[arrStart] == ',') { arrStart++; continue; }
                            if (json[arrStart] == '"') {
                                target->push_back(extractString(json, arrStart));
                            } else arrStart++;
                        }
                    }
                }
                if (arrStart < json.size() && json[arrStart] == ']') arrStart++;
            } else if (json[arrStart] >= '0' && json[arrStart] <= '9') {
                int v = extractInt(json, arrStart);
                if (key == "fog") z.fog = v;
            }

            skipWhitespace(json, arrStart);
            if (arrStart < json.size() && json[arrStart] == ',') arrStart++;
        }

        if (!z.id.empty() && z.id.find(disc_id) != std::string::npos) {
            zones_.push_back(z);
            loaded++;
        }

        skipWhitespace(json, arrStart);
        if (arrStart < json.size() && json[arrStart] == ',') arrStart++;
    }

    if (loaded > 0) {
        std::printf("[World3D] Seele: loaded %d zones for %s\n", loaded, disc_id.c_str());
    }
    return loaded > 0;
}

} // namespace mine
