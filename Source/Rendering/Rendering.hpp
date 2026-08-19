#pragma once
#include "Common.hpp"
namespace dot {
class Renderer {
public:
  bool init(const AppConfig& cfg);
  void beginFrame();
  void endFrame();
  void shutdown();
};
}
