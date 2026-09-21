#include "npc_db.h"
#include <cstdio>
#include <algorithm>

namespace dhack {

bool CharacterManager::gain_exp(uint32_t amount) {
    m_state.exp += amount;
    printf("[Char] +%u EXP", amount);
    bool leveled = false;
    while (m_state.exp >= m_state.exp_to_next) {
        m_state.exp -= m_state.exp_to_next;
        level_up();
        leveled = true;
    }
    printf(" (%u/%u)\n", m_state.exp, m_state.exp_to_next);
    return leveled;
}

void CharacterManager::level_up() {
    m_state.level++;
    m_state.max_hp += 10;
    m_state.max_mp += 5;
    m_state.hp = m_state.max_hp;
    m_state.mp = m_state.max_mp;
    m_state.attack += 2;
    m_state.defense += 1;
    m_state.speed += 1;
    m_state.exp_to_next = m_state.level * 100;
    printf("[Char] LEVEL UP! -> Level %u\n", m_state.level);
}

bool CharacterManager::learn_skill(const std::string& skill) {
    for (const auto& s : m_state.skills) if (s == skill) return false;
    m_state.skills.push_back(skill);
    printf("[Char] Learned skill: %s\n", skill.c_str());
    return true;
}

void CharacterManager::add_item(const std::string& item) {
    m_state.inventory.push_back(item);
}

void CharacterManager::remove_item(const std::string& item) {
    auto it = std::find(m_state.inventory.begin(), m_state.inventory.end(), item);
    if (it != m_state.inventory.end()) m_state.inventory.erase(it);
}

void CharacterManager::restore_hp(uint32_t amount) {
    m_state.hp = std::min(m_state.max_hp, m_state.hp + amount);
}

void CharacterManager::restore_mp(uint32_t amount) {
    m_state.mp = std::min(m_state.max_mp, m_state.mp + amount);
}

} // namespace dhack
