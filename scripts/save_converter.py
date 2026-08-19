#!/usr/bin/env python3
"""Convert .hack//G.U. Last Recode Steam save files to .hack// remake JSON format.

Parses the binary save file (data01) and extracts character data including
name, level, class, EXP, gold, HP, MP, and infection level.

Save format discovered via reverse-engineering data01 (149456 bytes):
  - Character name at offset 0x1D174 (ASCII, 12 bytes, null-padded)
  - Class byte at 0x1D186
  - Level byte at 0x1D187
  - EXP (u32) at offset 0x0700
  - Gold (u32) at offset 0x076C
  - HP (u32, /10 scaling) at offset 0x074C
  - MP/SP (u32, /10 scaling) at offset 0x0750
  - Infection level (u16) at data00:0x001E
  - Level confirmation (u32) at data00:0x0018

Character stats (STR/VIT/AGI/DEX) appear at 0x0738-0x073E as u16 values.
Party member blocks at 0x0600, 0x0624, 0x0648, 0x066C (36-byte records).
"""

import json
import struct
from pathlib import Path
import sys
import os

GAMES = {
    0: 0,  # Twin Blade  -> Twin Blade
    1: 0,  # Blade Brandier -> maps to 0 in remake
    2: 0,  # Long Arm -> maps to 0
    3: 0,  # Heavy Blade -> maps to 0
    4: 0,  # Steam Gunner -> maps to 0
    5: 0,  # Wave Master -> maps to 0
    6: 0,  # Adept Rogue -> maps to 0
    7: 0,  # Harvest Cleric -> maps to 0
    8: 0,  # Flick Reaper -> maps to 0
    9: 0,  # Tribal Grappler -> maps to 0
}

WEAPON_NAMES = ["Dual Swords", "Dual Blades", "Dual Guns", "Bayonet", "Staff"]


def parse_guhk_save(data01_path, data00_path=None):
    with open(data01_path, 'rb') as f:
        data = f.read()

    name_bytes = data[0x1D174:0x1D184]
    name = name_bytes.split(b'\x00')[0].decode('ascii', errors='replace')
    if not name:
        name = "haseo"

    level = struct.unpack_from('<B', data, 0x1D187)[0] if len(data) > 0x1D187 else 1

    cls_byte = struct.unpack_from('<B', data, 0x1D186)[0] if len(data) > 0x1D186 else 0
    cls = GAMES.get(cls_byte, 0)

    exp = struct.unpack_from('<I', data, 0x0700)[0] if len(data) > 0x0704 else 0
    if exp == 0 or exp > 99999999:
        exp = 0

    gold = struct.unpack_from('<I', data, 0x076C)[0] if len(data) > 0x0770 else 0
    if gold == 0 or gold > 9999999:
        gold = struct.unpack_from('<I', data, 0x06B8)[0] if len(data) > 0x06BC else 0
    if gold == 0 or gold > 9999999:
        gold = 0

    hp_raw = struct.unpack_from('<I', data, 0x074C)[0] if len(data) > 0x0750 else 0
    hp = hp_raw if hp_raw <= 9999 else int(hp_raw / 10) if hp_raw > 0 else 100

    mp_raw = struct.unpack_from('<I', data, 0x0750)[0] if len(data) > 0x0754 else 0
    mp = mp_raw if mp_raw <= 9999 else int(mp_raw / 10) if mp_raw > 0 else 50

    disk = 4

    if data00_path and os.path.exists(data00_path):
        with open(data00_path, 'rb') as f:
            d00 = f.read()
        if len(d00) > 0x0018:
            level = struct.unpack_from('<I', d00, 0x0018)[0]
        if len(d00) > 0x001E:
            infection_raw = struct.unpack_from('<H', d00, 0x001E)[0]
            if 0 <= infection_raw <= 8:
                disk = infection_raw
            elif infection_raw <= 100:
                disk = min(8, max(0, infection_raw * 8 // 100))

    infection_raw = struct.unpack_from('<I', data, 0x1D1B8)[0] if len(data) > 0x1D1BC else 0
    if 0 <= infection_raw <= 8 and infection_raw > 0:
        disk = infection_raw

    kills = 0
    quest_kills = 0

    hp_max = 10 * level
    mp_max = 5 * level
    hp = min(hp, hp_max)
    mp = min(mp, mp_max)

    potions = 1

    player = {
        "name": name,
        "cls": cls,
        "level": level,
        "xp": exp,
        "gold": gold,
        "potions": potions,
        "hp": hp,
        "mp": mp,
        "disk": disk,
        "kills": kills,
        "questKills": quest_kills,
        "weapon": 0,
        "weaponAtkMul": 1.0,
        "skills": [0, 0, 0, 0, 0, 0, 0, 0]
    }

    return player


def main():
    steam_save_root = Path.home() / ".local/share/Steam/userdata"

    if len(sys.argv) >= 2:
        data01_path = Path(sys.argv[1])
    else:
        vol_dirs = sorted(steam_save_root.glob("*/*/525480/remote/savedata/vol1"))
        if vol_dirs:
            data01_path = vol_dirs[0] / "data01"
        else:
            print("No Vol.1 save directory found. Usage: save_converter.py <path_to_data01>")
            sys.exit(1)

    if not data01_path.exists():
        print(f"Save file not found: {data01_path}")
        sys.exit(1)

    data00_path = data01_path.parent / "data00"

    print(f"Converting save from: {data01_path}")

    player = parse_guhk_save(str(data01_path), str(data00_path) if data00_path.exists() else None)

    print(f"  Name: {player['name']}")
    print(f"  Level: {player['level']}")
    print(f"  Class: {player['cls']}")
    print(f"  EXP: {player['xp']}")
    print(f"  Gold: {player['gold']}")
    print(f"  HP: {player['hp']}")
    print(f"  MP: {player['mp']}")
    print(f"  Infection (disk): {player['disk']}")

    save_dir = Path.home() / ".mineos/saves"
    save_dir.mkdir(parents=True, exist_ok=True)
    output_path = save_dir / "haseo.json"

    with open(output_path, 'w') as f:
        json.dump(player, f, indent=2)

    print(f"\nSave written to: {output_path}")
    print(f"Player: {json.dumps(player)}")


if __name__ == '__main__':
    main()
