#pragma once
#include "Common.hpp"
namespace dot {
class Physics {
public:
  bool init(const AppConfig& cfg);
  void step(float dt);
  void shutdown();
};
}
