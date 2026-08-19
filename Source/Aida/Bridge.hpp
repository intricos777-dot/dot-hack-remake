#pragma once
#include <atomic>
#include <cstdint>
#include <deque>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "GameState.hpp"

namespace aida {

struct Action {
  std::string cmd;                 // say | alert | event | world.set | effect | quest
  std::map<std::string, std::string> args;
};

struct Msg {
  enum Kind { Reply, Action_, Status, Story_ } kind = Reply;
  std::string text;
  Action action;
  Story story;
  uint64_t id = 0;
  uint64_t seq = 0;
};

// Bridge spawns scripts/aida_bridge.py (hermes-driven) and talks JSON lines
// over stdio. The game sends user text + world state; the bridge replies with
// in-character text and/or structured actions the game executes.
class Bridge {
public:
  bool start(const std::string& python, const std::string& script, const std::string& personaJson);
  void stop();
  bool running() const { return running_; }
  bool alive() const { return alive_; }

  void sendUser(const std::string& text);
  void sendState(const GameState& st, bool notify);
  void sendPlayer(const std::string& name, bool ask);
  void sendStoryRequest();
  void sendActionResult(uint64_t id, bool ok, const std::string& note);

  // messages received since last call
  std::vector<Msg> poll();

  uint64_t nextId() { return seq_++; }

private:
  void readerLoop();
  void writeLine(const std::string& line);
  static std::map<std::string, std::string> parseLine(const std::string& line);
  static std::string unescape(const std::string& s);

  std::atomic<bool> running_{false};
  std::atomic<bool> alive_{false};
  int inFd_ = -1;    // -> child stdin
  int outFd_ = -1;   // <- child stdout
  int pid_ = -1;
  std::thread rd_;
  std::mutex mu_;
  std::deque<Msg> q_;
  uint64_t seq_ = 1;
};

} // namespace aida
