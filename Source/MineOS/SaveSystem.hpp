#pragma once
#include <string>
#include <vector>
#include <map>

namespace mine {

// Save data structure
struct SaveData {
    std::string player_name = "sin";
    int level = 1;
    int hp = 100;
    int mp = 50;
    int exp = 0;
    std::string current_disk;
    std::string current_zone;
    float pos_x = 0.0f, pos_y = 0.0f, pos_z = 0.0f;
    float rotation = 0.0f;
    std::vector<std::string> items;
    std::vector<std::string> skills;
    std::map<std::string, bool> story_flags;
    int play_time = 0;
    std::string timestamp;

    std::string toJson() const;
    static SaveData fromJson(const std::string& json);
};

// Save system
class SaveSystem {
public:
    static SaveSystem& instance();

    // Save/Load
    bool save(int slot, const SaveData& data);
    bool load(int slot, SaveData& data);
    bool autoSave(const SaveData& data);
    bool loadAuto(SaveData& data);

    // Disk porting
    bool portSave(const std::string& from_disk, const std::string& to_disk, SaveData& data);

    // Save management
    std::vector<int> listSaves();
    bool deleteSave(int slot);
    bool exists(int slot);

    // Port validation
    bool canPort(const std::string& from_disk, const std::string& to_disk);

private:
    SaveSystem() = default;
    std::string getSavePath(int slot) const;
    std::string getAutoSavePath() const;
    std::string getSavesDir() const;
};

} // namespace mine
