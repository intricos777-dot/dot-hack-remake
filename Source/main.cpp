#include "Common.hpp"
#include "Rendering.hpp"
#include "WorldState.hpp"
#include "Timeline.hpp"
#include "CharacterCreator.hpp"
#include "Customization.hpp"
#include "Quest/Quest.hpp"
#include "ModSDK.hpp"
#include "Networking.hpp"
#include "UI.hpp"
#include "VR.hpp"
#include "MineOS/MineOS.hpp"
#include <iostream>
int main() {
  dot::AppConfig cfg;
  cfg.title = ".hack World Remake";
  cfg.vrEnabled = true;
  dot::Renderer renderer;
  dot::WorldState world;
  dot::Timeline timeline;
  dot::CharacterCreator creator;
  dot::Customization customization;
  dot::QuestManager quests;
  quests.loadCanonQuests();
  quests.loadCanonLevels();
  dot::ModSDK mods;
  dot::WorldNetwork net;
  dot::UI ui;
  dot::VR vr;
  if (!renderer.init(cfg)) return 1;
  if (cfg.vrEnabled) {
    if (vr.init(cfg)) {
      std::cout << "VR active: " << vr.runtimeName() << " / " << vr.hmdName() << std::endl;
    } else {
      std::cout << "VR requested but inactive" << std::endl;
    }
  }
  world.loadCanonTimeline();
  timeline.loadCanonArc();
  vr.beginFrame();
  vr.endFrame();
  std::cout << "The World: initialized" << std::endl;
  mine::MineOS mineos;
  mineos.scriptsDir_ = "/home/sin/Projects/dot-hack-remake/scripts";
  std::cout << "MineOS scriptsDir set: " << mineos.scriptsDir_ << std::endl;
  bool initResult = mineos.init(mineos.scriptsDir_);
  std::cout << "MineOS init result: " << initResult << std::endl;
  if (!initResult) {
    std::cerr << "Failed to initialize Mine OS" << std::endl;
    return 1;
  }
  mineos.run();
  mineos.shutdown();
  return 0;
}
