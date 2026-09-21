#include "npc_db.h"
#include <cstdio>
#include <algorithm>

namespace dhack {

void WorldData::register_defaults() {
    m_zones = {
        {"mac_anu", "Mac Anu", "server_alpha", "Ancient temple, starting zone, monsters and merchants", {}, {}, 1, 10, 7919},
        {"lost_belt", "Lost Belt", "server_beta", "Sky-floating platforms, guild headquarters", {}, {"AIDA_spawn", "corrupted_wolf"}, 8, 20, 6231},
        {"highbury", "Highbury", "server_gamma", "Snow-covered village, quiet but haunted", {}, {"ice_dragon", "frost_golem"}, 15, 30, 4519},
        {"neraks", "Neraks", "server_delta", "Underground caverns, labyrinthine tunnels", {}, {"cave_bat", "minotaur", "dark_elf"}, 20, 40, 8731},
        {"alpha_island", "Alpha Island", "server_alpha", "Small peaceful island with leveling grounds", {}, {"beach_crab", "sand_elemental"}, 1, 5, 1021},
        {"net_slum", "Net Slum", "server_omega", "Corrupted network district, AIDA's stronghold", {}, {"virus_knight", "data_worm", "AIDA_elite"}, 30, 50, 9913},
    };

    m_items = {
        {"dual_swords", "Dual Swords", "weapon", "Twin blades of the Blademaster", 100, 1},
        {"cursed_katana", "Cursed Katana", "weapon", "BlackRose's signature weapon", 500, 5},
        {"thunder_scroll", "Thunder Scroll", "skill", "Unleashes a bolt of lightning", 50, 1},
        {"cure_potion", "Cure Potion", "consumable", "Restores 50 HP", 25, 1},
        {"hi_potion", "Hi Potion", "consumable", "Restores 200 HP", 100, 1},
        {"key_of_twilight", "Key of the Twilight", "key", "The legendary weapon that breaks all barriers", 0, 1},
        {"flame_armor", "Flame Armor", "armor", "Reduces fire damage by half", 300, 5},
        {"mana_crystal", "Mana Crystal", "consumable", "Restores 30 MP", 50, 1},
    };
    printf("[World] %zu zones, %zu items\n", m_zones.size(), m_items.size());
}

const WorldZone* WorldData::find_zone(const std::string& id) const {
    for (const auto& z : m_zones) if (z.id == id) return &z;
    return nullptr;
}

const WorldItem* WorldData::find_item(const std::string& id) const {
    for (const auto& i : m_items) if (i.id == id) return &i;
    return nullptr;
}

} // namespace dhack
