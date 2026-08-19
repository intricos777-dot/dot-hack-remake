# Modular Open World MMO Design

## Goals
- Fully moddable .hack open world
- Canon story timeline as playable modules
- Player-created characters and stories
- MMORPG infrastructure with local-first + optional online

## Core Systems
- WorldState: canonical timeline + player branch points
- Timeline: chapter/episode loader with mod hooks
- CharacterCreator: full avatar customization from presets
- Customization: hair/accessories/clothes/weapon skins
- Quest: player-authored quest chains
- ModSDK: hot-reload Lua/JSON content scripts
- Networking: peer-to-peer or dedicated server relay

## Mod Format
- Packaged as `.dothackmod` zip:
  - `manifest.json`
  - `story.json`
  - `characters/`
  - `quests/`
  - `assets/`

## Character Creator Presets
- Base body, face, hair
- Class presets: Twin Blade, Heavy Blade, Blademaster, Wavemaster
- Accessory slots: head, body, weapon, cape
- Skin system: PBR textures + material params
- Weapon skins: model swap + FX swap
