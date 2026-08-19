#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "../Aida/Bridge.hpp"

namespace dot {

struct TimelineChapter { 
    std::string id; 
    std::string title; 
    bool canon = false;
    
    struct Quest {
        std::string id;
        std::string name;
        std::string description;
        std::string NPC;
        std::vector<std::string> conditions;
        std::vector<std::string> rewards;
    };
    
    struct Monster {
        std::string name;
        int level;
        int hp;
        int mp;
        std::vector<std::string> drops;
    };
};

struct CharacterState { 
    std::string characterId; 
    std::string zone; 
    int level = 1;
    int hp = 100;
    int mp = 50;
    int exp = 0;
    std::string weapon = "Dual Swords";
    std::vector<std::string> skills;
};

class WorldState {
public:
    enum class Status { Normal, Infected, Corrupted, Dead };
    
    WorldState();
    
    void loadCanonTimeline();
    void loadModTimeline(const std::string& modId);
    void setCharacterState(const CharacterState& state);
    CharacterState getCharacterState(const std::string& characterId) const;
    std::vector<TimelineChapter> chapters() const;
    
    // Gameplay methods
    void enterDungeon(const std::string& dungeonId);
    void exitDungeon();
    
    // Player state
    int playerLevel;
    int hp;
    int mp;
    int exp;
    Status status;
    std::string location;
    std::string playerName;
    
    // World data
    std::string worldId;
    std::vector<std::string> zones;
    std::vector<std::string> timelines;
    std::vector<std::string> canonCharacters;
    aida::Bridge* bridge = nullptr;
    
private:
    std::vector<TimelineChapter> mChapters;
    std::string currentLocation;
    std::unordered_map<std::string, CharacterState> mCharacterStates;
};

// Global accessor
WorldState& GetWorldState();

} // namespace dot