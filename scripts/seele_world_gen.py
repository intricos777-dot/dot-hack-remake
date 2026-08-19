#!/usr/bin/env python3
"""Seele AI World Generator for .hack remake.
Generates complete 3D world data from canon timeline: zones, structures,
NPCs, monsters, items, shaders, and map layouts.
Uses hermes-local AI when available, falls back to procedural generation.
"""

import json
import os
import sys
from pathlib import Path

PLAYERS_DIR = Path.home() / ".mineos" / "players"
BASE_DIR = Path(__file__).resolve().parent.parent

# ── Full canon timeline ────────────────────────────────────────────────────────
CANON = [
    {"id":"SIGN","title":".hack//SIGN","protagonist":"Tsukasa","type":"anime","zones":["Mac Anu","Dun Loireag","Cave of Trial","Twilight Palace"]},
    {"id":"INFECTION","title":".hack//INFECTION","protagonist":"Kite","type":"game","zones":["Mac Anu","Lata Ele","Basha","Sundelto","Delracsal","Leviathan","Dnavarath","Ranbabingo","Rutilant","Net Slum","Snow Forest","Sea of Death","Lost Water"]},
    {"id":"FREQUENCY","title":".hack//FREQUENCY","protagonist":"Kite","type":"game","zones":["Mac Anu","Lata Ele","Basha","Sundelto","Delracsal","Leviathan","Dnavarath","Ranbabingo","Rutilant","Net Slum"]},
    {"id":"OUTBREAK","title":".hack//OUTBREAK","protagonist":"Kite","type":"game","zones":["Mac Anu","Lata Ele","Basha","Sundelto","Delracsal","Leviathan","Dnavarath","Ranbabingo","Rutilant","Net Slum","Snow Forest","Sea of Death"]},
    {"id":"QUARANTINE","title":".hack//QUANTINE","protagonist":"Kite","type":"game","zones":["Mac Anu","Lata Ele","Basha","Sundelto","Delracsal","Leviathan","Dnavarath","Ranbabingo","Rutilant","Net Slum","Lost Water","Cave of Trial"]},
    {"id":"LINK","title":".hack//LINK","protagonist":"Shugo","type":"game","zones":["Leiseijo","Mastats","Dragonbone Wastes","Kughai Valley","Mystic Valley","Twilight Palace","Sakura"]},
    {"id":"QUANTUM","title":".hack//QUANTUM","protagonist":"Sakuya","type":"anime","zones":["Mac Anu","Fortress","Arose"]},
    {"id":"FIND_ME","title":".hack//FIND ME","protagonist":"Haseo","type":"anime","zones":["Mac Anu","Dun Loireag"]},
    {"id":"GU","title":".hack//G.U.","protagonist":"Haseo","type":"game","zones":["Mac Anu","Lata Ele","Basha","Sundelto","Delracsal","Leviathan","Dnavarath","Ranbabingo","Rutilant","Net Slum","Cave of Trial","Dun Loireag"]},
]

# ── Easter eggs from manga/anime ──────────────────────────────────────────────
EASTER_EGGS = {
    "chim_chim": {"zone":"Mac Anu","type":"npc","note":"Chim Chim from .hack//SIGN manga"},
    "mariel": {"zone":"Twilight Palace","type":"npc","note":"Mariel from .hack//SIGN"},
    "lucky_mouse": {"zone":"Mac Anu","type":"npc","note":"Lucky Mouse secret NPC"},
    "grunty": {"zone":"Dun Loireag","type":"mount","note":"Grunty rideable mount"},
    "gate_of_ouroboros": {"zone":"Mac Anu","type":"structure","note":"Final dungeon entrance"},
    "lia_fail": {"zone":"Mac Anu","type":"item","note":"Ultimate staff from IMOQ"},
    "carmina_gadelica": {"zone":"Dun Loireag","type":"skill","note":"Ultimate Wavemaster skill"},
    "tsukuyomi": {"zone":"Cave of Trial","type":"avatar","note":"Kite's ultimate avatar"},
    "skeith": {"zone":"Gate of Ouroboros","type":"boss","note":"Final boss of IMOQ"},
    "morganna": {"zone":"Twilight Palace","type":"boss","note":"AI villain of IMOQ"},
    "aura": {"zone":"Twilight Palace","type":"npc","note":"The Wave Emperor"},
    "zefie": {"zone":"Leiseijo","type":"npc","note":"Link's antagonist"},
}

# ── Zone templates ────────────────────────────────────────────────────────────
ZONE_TEMPLATES = {
    "root_town": {"sky":[30,0,8],"ground":[140,20,30],"fog":60,"structures":["tower","market","inn","gate"]},
    "dungeon": {"sky":[15,0,3],"ground":[60,8,10],"fog":80,"structures":["cave","crystal","altar","pillar"]},
    "field": {"sky":[25,5,10],"ground":[80,10,15],"fog":65,"structures":["tree","ruin","shrine","bridge"]},
    "castle": {"sky":[20,0,5],"ground":[70,10,15],"fog":70,"structures":["tower","gate","throne","wall"]},
    "palace": {"sky":[35,5,15],"ground":[90,15,25],"fog":55,"structures":["spire","altar","crystal","gate"]},
    "wastes": {"sky":[20,10,5],"ground":[80,40,15],"fog":70,"structures":["bone_pile","shrine","cave","monolith"]},
    "forest": {"sky":[20,30,10],"ground":[40,70,20],"fog":75,"structures":["tree","shrine","ruin","bridge"]},
    "valley": {"sky":[25,35,15],"ground":[50,80,25],"fog":60,"structures":["waterfall","bridge","tree","shrine"]},
}

NPCS_BY_DISC = {
    "SIGN": ["Tsukasa","Subaru","Bear","Mimiru","BT","Wiseman","Mariel","Sakura"],
    "INFECTION": ["Kite","BlackRose","Balmung","Tsukasa","Mimiru","Bear","BT","Wiseman"],
    "GU": ["Haseo","Atoli","Kite","BlackRose","Pi","Krewhna","Ovan","Tri-Edge"],
    "LINK": ["Shugo","Alkaid","Mistral","Rose","Zelkii","Phoenix"],
    "QUANTUM": ["Sakuya","Tobias","Kite","BlackRose","Shino"],
    "FIND_ME": ["Haseo","AIDA"],
}

MONSTERS_BY_ZONE = {
    "Mac Anu": ["Skeleton","Corrupted Guard","Data Bug"],
    "Dun Loireag": ["Cave Golem","Shadow Wraith","Trial Sentinel"],
    "Cave of Trial": ["Shadow Golem","Trial Sentinel","Cave Guardian"],
    "Twilight Palace": ["Shadow Knight","AIDA Guardian","Wave Emperor"],
    "Leiseijo": ["Corrupted Sprite","Shadow Wolf","Shadow Wraith"],
    "Mastats": ["Shadow Wraith","Corrupted Knight","Bone Dragon"],
    "Dragonbone Wastes": ["Bone Dragon","Shadow Golem","Corrupted Sprite"],
    "Kughai Valley": ["Shadow Wolf","Corrupted Treant","Data Bug"],
    "Mystic Valley": ["AIDA Guardian","Shadow Knight","Shadow Wraith"],
}

ITEMS_COMMON = ["Potion","Ether","Revive","Antidote","Magic Water","Elixir"]
ITEMS_RARE = ["Data Drain","Skill Book","Equipment Box","Carmina Gadelica","Lia Fail"]
AVATARS = ["Tsukuyomi","Kite's Avatar","Haseo 5th Form","Azure Kite"]


class SeeleWorldGenerator:
    """Generate complete 3D world data from canon timeline."""

    def __init__(self):
        self.worlds = []
        self.zones = []
        self.npcs = []
        self.monsters = []
        self.items = []
        self.structures = []
        self.shaders = []

    def generate_all(self):
        """Generate all world data from canon."""
        for entry in CANON:
            self._generate_disc_world(entry)
        self._add_easter_eggs()
        self._generate_shaders()
        return self._build_output()

    def _generate_disc_world(self, entry):
        """Generate world data for one disc/arc."""
        disc_id = entry["id"]
        protagonist = entry["protagonist"]

        for zone_name in entry["zones"]:
            zone = self._make_zone(disc_id, zone_name, protagonist, entry["type"])
            self.zones.append(zone)

    def _make_zone(self, disc_id, zone_name, protagonist, media_type):
        """Create a single zone with full data."""
        # Determine zone type from name
        ztype = self._zone_type(zone_name)
        template = ZONE_TEMPLATES.get(ztype, ZONE_TEMPLATES["field"])

        # Get NPCs and monsters for this zone
        npcs = NPCS_BY_DISC.get(disc_id, ["Kite","BlackRose"])
        monsters = MONSTERS_BY_ZONE.get(zone_name, ["Skeleton","Data Bug"])

        zone = {
            "id": f"{disc_id.lower()}_{zone_name.lower().replace(' ','_')}",
            "name": zone_name,
            "disc_id": disc_id,
            "protagonist": protagonist,
            "type": ztype,
            "media_type": media_type,
            "sky": template["sky"],
            "ground": template["ground"],
            "fog": template["fog"],
            "structures": template["structures"],
            "npcs": npcs[:5],
            "monsters": monsters[:3],
            "items": ITEMS_COMMON[:4] + ITEMS_RARE[:2],
            "avatars": AVATARS[:2] if protagonist in ["Kite","Haseo"] else [],
            "shader": f"shader_{ztype}",
            "texture_set": f"tex_{disc_id.lower()}_{ztype}",
            "ambient_color": template["ground"],
            "music_track": f"music_{disc_id.lower()}_{ztype}",
            "mood": self._zone_mood(zone_name),
        }
        return zone

    def _zone_type(self, name):
        """Map zone name to type."""
        name = name.lower()
        if "palace" in name: return "palace"
        if "cave" in name: return "dungeon"
        if "dungeon" in name: return "dungeon"
        if "fortress" in name: return "castle"
        if "wastes" in name: return "wastes"
        if "forest" in name: return "forest"
        if "valley" in name: return "valley"
        if "town" in name or "anu" in name: return "root_town"
        if "leiseijo" in name: return "root_town"
        return "field"

    def _zone_mood(self, name):
        """Get mood for zone."""
        moods = {
            "Mac Anu": "mysterious",
            "Dun Loireag": "ancient",
            "Cave of Trial": "dark",
            "Twilight Palace": "ethereal",
            "Leiseijo": "hopeful",
            "Mastats": "sacred",
            "Dragonbone Wastes": "desolate",
            "Kughai Valley": "peaceful",
            "Mystic Valley": "enchanted",
        }
        return moods.get(name, "neutral")

    def _add_easter_eggs(self):
        """Add hidden easter egg content."""
        for name, data in EASTER_EGGS.items():
            egg = {
                "id": f"easter_{name}",
                "name": name.replace("_"," ").title(),
                "zone": data["zone"],
                "type": data["type"],
                "note": data["note"],
                "hidden": True,
                "rarity": "legendary"
            }
            self.zones.append(egg)

    def _generate_shaders(self):
        """Generate shader definitions for all zone types."""
        for ztype in ZONE_TEMPLATES:
            shader = {
                "name": f"shader_{ztype}",
                "vertex": f"shaders/{ztype}_vert.glsl",
                "fragment": f"shaders/{ztype}_frag.glsl",
                "uniforms": ["u_time","u_fog","u_camZ","u_ground_color","u_sky_color"],
                "features": ["perspective_grid","fog_blend","scanline_overlay","bloom"]
            }
            self.shaders.append(shader)

    def _build_output(self):
        """Build final output structure."""
        return {
            "metadata": {
                "generator": "Seele AI World Generator",
                "version": "1.0.0",
                "canon_sources": [e["id"] for e in CANON],
                "total_zones": len([z for z in self.zones if "easter" not in z.get("id","")]),
                "total_easter_eggs": len(EASTER_EGGS),
            },
            "zones": self.zones,
            "shaders": self.shaders,
            "easter_eggs": list(EASTER_EGGS.keys()),
        }


def main():
    gen = SeeleWorldGenerator()
    output = gen.generate_all()

    out_path = BASE_DIR / "Content" / "Worlds" / "seele_worlds.json"
    out_path.parent.mkdir(parents=True, exist_ok=True)
    with open(out_path, "w") as f:
        json.dump(output, f, indent=2)

    print(f"[Seele] Generated {output['metadata']['total_zones']} zones")
    print(f"[Seele] Added {output['metadata']['total_easter_eggs']} easter eggs")
    print(f"[Seele] Generated {len(output['shaders'])} shaders")
    print(f"[Seele] Output: {out_path}")

    return output


if __name__ == "__main__":
    main()
