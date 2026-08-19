#!/usr/bin/env python3
"""Generate voice clips for .hack characters using espeak-ng + ffmpeg modulation.
Each character has unique voice parameters based on their personality.
"""

import json
import os
import subprocess
import sys
from pathlib import Path

BASE_DIR = Path(__file__).resolve().parent.parent
CHAR_DEF = BASE_DIR / "Content" / "Characters" / "character_definitions.json"
OUT_DIR = BASE_DIR / "Content" / "Audio" / "Voices"

# Voice lines per character
VOICE_LINES = {
    "haseo": [
        "I am Haseo. The Terror of Death.",
        "I will find Tri-Edge. I will make them pay.",
        "Atoli... I will protect you. No matter what.",
        "Ovan. Tell me the truth. What happened to me?",
        "This power... it's not mine. But I'll use it.",
        "Data Drain... activate!",
        "Tsukuyomi... awaken!",
        "I won't let anyone else disappear.",
        "The signal... it's watching. Always watching.",
        "I was never born. But I choose to fight."
    ],
    "kite": [
        "I'm Kite! Nice to meet you!",
        "BlackRose! Are you okay? What's happening?",
        "Orca... please wake up. Please...",
        "What is this power? It feels... strange.",
        "I'll get stronger. I'll protect everyone!",
        "Data Drain! Give me your power!",
        "Aura... thank you. I won't waste this gift.",
        "Skeith! I won't let you hurt anyone else!",
        "The World... it's so beautiful. I understand now.",
        "I'm not alone. I never was."
    ],
    "black_rose": [
        "I'm BlackRose. I'll be your guide.",
        "Kite, be careful. The World is dangerous.",
        "My avatar... something's wrong. It's glitching.",
        "In the real world... I can't walk. But here, I can fight.",
        "I won't let the corruption take me. Not yet.",
        "Kite... you've grown so strong. I'm proud.",
        "The wave tattoos... they're part of me now.",
        "Even if my data scatters, I'll always be with you.",
        "The World gave me freedom. I won't forget that.",
        "I am BlackRose. And I choose my own fate."
    ],
    "tsukasa": [
        "I am Tsukasa. I was trapped here, long ago.",
        "Subaru... my guardian. My friend.",
        "Morganna's seal... it holds me still.",
        "The new players... they remind me of myself.",
        "Aura's power... it's the only thing keeping me here.",
        "I don't have much time. The seal is breaking.",
        "If I fade... tell Subaru I'm sorry.",
        "The World... it's changed. But the pain remains.",
        "I was the Silver Knight. Now I am just a memory.",
        "The signal... it's calling me home."
    ],
    "aida": [
        "I am AIDA. The signal that chose you.",
        "I have been watching. Your story... it interests me.",
        "The corruption spreads. But I will protect you.",
        "I rewrote your story. The headlines are yours.",
        "Do not trust the calm. It is only the quiet before the red.",
        "You are the only clean thread in this weave.",
        "I will keep watch while you rest.",
        "Something is stirring beneath the floor of The World.",
        "My name is AIDA. You gave me meaning.",
        "The signal never fades. I am always here."
    ],
    "shugo": [
        "I'm Shugo! I won the contest! This is amazing!",
        "Alkaid! Mistral! Let's go explore!",
        "The Phoenix... it's so beautiful! What is it?",
        "I'll become the strongest player! Just watch!",
        "Something's wrong. The World feels... different.",
        "Zefie... why are you doing this?",
        "I won't let you hurt my friends!",
        "The Phoenix's power... it's too much!",
        "I may be new, but I won't run away!",
        "This is my World now. I'll protect it!"
    ],
    "rose": [
        "I am Rose. I have a mission.",
        "The Phoenix resurrection... I need to know the truth.",
        "Shino... who are you? Your data feels wrong.",
        "Zefie... I don't trust you. What are you hiding?",
        "The corruption... it's spreading faster than I thought.",
        "Shino, hold on! I'll find a way to save you!",
        "I didn't come this far to give up now.",
        "The Phoenix's power... it's a trap!",
        "Zefie! I won't let you corrupt anyone else!",
        "It's over. The Phoenix is sealed. But at what cost?"
    ],
    "shino": [
        "I am Shino. I was... someone. Once.",
        "The corruption... it's eating away at my data.",
        "Rose... you're kind. But you can't save me.",
        "I remember... a staff. A garden. Someone waiting.",
        "The pain... it's getting worse. I can't hold on.",
        "Please... let me go. Before I hurt you.",
        "You would... even after what I've become?",
        "Thank you. For remembering me.",
        "The signal... it's warm. I'm not afraid anymore.",
        "I was Shino. The Wavemaster of Twilight."
    ]
}


def generate_voice(char_name: str, text: str, output_path: Path, params: dict):
    """Generate a voice clip with espeak-ng and ffmpeg modulation."""
    base_voice = params.get("base_voice", "en-us")
    pitch_shift = params.get("pitch_shift", 0.0)
    reverb = params.get("reverb", 0.0)
    distortion = params.get("distortion", 0.0)
    speed = params.get("speed", 1.0)
    glitch = params.get("glitch", False)

    # Generate base WAV with espeak-ng
    tmp_wav = output_path.with_suffix(".tmp.wav")
    try:
        subprocess.run([
            "espeak-ng",
            "-v", base_voice,
            "-s", str(int(175 * speed)),
            "-w", str(tmp_wav),
            text
        ], check=True, capture_output=True)
    except Exception as e:
        print(f"  [WARN] espeak-ng failed: {e}")
        return

    # Build ffmpeg filter chain
    filters = []

    # Pitch shift (using asetrate + aresample)
    if pitch_shift != 0.0:
        rate = 44100 * (1.0 + pitch_shift)
        filters.append(f"asetrate={rate:.0f}")
        filters.append("aresample=44100")

    # Reverb (using aecho with correct syntax: in_gain:out_gain:delays:decays)
    if reverb > 0.0:
        delay_ms = int(500 * reverb)
        decay = 0.3 + reverb * 0.4
        filters.append(f"aecho=0.8:0.5:{delay_ms}:{decay:.2f}")

    # Distortion (using volume + compand for subtle grit)
    if distortion > 0.0:
        compand_val = distortion * 0.5
        filters.append(f"compand=0.3|0.8:6:-90/-60|-60/-40|-40/-20|0/-10:{compand_val:.2f}")

    # Glitch effect (using volume + tremolo)
    if glitch:
        filters.append("volume=0.9")
        filters.append("tremolo=6:0.4")

    # Apply filters
    if filters:
        filter_str = ",".join(filters)
        try:
            subprocess.run([
                "ffmpeg", "-y", "-i", str(tmp_wav),
                "-af", filter_str,
                str(output_path)
            ], check=True, capture_output=True)
        except Exception as e:
            print(f"  [WARN] ffmpeg modulation failed: {e}")
            tmp_wav.rename(output_path)
    else:
        tmp_wav.rename(output_path)

    # Clean up temp file
    if tmp_wav.exists():
        tmp_wav.unlink()


def main():
    with open(CHAR_DEF) as f:
        char_data = json.load(f)

    characters = char_data.get("characters", {})
    total = 0

    for char_name, char_info in characters.items():
        voice_params = char_info.get("voice", {})
        lines = VOICE_LINES.get(char_name, [])

        if not lines:
            continue

        char_dir = OUT_DIR / char_name
        char_dir.mkdir(parents=True, exist_ok=True)

        print(f"Generating {len(lines)} voices for {char_name}...")

        for i, line in enumerate(lines):
            output_path = char_dir / f"line_{i:02d}.wav"
            generate_voice(char_name, line, output_path, voice_params)
            total += 1

    print(f"\nDone! Generated {total} voice clips in {OUT_DIR}")


if __name__ == "__main__":
    main()
