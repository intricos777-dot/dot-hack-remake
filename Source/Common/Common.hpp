#pragma once
#include <string>
namespace dot {
struct AppConfig {
  std::string title = ".hack World Remake";
  int width = 1920;
  int height = 1080;
  bool vrEnabled = true;
};
}
