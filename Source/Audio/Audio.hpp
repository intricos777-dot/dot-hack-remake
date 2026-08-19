#pragma once
#include <string>
namespace dot {
class Audio {
public:
  bool load(const std::string& path);
  void play();
  void stop();
};
}
