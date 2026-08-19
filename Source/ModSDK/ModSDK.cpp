#include "ModSDK.hpp"
namespace dot {
bool ModSDK::loadMod(const std::string&) { return true; }
void ModSDK::unloadMod(const std::string&) {}
void ModSDK::callHook(const std::string&, const std::unordered_map<std::string,std::string>&) {}
}
