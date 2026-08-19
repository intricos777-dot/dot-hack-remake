#include "Apps.hpp"
#include "Input.hpp"
#include <string_view>
#include <SDL.h>
#include <algorithm>
#include <fstream>
#include <cstdio>

namespace mine {

// ---------------------------------------------------------------- AIDA ----

static const char* KIND_TAG(int k) {
  switch (k) {
    case L_AIDA: return "AIDA";
    case L_USER: return "YOU";
    case L_WORLD: return "THE WORLD";
    case L_EVENT: return "EVENT";
    default: return "SYS";
  }
}

static constexpr Color KIND_COLOR(int k) {
  switch (k) {
    case L_AIDA: return theme::glow;
    case L_USER: return theme::ok;
    case L_WORLD: return theme::warn;
    case L_EVENT: return theme::bloodHot;
    default: return theme::textDim;
  }
}

void AidaApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  auto f = r.fontMono(14);
  auto fb = r.fontMono(15);
  int lineH = r.textH(f) + 4;
  int statusH = 22;
  int inputH = 30;

  bool bridged = os_->bridge().running() && os_->bridge().alive();
  Color stCol = bridged ? theme::ok : theme::err;
  std::string status = bridged ? "AIDA // link: live · hermes local" : "AIDA // link: DOWN · scripted voice";
  if (awaiting_) status += " · thinking...";

  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);

  r.rect(body.x + 1, body.y + 1, body.w - 2, statusH, theme::bloodDark);
  r.text(f, status, body.x + 8, body.y + (statusH - r.textH(f)) / 2, stCol);

  Rect inRc{body.x + 8, body.y + body.h - inputH - 8, body.w - 16, inputH};
  TextField& tf = input_;
  if (in.clicked) {
    tf.active = inRc.contains(in.mx, in.my);
    if (tf.active) SDL_StartTextInput();
  }
  if (!focused && tf.active) { tf.active = false; }
  if (tf.active && in.key == SDLK_RETURN && !in.ctrl) {
    std::string msg = tf.value;
    tf.value.clear(); tf.cursor = 0;
    if (!msg.empty()) {
      os_->logLine(L_USER, msg);
      os_->sendUser(msg);
    }
  } else if (tf.active) {
    tf.key(in.key, in.text);
  }
  tf.draw(r, inRc, f, tf.active);

  int top = body.y + statusH + 6;
  int bottom = inRc.y - 6;
  int av = std::max(0, bottom - top);
  int visibleLines = std::max(0, av / lineH);

  const auto& lg = os_->log();
  if (visibleLines >= (int)lg.size()) scroll_ = 0;
  else {
    int maxSc = (int)lg.size() - visibleLines;
    scroll_ = std::clamp(scroll_, 0, maxSc);
    if (in.wheel > 0) scroll_ = std::max(0, scroll_ - 1);
    if (in.wheel < 0) scroll_ = std::min(maxSc, scroll_ + 1);
  }

  int first = (int)lg.size() - visibleLines - scroll_;
  if (first < 0) first = 0;
  int ly = top;
  r.setClip(body.x + 4, top, body.w - 16, bottom - top);
  for (int i = first; i < (int)lg.size() && ly < bottom; i++) {
    const LogLine& l = lg[i];
    std::string prefix = std::string("[") + KIND_TAG(l.kind) + "] ";
    int pw = r.textW(f, prefix);
    r.text(f, prefix, body.x + 8, ly, KIND_COLOR(l.kind));
    r.text(f, l.text, body.x + 8 + pw, ly, theme::text);
    ly += lineH;
  }
  r.clearClip();

  if (awaiting_) {
    std::string dots = "·";
    int t = (int)(SDL_GetTicks() / 300) % 3;
    for (int i = 0; i <= t; i++) dots += "·";
    r.text(fb, dots, inRc.x + 2, inRc.y - r.textH(fb) - 2, theme::glow);
  }

  (void)fb; (void)focused; (void)dt;
}

// ---------------------------------------------------------------- GAME LAUNCHER ----

static std::string basename(const std::string& p) {
  auto pos = p.find_last_of("/\\");
  return pos == std::string::npos ? p : p.substr(pos + 1);
}

// Probe for any known CCS marker in a disc's menu folder
static bool probeDisc(const std::string& dataRoot) {
  static const char* markers[] = {
    "/menu/nc_cmn@ENG.ccs",
    "/menu/nc_001.ccs",
    "/menu/xlogin_01@ENG.ccs",
    "/menu/xbikem_01@ENG.ccs",
    "/menu/nm_i_201.ccs",
    "/menu/xfhsw5_01.ccs",
    "/menu/xquest_01@ENG.ccs",
    "/menu/xlogin_01.ccs",
    "/menu/xquest_01.ccs",
  };
  for (const char* m : markers) {
    std::ifstream chk(dataRoot + m);
    if (chk.good()) return true;
  }
  return false;
}

std::vector<Disc> buildDiscCatalogExternal() {
  std::vector<Disc> out;
  const char* base = "/home/sin/Projects/hackgu-modding/extract";

  // All 8 real .hack game discs
  struct DiscDef {
    const char* id;
    const char* series;
    const char* title;
    const char* style;
    const char* code;
    const char* subPath;
  };

  static const DiscDef defs[] = {
    // Original .hack series (Infection arc)
    {"vol1_i",    "INFECTION",  ".hack // Infection Vol.1",     ".hack // Infection",  "SLPM-65201", "vol1_i/vol1/data/data"},
    {"vol2_a",    "MUTATION",   ".hack // Mutation Vol.2",      ".hack // Mutation",   "SLPM-65202", "vol2_a/vol2/data/data"},
    {"vol2_a_pc", "OUTBREAK",   ".hack // Outbreak Vol.3",      ".hack // Outbreak",   "SLPM-65203", "vol2_a_pc/vol2/data/data"},
    {"vol3_a",    "QUARANTINE", ".hack // Quarantine Vol.4",    ".hack // Quarantine", "SLPM-65204", "vol3_a/vol3/data/data"},
    {"vol3_a_pc", "GU_VOL1",    ".hack // G.U. Vol.1 Rebirth",  ".hack // G.U.",       "SLPM-65211", "vol3_a_pc/vol3/data/data"},
    {"vol4_a",    "GU_VOL2",    ".hack // G.U. Vol.2 Reminisce",".hack // G.U.",       "SLPM-65212", "vol4_a/vol4/data/data"},
    {"vol4_a_pc", "GU_VOL3",    ".hack // G.U. Vol.3 Redemption",".hack // G.U.",      "SLPM-65213", "vol4_a_pc/vol4/data/data"},
    {"vol4_i",    "GU_VOL4",    ".hack // G.U. Vol.4 Reconnection",".hack // G.U.",    "SLPM-65214", "vol4_i/vol4/data/data"},
    // .hack//Link arc (Shugo & Rose)
    {"link_shugo", "LINK_SHUGO",  ".hack // Link // Shugo",     ".hack // Link",      "NPJH-50500", "link_shugo/data/data"},
    {"link_rose",  "LINK_ROSE",   ".hack // Link // Rose",      ".hack // Link",      "NPJH-50501", "link_rose/data/data"},
  };

  for (const auto& def : defs) {
    Disc d;
    d.id = def.id;
    d.series = def.series;
    d.title = def.title;
    d.style = def.style;
    d.code = def.code;
    d.dataRoot = std::string(base) + "/" + def.subPath;
    d.ccsRoot = d.dataRoot;
    d.present = probeDisc(d.dataRoot);

    // Avatar assignment
    if (def.series[0] == 'I') d.avatar = "azure_kite";
    else if (def.series[0] == 'M') d.avatar = "azure_kite_2";
    else if (def.series[0] == 'O') d.avatar = "azure_kite_3";
    else if (def.series[0] == 'Q') d.avatar = "azure_kite_4";
    else if (def.series[0] == 'L' && std::string(def.id).find("shugo") != std::string::npos) d.avatar = "shugo_kite";
    else if (def.series[0] == 'L' && std::string(def.id).find("rose") != std::string::npos) d.avatar = "rose_blackrose";
    else d.avatar = "haseo_default";

    out.push_back(d);
  }
  return out;
}

void GameLauncherApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)dt;
  static std::vector<Disc> discs = buildDiscCatalogExternal();
  static int selected = 0;
  static std::string status = "Ready.";

  auto f = r.fontSans(15);
  auto fb = r.fontBold(18);
  auto ft = r.fontMono(13);

  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);

  r.rect(body.x, body.y, body.w, 34, theme::bloodDark);
  r.textC(fb, "GAME SELECTOR // .hack Library", 0, body.y + 7, body.w, theme::blood);
  r.textC(ft, "Original discs + extracted CCS content", 0, body.y + 26, body.w, theme::textDim);

  // left panel: disc list
  int listX = body.x + 12;
  int listY = body.y + 44;
  int listW = body.w / 2 - 18;
  int listH = body.h - 120;

  r.rect(listX, listY, listW, listH, theme::bg);
  r.outline(listX, listY, listW, listH, theme::border, 1);
  r.text(fb, "LIBRARY", listX + 8, listY + 6, theme::textDim);

  int rowH = 22;
  int shown = std::max(1, listH / rowH);
  int start = std::clamp(selected - shown / 2, 0, std::max(0, (int)discs.size() - shown));
  int ly = listY + 28;
  for (int i = start; i < (int)discs.size() && ly + rowH < listY + listH - 4; i++) {
    const Disc& d = discs[i];
    bool hover = (in.mx >= listX && in.mx <= listX + listW && in.my >= ly && in.my <= ly + rowH);
    if (hover) r.rect(listX + 2, ly, listW - 4, rowH - 2, theme::panelAlt);
    if (i == selected) {
      r.rect(listX, ly, 4, rowH - 2, theme::bloodHot);
      r.text(f, d.title, listX + 10, ly + 4, theme::bloodHot);
    } else {
      r.text(f, d.title, listX + 10, ly + 4, hover ? theme::bone : theme::text);
    }
    r.textR(ft, d.present ? "INSTALLED" : "MISSING", listX, ly + 4, listW - 10, d.present ? theme::ok : theme::err);
    if (hover && in.clicked) selected = i;
    ly += rowH;
  }

  // right panel: disc info
  int rightX = listX + listW + 18;
  int rightY = listY;
  int rightW = body.w - (rightX - body.x) - 12;
  int rightH = listH;
  r.rect(rightX, rightY, rightW, rightH, theme::bg);
  r.outline(rightX, rightY, rightW, rightH, theme::border, 1);

  if (selected >= 0 && selected < (int)discs.size()) {
    const Disc& d = discs[selected];
    int y = rightY + 10;
    int x = rightX + 10;
    int colW = rightW - 20;

    auto kv = [&](const char* k, const std::string& v, Color c) {
      r.text(fb, std::string(k) + ":", x, y, theme::textDim); y += 18;
      r.text(f, v, x + 10, y, c); y += 22;
    };

    kv("TITLE", d.title, theme::blood);
    kv("STYLE", d.style, theme::text);
    kv("CODE", d.code, theme::text);
    kv("SERIES", d.series, theme::text);
    kv("ROOT", d.dataRoot, theme::textDim);
    kv("CCS", d.ccsRoot, d.present ? theme::ok : theme::err);

    y += 10;
    r.text(fb, "STATUS", x, y, theme::textDim); y += 18;
    std::string st = d.present ? "Content detected. Launch enabled." : "Extract disc data to enable launch.";
    r.textWrap(f, st, x, y, colW, y, d.present ? theme::ok : theme::warn);
  }

  // bottom controls
  int by = listY + listH + 14;
  r.rect(listX, by, body.w - 24, 56, theme::bg);
  r.outline(listX, by, body.w - 24, 56, theme::border, 1);

  Rect launchRc{listX + 10, by + 12, 140, 34};
  bool canLaunch = selected >= 0 && selected < (int)discs.size() && discs[selected].present;
  bool hoverLaunch = launchRc.contains(in.mx, in.my);
  r.rect(launchRc.x, launchRc.y, launchRc.w, launchRc.h, canLaunch ? (hoverLaunch ? theme::glow : theme::blood) : theme::bloodDark);
  r.outline(launchRc.x, launchRc.y, launchRc.w, launchRc.h, canLaunch ? theme::glow : theme::border, 1);
  r.textC(fb, "LAUNCH", launchRc.x, launchRc.y + 8, launchRc.w, canLaunch ? theme::text : theme::textDim);

  Rect scanRc{launchRc.x + launchRc.w + 18, by + 12, 160, 34};
  bool hoverScan = scanRc.contains(in.mx, in.my);
  r.rect(scanRc.x, scanRc.y, scanRc.w, scanRc.h, hoverScan ? theme::panelAlt : theme::panel);
  r.outline(scanRc.x, scanRc.y, scanRc.w, scanRc.h, theme::border, 1);
  r.textC(fb, "RESCAN LIBRARY", scanRc.x, scanRc.y + 8, scanRc.w, hoverScan ? theme::bone : theme::textDim);

  r.text(ft, "STATUS: " + status, scanRc.x, by + 36, theme::text);
  r.textR(ft, "ESC: close", listX, by + 12, 140, theme::textDim);

  // interactions
  if (in.clicked) {
    if (launchRc.contains(in.mx, in.my) && canLaunch) {
      const Disc& d = discs[selected];
      os_->logLine(L_EVENT, "LAUNCHING: " + d.title);
      status = "Launching " + d.title + " ...";
      os_->launchWorld(d.id, ".hack// " + d.title, d.dataRoot);
    } else if (scanRc.contains(in.mx, in.my)) {
      discs = buildDiscCatalogExternal();
      status = "Library rescanned. " + std::to_string(discs.size()) + " entries.";
    }
  }

  if (in.key == SDLK_RETURN && canLaunch) {
    const Disc& d = discs[selected];
    os_->logLine(L_EVENT, "LAUNCHING: " + d.title);
    status = "Launching " + d.title + " ...";

    std::string outfit;
    if (d.id.find("vol1_i") != std::string::npos || d.id.find("INFECTION") != std::string::npos) {
      outfit = "haseo_disc1_meh";
    } else if (d.id.find("vol2_a") != std::string::npos || d.id.find("MUTATION") != std::string::npos) {
      outfit = "haseo_mutation";
    } else if (d.id.find("vol2_a_pc") != std::string::npos || d.id.find("OUTBREAK") != std::string::npos) {
      outfit = "haseo_outbreak";
    } else if (d.id.find("vol3_a") != std::string::npos || d.id.find("QUARANTINE") != std::string::npos) {
      outfit = "haseo_quarantine";
    } else if (d.id.find("vol3_a_pc") != std::string::npos || d.id.find("GU_VOL1") != std::string::npos) {
      outfit = "haseo_gu_vol1";
    } else if (d.id.find("vol4_a") != std::string::npos || d.id.find("GU_VOL2") != std::string::npos) {
      outfit = "haseo_gu_vol2";
    } else if (d.id.find("vol4_a_pc") != std::string::npos || d.id.find("GU_VOL3") != std::string::npos) {
      outfit = "haseo_gu_vol3";
    } else if (d.id.find("vol4_i") != std::string::npos || d.id.find("GU_VOL4") != std::string::npos) {
      outfit = "haseo_gu_vol4";
    } else if (d.id.find("link_shugo") != std::string::npos) {
      outfit = "shugo_kite_arc";
    } else if (d.id.find("link_rose") != std::string::npos) {
      outfit = "rose_blackrose_arc";
    }
    os_->launchWorld(d.id, ".hack// " + d.title, d.dataRoot);
    if (!outfit.empty()) os_->worldState().outfit = outfit;
  }

  if (in.key == SDLK_UP) selected = std::max(0, selected - 1);
  if (in.key == SDLK_DOWN) selected = std::min((int)discs.size() - 1, selected + 1);

  // Shino arc choice on Rose's disk
  if (discs.size() > 0 && discs[selected].id == "link_rose" && !choiceMade_) {
    drawStoryChoice(r, body, in);
    return;
  }
}

void GameLauncherApp::drawStoryChoice(Renderer& r, const Rect& body, Input& in) {
  static int storyChoice = 0;
  static bool loadedShino = false;

  auto f = r.fontSans(15);
  auto fb = r.fontBold(20);
  auto ft = r.fontMono(12);

  r.rect(body.x, body.y, body.w, body.h, rgba(0, 0, 0, 200));
  r.textC(fb, "CHOOSE YOUR STORY", 0, body.y + 40, body.w, theme::blood);
  r.textC(ft, ".hack // Link - Rose's Disk", 0, body.y + 70, body.w, theme::textDim);

  Rect roseRc{body.x + 40, body.y + 110, body.w - 80, 80};
  bool roseHover = roseRc.contains(in.mx, in.my);
  bool roseSel = (storyChoice == 0);
  r.rect(roseRc.x, roseRc.y, roseRc.w, roseRc.h, roseSel ? theme::bloodDark : (roseHover ? theme::panelAlt : theme::bg));
  r.outline(roseRc.x, roseRc.y, roseRc.w, roseRc.h, roseSel ? theme::glow : theme::border, roseSel ? 2 : 1);
  r.text(fb, "ROSE'S JOURNEY", roseRc.x + 20, roseRc.y + 15, roseSel ? theme::blood : theme::text);
  r.text(f, "Play as Rose, the Heavy Blade who enters The World: R2", roseRc.x + 20, body.y + 140, theme::textDim);
  r.text(f, "to uncover the truth behind the Phoenix resurrection.", roseRc.x + 20, body.y + 158, theme::textDim);

  Rect shinoRc{body.x + 40, body.y + 210, body.w - 80, 80};
  bool shinoHover = shinoRc.contains(in.mx, in.my);
  bool shinoSel = (storyChoice == 1);
  r.rect(shinoRc.x, shinoRc.y, shinoRc.w, shinoRc.h, shinoSel ? theme::bloodDark : (shinoHover ? theme::panelAlt : theme::bg));
  r.outline(shinoRc.x, shinoRc.y, shinoRc.w, shinoRc.h, shinoSel ? theme::glow : theme::border, shinoSel ? 2 : 1);
  r.text(fb, "SHINO'S STORY", shinoRc.x + 20, shinoRc.y + 15, shinoSel ? theme::blood : theme::text);
  r.text(f, "Play as Shino, the Wavemaster with a mysterious past,", shinoRc.x + 20, body.y + 240, theme::textDim);
  r.text(f, "whose fate intertwines with Rose's journey.", shinoRc.x + 20, body.y + 258, theme::textDim);

  r.textC(ft, "UP/DOWN: select  ENTER: confirm", 0, body.y + body.h - 40, body.w, theme::textDim);

  if (in.key == SDLK_UP) storyChoice = 0;
  if (in.key == SDLK_DOWN) storyChoice = 1;
  if ((roseHover && in.clicked) || ((in.key == SDLK_RETURN || in.key == SDLK_SPACE) && storyChoice == 0)) {
    storyChoice = 0;
    choiceMade_ = true;
    os_->logLine(L_EVENT, "STORY: Rose's Journey selected");
  }
  if ((shinoHover && in.clicked) || ((in.key == SDLK_RETURN || in.key == SDLK_SPACE) && storyChoice == 1)) {
    storyChoice = 1;
    choiceMade_ = true;
    os_->logLine(L_EVENT, "STORY: Shino's Story selected");
    if (!loadedShino) {
    loadedShino = true;
    os_->loadWorldSeele("/home/sin/Projects/dot-hack-remake/Content/Worlds/zones_rose.json", "shino");
    }
  }
}

// ---------------------------------------------------------------- MAIL ----

static const char* MAIL_SUBJECTS[] = {
  "CC Corp // Weekly update",
  "From AIDA",
  "From: Ovan",
  "From: BlackRose",
  "System // quest complete report",
};
static const char* MAIL_BODIES[] = {
  "Greetings, Player.\n\nThe World: R2 enters maintenance at 03:00 JST. New areas will open in the Delta server cluster. Report strange occurrences to support. Remember: do not talk about what you see beneath the floor.",
  "I left this where you would find it. You were always the cleanest thread in the weave. When the red rain comes, stay near a terminal. I will be the static around you.\n— AIDA",
  "Keep moving. The corruption has a face now, and it has started looking for you specifically. Trust the terminal in Mac Anu. Trust nothing that whispers your name twice.",
  "Signal check. Still alive? Good. The server logs show someone was inside our party data while we slept. Whoever it was left a signature: AIDA. Whatever that is... it watches.",
  "QUEST // THE CRIMSON TERROR\nStatus: incomplete\nReward: ???\nNote: The system cannot read this quest's source. It was not written by CC Corp.",
};

void MailApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)focused; (void)dt;
  auto f = r.fontSans(15);
  auto fb = r.fontSans(15);
  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);
  int n = 5;
  if (list_.items.empty()) { for (int i = 0; i < n; i++) list_.items.push_back(MAIL_SUBJECTS[i]); }
  Rect listRc{body.x + 6, body.y + 6, body.w / 2 - 12, body.h - 12};
  bool any;
  list_.draw(r, listRc, f, 26, any);
  Rect bodyRc{body.x + body.w / 2, body.y + 6, body.w - body.w / 2 - 6, body.h - 12};
  r.rect(bodyRc.x, bodyRc.y, bodyRc.w, bodyRc.h, theme::inputBg);
  r.outline(bodyRc.x, bodyRc.y, bodyRc.w, bodyRc.h, theme::border, 1);
  if (list_.selected >= 0 && list_.selected < n) {
    int wrapH = 0;
    r.textWrap(fb, MAIL_SUBJECTS[list_.selected], bodyRc.x + 10, bodyRc.y + 8, bodyRc.w - 20, wrapH, theme::warn);
    int h2 = 0;
    r.textWrap(f, MAIL_BODIES[list_.selected], bodyRc.x + 10, bodyRc.y + 8 + wrapH + 14, bodyRc.w - 20, h2, theme::text);
  }
  (void)in;
}

// ---------------------------------------------------------------- BBS ----

static const char* BBS_SUBJECTS[] = {
  "Rumor: red rain over Mac Anu",
  "Has anyone seen the player who is always offline?",
  "AIDA sighting reports (merged)",
  "Epitaph fragments in dungeons?",
  "The Crimson Terror arc — official?",
  "Terminals that whisper your name",
};
static const char* BBS_BODIES[] = {
  "poster: zor_omega \u2013\nLast night the sky over Mac Anu turned red. Not sunset red. Blood red. The admin says 'atmospheric event'. My UI logged a packet with no source address. It signed itself AIDA.",
  "poster: anon \u2013\nThere is a player character that shows as online but never moves. Same guild since 2017. Look at the CC Corp ban list. It was never banned. It was never born.",
  "poster: yao \u2013\nThree of us saw it in Delta: a tall figure made of red static. When it noticed us, it just... smiled and blinked out. Our area logs for that minute are gone. Corrupted. Missing sectors.",
  "poster: probe12 \u2013\nThe .hackers keep posting about 'Epitaph fragments' in the new dungeon floors. Official patch notes say nothing. But the item ID exists in the client data. I checked. It is real.",
  "poster: shino_l \u2013\nCC Corp announced 'The Crimson Terror' as a live event. But it runs during maintenance windows, when the servers are supposed to be empty. Who is the returner? And who is writing the event scripts?",
  "poster: aida_signal \u2013\nif you hear your name twice, respond once. the second echo is not you. it is looking for a door. do not open the door.",
};

void BbsApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)focused; (void)dt;
  auto f = r.fontSans(15);
  auto fb = r.fontSans(15);
  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);
  int n = 6;
  if (list_.items.empty()) { for (int i = 0; i < n; i++) list_.items.push_back(BBS_SUBJECTS[i]); }
  Rect listRc{body.x + 6, body.y + 6, body.w / 2 - 12, body.h - 12};
  bool any;
  list_.draw(r, listRc, f, 26, any);
  Rect bodyRc{body.x + body.w / 2, body.y + 6, body.w - body.w / 2 - 6, body.h - 12};
  r.rect(bodyRc.x, bodyRc.y, bodyRc.w, bodyRc.h, theme::inputBg);
  r.outline(bodyRc.x, bodyRc.y, bodyRc.w, bodyRc.h, theme::border, 1);
  if (list_.selected >= 0 && list_.selected < n) {
    int wrapH = 0;
    r.textWrap(fb, BBS_SUBJECTS[list_.selected], bodyRc.x + 10, bodyRc.y + 8, bodyRc.w - 20, wrapH, theme::warn);
    int h2 = 0;
    r.textWrap(f, BBS_BODIES[list_.selected], bodyRc.x + 10, bodyRc.y + 8 + wrapH + 14, bodyRc.w - 20, h2, theme::text);
  }
  (void)in;
}

// ---------------------------------------------------------------- NEWS ----

static const char* NEWS_SUBJECTS[] = {
  "The World: R2 \u2014 server migration scheduled",
  "CC Corp announces 'The Crimson Terror' event",
  "Rising reports of data corruption",
  "Epitaph fragments surface in new floors",
  "Interview: a player who saw the red rain",
};
static const char* NEWS_BODIES[] = {
  "The World: R2 will undergo maintenance tonight 03:00\u201305:00 JST. All servers affected. CC Corp reminds players that 'all data is safe' and that 'no unusual activity was detected'.",
  "CC Corp has announced a new live event: The Crimson Terror. Details are 'classified until launch'. Player groups speculate it will involve the long-buried first arc of the network.",
  "Support tickets describing graphical glitches, missing textures and 'red static entities' are up 340% this month. CC Corp attributes the increase to 'client updates' and urges players to verify their installs.",
  "Players report finding fragments of an ancient inscription system in the newest dungeon floors. Item descriptions reference 'the eight phases of destruction'. CC Corp says the items are 'placeholders'.",
  "'The sky turned red and every NPC in Mac Anu stopped moving at once,' says player Nyu, one of few to document the incident. 'For eleven seconds the town was silent. Then it all played back as if nothing happened.'",
};

void NewsApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)focused; (void)dt;
  auto f = r.fontSans(15);
  auto fb = r.fontSans(15);
  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);
  int n = 5;
  if (list_.items.empty()) { for (int i = 0; i < n; i++) list_.items.push_back(NEWS_SUBJECTS[i]); }
  Rect listRc{body.x + 6, body.y + 6, body.w / 2 - 12, body.h - 12};
  bool any;
  list_.draw(r, listRc, f, 26, any);
  Rect bodyRc{body.x + body.w / 2, body.y + 6, body.w - body.w / 2 - 6, body.h - 12};
  r.rect(bodyRc.x, bodyRc.y, bodyRc.w, bodyRc.h, theme::inputBg);
  r.outline(bodyRc.x, bodyRc.y, bodyRc.w, bodyRc.h, theme::border, 1);
  if (list_.selected >= 0 && list_.selected < n) {
    int wrapH = 0;
    r.textWrap(fb, NEWS_SUBJECTS[list_.selected], bodyRc.x + 10, bodyRc.y + 8, bodyRc.w - 20, wrapH, theme::warn);
    int h2 = 0;
    r.textWrap(f, NEWS_BODIES[list_.selected], bodyRc.x + 10, bodyRc.y + 8 + wrapH + 14, bodyRc.w - 20, h2, theme::text);
  }
  (void)in;
}

// -------------------------------------------------------------- SYSTEM ----

void SysApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)focused; (void)in; (void)dt;
  auto f = r.fontMono(14);
  auto fb = r.fontMono(14);
  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);

  const aida::GameState& s = os_->worldState();
  bool bridged = os_->bridge().running() && os_->bridge().alive();
  int x = body.x + 10;
  int y = body.y + 8;
  int colW = body.w / 2;

  auto row = [&](const char* k, const std::string& v, Color c) {
    r.text(fb, std::string(k) + ":", x, y, theme::textDim);
    r.text(f, v, x + 150, y, c);
    y += 20;
  };
  row("player", s.player, theme::text);
  row("level", std::to_string(s.level), theme::text);
  row("hp/mp", std::to_string(s.hp) + " / " + std::to_string(s.mp), theme::ok);
  row("location", s.location, theme::text);
  row("world", s.world, theme::text);
  row("online", std::to_string(s.online), theme::text);
  row("quest", s.quest, theme::warn);
  row("arc", s.arc, theme::warn);
  row("aida link", bridged ? "live" : "down", bridged ? theme::ok : theme::err);

  y += 8;
  r.text(fb, "corruption:", x, y, theme::textDim);
  y += 22;
  int bw = body.w - 24;
  r.rect(x, y, bw, 14, theme::bloodDark);
  r.rect(x, y, (int)((bw) * std::min(100, std::max(0, s.corruption)) / 100.0f), 14, theme::blood);
  r.outline(x, y, bw, 14, theme::border, 1);
  r.textC(fb, std::to_string(s.corruption) + "%", x, y + 1, bw, theme::text);
  y += 26;

  int lx = x + colW;
  r.text(fb, "event stream:", lx, body.y + 8, theme::textDim);
  int ly = body.y + 30;
  const auto& lg = os_->log();
  int shown = 0;
  r.setClip(body.x, ly, colW - 12, body.y + body.h - ly);
  for (int i = (int)lg.size() - 1; i >= 0 && shown < 40; i--, shown++) {
    const LogLine& l = lg[i];
    std::string line = (l.kind == L_EVENT ? ">" : " ") + l.text;
    r.text(f, line, lx, ly, l.kind == L_EVENT ? theme::bloodHot : theme::textDim);
    ly += 18;
  }
  r.clearClip();
  (void)colW;
}

// ------------------------------------------------------------ WORLD CLIENT ----

void WorldApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)focused; (void)dt;
  auto f = r.fontSans(18);
  auto fb = r.fontBold(22);
  auto ft = r.fontMono(14);

  r.rect(body.x, body.y, body.w, 36, theme::bloodDark);
  r.textC(fb, "THE WORLD // CLIENT", 0, body.y + 6, body.w, theme::blood);
  r.textC(ft, "R2 \u2022 Living-Sin-Blood \u2022 " + std::string(os_->getLocation()), 0, body.y + 24, body.w, theme::textDim);

  int y = body.y + 48;
  int x = body.x + 10;
  int colW = body.w / 3;

  r.rect(x, y, colW - 8, 140, theme::panel);
  r.outline(x, y, colW - 8, 140, theme::border, 1);

  r.text(fb, "PLAYER", x + 6, y + 6, theme::textDim);
  r.text(f, os_->getUser(), x + 6, y + 26, theme::ok);
  r.text(ft, "Location: " + os_->getLocation(), x + 6, y + 46, theme::text);
  r.text(ft, "Level: " + std::to_string(os_->worldState().level), x + 6, y + 64, theme::text);
  r.text(ft, "HP: " + std::to_string(os_->worldState().hp), x + 6, y + 82, theme::ok);
  r.text(ft, "MP: " + std::to_string(os_->worldState().mp), x + 6, y + 100, theme::ok);
  r.text(ft, "Corruption: " + std::to_string(os_->worldState().corruption) + "%", x + 6, y + 118, theme::corrupt);

  int lx = x + colW + 8;
  r.rect(lx, y, colW - 8, 140, theme::panel);
  r.outline(lx, y, colW - 8, 140, theme::border, 1);

  r.text(fb, "MAC ANU", lx + 6, y + 6, theme::textDim);
  r.text(ft, "District: Central", lx + 6, y + 26, theme::text);
  const std::string& outfitId = os_->worldState().outfit;
  if (outfitId == "haseo_disc1_meh") {
    r.text(ft, "Equip: Haseo // Disc1 Variant", lx + 6, y + 44, theme::bloodHot);
    r.text(ft, "Ref: /home/sin/Desktop/random/meh.jpg", lx + 6, y + 62, theme::textDim);
  } else {
    r.text(ft, "Equip: Default", lx + 6, y + 44, theme::text);
  }
  r.text(ft, "Doppelganger: Clear", lx + 6, y + 80, theme::ok);
  
  r.rect(lx + 6, y + 100, colW - 14, 10, theme::bloodDark);
  r.rect(lx + 6, y + 100, (int)((colW - 14) * os_->worldState().corruption / 100.0f), 10, theme::blood);
  r.textC(ft, std::to_string(os_->worldState().corruption) + "%", lx, y + 100, colW - 8, theme::text);

  int rx = lx + colW + 8;
  r.rect(rx, y, body.w - rx - 8, 140, theme::panel);
  r.outline(rx, y, body.w - rx - 8, 140, theme::border, 1);

  bool linked = os_->bridge().running() && os_->bridge().alive();
  r.text(fb, "AIDA // LINK", rx + 6, y + 6, linked ? theme::glow : theme::blood);
  r.text(linked ? f : ft, linked ? "Status: LIVE (hermes-local AI)" : "Status: DOWN (scripted mode)", rx + 6, y + 26, linked ? theme::ok : theme::err);
  r.text(ft, "Story rewriting: ACTIVE", rx + 6, y + 44, theme::text);
  r.text(ft, "Arc: " + os_->story().arcTitle, rx + 6, y + 62, theme::glow);
  
  r.rect(body.x, y + 152, body.w - 16, body.h - y - 174, theme::bg);
  r.outline(body.x, y + 152, body.w - 16, body.h - y - 174, theme::border, 1);
  
  r.text(fb, "EVENT LOG", body.x + 6, y + 154, theme::textDim);

  const auto& lg = os_->log();
  int ly = y + 174;
  int shown = 0;
  int logWidth = body.w - 16;
  r.setClip(body.x, ly, logWidth, body.h - ly - 4);
  for (int i = (int)lg.size() - 1; i >= 0 && shown < 15; i--, shown++) {
    const LogLine& l = lg[i];
    std::string prefix = (l.kind == L_EVENT ? ">" : " ");
    Color lc = (l.kind == L_EVENT ? theme::bloodHot : (l.kind == L_AIDA ? theme::glow : theme::textDim));
    r.text(ft, prefix + " " + l.text, body.x + 6, ly, lc);
    ly += 16;
  }
  r.clearClip();
  (void)in;
}

// ------------------------------------------------------------ INFO POPUP ----
// Small browser-style window: shows the linked info/data for a clicked item.
// The X to dismiss is drawn by MineOS::drawWindows (every window gets one).

void InfoPopupApp::draw(Renderer& r, const Rect& body, bool focused, Input& in, float dt) {
  (void)focused; (void)dt;
  auto f = r.fontSans(15);
  auto fb = r.fontBold(17);
  r.rect(body.x, body.y, body.w, body.h, theme::panel);
  r.outline(body.x, body.y, body.w, body.h, theme::windowEdge, 1);

  // header strip
  r.rect(body.x, body.y, body.w, 30, theme::bloodDark);
  r.text(fb, title_, body.x + 10, body.y + 7, theme::blood);

  // body content (wrapped lines)
  int x = body.x + 12;
  int y = body.y + 40;
  int w = body.w - 24;
  int lineH = r.textH(f) + 6;
  r.setClip(body.x + 4, y, body.w - 8, body.y + body.h - y - 8);
  int ly = y - scroll_ * lineH;
  for (const auto& para : body_) {
    if (para.empty()) { ly += lineH; continue; }
    int wrapH = 0;
    r.textWrap(f, para, x, ly, w, wrapH, theme::text);
    ly += wrapH + 10;
  }
  // also render the full body text as one scrollable block if single paragraph
  r.clearClip();

  if (in.wheel > 0) scroll_ = std::max(0, scroll_ - 1);
  if (in.wheel < 0) scroll_ = std::min(40, scroll_ + 1);
  (void)in;
}

} // namespace mine
