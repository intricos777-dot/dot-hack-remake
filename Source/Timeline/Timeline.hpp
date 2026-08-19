#pragma once
#include <string>
#include <vector>
namespace dot {
struct StoryNode { std::string id; std::string title; std::string characterId; };
class Timeline {
public:
  bool loadCanonArc();
  bool loadModArc(const std::string& modId);
  std::vector<StoryNode> nodes() const;
private:
  std::vector<StoryNode> mNodes;
};
}
