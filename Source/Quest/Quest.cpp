#include "Quest.hpp"
#include <iostream>
#include <fstream>
#include <sstream>

namespace dot {

QuestManager::QuestManager() {}
QuestManager::~QuestManager() {}

bool QuestManager::loadCanonQuests() {
    // Load quest data from all disc quest files
    std::vector<std::string> questFiles = {
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/infection_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/frequency_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/outbreak_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/quarantine_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/link_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/quantum_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/findme_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/sign_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/gu_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/gu_vol1_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/gu_vol2_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/gu_vol3_quests.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Quests/gu_vol4_quests.json"
    };
    
    for (const auto& file : questFiles) {
        std::ifstream f(file);
        if (!f.good()) {
            std::cerr << "[QuestManager] Warning: Cannot open " << file << std::endl;
            continue;
        }
        
        std::stringstream ss;
        ss << f.rdbuf();
        std::string json = ss.str();
        
        // Simple JSON parsing for quest data
        parseQuestFile(json);
    }
    
    std::cout << "[QuestManager] Loaded " << m_quests.size() << " quests" << std::endl;
    return !m_quests.empty();
}

void QuestManager::parseQuestFile(const std::string& json) {
    // Find quests array
    size_t pos = json.find("\"quests\"");
    if (pos == std::string::npos) return;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return;
    pos++;
    
    std::string disc_id;
    std::string protagonist;
    std::string arc;
    
    // Find disc info
    size_t disc_pos = json.find("\"disc_id\"");
    if (disc_pos != std::string::npos) {
        size_t quote = json.find('"', disc_pos + 10);
        if (quote != std::string::npos) {
            size_t end = json.find('"', quote + 1);
            disc_id = json.substr(quote + 1, end - quote - 1);
        }
    }
    
    while (pos < json.size()) {
        // Skip whitespace
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\r' || json[pos] == '\t' || json[pos] == ',')) pos++;
        if (pos >= json.size() || json[pos] == ']') break;
        if (json[pos] != '{') { pos++; continue; }
        
        Quest quest;
        quest.disc_id = disc_id;
        quest.arc = arc;
        quest.status = QuestStatus::Available;
        pos++;
        
        while (pos < json.size()) {
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\t')) pos++;
            if (pos >= json.size() || json[pos] == '}') { pos++; break; }
            if (json[pos] != '"') { pos++; continue; }
            
            // Read key
            pos++;
            size_t key_start = pos;
            while (pos < json.size() && json[pos] != '"') pos++;
            std::string key = json.substr(key_start, pos - key_start);
            if (pos < json.size()) pos++;
            
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':')) pos++;
            if (pos >= json.size()) break;
            
            if (json[pos] == '"') {
                pos++;
                size_t val_start = pos;
                while (pos < json.size() && json[pos] != '"') pos++;
                std::string val = json.substr(val_start, pos - val_start);
                if (pos < json.size()) pos++;
                
                if (key == "id") quest.id = val;
                else if (key == "title") quest.title = val;
                else if (key == "description") quest.description = val;
                else if (key == "type") {
                    if (val == "main") quest.type = QuestType::Main;
                    else if (val == "side") quest.type = QuestType::Side;
                    else if (val == "dungeon") quest.type = QuestType::Dungeon;
                    else quest.type = QuestType::Main;
                }
                else if (key == "giver") quest.giver_npc = val;
                else if (key == "turnin") quest.turnin_npc = val;
                else if (key == "arc") quest.arc = val;
            }
            else if (json[pos] == '{') {
                // Parse reward object
                if (key == "reward") {
                    while (pos < json.size()) {
                        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\t')) pos++;
                        if (pos >= json.size() || json[pos] == '}') { pos++; break; }
                        if (json[pos] == '"') {
                            pos++;
                            size_t kstart = pos;
                            while (pos < json.size() && json[pos] != '"') pos++;
                            std::string k = json.substr(kstart, pos - kstart);
                            if (pos < json.size()) pos++;
                            while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':')) pos++;
                            if (pos < json.size() && json[pos] == '"') {
                                pos++;
                                size_t vstart = pos;
                                while (pos < json.size() && json[pos] != '"') pos++;
                                std::string v = json.substr(vstart, pos - vstart);
                                if (pos < json.size()) pos++;
                                if (k == "exp") quest.reward.exp = std::stoi(v);
                                else if (k == "gold") quest.reward.gold = std::stoi(v);
                            }
                            else if (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
                                int v = 0;
                                while (pos < json.size() && json[pos] >= '0' && json[pos] <= '9') {
                                    v = v * 10 + (json[pos] - '0');
                                    pos++;
                                }
                                if (k == "exp") quest.reward.exp = v;
                                else if (k == "gold") quest.reward.gold = v;
                            }
                        }
                        else pos++;
                    }
                }
            }
        }
        
        if (!quest.id.empty()) {
            m_quests[quest.id] = quest;
        }
    }
}

bool QuestManager::loadCanonLevels() {
    std::vector<std::string> levelFiles = {
        std::string(MINE_OS_CONTENT_DIR) + "/Levels/infection_levels.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Levels/gu_levels.json",
        std::string(MINE_OS_CONTENT_DIR) + "/Levels/sign_levels.json"
    };
    
    for (const auto& file : levelFiles) {
        std::ifstream f(file);
        if (!f.good()) {
            std::cerr << "[QuestManager] Warning: Cannot open " << file << std::endl;
            continue;
        }
        std::stringstream ss;
        ss << f.rdbuf();
        std::string json = ss.str();
        parseLevelFile(json);
    }
    
    std::cout << "[QuestManager] Loaded " << m_levels.size() << " levels" << std::endl;
    return !m_levels.empty();
}

void QuestManager::parseLevelFile(const std::string& json) {
    size_t pos = json.find("\"levels\"");
    if (pos == std::string::npos) return;
    pos = json.find('[', pos);
    if (pos == std::string::npos) return;
    pos++;
    
    while (pos < json.size()) {
        while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\t' || json[pos] == ',')) pos++;
        if (pos >= json.size() || json[pos] == ']') break;
        if (json[pos] != '{') { pos++; continue; }
        
        DungeonLevel level;
        level.layout_type = "linear";
        pos++;
        
        while (pos < json.size()) {
            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\t')) pos++;
            if (pos >= json.size() || json[pos] == '}') { pos++; break; }
            if (json[pos] == '"') {
                pos++;
                size_t key_start = pos;
                while (pos < json.size() && json[pos] != '"') pos++;
                std::string key = json.substr(key_start, pos - key_start);
                if (pos < json.size()) pos++;
                while (pos < json.size() && (json[pos] == ' ' || json[pos] == ':')) pos++;
                
                if (pos < json.size() && json[pos] == '"') {
                    pos++;
                    size_t val_start = pos;
                    while (pos < json.size() && json[pos] != '"') pos++;
                    std::string val = json.substr(val_start, pos - val_start);
                    if (pos < json.size()) pos++;
                    
                    if (key == "id") level.id = val;
                    else if (key == "name") level.name = val;
                    else if (key == "zone_id") level.zone_id = val;
                    else if (key == "description") level.description = val;
                    else if (key == "layout_type") level.layout_type = val;
                }
                else if (pos < json.size() && json[pos] == '[') {
                    if (key == "floors") {
                        // Parse floors array - simplified
                        level.floor_count = 0;
                        while (pos < json.size()) {
                            while (pos < json.size() && (json[pos] == ' ' || json[pos] == '\n' || json[pos] == '\t' || json[pos] == ',')) pos++;
                            if (pos >= json.size() || json[pos] == ']') break;
                            if (json[pos] == '{') {
                                level.floor_count++;
                                // Skip floor object details for now
                                int depth = 0;
                                while (pos < json.size()) {
                                    if (json[pos] == '{') depth++;
                                    else if (json[pos] == '}') { depth--; pos++; if (depth == 0) break; }
                                    pos++;
                                }
                            }
                            else pos++;
                        }
                    }
                }
            }
            else pos++;
        }
        
        if (!level.id.empty()) {
            m_levels[level.id] = level;
        }
    }
}

// Quest operations
bool QuestManager::startQuest(const std::string& questId) {
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    if (it->second.status != QuestStatus::Available) return false;
    it->second.status = QuestStatus::Active;
    std::cout << "[Quest] Started: " << it->second.title << std::endl;
    return true;
}

bool QuestManager::completeObjective(const std::string& questId, int objectiveIndex) {
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    if (objectiveIndex >= (int)it->second.objectives.size()) return false;
    it->second.objectives[objectiveIndex].completed = true;
    return true;
}

bool QuestManager::advanceStage(const std::string& questId) {
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    if (it->second.current_stage < (int)it->second.stages.size() - 1) {
        it->second.current_stage++;
        return true;
    }
    return false;
}

bool QuestManager::completeQuest(const std::string& questId) {
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    it->second.status = QuestStatus::Completed;
    std::cout << "[Quest] Completed: " << it->second.title << std::endl;
    
    // Unlock next quest in chain
    if (!it->second.next_quest_in_chain.empty()) {
        auto next = m_quests.find(it->second.next_quest_in_chain);
        if (next != m_quests.end()) {
            next->second.status = QuestStatus::Available;
        }
    }
    return true;
}

bool QuestManager::failQuest(const std::string& questId) {
    auto it = m_quests.find(questId);
    if (it == m_quests.end()) return false;
    it->second.status = QuestStatus::Failed;
    return true;
}

// Queries
std::vector<Quest*> QuestManager::getAvailableQuests() {
    std::vector<Quest*> result;
    for (auto& [id, q] : m_quests) {
        if (q.status == QuestStatus::Available) result.push_back(&q);
    }
    return result;
}

std::vector<Quest*> QuestManager::getActiveQuests() {
    std::vector<Quest*> result;
    for (auto& [id, q] : m_quests) {
        if (q.status == QuestStatus::Active) result.push_back(&q);
    }
    return result;
}

std::vector<Quest*> QuestManager::getCompletedQuests() {
    std::vector<Quest*> result;
    for (auto& [id, q] : m_quests) {
        if (q.status == QuestStatus::Completed) result.push_back(&q);
    }
    return result;
}

std::vector<Quest*> QuestManager::getQuestsForDisc(const std::string& discId) {
    std::vector<Quest*> result;
    for (auto& [id, q] : m_quests) {
        if (q.disc_id == discId) result.push_back(&q);
    }
    return result;
}

std::vector<Quest*> QuestManager::getQuestsForZone(const std::string& zoneId) {
    std::vector<Quest*> result;
    for (auto& [id, q] : m_quests) {
        for (const auto& obj : q.objectives) {
            if (obj.target.find(zoneId) != std::string::npos) {
                result.push_back(&q);
                break;
            }
        }
    }
    return result;
}

std::vector<Quest*> QuestManager::getQuestsForNpc(const std::string& npcId) {
    std::vector<Quest*> result;
    for (auto& [id, q] : m_quests) {
        if (q.giver_npc == npcId || q.turnin_npc == npcId) {
            result.push_back(&q);
        }
    }
    return result;
}

Quest* QuestManager::getQuest(const std::string& questId) {
    auto it = m_quests.find(questId);
    if (it != m_quests.end()) return &it->second;
    return nullptr;
}

DungeonLevel* QuestManager::getDungeon(const std::string& dungeonId) {
    auto it = m_levels.find(dungeonId);
    if (it != m_levels.end()) return &it->second;
    return nullptr;
}

std::vector<DungeonLevel*> QuestManager::getDungeonsForZone(const std::string& zoneId) {
    std::vector<DungeonLevel*> result;
    for (auto& [id, l] : m_levels) {
        if (l.zone_id == zoneId) result.push_back(&l);
    }
    return result;
}

std::vector<DungeonLevel*> QuestManager::getDungeonsForDisc(const std::string& discId) {
    std::vector<DungeonLevel*> result;
    for (auto& [id, l] : m_levels) {
        if (l.disc_id == discId) result.push_back(&l);
    }
    return result;
}

// AIDA integration
Quest* QuestManager::generateAIDAVariant(const std::string& baseQuestId, const std::string& reason) {
    auto base = getQuest(baseQuestId);
    if (!base) return nullptr;
    
    Quest variant = *base;
    variant.id = base->id + "_aida_" + std::to_string(m_quests.size());
    variant.is_aida_variant = true;
    variant.base_quest_id = baseQuestId;
    variant.title = "[AIDA] " + base->title;
    variant.description = "[AIDA Modified] " + base->description + "\n\nAIDA's reason: " + reason;
    variant.can_be_modified_by_aida = false;
    
    m_quests[variant.id] = variant;
    return &m_quests[variant.id];
}

void QuestManager::applyAIDAModification(const std::string& questId, const AIDAModification& mod) {
    auto quest = getQuest(questId);
    if (!quest) return;
    
    switch (mod.type) {
        case AIDAModification::AddObjective:
            quest->objectives.push_back(mod.new_objective);
            break;
        case AIDAModification::ChangeTarget:
            for (auto& obj : quest->objectives) {
                if (obj.target == mod.old_target) obj.target = mod.new_target;
            }
            break;
        case AIDAModification::BuffEnemy:
            // Apply buff to enemies in level
            break;
        default:
            break;
    }
    
    m_activeModifications.push_back(mod);
}

void QuestManager::revertAIDAModification(const std::string& questId) {
    for (auto it = m_activeModifications.begin(); it != m_activeModifications.end(); ) {
        if (it->target_quest_id == questId) {
            it = m_activeModifications.erase(it);
        }
        else {
            ++it;
        }
    }
}

Quest* QuestManager::generateAIDASideQuest(const std::string& context) {
    Quest quest;
    quest.id = "aida_side_" + std::to_string(m_quests.size());
    quest.title = "[AIDA] " + context;
    quest.description = "AIDA has created this quest based on your actions.";
    quest.type = QuestType::Side;
    quest.is_aida_variant = true;
    quest.status = QuestStatus::Available;
    quest.can_be_modified_by_aida = false;
    quest.level_required = 1;
    quest.difficulty = QuestDifficulty::Normal;
    
    QuestObjective obj;
    obj.id = "aida_obj_1";
    obj.text = "Investigate the anomaly";
    obj.type = "explore";
    obj.target = "unknown";
    quest.objectives.push_back(obj);
    
    QuestReward reward;
    reward.exp = 500;
    reward.gold = 100;
    reward.items = {"AIDA Shard"};
    quest.reward = reward;
    
    m_quests[quest.id] = quest;
    return &m_quests[quest.id];
}

// Save/Load
bool QuestManager::saveQuests(const std::string& path) {
    std::ofstream f(path);
    if (!f.good()) return false;
    f << "{\n  \"quests\": [\n";
    bool first = true;
    for (const auto& [id, q] : m_quests) {
        if (!first) f << ",\n";
        first = false;
        f << "    {\n";
        f << "      \"id\": \"" << q.id << "\",\n";
        f << "      \"title\": \"" << q.title << "\",\n";
        f << "      \"status\": " << (int)q.status << ",\n";
        f << "      \"stage\": " << q.current_stage << "\n";
        f << "    }";
    }
    f << "\n  ]\n}\n";
    return true;
}

bool QuestManager::loadQuests(const std::string& path) {
    return loadCanonQuests();
}

int QuestManager::completedQuestCount() const {
    int count = 0;
    for (const auto& [id, q] : m_quests) {
        if (q.status == QuestStatus::Completed) count++;
    }
    return count;
}

int QuestManager::activeQuestCount() const {
    int count = 0;
    for (const auto& [id, q] : m_quests) {
        if (q.status == QuestStatus::Active) count++;
    }
    return count;
}

} // namespace dot
