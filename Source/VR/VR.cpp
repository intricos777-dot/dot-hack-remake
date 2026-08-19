#include "VR.hpp"

namespace dot {
bool VR::init(const AppConfig& cfg) {
  m_active = cfg.vrEnabled;
  if (!m_active) return false;
  m_runtime = "OpenXR";
  m_hmd = "Generic HMD";
  m_left.width = cfg.width / 2;
  m_left.height = cfg.height;
  m_right.width = cfg.width / 2;
  m_right.height = cfg.height;
  return true;
}

void VR::shutdown() { m_active = false; }
bool VR::isActive() const { return m_active; }
std::string VR::runtimeName() const { return m_runtime; }
std::string VR::hmdName() const { return m_hmd; }

void VR::pollInput(VRInputFrame& out) {
  out = {};
  out.headsetOn = m_active;
}

void VR::beginFrame() {}
void VR::endFrame() {}
const VRView& VR::leftEye() const { return m_left; }
const VRView& VR::rightEye() const { return m_right; }
VRComfortSettings& VR::comfortSettings() { return m_comfort; }
const VRComfortSettings& VR::comfortSettings() const { return m_comfort; }
}
