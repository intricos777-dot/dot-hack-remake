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

// Lia.CLI task bridge: spawns scripts/lia_bridge.py and talks JSON lines
class LiaBridge {
public:
  bool start(const std::string& python, const std::string& script);
  void stop();
  bool running() const { return running_; }
  bool alive() const { return alive_; }

  // Send task operations
  void createTask(const std::string& project, const std::string& title, const std::string& context);
  void queryTasks(const std::string& project);
  void updateTask(const std::string& id, const std::string& action, const std::string& result);
  void getSummary(const std::string& project);

  // Poll for responses
  std::vector<std::map<std::string, std::string>> poll();

private:
  void readerLoop();
  void writeLine(const std::string& line);
  static std::map<std::string, std::string> parseLine(const std::string& line);

  std::atomic<bool> running_{false};
  std::atomic<bool> alive_{false};
  int inFd_ = -1;
  int outFd_ = -1;
  int pid_ = -1;
  std::thread rd_;
  std::mutex mu_;
  std::deque<std::map<std::string, std::string>> q_;
  uint64_t seq_ = 1;
};

} // namespace aida
