#pragma once
#include <string>
namespace dot {
class WorldNetwork {
public:
  bool connectToServer(const std::string& host, int port);
  void update();
};
}
