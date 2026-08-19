#include "AvatarDef.hpp"
#include <fstream>
#include <sstream>

namespace mine {

static std::string extractString(const std::string& s, size_t& pos) {
    while (pos < s.size() && s[pos] != '"') pos++;
    if (pos >= s.size()) return "";
    pos++;
    size_t start = pos;
    while (pos < s.size() && s[pos] != '"') {
        if (s[pos] == '\\') pos++;
        pos++;
    }
    std::string result = s.substr(start, pos - start);
    if (pos < s.size()) pos++;
    return result;
}

static int extractInt(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] < '0' || s[pos] > '9') && s[pos] != '-') pos++;
    int sign = 1;
    if (pos < s.size() && s[pos] == '-') { sign = -1; pos++; }
    int result = 0;
    while (pos < s.size() && s[pos] >= '0' && s[pos] <= '9') {
        result = result * 10 + (s[pos] - '0');
        pos++;
    }
    return result * sign;
}

static float extractFloat(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] < '0' || s[pos] > '9') && s[pos] != '-' && s[pos] != '.') pos++;
    std::string num;
    while (pos < s.size() && ((s[pos] >= '0' && s[pos] <= '9') || s[pos] == '-' || s[pos] == '.')) {
        num += s[pos++];
    }
    return num.empty() ? 0.0f : std::stof(num);
}

static void skipWhitespace(const std::string& s, size_t& pos) {
    while (pos < s.size() && (s[pos] == ' ' || s[pos] == '\n' || s[pos] == '\r' || s[pos] == '\t')) pos++;
}

std::vector<CharacterDef> loadCharacters(const std::string& jsonPath) {
    std::vector<CharacterDef> result;
    std::ifstream f(jsonPath);
    if (!f.good()) return result;

    std::stringstream ss;
    ss << f.rdbuf();
    std::string json = ss.str();

    size_t pos = json.find("\"characters\"");
    if (pos == std::string::npos) return result;

    size_t objStart = json.find('{', pos);
    if (objStart == std::string::npos) return result;

    // Iterate character objects
    size_t charPos = objStart + 1;
    while (charPos < json.size()) {
        skipWhitespace(json, charPos);
        if (charPos >= json.size() || json[charPos] == '}') break;
        if (json[charPos] != '"') { charPos++; continue; }

        std::string charName = extractString(json, charPos);
        skipWhitespace(json, charPos);
        if (charPos < json.size() && json[charPos] == ':') charPos++;
        skipWhitespace(json, charPos);
        if (charPos >= json.size() || json[charPos] != '{') { charPos++; continue; }

        CharacterDef def;
        def.name = charName;
        charPos++; // skip {

        // Parse character object
        while (charPos < json.size()) {
            skipWhitespace(json, charPos);
            if (charPos >= json.size() || json[charPos] == '}') { charPos++; break; }
            if (json[charPos] != '"') { charPos++; continue; }

            std::string key = extractString(json, charPos);
            skipWhitespace(json, charPos);
            if (charPos < json.size() && json[charPos] == ':') charPos++;
            skipWhitespace(json, charPos);
            if (charPos >= json.size()) break;

            if (json[charPos] == '"') {
                std::string val = extractString(json, charPos);
                if (key == "arc") def.arc = val;
                else if (key == "class") def.character_class = val;
                else if (key == "description") def.description = val;
            } else if (json[charPos] == '[') {
                charPos++;
                if (key == "disks") {
                    while (charPos < json.size() && json[charPos] != ']') {
                        skipWhitespace(json, charPos);
                        if (json[charPos] == ',') { charPos++; continue; }
                        if (json[charPos] == '"') {
                            def.disks.push_back(extractString(json, charPos));
                        } else charPos++;
                    }
                } else if (key == "voice_clips") {
                    while (charPos < json.size() && json[charPos] != ']') {
                        skipWhitespace(json, charPos);
                        if (json[charPos] == ',') { charPos++; continue; }
                        if (json[charPos] == '"') {
                            def.voice_clips.push_back(extractString(json, charPos));
                        } else charPos++;
                    }
                }
                if (charPos < json.size() && json[charPos] == ']') charPos++;
            } else if (json[charPos] == '{') {
                // Nested object (appearance, voice, animations)
                charPos++;
                if (key == "voice") {
                    while (charPos < json.size()) {
                        skipWhitespace(json, charPos);
                        if (charPos >= json.size() || json[charPos] == '}') { charPos++; break; }
                        if (json[charPos] != '"') { charPos++; continue; }
                        std::string vkey = extractString(json, charPos);
                        skipWhitespace(json, charPos);
                        if (charPos < json.size() && json[charPos] == ':') charPos++;
                        skipWhitespace(json, charPos);
                        if (charPos >= json.size()) break;
                        if (json[charPos] == '"') {
                            extractString(json, charPos); // skip string values
                        } else if (json[charPos] == 't' || json[charPos] == 'f') {
                            def.voice.glitch = (json[charPos] == 't');
                            while (charPos < json.size() && json[charPos] != ',' && json[charPos] != '}') charPos++;
                        } else {
                            float v = extractFloat(json, charPos);
                            if (vkey == "pitch_shift") def.voice.pitch_shift = v;
                            else if (vkey == "reverb") def.voice.reverb = v;
                            else if (vkey == "distortion") def.voice.distortion = v;
                            else if (vkey == "speed") def.voice.speed = v;
                        }
                        skipWhitespace(json, charPos);
                        if (charPos < json.size() && json[charPos] == ',') charPos++;
                    }
                } else {
                    // Skip other nested objects
                    int depth = 1;
                    while (charPos < json.size() && depth > 0) {
                        if (json[charPos] == '{') depth++;
                        else if (json[charPos] == '}') depth--;
                        charPos++;
                    }
                }
            } else if (json[charPos] >= '0' && json[charPos] <= '9') {
                extractFloat(json, charPos);
            }

            skipWhitespace(json, charPos);
            if (charPos < json.size() && json[charPos] == ',') charPos++;
        }

        if (!def.name.empty()) {
            result.push_back(def);
        }

        skipWhitespace(json, charPos);
        if (charPos < json.size() && json[charPos] == ',') charPos++;
    }

    return result;
}

const CharacterDef* findCharacter(const std::vector<CharacterDef>& chars, const std::string& name) {
    for (const auto& c : chars) {
        if (c.name == name) return &c;
    }
    return nullptr;
}

const CharacterDef* findCharacterForDisk(const std::vector<CharacterDef>& chars, const std::string& diskId) {
    for (const auto& c : chars) {
        for (const auto& d : c.disks) {
            if (d == diskId) return &c;
        }
    }
    return nullptr;
}

} // namespace mine
