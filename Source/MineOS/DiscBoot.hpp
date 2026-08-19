#pragma once
#include <string>
#include <vector>
#include <map>

namespace mine {

// ============================================================
// DISC BOOT MANAGER — Knows where each character starts
// ============================================================

enum class CharacterArc {
    Kite_Infection,
    Kite_Mutation,
    Kite_Outbreak,
    Kite_Quarantine,
    Haseo_GU_Rebirth,
    Haseo_GU_Reminisce,
    Haseo_GU_Redemption,
    Haseo_GU_Reconnection,
    Shugo_Link,
    Rose_Link,
    Shino_Link,
    Tsukasa_SIGN,
    Sakuya_Quantum,
    Haseo_FindMe
};

struct CharacterStart {
    std::string character;
    std::string outfit;
    std::string zone;
    std::string disc_id;
    std::string quest_file;
    std::string level_file;
    std::string theme_file;
    float sky[3];
    float ground[3];
};

class DiscBootManager {
public:
    static DiscBootManager& instance();
    
    // Get character start data for a disc
    CharacterStart getStart(const std::string& disc_id) const;
    
    // Get character arc for a disc
    CharacterArc getArc(const std::string& disc_id) const;
    
    // Get all disc IDs for a character
    std::vector<std::string> getDiscsForCharacter(const std::string& character) const;
    
    // Get all disc IDs
    std::vector<std::string> getAllDiscs() const;
    
    // Get quest file for a disc
    std::string getQuestFile(const std::string& disc_id) const;
    
    // Get level file for a disc
    std::string getLevelFile(const std::string& disc_id) const;
    
    // Get theme file for a disc
    std::string getThemeFile(const std::string& disc_id) const;

private:
    DiscBootManager();
    std::map<std::string, CharacterStart> m_starts;
    std::map<std::string, CharacterArc> m_arcs;
};

} // namespace mine
