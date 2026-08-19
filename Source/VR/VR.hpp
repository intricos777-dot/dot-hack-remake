#pragma once
#include "Common.hpp"
#include <string>
#include <cstdint>

namespace dot {
struct VRView {
  float eyeOffset[2]{0.0f, 0.0f};
  float projection[16]{};
  float view[16]{};
  int width = 0;
  int height = 0;
};

struct VRInputFrame {
  bool headsetOn = false;
  bool leftHandTrigger = false;
  bool rightHandTrigger = false;
  float leftTriggerValue = 0.0f;
  float rightTriggerValue = 0.0f;
  bool menuPressed = false;
  bool systemPressed = false;
};

struct VRComfortSettings {
  float snapTurnAngle = 45.0f;
  bool snapTurnEnabled = true;
  bool vignetteEnabled = true;
  float vignetteStrength = 1.0f;
  bool smoothLocomotionEnabled = false;
  float moveSpeed = 3.0f;
  bool comfortMode = true;
};

class VR {
public:
  bool init(const AppConfig& cfg);
  void shutdown();
  bool isActive() const;
  std::string runtimeName() const;
  std::string hmdName() const;
  void pollInput(VRInputFrame& out);
  void beginFrame();
  void endFrame();
  const VRView& leftEye() const;
  const VRView& rightEye() const;
  VRComfortSettings& comfortSettings();
  const VRComfortSettings& comfortSettings() const;
private:
  bool m_active = false;
  std::string m_runtime;
  std::string m_hmd;
  VRView m_left;
  VRView m_right;
  VRComfortSettings m_comfort;
};
}
