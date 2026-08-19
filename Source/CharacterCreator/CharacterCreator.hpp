#pragma once
#include <string>
namespace dot {
struct CharacterPreset { std::string presetId; std::string name; std::string playerClass; };
class CharacterCreator {
public:
  bool createFromPreset(const CharacterPreset& preset);
  bool randomize();
};
}
