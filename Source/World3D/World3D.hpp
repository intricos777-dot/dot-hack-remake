#pragma once

#include <string>
#include <vector>
#include <memory>

namespace mine {

class Renderer;
class Render3D;
struct Color;

// Zone loaded from JSON content
struct Zone {
    std::string id;
    std::string name;
    std::string type;
    std::string protagonist;
    std::string media_type;
    std::string mood;
    int sky[3] = {20,0,4};
    int ground[3] = {140,20,30};
    int fog = 60;
    std::vector<std::string> structures;
    std::vector<std::string> npcs;
    std::vector<std::string> monsters;
    std::vector<std::string> items;
    std::vector<std::string> avatars;
};

// World definition
struct WorldDef {
    std::string name;
    std::string era;
    std::string description;
    int skyTop[3] = {25,5,10};
    int skyBottom[3] = {140,20,30};
    int groundPrimary[3] = {140,20,30};
    int groundGrid[3] = {180,30,40};
    int fogDensity = 60;
    std::vector<std::string> rootTowns;
    std::vector<std::string> dungeons;
    std::vector<std::string> bossAreas;
};

struct World3DConfig {
    std::string disc_id;
    std::string title;
    std::string data_root;
    bool fullscreen = true;
    class Renderer* renderer = nullptr;
};

class World3D {
public:
    World3D() = default;
    ~World3D();

    bool initialize(const World3DConfig& cfg);
    void shutdown();

    void runFrame(float dt);
    void drawScene(class Renderer& r);
    bool isRunning() const { return running_; }

    const Zone* currentZone() const { return currentZone_ ? currentZone_.get() : nullptr; }
    const std::vector<Zone>& zones() const { return zones_; }
    void enterZone(int index);
    const std::string& discId() const { return cfg_.disc_id; }

    bool loadSeeleWorlds(const std::string& path, const std::string& disc_id);

private:
    bool loadZonesForDisc(const std::string& disc_id);
    bool loadZoneFile(const std::string& path);
    bool loadWorldDef(const std::string& disc_id);
    void applyWorldDef(const WorldDef& def);

    void drawSky(Renderer& r, const Zone& z);
    void drawGroundGrid(Renderer& r, const Zone& z);
    void drawStructures(Renderer& r, const Zone& z);
    void drawMotes(Renderer& r, const Zone& z);
    void drawAvatar(Renderer& r, const Zone& z);
    void drawZoneLabel(Renderer& r, const Zone& z);
    void drawNpcMarkers(Renderer& r, const Zone& z);
    void drawMonsterEncounters(Renderer& r, const Zone& z);
    void drawHUD(Renderer& r);

    // World-specific rendering
    void drawR1World(Renderer& r, const Zone& z);  // Infection era
    void drawR2World(Renderer& r, const Zone& z);  // G.U. era
    void drawLinkWorld(Renderer& r, const Zone& z); // Link era

    World3DConfig cfg_;
    bool running_ = false;
    float camZ_ = 0.0f;
    float t_ = 0.0f;
    std::vector<Zone> zones_;
    std::unique_ptr<Zone> currentZone_;
    int currentZoneIdx_ = 0;
    float bobPhase_ = 0.0f;

    WorldDef worldDef_;
    std::string worldEra_;  // "R1", "R2", "Link"
};

} // namespace mine
