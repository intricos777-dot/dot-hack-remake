#include "npc_db.h"
#include <cstdio>

namespace dhack {

void NPCDb::register_defaults() {
    // Player characters from canon
    NPCDef kite;
    kite.id = "kite"; kite.name = "Kite"; kite.role = "player";
    kite.zone = "mac_anu"; kite.appearance = "Blue-clad boy, twin-blade Key of the Twilight wielder";
    kite.general_lines = {
        "Let's go, BlackRose.",
        "We need to find the cause of the coma.",
        "AIDA... I will stop you."
    };
    m_npcs.push_back(kite);

    NPCDef blackrose;
    blackrose.id = "blackrose"; blackrose.name = "BlackRose"; blackrose.role = "player";
    blackrose.zone = "mac_anu"; blackrose.appearance = "Black armor, katana wielder, fierce and proud";
    blackrose.general_lines = {
        "A lot of monsters here.",
        "We need to level up before going deeper.",
        "The Twilight Brigade saved me."
    };
    m_npcs.push_back(blackrose);

    NPCDef hmm;
    hmm.id = "hhm"; hmm.name = "Haseo"; hmm.role = "player";
    hmm.zone = "mac_anu"; hmm.appearance = "Scarred avenger, Avatar of the Twilight Brigade";
    hmm.general_lines = {
        "Atoli... I have to save Atoli.",
        "I am the Tri-Edge. The Avenger."
    };
    m_npcs.push_back(hmm);

    // Guild NPCs
    NPCDef atoli;
    atoli.id = "atoli"; atoli.name = "Atoli"; atoli.role = "quest";
    atoli.zone = "mac_anu"; atoli.appearance = "Gentle halberd user, guild officer";
    {
        NPCSchedule sched;
        sched.location = "mac_anu";
        sched.lines = {
            {2, "Haseo, please don't push yourself too hard."},
            {8, "The Phases grow stronger each day."},
            {14, "Let's find a safe zone and talk."},
            {20, "I believe in you, always."}
        };
        atoli.schedules.push_back(sched);
    }
    atoli.general_lines = {"The world is getting darker."};
    m_npcs.push_back(atoli);

    NPCDef gpt;
    gpt.id = "gpt"; gpt.name = "GPT (Gold Packet Treaty)"; gpt.role = "guild";
    gpt.zone = "lost_belt"; gpt.appearance = "Friendly guild, white armor, packet traders";
    {
        NPCSchedule sched;
        sched.location = "lost_belt";
        sched.lines = {
            {4, "Welcome to the Lost Belt. Rare items for rare members."},
            {12, "Our treasury is open to guild members only."},
            {18, "Need skill keys? We have them."}
        };
        gpt.schedules.push_back(sched);
    }
    m_npcs.push_back(gpt);

    NPCDef pi;
    pi.id = "pi"; pi.name = "Pi"; pi.role = "enemy";
    pi.zone = "mac_anu"; pi.appearance = "Mysterious hacker, red eyes, dangerous";
    pi.general_lines = {
        "The system cannot stop me.",
        "Your data is mine now.",
        "I am the virus that eats worlds."
    };
    m_npcs.push_back(pi);

    NPCDef aida;
    aida.id = "aida"; aida.name = "AIDA"; aida.role = "enemy";
    aida.zone = "lost_belt"; aida.appearance = "Distorted monster, crimson, reality-corrupting";
    {
        NPCSchedule sched;
        sched.location = "lost_belt";
        sched.lines = {
            {3, "Kite... you should not have come here."},
            {9, "The coma is a side effect of my awakening."},
            {15, "I was born from the system. I am the system."},
            {21, "Join me, Kite. Together we can remake The World."}
        };
        aida.schedules.push_back(sched);
    }
    m_npcs.push_back(aida);

    NPCDef merc;
    merc.id = "merc"; merc.name = "Mac Anu Merchant"; merc.role = "merchant";
    merc.zone = "mac_anu"; merc.appearance = "Trade-booth NPC, gold scales, endless stock";
    {
        NPCSchedule sched;
        sched.location = "mac_anu";
        sched.lines = {
            {6, "Potions, weapons, rare skills — I have it all."},
            {12, "Running low on gold? Clear a dungeon."},
            {18, "New shipment of Thunder Scrolls!"}
        };
        merc.schedules.push_back(sched);
    }
    m_npcs.push_back(merc);

    printf("[NPC] %zu .hack roster registered\n", m_npcs.size());
}

const NPCDef* NPCDb::find(const std::string& id) const {
    for (const auto& n : m_npcs) if (n.id == id) return &n;
    return nullptr;
}

std::vector<const NPCDef*> NPCDb::in_zone(const std::string& zone) const {
    std::vector<const NPCDef*> out;
    for (const auto& n : m_npcs)
        if (n.zone == zone || n.zone == "any") out.push_back(&n);
    return out;
}

std::string NPCDb::get_line(const NPCDef& npc, Hour hour) const {
    for (const auto& sched : npc.schedules) {
        auto it = sched.lines.find(hour);
        if (it != sched.lines.end()) return it->second;
    }
    if (!npc.general_lines.empty())
        return npc.general_lines[hour % npc.general_lines.size()];
    return "";
}

} // namespace dhack
