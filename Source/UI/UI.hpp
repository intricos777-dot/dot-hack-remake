#pragma once
#include "Common.hpp"
namespace dot {
class UI {
public:
  bool init(const AppConfig& cfg);
  void showCharacterCreator();
  void showModManager();
  void showTimelineSelector();
};
}
