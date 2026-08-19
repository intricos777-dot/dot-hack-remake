#pragma once
#include <string>
#include <unordered_map>
namespace dot {
struct ModManifest { std::string id; std::string version; std::string entry; };
class ModSDK {
public:
  bool loadMod(const std::string& path);
  void unloadMod(const std::string& id);
  void callHook(const std::string& name, const std::unordered_map<std::string,std::string>& args);
};
}
