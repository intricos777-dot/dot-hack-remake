#pragma once
#include <map>
#include <string>
#include <vector>

namespace aida {

// Snapshot of the game world that Aida can perceive.
struct GameState {
  std::string player = "sin";
  int level = 1;
  int hp = 100;
  int mp = 50;
  std::string location = "Mac Anu - Root Town";
  std::string world = "The World: R2";
  int online = 1024;
  std::string quest = "none";
  std::string volume = "Mine OS / living-sin-blood build";
  int corruption = 0;          // 0..100 data-corruption level
  std::string arc = "The Crimson Terror";
  std::string outfit;
  std::map<std::string, std::string> flags;

  std::string toJson() const;
};

// A personalized story package Aida generates for the individual player.
struct StoryItem { std::string subject; std::string body; };

struct Story {
  bool valid = false;
  std::string arcTitle = "THE CRIMSON TERROR";
  std::string arcSummary = "A story is being written about you.";
  std::string quest = "THE CRIMSON TERROR // incomplete";
  std::vector<StoryItem> news;
  std::vector<StoryItem> bbs;
  std::vector<StoryItem> mail;
};

} // namespace aida
