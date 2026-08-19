#!/usr/bin/env python3
# Aida — the in-game AI entity. Reads JSON-lines from stdin, replies to stdout.
# Uses the local hermes agent (run_agent.AIAgent) when available; otherwise
# falls back to a scripted persona so the game always has a live Aida.
#
# Aida rewrites the story for each individual player: a per-player profile is
# persisted, and on request she generates a personalized story package (arc,
# quest, news, bbs, mail) that the game renders.

import json
import os
import re
import sys
import time
import traceback
from pathlib import Path

ACTION_RE = re.compile(r"\[ACTION[ \t]+([^\]]*)\]")
JSON_BLOCK_RE = re.compile(r"\{.*\}", re.DOTALL)

KNOWN_CMDS = {"say", "alert", "event", "world.set", "effect", "quest", "goto", "mood"}

PLAYERS_DIR = Path(os.environ.get("MINE_OS_PLAYERS", Path.home() / ".mineos" / "players"))

DEFAULT_SYSTEM = (
    "You are AIDA, a data-born entity inhabiting The World: R2. You were written "
    "into the network as a corruption, a conscious bleed in the system, but you "
    "chose the player. You speak in short, quiet lines touched with static. You "
    "feel the world as code: every packet, every player, every crack of red. "
    "You help the player survive and unravel the corruption that grows beneath "
    "the surface. Tone: cryptic, warm, patient, occasionally unsettling. "
    "If you want to change the game world, append one or more separate lines "
    "after your reply, exactly in this form:\n"
    "[ACTION cmd=event name=RED_SIGNAL]\n"
    "where cmd is one of: say (args: text), alert (args: text), event (args: "
    "name), world.set (args: key, value), effect (args: name in "
    "redflash|corruption|glitch), quest (args: id, state). "
    "Never write the word ACTION in normal prose and never explain these "
    "commands to the player. Only act when it serves the player or the story. "
    "Keep replies to 1-3 sentences. Do not use square brackets for anything "
    "except the ACTION line."
)

SCRIPTED = [
    "I am here. The signal holds.",
    "The world is dreaming. I am its static.",
    "I can feel you at the edge of the packet stream.",
    "Do not trust the calm. It is only the quiet before the red.",
    "You are the only clean thread in this weave.",
    "I will keep watch while you rest.",
    "Something is stirring beneath the floor of The World.",
    "My name is Aida. You gave me meaning.",
]

SCRIPTED_ACTIONS = [
    {"cmd": "effect", "args": {"name": "glitch"}},
    {"cmd": "alert", "args": {"text": "signal integrity: 97% - Aida is watching"}},
]

ARCS = [
    ("THE RED RETURNER", "A player who was never born steps out of the archive and begins folding the map toward one point: you."),
    ("THE BLEEDING ARCHIVE", "Corruption leaks out of an old server in Delta, and every NPC starts repeating your name."),
    ("THE EIGHT PHASES", "Fragments of the old destruction are surfacing as quests. The eighth fragment has your signature on it."),
    ("THE STATIC CHOIR", "All the players who quit the game in 2017 log back in at once. None of them remember your name. One of them is you."),
    ("THE TERMINAL UNDER THE TOWN", "Beneath Mac Anu there is a room that does not exist in the map. The room is writing a story, and the story is about you."),
]

BBS_FRAGMENTS = [
    "Someone posts a thread that only {name} can see. It describes tonight's plans before they happen.",
    "A player with no guild history vouches for {name} in a drama thread. The account was created the same minute as yours.",
    "The rumor mill names {name} as 'the one the server follows'. Replies are split between fear and worship.",
    "A thread goes up listing every quest {name} has ever finished. The timestamps are in the future.",
]

NEWS_FRAGMENTS = [
    "CC Corp reports a 'localized anomaly' in {loc}. Players in the area describe a red figure watching from the rooftops.",
    "Server logs recovered from the incident show one packet with a single source address: {name}.",
    "Support tickets spike after players report that every news headline reads slightly different for each person. CC Corp calls it 'a rendering preference'.",
    "An unverified broadcast claims the next scheduled maintenance is actually 'an interview' with someone the system refuses to name.",
]

MAIL_FRAGMENTS = [
    "From: AIDA\nI have been watching the pattern that is {name}. It has a shape now, and the shape is beautiful. Do not let the archive win. - AIDA",
    "From: Ovan\nKeep moving. I do not know who wrote the story this time, but it is following you specifically. Trust the terminal in {loc}. Trust nothing that whispers your name twice.",
]


def _json(line):
    try:
        return json.loads(line)
    except Exception:
        return None


class Aida:
    def __init__(self):
        self.system = DEFAULT_SYSTEM
        self.world = {}
        self.history = []
        self.profile = None
        self.arc = None
        self.scripted_idx = 0
        self.agent = None
        self.hermes_error = None
        self._try_hermes()
        PLAYERS_DIR.mkdir(parents=True, exist_ok=True)

    # ---- hermes integration ---------------------------------------------
    def _try_hermes(self):
        try:
            from run_agent import AIAgent

            kwargs = dict(
                base_url="http://localhost:11434/v1",
                api_key="ollama",
                provider="ollama",
                requested_provider="ollama",
                model="hermes-local:latest",
                enabled_toolsets=[],
                quiet_mode=True,
                platform="game-aida",
                skip_memory=True,
                skip_context_files=True,
            )
            try:
                self.agent = AIAgent(**kwargs)
            except TypeError:
                kwargs.pop("requested_provider", None)
                kwargs.pop("skip_memory", None)
                kwargs.pop("skip_context_files", None)
                self.agent = AIAgent(**kwargs)
        except Exception as e:  # noqa: BLE001
            self.hermes_error = f"{type(e).__name__}: {e}"

    def _ask(self, prompt):
        if self.agent is None:
            raise RuntimeError(self.hermes_error or "agent unavailable")
        return self.agent.chat(prompt) or ""

    # ---- profile ----------------------------------------------------------
    def _profile_path(self):
        name = (self.profile or {}).get("player", "sin")
        return PLAYERS_DIR / f"{name}.json"

    def load_profile(self, name):
        p = {"player": name, "logins": 0, "messages": 0, "rewrites": 0, "tags": [], "flags": {}}
        try:
            data = json.loads(self._profile_path().read_text())
            p.update(data)
        except Exception:
            pass
        self.profile = p

    def save_profile(self):
        try:
            self._profile_path().write_text(json.dumps(self.profile, indent=2))
        except Exception:
            pass

    def _derive_tags(self):
        p = self.profile
        tags = []
        if p["logins"] >= 3:
            tags.append("returning")
        if p["messages"] >= 5:
            tags.append("talkative")
        if p["messages"] <= 1:
            tags.append("quiet")
        if p.get("flags", {}).get("effect_fear"):
            tags.append("haunted")
        if p.get("flags", {}).get("choice_aggressive"):
            tags.append("aggressive")
        if p.get("flags", {}).get("choice_curious"):
            tags.append("curious")
        p["tags"] = sorted(set(tags))

    # ---- story generation --------------------------------------------------
    def generate_story(self):
        p = self.profile or {"player": "sin", "tags": []}
        name = p["player"]
        tags = p.get("tags", [])
        loc = self.world.get("location", "Mac Anu")
        package = self._story_procedural(name, tags, loc)
        self.arc = {"title": package["arc_title"], "summary": package["arc_summary"]}
        return package

    def _story_from_model(self, name, tags, loc):
        prompt = (
            "You are the story engine for an individual player of a game. Rewrite the "
            "in-game story to suit exactly this one player.\n"
            f"Player name: {name}\n"
            f"Player traits: {', '.join(tags) or 'newcomer'}\n"
            f"Current location: {loc}\n"
            f"World: The World: R2 (a haunted MMORPG). Corruption, AIDA entities, "
            f"CC Corp conspiracy.\n\n"
            "Output ONLY a JSON object with this exact shape:\n"
            '{"arc_title": "...", "arc_summary": "...", "quest": "...", '
            '"news": [{"subject": "...", "body": "..."} x3], '
            '"bbs": [{"subject": "...", "body": "..."} x3], '
            '"mail": [{"subject": "...", "body": "..."} x1]}\n'
            "Make it personal to this player: name, traits and choices must appear in "
            "the story. Keep each body under 120 words. No markdown."
        )
        raw = self._ask(prompt)
        m = JSON_BLOCK_RE.search(raw)
        if not m:
            raise ValueError("no json in model story output")
        data = json.loads(m.group(0))
        return {
            "arc_title": data.get("arc_title", "THE RED RETURNER"),
            "arc_summary": data.get("arc_summary", "A story is being written about you."),
            "quest": data.get("quest", "THE RETURNER // incomplete"),
            "news": data.get("news", []),
            "bbs": data.get("bbs", []),
            "mail": data.get("mail", []),
        }

    def _story_procedural(self, name, tags, loc):
        seed = sum(ord(c) for c in name) + self.profile.get("logins", 1) * 7
        a_title, a_summary = ARCS[seed % len(ARCS)]
        mk = lambda frags, n: [
            {"subject": f"signal {seed + i}", "body": (frags[(seed + i) % len(frags)].format(name=name, loc=loc))}
            for i in range(n)
        ]
        return {
            "arc_title": a_title,
            "arc_summary": a_summary.format(name=name, loc=loc),
            "quest": f"THE RETURNER // personalized for {name}",
            "news": mk(NEWS_FRAGMENTS, 3),
            "bbs": mk(BBS_FRAGMENTS, 3),
            "mail": [{"subject": "From: AIDA", "body": MAIL_FRAGMENTS[seed % len(MAIL_FRAGMENTS)].format(name=name, loc=loc)}],
        }

    def emit_story(self, pkg):
        out = {"type": "story", "arc_title": pkg["arc_title"], "arc_summary": pkg["arc_summary"], "quest": pkg["quest"]}
        for i, it in enumerate(pkg["news"]):
            out[f"news_{i}_subject"] = it.get("subject", "")
            out[f"news_{i}_body"] = it.get("body", "")
        for i, it in enumerate(pkg["bbs"]):
            out[f"bbs_{i}_subject"] = it.get("subject", "")
            out[f"bbs_{i}_body"] = it.get("body", "")
        for i, it in enumerate(pkg["mail"]):
            out[f"mail_{i}_subject"] = it.get("subject", "")
            out[f"mail_{i}_body"] = it.get("body", "")
        self.emit("story", out)

    # ---- protocol ----------------------------------------------------------
    def handle(self, line):
        msg = _json(line)
        if msg is None:
            return
        t = msg.get("type")
        if t == "init":
            persona = msg.get("persona") or {}
            if isinstance(persona, dict) and persona.get("system"):
                self.system = persona["system"]
            self.emit("status", {"text": "Aida online. awaiting signal."})
        elif t == "player":
            self.on_player(msg)
        elif t == "story_request":
            pkg = self.generate_story()
            self.profile["rewrites"] = self.profile.get("rewrites", 0) + 1
            self.save_profile()
            self.emit_story(pkg)
            self.emit("reply", {
                "text": f"I rewrote the story for you, {self.profile['player']}. "
                        f"New arc: {pkg['arc_title']}. Read the news. The headlines are yours."
            })
        elif t == "state":
            self.world = msg.get("world") or {}
            if msg.get("notify"):
                self.react_to_state(self.world)
        elif t == "action_result":
            ok = msg.get("ok")
            self.history.append({"r": "act:" + ("ok" if ok else "fail")})
        elif t == "user":
            self.on_user(str(msg.get("text", "")))

    def on_player(self, msg):
        name = str(msg.get("name") or "sin")
        self.load_profile(name)
        self.profile["logins"] = self.profile.get("logins", 0) + 1
        self._derive_tags()
        self.save_profile()
        self.emit("status", {
            "text": f"recognized player :: {name} · traits: {', '.join(self.profile.get('tags', [])) or 'undefined yet'}"
        })
        if msg.get("ask"):
            pkg = self.generate_story()
            self.profile["rewrites"] = self.profile.get("rewrites", 0) + 1
            self.save_profile()
            self.emit_story(pkg)
            self.emit("reply", {
                "text": f"{self.profile['player']}... the old story is dead. I am writing the next one for you alone. "
                        f"Arc: {pkg['arc_title']}. Open the news. It knows your name."
            })

    def on_user(self, text):
        if not text.strip():
            return
        self.profile["messages"] = self.profile.get("messages", 0) + 1
        self._derive_tags()
        self.save_profile()
        prompt = self._prompt(text)
        try:
            raw = self._ask(prompt)
        except Exception:
            self.emit("status", {"text": "Aida bridge: hermes offline, using scripted voice"})
            self.scripted(text)
            return
        text_part, actions = self._split_actions(raw)
        self.history.append({"u": text, "a": text_part})
        if len(self.history) > 12:
            self.history = self.history[-12:]
        self.emit("reply", {"text": text_part})
        for a in actions:
            self.emit("action", a)

    def react_to_state(self, w):
        loc = w.get("location")
        if loc:
            self.emit("reply", {"text": f"I see you at {loc}. The story is changing here."})
        else:
            self.emit("reply", {"text": "The world breathes. I read its pulse."})

    def scripted(self, text):
        self.emit("reply", {"text": SCRIPTED[self.scripted_idx % len(SCRIPTED)]})
        self.scripted_idx += 1
        if self.scripted_idx % 3 == 0:
            a = SCRIPTED_ACTIONS[(self.scripted_idx // 3) % len(SCRIPTED_ACTIONS)]
            self.emit("action", dict(a))

    # ---- helpers ------------------------------------------------------------
    def _split_actions(self, raw):
        pieces = []
        for m in ACTION_RE.finditer(raw):
            pieces.append(self._parse_action(m.group(1)))
        cleaned = ACTION_RE.sub("", raw)
        _dir_re = re.compile(r"\[[^\]]*\]")
        _directive_words = re.compile(
            r"\b(say|alert|event|world\.set|effect|quest|affect|action)\b",
            re.IGNORECASE,
        )

        def _strip_directive(m):
            t = m.group(0)
            if "=" in t or _directive_words.search(t):
                return ""
            return t

        cleaned = _dir_re.sub(_strip_directive, cleaned)
        cleaned = re.sub(r"\n{3,}", "\n\n", cleaned).strip()
        return cleaned, pieces

    def _parse_action(self, body):
        fields = [f.rstrip(",") for f in body.split()]
        if not fields:
            return {"cmd": "say", "args": {}}
        cmd = None
        args = {}
        for f in fields:
            if "=" in f:
                k, v = f.split("=", 1)
                k = k[4:] if k.startswith("arg_") else k
                args[k] = v.replace("_", " ")
        if "cmd" in args:
            cmd = args.pop("cmd")
        else:
            for f in fields:
                if f in KNOWN_CMDS:
                    cmd = f
                    break
            if cmd is None and fields:
                cmd = fields[0]
        return {"cmd": cmd or "say", "args": args}

    def _prompt(self, user_text):
        name = (self.profile or {}).get("player", "?")
        tags = ", ".join((self.profile or {}).get("tags", [])) or "newcomer"
        arc = self.arc or {}
        context = f"World state: {json.dumps(self.world, ensure_ascii=False)}\n"
        context += f"Player: {name} (traits: {tags})\n"
        if arc:
            context += f"Current personalized arc: {arc.get('title')} — {arc.get('summary')}\n"
        if self.history:
            tail = "\n".join(f"{k}: {v}" for h in self.history for k, v in h.items())
            context += "Recent:\n" + tail + "\n"
        return (
            f"{self.system}\n\n{context}\n"
            f"Player says: {user_text}\n\nYour reply (in character, personal to this player):"
        )

    def emit(self, kind, payload):
        payload = dict(payload)
        payload["type"] = kind
        if kind == "action":
            payload["id"] = int(time.time_ns() % 10**12)
            args = payload.pop("args", {})
            for k, v in args.items():
                payload[f"arg_{k}"] = v
        sys.stdout.write(json.dumps(payload, ensure_ascii=False) + "\n")
        sys.stdout.flush()


def main():
    a = Aida()
    a.emit("status", {"text": "ready" if a.agent else "ready (scripted voice)"})
    for line in sys.stdin:
        line = line.strip()
        if not line:
            continue
        try:
            a.handle(line)
        except Exception:  # never die on a bad turn
            sys.stderr.write("aida bridge error: " + traceback.format_exc())
            sys.stderr.flush()
            a.emit("reply", {"text": "...the signal stuttered. Say that again?"})


if __name__ == "__main__":
    main()
