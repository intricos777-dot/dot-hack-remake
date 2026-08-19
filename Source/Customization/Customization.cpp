#include "Customization.hpp"
namespace dot {
bool Customization::applySkin(const std::string&) { return true; }
bool Customization::applyWeaponSkin(const std::string&) { return true; }
bool Customization::applyAccessory(const std::string&, const std::string&) { return true; }
std::unordered_map<std::string,std::string> Customization::currentLoadout() const { return {}; }
}
