#include "DiscBoot.hpp"
#include <map>

namespace mine {

DiscBootManager& DiscBootManager::instance() {
    static DiscBootManager inst;
    return inst;
}

DiscBootManager::DiscBootManager() {
    // Kite arc - starts at Mac Anu in each game
    m_starts["vol1_i"] = {"Kite", "azure_kite", "mac_anu", "vol1_i",
        "Content/Quests/infection_quests.json",
        "Content/Levels/infection_levels.json",
        "Content/Assets/themes/infection_theme.json",
        {30, 0, 8}, {140, 20, 30}};
    m_arcs["vol1_i"] = CharacterArc::Kite_Infection;
    
    m_starts["vol2_a"] = {"Kite", "azure_kite_2", "mac_anu", "vol2_a",
        "Content/Quests/frequency_quests.json",
        "Content/Levels/infection_levels.json",
        "Content/Assets/themes/frequency_theme.json",
        {25, 5, 10}, {80, 10, 15}};
    m_arcs["vol2_a"] = CharacterArc::Kite_Mutation;
    
    m_starts["vol2_a_pc"] = {"Kite", "azure_kite_3", "mac_anu", "vol2_a_pc",
        "Content/Quests/outbreak_quests.json",
        "Content/Levels/infection_levels.json",
        "Content/Assets/themes/outbreak_theme.json",
        {35, 0, 0}, {160, 0, 0}};
    m_arcs["vol2_a_pc"] = CharacterArc::Kite_Outbreak;
    
    m_starts["vol3_a"] = {"Kite", "azure_kite_4", "mac_anu", "vol3_a",
        "Content/Quests/quarantine_quests.json",
        "Content/Levels/infection_levels.json",
        "Content/Assets/themes/quarantine_theme.json",
        {20, 0, 5}, {100, 15, 20}};
    m_arcs["vol3_a"] = CharacterArc::Kite_Quarantine;
    
    // Haseo arc - starts at Mac Anu in G.U. games
    m_starts["vol3_a_pc"] = {"Haseo", "haseo_gu_vol1", "mac_anu_gu", "vol3_a_pc",
        "Content/Quests/gu_vol1_quests.json",
        "Content/Levels/gu_levels.json",
        "Content/Assets/themes/gu_theme.json",
        {10, 20, 50}, {40, 80, 160}};
    m_arcs["vol3_a_pc"] = CharacterArc::Haseo_GU_Rebirth;
    
    m_starts["vol4_a"] = {"Haseo", "haseo_gu_vol2", "mac_anu_gu", "vol4_a",
        "Content/Quests/gu_vol2_quests.json",
        "Content/Levels/gu_levels.json",
        "Content/Assets/themes/gu_vol1-4_theme.json",
        {12, 25, 55}, {35, 70, 140}};
    m_arcs["vol4_a"] = CharacterArc::Haseo_GU_Reminisce;
    
    m_starts["vol4_a_pc"] = {"Haseo", "haseo_gu_vol3", "mac_anu_gu", "vol4_a_pc",
        "Content/Quests/gu_vol3_quests.json",
        "Content/Levels/gu_levels.json",
        "Content/Assets/themes/gu_vol1-4_theme.json",
        {15, 30, 60}, {30, 60, 120}};
    m_arcs["vol4_a_pc"] = CharacterArc::Haseo_GU_Redemption;
    
    m_starts["vol4_i"] = {"Haseo", "haseo_gu_vol4", "mac_anu_gu", "vol4_i",
        "Content/Quests/gu_vol4_quests.json",
        "Content/Levels/gu_levels.json",
        "Content/Assets/themes/gu_vol1-4_theme.json",
        {8, 15, 40}, {25, 50, 100}};
    m_arcs["vol4_i"] = CharacterArc::Haseo_GU_Reconnection;
    
    // Shugo arc - starts at Leiseijo
    m_starts["link_shugo"] = {"Shugo", "shugo_kite_arc", "leiseijo", "link_shugo",
        "Content/Quests/link_quests.json",
        "Content/Levels/sign_levels.json",
        "Content/Assets/themes/link_theme.json",
        {20, 40, 15}, {40, 90, 30}};
    m_arcs["link_shugo"] = CharacterArc::Shugo_Link;
    
    // Rose arc - starts at Leiseijo
    m_starts["link_rose"] = {"Rose", "rose_blackrose_arc", "leiseijo_rose", "link_rose",
        "Content/Quests/link_quests.json",
        "Content/Levels/sign_levels.json",
        "Content/Assets/themes/link_theme.json",
        {25, 20, 45}, {50, 30, 90}};
    m_arcs["link_rose"] = CharacterArc::Rose_Link;
    
    // Shino arc - starts at Leiseijo (Twilight Palace)
    m_starts["link_shino"] = {"Shino", "shino_wavemaster", "leiseijo_shino", "link_shino",
        "Content/Quests/link_quests.json",
        "Content/Levels/sign_levels.json",
        "Content/Assets/themes/quantum_theme.json",
        {30, 15, 40}, {55, 25, 85}};
    m_arcs["link_shino"] = CharacterArc::Shino_Link;
    
    // Tsukasa arc - starts at Dun Loireag
    m_starts["sign_arc"] = {"Tsukasa", "tsukasa_trapped", "dun_loireag", "sign_arc",
        "Content/Quests/sign_quests.json",
        "Content/Levels/sign_levels.json",
        "Content/Assets/themes/sign_theme.json",
        {25, 5, 10}, {80, 10, 15}};
    m_arcs["sign_arc"] = CharacterArc::Tsukasa_SIGN;
    
    // Sakuya arc - starts at Mac Anu (Fortress)
    m_starts["quantum_arc"] = {"Sakuya", "sakuya_tobias", "fortress", "quantum_arc",
        "Content/Quests/quantum_quests.json",
        "Content/Levels/sign_levels.json",
        "Content/Assets/themes/quantum_theme.json",
        {20, 30, 10}, {40, 60, 20}};
    m_arcs["quantum_arc"] = CharacterArc::Sakuya_Quantum;
    
    // Haseo Find Me arc - starts at Dun Loireag
    m_starts["findme_arc"] = {"Haseo", "haseo_solo", "dun_loireag", "findme_arc",
        "Content/Quests/findme_quests.json",
        "Content/Levels/sign_levels.json",
        "Content/Assets/themes/findme_theme.json",
        {25, 5, 10}, {80, 10, 15}};
    m_arcs["findme_arc"] = CharacterArc::Haseo_FindMe;
}

CharacterStart DiscBootManager::getStart(const std::string& disc_id) const {
    auto it = m_starts.find(disc_id);
    if (it != m_starts.end()) return it->second;
    // Default to Haseo G.U. Vol.1
    return m_starts.at("vol3_a_pc");
}

CharacterArc DiscBootManager::getArc(const std::string& disc_id) const {
    auto it = m_arcs.find(disc_id);
    if (it != m_arcs.end()) return it->second;
    return CharacterArc::Haseo_GU_Rebirth;
}

std::vector<std::string> DiscBootManager::getDiscsForCharacter(const std::string& character) const {
    std::vector<std::string> result;
    for (const auto& [id, start] : m_starts) {
        if (start.character == character) result.push_back(id);
    }
    return result;
}

std::vector<std::string> DiscBootManager::getAllDiscs() const {
    std::vector<std::string> result;
    for (const auto& [id, start] : m_starts) {
        result.push_back(id);
    }
    return result;
}

std::string DiscBootManager::getQuestFile(const std::string& disc_id) const {
    return getStart(disc_id).quest_file;
}

std::string DiscBootManager::getLevelFile(const std::string& disc_id) const {
    return getStart(disc_id).level_file;
}

std::string DiscBootManager::getThemeFile(const std::string& disc_id) const {
    return getStart(disc_id).theme_file;
}

} // namespace mine
