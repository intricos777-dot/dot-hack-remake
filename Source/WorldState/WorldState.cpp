#include "WorldState.hpp"
#include "../Aida/Bridge.hpp"
#include <iostream>

namespace dot {

WorldState::WorldState() : 
    playerLevel(1),
    hp(100),
    mp(50),
    exp(0),
    status(Status::Normal),
    location("Mac Anu"),
    playerName("Haseo"),
    worldId("r1") {
}

void WorldState::loadCanonTimeline() {
    // Default timeline for .hack//G.U. - INFECTION arc
    timelines = {"INFECTION", "FREQUENCY", "OUTBREAK", "QUARANTINE"};
    zones = {"Mac Anu", "Dun Loireag", "Barney", "Forest of Pain", "Cave of Trial"};
    canonCharacters = {"Kite", "BlackRose", "Balmung", "Tsukasa", "Mimiru", "Haseo"};
    worldId = "r1";
}

void WorldState::loadModTimeline(const std::string& modId) {
    std::cout << "WorldState: Loading mod timeline: " << modId << std::endl;
}

void WorldState::setCharacterState(const CharacterState& state) {
    mCharacterStates[state.characterId] = state;
}

CharacterState WorldState::getCharacterState(const std::string& characterId) const {
    auto it = mCharacterStates.find(characterId);
    if (it != mCharacterStates.end()) return it->second;
    return CharacterState{characterId, "Unknown", 1};
}

std::vector<TimelineChapter> WorldState::chapters() const {
    return mChapters;
}

void WorldState::enterDungeon(const std::string& dungeonId) {
    currentLocation = "DUNGEON:" + dungeonId;
    location = currentLocation;
    std::cout << "WorldState: Entering dungeon " << dungeonId << std::endl;
}

void WorldState::exitDungeon() {
    if (!worldId.empty()) {
        currentLocation = worldId;
        location = worldId;
    }
    std::cout << "WorldState: Exited dungeon" << std::endl;
}

// Global instance
WorldState g_worldState;

WorldState& GetWorldState() {
    return g_worldState;
}

} // namespace dot