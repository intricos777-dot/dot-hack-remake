#pragma once
#include <string>
#include <vector>
#include <map>
#include <cstdint>

namespace dhack {

// ---- NPC System ----------------------------------------------------------

using Hour = uint32_t;

struct NPCSchedule {
    std::string location;
    std::map<Hour, std::string> lines;
};

struct NPCDef {
    std::string id;
    std::string name;
    std::string role;      // "player", "guild", "merchant", "quest", "enemy"
    std::string zone;      // home zone in The World
    std::string appearance;
    std::vector<NPCSchedule> schedules;
    std::vector<std::string> general_lines;
};

class NPCDb {
public:
    void register_defaults();
    const std::vector<NPCDef>& npcs() const { return m_npcs; }
    const NPCDef* find(const std::string& id) const;
    std::vector<const NPCDef*> in_zone(const std::string& zone) const;
    std::string get_line(const NPCDef& npc, Hour hour) const;
private:
    std::vector<NPCDef> m_npcs;
};

// ---- World System --------------------------------------------------------

struct WorldZone {
    std::string id;
    std::string name;
    std::string server_id;
    std::string description;
    std::vector<std::string> npcs;
    std::vector<std::string> monsters;
    uint32_t level_min = 1;
    uint32_t level_max = 10;
    uint32_t seed = 0;
};

struct WorldItem {
    std::string id;
    std::string name;
    std::string type;  // "weapon", "armor", "skill", "key", "consumable"
    std::string description;
    uint32_t value = 0;
    uint32_t level = 1;
};

class WorldData {
public:
    void register_defaults();
    const std::vector<WorldZone>& zones() const { return m_zones; }
    const std::vector<WorldItem>& items() const { return m_items; }
    const WorldZone* find_zone(const std::string& id) const;
    const WorldItem* find_item(const std::string& id) const;
private:
    std::vector<WorldZone> m_zones;
    std::vector<WorldItem> m_items;
};

// ---- Combat System --------------------------------------------------------

struct CombatMove {
    std::string name;
    std::string type;  // "attack", "magic", "heal", "buff", "skill"
    uint32_t power = 0;
    uint32_t mp_cost = 0;
};

struct EnemyState {
    std::string name;
    uint32_t hp;
    uint32_t max_hp;
    uint32_t mp;
    uint32_t attack;
    uint32_t defense;
    uint32_t speed;
    std::vector<std::string> skills;
    std::vector<std::string> drops;
    uint32_t exp_reward = 0;
    uint32_t gold_reward = 0;
};

enum class BattlePhase { intro, player_turn, enemy_turn, victory, defeat };

class CombatEngine {
public:
    void engage(const EnemyState& enemy, uint32_t player_hp, uint32_t player_max_hp,
                uint32_t player_mp, uint32_t player_max_mp);

    std::vector<std::string> attack();
    std::vector<std::string> use_skill(const std::string& skill);
    std::vector<std::string> defend();
    std::vector<std::string> use_item(const std::string& item);
    std::vector<std::string> flee();

    bool is_over() const;
    bool player_won() const { return m_phase == BattlePhase::victory; }
    uint32_t enemy_hp() const { return m_enemy_hp; }
    uint32_t player_hp() const { return m_player_hp; }
    uint32_t player_mp() const { return m_player_mp; }
    BattlePhase phase() const { return m_phase; }
    std::vector<std::string> drops_earned() const { return m_drops_earned; }

private:
    BattlePhase m_phase = BattlePhase::intro;
    EnemyState m_enemy;
    uint32_t m_enemy_hp = 0;
    uint32_t m_enemy_max_hp = 0;
    uint32_t m_player_hp = 0;
    uint32_t m_player_max_hp = 0;
    uint32_t m_player_mp = 0;
    uint32_t m_player_max_mp = 0;
    uint32_t m_player_defending = false;
    std::vector<std::string> m_drops_earned;
    std::vector<CombatMove> m_player_moves;

    void enemy_turn();
    void check_victory();
};

// ---- Character Progression -----------------------------------------------

struct CharacterState {
    std::string name = "Kite";
    std::string class_name = "Blademaster";
    uint32_t level = 1;
    uint32_t exp = 0;
    uint32_t exp_to_next = 100;
    uint32_t hp = 100;
    uint32_t max_hp = 100;
    uint32_t mp = 50;
    uint32_t max_mp = 50;
    uint32_t attack = 10;
    uint32_t defense = 8;
    uint32_t speed = 12;
    uint32_t gold = 0;
    std::string current_zone = "mac_anu";
    std::vector<std::string> inventory;
    std::vector<std::string> skills = {"Slash", "Thunder", "Cure"};
    std::vector<std::string> quest_log;
};

class CharacterManager {
public:
    CharacterState& state() { return m_state; }
    bool gain_exp(uint32_t amount);
    void level_up();
    bool learn_skill(const std::string& skill);
    void add_item(const std::string& item);
    void remove_item(const std::string& item);
    void restore_hp(uint32_t amount);
    void restore_mp(uint32_t amount);
private:
    CharacterState m_state;
};

} // namespace dhack
