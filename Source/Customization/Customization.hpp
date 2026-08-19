#pragma once
#include <string>
#include <unordered_map>
namespace dot {
class Customization {
public:
  bool applySkin(const std::string& assetId);
  bool applyWeaponSkin(const std::string& assetId);
  bool applyAccessory(const std::string& slot, const std::string& assetId);
  std::unordered_map<std::string,std::string> currentLoadout() const;
};
}
