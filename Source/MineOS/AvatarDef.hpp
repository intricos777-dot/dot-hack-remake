#pragma once
#include <string>
#include <vector>
#include <map>

namespace mine {

// Avatar appearance definition
struct AvatarAppearance {
    std::string body_type;
    struct { std::string style; std::string color; } hair;
    struct { std::string shape; std::string color; } eyes;
    std::string skin;
    struct {
        std::string type;
        std::string color;
        std::string accents;
    } outfit_top, outfit_bottom;
    std::vector<std::string> accessories;
    std::vector<std::string> weapons;
    float height = 1.0f;
};

// Animation definition
struct Animation {
    std::string name;
    int frames = 60;
    bool loop = true;
    float fps = 30.0f;
    std::vector<std::string> bone_tracks;  // which bones this anim affects
    std::vector<std::string> morph_tracks; // face morph targets
};

// Character definition loaded from JSON
struct CharacterDef {
    std::string name;
    std::string title;
    std::string arc;
    std::string character_class;
    std::vector<std::string> disks;
    std::string description;
    AvatarAppearance appearance;
    std::map<std::string, Animation> animations;
    struct {
        std::string base_voice;
        float pitch_shift = 0.0f;
        float reverb = 0.0f;
        float distortion = 0.0f;
        float speed = 1.0f;
        bool glitch = false;
    } voice;
    std::vector<std::string> voice_clips;
};

// Load character definitions from JSON
std::vector<CharacterDef> loadCharacters(const std::string& jsonPath);
const CharacterDef* findCharacter(const std::vector<CharacterDef>& chars, const std::string& name);
const CharacterDef* findCharacterForDisk(const std::vector<CharacterDef>& chars, const std::string& diskId);

} // namespace mine
