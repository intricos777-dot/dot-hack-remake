#pragma once
#include "MineOS.hpp"
#include "Input.hpp"
#include "Widgets.hpp"

namespace mine {

class AidaApp : public App {
public:
  explicit AidaApp(MineOS* os) : os_(os) {}
  const char* title() const override { return "AIDA // LINK"; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  TextField input_;
  bool awaiting_ = false;
  int scroll_ = 0;   // lines scrolled back from the newest
  int hoverAction_ = 0;
};

class GameLauncherApp : public App {
public:
  explicit GameLauncherApp(MineOS* os) : os_(os) {}
  const char* title() const override { return "GAME SELECTOR"; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  bool choiceMade_ = false;
  void drawStoryChoice(Renderer& r, const Rect& body, Input& in);
};

class MailApp : public App {
public:
  explicit MailApp(MineOS* os) : os_(os) {}
  const char* title() const override { return ""; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  ListBox list_;
  int offset_ = 0;
};

class BbsApp : public App {
public:
  explicit BbsApp(MineOS* os) : os_(os) {}
  const char* title() const override { return "BBS // NET SLUM"; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  ListBox list_;
  int offset_ = 0;
};

class NewsApp : public App {
public:
  explicit NewsApp(MineOS* os) : os_(os) {}
  const char* title() const override { return "THE WORLD NEWS"; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  ListBox list_;
  int offset_ = 0;
};

class SysApp : public App {
public:
  explicit SysApp(MineOS* os) : os_(os) {}
  const char* title() const override { return "SYSTEM // MONITOR"; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  int offset_ = 0;
};

class WorldApp : public App {
public:
  explicit WorldApp(MineOS* os) : os_(os) {}
  const char* title() const override { return "THE WORLD // CLIENT"; }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
};

class InfoPopupApp : public App {
public:
  InfoPopupApp(MineOS* os, const std::string& title, const std::vector<std::string>& body)
    : os_(os), title_(title), body_(body) {}
  const char* title() const override { return title_.c_str(); }
  void draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) override;

private:
  MineOS* os_;
  std::string title_;
  std::vector<std::string> body_;
  int scroll_ = 0;
};

} // namespace mine

// Disc catalog shared between terminal DISK tab and game launcher
namespace mine {
struct Disc {
  std::string id;
  std::string series;
  std::string title;
  std::string style;
  std::string code;
  std::string dataRoot;
  std::string ccsRoot;
  std::string avatar;
  bool present = false;
};
std::vector<Disc> buildDiscCatalogExternal();
}