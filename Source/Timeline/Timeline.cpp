#include "Timeline.hpp"
namespace dot {
bool Timeline::loadCanonArc() { return true; }
bool Timeline::loadModArc(const std::string&) { return true; }
std::vector<StoryNode> Timeline::nodes() const { return mNodes; }
}
