#include "npc_db.h"
#include <cstdio>
#include <cstdlib>
#include <ctime>
#include <algorithm>

namespace dhack {

void CombatEngine::engage(const EnemyState& enemy, uint32_t player_hp, uint32_t player_max_hp,
                          uint32_t player_mp, uint32_t player_max_mp) {
    m_enemy = enemy;
    m_enemy_hp = enemy.hp;
    m_enemy_max_hp = enemy.max_hp;
    m_player_hp = player_hp;
    m_player_max_hp = player_max_hp;
    m_player_mp = player_mp;
    m_player_max_mp = player_max_mp;
    m_phase = BattlePhase::intro;
    m_drops_earned.clear();
    printf("\n  \x1b[38;5;196m\x1b[1mENCOUNTER: %s (HP: %u)\x1b[0m\n", enemy.name.c_str(), enemy.hp);

    // Player move set
    m_player_moves = {
        {"Slash", "attack", 15, 0},
        {"Thunder", "magic", 35, 10},
        {"Cure", "heal", 0, 8},
        {"Defend", "buff", 0, 0},
    };
}

std::vector<std::string> CombatEngine::attack() {
    std::vector<std::string> log;
    if (m_phase != BattlePhase::player_turn && m_phase != BattlePhase::intro) return log;
    m_phase = BattlePhase::player_turn;

    uint32_t dmg = 15 + std::rand() % 10;
    bool crit = (std::rand() % 100) < 12;
    if (crit) dmg = dmg * 2;

    m_enemy_hp = std::max(0u, m_enemy_hp - dmg);
    log.push_back("You slash " + m_enemy.name + " for " + std::to_string(dmg) + (crit ? " (CRIT!)" : ""));
    check_victory();
    if (m_phase != BattlePhase::victory) {
        m_phase = BattlePhase::enemy_turn;
        enemy_turn();
    }
    return log;
}

std::vector<std::string> CombatEngine::use_skill(const std::string& skill) {
    std::vector<std::string> log;
    if (m_phase != BattlePhase::player_turn && m_phase != BattlePhase::intro) return log;
    m_phase = BattlePhase::player_turn;

    uint32_t mp_cost = 10;
    uint32_t dmg = 35 + std::rand() % 15;

    if (m_player_mp < mp_cost) {
        log.push_back("Not enough MP!");
        return log;
    }
    m_player_mp -= mp_cost;

    if (skill == "Cure") {
        uint32_t heal = 40 + std::rand() % 20;
        m_player_hp = std::min(m_player_max_hp, m_player_hp + heal);
        log.push_back("Cure! Restored " + std::to_string(heal) + " HP");
    } else {
        m_enemy_hp = std::max(0u, m_enemy_hp - dmg);
        log.push_back(skill + "! " + std::to_string(dmg) + " damage to " + m_enemy.name);
    }

    check_victory();
    if (m_phase != BattlePhase::victory) {
        m_phase = BattlePhase::enemy_turn;
        enemy_turn();
    }
    return log;
}

std::vector<std::string> CombatEngine::defend() {
    std::vector<std::string> log;
    if (m_phase != BattlePhase::player_turn && m_phase != BattlePhase::intro) return log;
    m_phase = BattlePhase::player_turn;

    m_player_defending = true;
    log.push_back("You brace for impact. Damage halved next hit.");
    m_phase = BattlePhase::enemy_turn;
    enemy_turn();
    return log;
}

std::vector<std::string> CombatEngine::use_item(const std::string& item) {
    std::vector<std::string> log;
    if (item.find("potion") != std::string::npos || item.find("cure") != std::string::npos) {
        uint32_t heal = (item.find("hi") != std::string::npos) ? 200 : 50;
        m_player_hp = std::min(m_player_max_hp, m_player_hp + heal);
        log.push_back(item + ": +" + std::to_string(heal) + " HP");
    } else if (item.find("mana") != std::string::npos) {
        m_player_mp = std::min(m_player_max_mp, m_player_mp + 30);
        log.push_back(item + ": +30 MP");
    } else {
        log.push_back(item + " has no effect in combat.");
    }
    return log;
}

std::vector<std::string> CombatEngine::flee() {
    std::vector<std::string> log;
    if ((std::rand() % 100) < 40) {
        m_phase = BattlePhase::defeat;  // signal retreat
        log.push_back("You disengage and retreat to the safe zone!");
    } else {
        log.push_back(m_enemy.name + " blocks your escape!");
        m_phase = BattlePhase::enemy_turn;
        enemy_turn();
    }
    return log;
}

bool CombatEngine::is_over() const {
    return m_phase == BattlePhase::victory || m_phase == BattlePhase::defeat;
}

void CombatEngine::enemy_turn() {
    if (m_enemy_hp == 0) return;
    uint32_t dmg = m_enemy.attack + std::rand() % 5;
    if (m_player_defending) dmg /= 2;
    m_player_defending = false;

    if ((std::rand() % 100) < 20) {
        // Use skill
        if (!m_enemy.skills.empty()) {
            printf("[Combat] %s uses %s!\n", m_enemy.name.c_str(), m_enemy.skills[std::rand() % m_enemy.skills.size()].c_str());
            dmg = (uint32_t)(dmg * 1.5);
        }
    }

    m_player_hp = std::max(0u, m_player_hp - dmg);
    printf("[Combat] %s hits you for %u\n", m_enemy.name.c_str(), dmg);

    if (m_player_hp == 0) {
        m_phase = BattlePhase::defeat;
        printf("[Combat] You fell in battle.\n");
    } else {
        m_phase = BattlePhase::player_turn;
    }
}

void CombatEngine::check_victory() {
    if (m_enemy_hp == 0) {
        m_phase = BattlePhase::victory;
        // Award drops
        for (const auto& d : m_enemy.drops) {
            if ((std::rand() % 100) < 50) {
                m_drops_earned.push_back(d);
            }
        }
        printf("[Combat] Victory! EXP: %u, Gold: %u\n", m_enemy.exp_reward, m_enemy.gold_reward);
    }
}

} // namespace dhack
