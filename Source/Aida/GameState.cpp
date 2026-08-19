#include "GameState.hpp"

namespace aida {

static std::string esc(const std::string& s) {
  std::string o;
  for (char c : s) {
    if (c == '"' || c == '\\') { o += '\\'; o += c; }
    else if (c == '\n') o += "\\n";
    else o += c;
  }
  return o;
}

std::string GameState::toJson() const {
  std::string f;
  for (const auto& [k, v] : flags) f += ",\"" + esc(k) + "\":\"" + esc(v) + "\"";
  return "{"
    "\"player\":\"" + esc(player) + "\","
    "\"level\":" + std::to_string(level) + ","
    "\"hp\":" + std::to_string(hp) + ","
    "\"mp\":" + std::to_string(mp) + ","
    "\"location\":\"" + esc(location) + "\","
    "\"world\":\"" + esc(world) + "\","
    "\"online\":" + std::to_string(online) + ","
    "\"quest\":\"" + esc(quest) + "\","
    "\"volume\":\"" + esc(volume) + "\","
    "\"corruption\":" + std::to_string(corruption) + ","
    "\"arc\":\"" + esc(arc) + "\","
    "\"outfit\":\"" + esc(outfit) + "\""
    + f + "}";
}

} // namespace aida
