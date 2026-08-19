#pragma once
#include <string>
#include <vector>
#include <map>
#include <functional>

namespace dot {

// ============================================================
// ORIGINAL GAME QUEST DATA — All 10 discs, canon faithful
// ============================================================

enum class QuestStatus { Available, Active, Completed, Failed, Locked };
enum class QuestType { Main, Side, Dungeon, Arena, Event, AIDA };
enum class QuestDifficulty { Easy, Normal, Hard, Legendary };

struct QuestReward {
    int exp = 0;
    int gold = 0;
    std::vector<std::string> items;
    std::string unlock_skill;
    std::string unlock_zone;
    std::string unlock_quest;
};

struct QuestObjective {
    std::string id;
    std::string text;
    std::string type;  // "kill", "collect", "talk", "explore", "escort", "defend", "boss"
    std::string target;  // NPC/monster/zone ID
    int required = 1;
    int current = 0;
    bool completed = false;
    bool optional = false;
};

struct QuestStage {
    int stage;
    std::string description;
    std::vector<std::string> dialogue;  // NPC dialogue lines
    std::string zone;  // Where this stage takes place
    std::string next_stage_id;  // Branching support
};

struct Quest {
    std::string id;
    std::string title;
    std::string description;
    QuestType type;
    QuestDifficulty difficulty;
    
    // Canon data
    std::string disc_id;  // Which disc this belongs to
    std::string arc;  // "infection", "gu", "sign", "link"
    int level_required = 1;
    int level_recommended = 1;
    
    // Objectives & Stages
    std::vector<QuestObjective> objectives;
    std::vector<QuestStage> stages;
    int current_stage = 0;
    
    // NPCs
    std::string giver_npc;
    std::string turnin_npc;
    std::vector<std::string> involved_npcs;
    
    // Rewards
    QuestReward reward;
    
    // Flow
    std::string prerequisite_quest;  // Must complete first
    std::vector<std::string> unlocks_quests;  // Quests this unlocks
    std::string next_quest_in_chain;  // Auto-start after completion
    
    // AIDA modification
    bool can_be_modified_by_aida = true;
    bool is_aida_variant = false;
    std::string base_quest_id;  // If AIDA variant, what's the original
    
    // State
    QuestStatus status = QuestStatus::Available;
    float time_limit = 0;  // 0 = no limit
    bool repeatable = false;
};

// ============================================================
// LEVEL / DUNGEON DATA — Original game zones
// ============================================================

struct LevelEnemy {
    std::string id;
    std::string name;
    int level;
    int hp;
    int mp;
    int attack;
    int defense;
    int speed;
    std::vector<std::string> skills;
    std::vector<std::string> drops;
    float drop_chance = 0.3f;
    bool is_boss = false;
    bool is_miniboss = false;
};

struct LevelLoot {
    std::string item_id;
    int quantity = 1;
    float chance = 0.1f;
};

struct LevelFloor {
    int floor_number;
    std::string name;
    std::string description;
    std::vector<LevelEnemy> enemies;
    std::vector<LevelLoot> loot;
    std::string boss_id;  // Boss for this floor (if any)
    bool is_rest_floor = false;  // Safe zone between battles
};

struct DungeonLevel {
    std::string id;
    std::string name;
    std::string description;
    std::string zone_id;  // Which zone this dungeon belongs to
    std::string disc_id;
    
    int min_level = 1;
    int max_level = 99;
    int floor_count = 1;
    std::vector<LevelFloor> floors;
    
    // Layout
    std::string layout_type;  // "linear", "branching", "open", "maze"
    float size = 1.0f;  // 1.0 = standard
    
    // Rewards
    QuestReward completion_reward;
    std::vector<std::string> unlocks_zones;
    
    // AIDA
    bool can_mutate = true;  // AIDA can modify enemies/layout
};

// ============================================================
// AIDA QUEST MODIFIER
// ============================================================

struct AIDAModification {
    enum Type { AddObjective, RemoveObjective, ChangeTarget, AddStage, 
                ReplaceEnemy, BuffEnemy, AddLoot, ChangeDialogue,
                AddTimeLimit, RemoveTimeLimit, ChangeReward };
    
    Type type;
    std::string target_quest_id;
    std::string description;  // Human-readable what AIDA did
    
    // Modification data
    QuestObjective new_objective;
    std::string old_target;
    std::string new_target;
    LevelEnemy enemy_change;
    LevelLoot loot_change;
    QuestReward reward_change;
    
    // Reasoning
    std::string aida_reason;  // Why AIDA made this change
    float story_relevance = 0.0f;  // How much this affects the story
};

// ============================================================
// QUEST MANAGER
// ============================================================

class QuestManager {
public:
    QuestManager();
    ~QuestManager();
    
    // Initialize with canon quest data
    bool loadCanonQuests();
    bool loadCanonLevels();
    
    // Quest operations
    bool startQuest(const std::string& questId);
    bool completeObjective(const std::string& questId, int objectiveIndex);
    bool advanceStage(const std::string& questId);
    bool completeQuest(const std::string& questId);
    bool failQuest(const std::string& questId);
    
    // Queries
    std::vector<Quest*> getAvailableQuests();
    std::vector<Quest*> getActiveQuests();
    std::vector<Quest*> getCompletedQuests();
    std::vector<Quest*> getQuestsForDisc(const std::string& discId);
    std::vector<Quest*> getQuestsForZone(const std::string& zoneId);
    std::vector<Quest*> getQuestsForNpc(const std::string& npcId);
    Quest* getQuest(const std::string& questId);
    
    // Level operations
    DungeonLevel* getDungeon(const std::string& dungeonId);
    std::vector<DungeonLevel*> getDungeonsForZone(const std::string& zoneId);
    std::vector<DungeonLevel*> getDungeonsForDisc(const std::string& discId);
    
    // AIDA integration
    void setAIDAEnabled(bool enabled) { m_aidaEnabled = enabled; }
    bool isAIDAEnabled() const { return m_aidaEnabled; }
    
    // AIDA modifies an existing canon quest
    Quest* generateAIDAVariant(const std::string& baseQuestId, const std::string& reason);
    void applyAIDAModification(const std::string& questId, const AIDAModification& mod);
    void revertAIDAModification(const std::string& questId);
    
    // AIDA generates new side quest based on player state
    Quest* generateAIDASideQuest(const std::string& context);
    
    // Save/Load
    bool saveQuests(const std::string& path);
    bool loadQuests(const std::string& path);
    
    // Stats
    int totalQuests() const { return (int)m_quests.size(); }
    int completedQuestCount() const;
    int activeQuestCount() const;
    int totalLevels() const { return (int)m_levels.size(); }
    
private:
    std::map<std::string, Quest> m_quests;
    std::map<std::string, DungeonLevel> m_levels;
    bool m_aidaEnabled = false;
    
    // JSON parsers
    void parseQuestFile(const std::string& json);
    void parseLevelFile(const std::string& json);
    
    // Canon quest data for all discs
    void createInfectionQuests();
    void createFrequencyQuests();
    void createOutbreakQuests();
    void createQuarantineQuests();
    void createLinkQuests();
    void createQuantumQuests();
    void createFindMeQuests();
    void createGUQuests();
    void createGU_Vol1_Quests();
    void createGU_Vol2_Quests();
    void createGU_Vol3_Quests();
    void createGU_Vol4_Quests();
    void createSIGNQuests();
    
    // Level data
    void createAllLevels();
    
    // Quest chain helpers
    void linkQuestChain(const std::vector<std::string>& questIds);
    void linkSideQuests();
    
    // AIDA
    std::vector<AIDAModification> m_activeModifications;
};

} // namespace dot
