#include "WorldState.hpp"
#include "Timeline.hpp"
#include "Quest.hpp"
#include "CharacterCreator.hpp"
#include "Customization.hpp"
#include "../Aida/Bridge.hpp"
#include <nlohmann/json.hpp>
#include <SDL.h>
#include <fstream>
#include <sstream>
#include <iostream>

namespace dot {

// World.cpp - Full .hack//G.U. World Implementation
// Integrates the Myine Network (The World: R2) with TS
// Supports both Infection (.hack//G.U.) and Link (.hack//Sign) storylines

WorldState::WorldState() : 
    playerLevel(1),
    hp(100),
    mp(50),
    exp(0),
    status(Status::Normal),
    location("Mac Anu"),
    playerName("Haseo") {
}

void WorldState::loadCanonTimeline() {
    // Load the canonical .hack timeline
    // INFECTION -> FREQUENCY -> OUTBREAK -> QUARANTINE -> LINK -> QUANTUM -> FIND_ME
    try {
        std::ifstream f("/home/sin/Projects/dot-hack-remake/Content/Worlds/TheWorld-R1.json");
        if (f.good()) {
            nlohmann::json doc;
            f >> doc;
            worldId = doc.value("worldId", "r1");
            // Load zones
            for (const auto& zone : doc["zones"]) {
                zones.push_back(zone.get<std::string>());
            }
            // Load timelines
            for (const auto& timeline : doc["timelines"]) {
                timelines.push_back(timeline.get<std::string>());
            }
            // Load canon characters
            for (const auto& chara : doc["canonCharacters"]) {
                canonCharacters.push_back(chara.get<std::string>());
            }
            std::cout << "WorldState: Loaded " << zones.size() << " zones, " 
                      << timelines.size() << " timelines, " 
                      << canonCharacters.size() << " canon characters" << std::endl;
        }
    } catch (const std::exception& e) {
        std::cerr << "WorldState: Error loading timeline: " << e.what() << std::endl;
        // Fallback to defaults
        zones = {"Mac Anu", "Dun Loireag", "Barney", "Forest of Pain", "Cave of Trial"};
        timelines = {"INFECTION", "FREQUENCY", "OUTBREAK", "QUARANTINE"};
        canonCharacters = {"Kite", "BlackRose", "Balmung", "Tsukasa", "Mimiru"};
    }
}

void WorldState::loadModTimeline(const std::string& modId) {
    // Load mod-specific timeline content
    std::cout << "WorldState: Loading mod timeline: " << modId << std::endl;
    // Mod loading structure ready for .dothackmod packages
}

void WorldState::setCharacterState(const CharacterState& state) {
    mCharacterStates[state.characterId] = state;
}

CharacterState WorldState::getCharacterState(const std::string& characterId) const {
    auto it = mCharacterStates.find(characterId);
    if (it != mCharacterStates.end()) return it->second;
    return CharacterState{characterId, "Unknown", 1};
}

// --- Additional World Systems ---

void WorldState::enterDungeon(const std::string& dungeonId) {
    // Transition to dungeon instance
    currentLocation = "DUNGEON:" + dungeonId;
    std::cout << "WorldState: Entering dungeon " << dungeonId << std::endl;
    
    // Send dungeon entry event to AIDA
    if (bridge) {
        bridge->sendEvent("DUNGEON_ENTER", {{"id", dungeonId}});
    }
}

void WorldState::exitDungeon() {
    // Return to previous location
    if (!worldId.empty()) {
        currentLocation = worldId;
    }
    std::cout << "WorldState: Exited dungeon" << std::endl;
}

std::vector<TimelineChapter> WorldState::chapters() const {
    return mChapters;
}

WorldState g_worldState;

// --- Global Access ---

WorldState& GetWorldState() {
    return g_worldState;
}

} // namespace dot