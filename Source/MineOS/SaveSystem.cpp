#include "SaveSystem.hpp"
#include <fstream>
#include <sstream>
#include <ctime>
#include <sys/stat.h>

namespace mine {

SaveSystem& SaveSystem::instance() {
    static SaveSystem inst;
    return inst;
}

std::string SaveSystem::getSavesDir() const {
    return "/home/sin/Projects/dot-hack-remake/Content/Saves";
}

std::string SaveSystem::getSavePath(int slot) const {
    return getSavesDir() + "/save_" + std::to_string(slot) + ".json";
}

std::string SaveSystem::getAutoSavePath() const {
    return getSavesDir() + "/autosave.json";
}

bool SaveSystem::save(int slot, const SaveData& data) {
    std::string dir = getSavesDir();
    mkdir(dir.c_str(), 0755);

    std::string path = getSavePath(slot);
    std::ofstream f(path);
    if (!f.good()) return false;
    f << data.toJson();
    return true;
}

bool SaveSystem::load(int slot, SaveData& data) {
    std::string path = getSavePath(slot);
    std::ifstream f(path);
    if (!f.good()) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    data = SaveData::fromJson(ss.str());
    return true;
}

bool SaveSystem::autoSave(const SaveData& data) {
    std::string dir = getSavesDir();
    mkdir(dir.c_str(), 0755);

    std::ofstream f(getAutoSavePath());
    if (!f.good()) return false;
    f << data.toJson();
    return true;
}

bool SaveSystem::loadAuto(SaveData& data) {
    std::ifstream f(getAutoSavePath());
    if (!f.good()) return false;
    std::stringstream ss;
    ss << f.rdbuf();
    data = SaveData::fromJson(ss.str());
    return true;
}

bool SaveSystem::portSave(const std::string& from_disk, const std::string& to_disk, SaveData& data) {
    // Validate port
    if (!canPort(from_disk, to_disk)) return false;

    // Update disk
    data.current_disk = to_disk;

    // R1 -> R2 transition: reset level to 1
    if ((from_disk == "vol3_a") && (to_disk == "vol3_a_pc")) {
        data.level = 1;
        data.exp = 0;
    }

    // Save
    return autoSave(data);
}

bool SaveSystem::canPort(const std::string& from_disk, const std::string& to_disk) {
    // Define valid ports
    static const std::map<std::string, std::string> valid_ports = {
        {"vol1_i", "vol2_a"},
        {"vol2_a", "vol2_a_pc"},
        {"vol2_a_pc", "vol3_a"},
        {"vol3_a", "vol3_a_pc"},
        {"vol3_a_pc", "vol4_a"},
        {"vol4_a", "vol4_a_pc"},
        {"vol4_a_pc", "vol4_i"},
        {"link_shugo", "link_rose"}
    };

    auto it = valid_ports.find(from_disk);
    if (it != valid_ports.end() && it->second == to_disk) return true;
    return false;
}

std::vector<int> SaveSystem::listSaves() {
    std::vector<int> saves;
    std::string dir = getSavesDir();
    for (int i = 1; i <= 99; i++) {
        std::string path = getSavePath(i);
        std::ifstream f(path);
        if (f.good()) saves.push_back(i);
    }
    return saves;
}

bool SaveSystem::deleteSave(int slot) {
    std::string path = getSavePath(slot);
    return remove(path.c_str()) == 0;
}

bool SaveSystem::exists(int slot) {
    std::ifstream f(getSavePath(slot));
    return f.good();
}

std::string SaveData::toJson() const {
    std::string json = "{";
    json += "\"player_name\":\"" + player_name + "\",";
    json += "\"level\":" + std::to_string(level) + ",";
    json += "\"hp\":" + std::to_string(hp) + ",";
    json += "\"mp\":" + std::to_string(mp) + ",";
    json += "\"exp\":" + std::to_string(exp) + ",";
    json += "\"current_disk\":\"" + current_disk + "\",";
    json += "\"current_zone\":\"" + current_zone + "\",";
    json += "\"play_time\":" + std::to_string(play_time) + ",";
    json += "\"timestamp\":\"" + timestamp + "\"";
    json += "}";
    return json;
}

SaveData SaveData::fromJson(const std::string& json) {
    SaveData data;
    // Minimal JSON parsing
    size_t pos = 0;
    auto extractStr = [&](const std::string& key) -> std::string {
        std::string search = "\"" + key + "\":\"";
        size_t start = json.find(search);
        if (start == std::string::npos) return "";
        start += search.size();
        size_t end = json.find("\"", start);
        if (end == std::string::npos) return "";
        return json.substr(start, end - start);
    };
    auto extractInt = [&](const std::string& key) -> int {
        std::string search = "\"" + key + "\":";
        size_t start = json.find(search);
        if (start == std::string::npos) return 0;
        start += search.size();
        size_t end = json.find_first_of(",}", start);
        if (end == std::string::npos) return 0;
        return std::stoi(json.substr(start, end - start));
    };

    data.player_name = extractStr("player_name");
    data.level = extractInt("level");
    data.hp = extractInt("hp");
    data.mp = extractInt("mp");
    data.exp = extractInt("exp");
    data.current_disk = extractStr("current_disk");
    data.current_zone = extractStr("current_zone");
    data.play_time = extractInt("play_time");
    data.timestamp = extractStr("timestamp");
    return data;
}

} // namespace mine
