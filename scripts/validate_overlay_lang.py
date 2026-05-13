#!/usr/bin/env python3
"""Validate RyazhTune overlay language JSON files.

The overlay is translated by libryazhahand at runtime, so every language file
must contain both the symbolic keys used by older code and the literal strings
that are actually drawn by the current UI.
"""

from __future__ import annotations

import argparse
import json
from pathlib import Path

LANG_DIR = Path("overlay/lang")
REQUIRED_LANGUAGES = {
    "de",
    "en",
    "es",
    "fr",
    "it",
    "ja",
    "ko",
    "nl",
    "pl",
    "pt",
    "ru",
    "uk",
    "zh",
    "zh-cn",
    "zh-tw",
}
ALIAS_SOURCE_KEYS = {
    "RyazhTune Settings": "OVERLAY_TITLE",
    "Shuffle": "SHUFFLE_MODE",
    "Repeat Mode": "REPEAT_MODE",
    "Volume": "VOLUME",
    "Auto-play Startup": "AUTO_PLAY_STARTUP",
    "Game: Play": "PLAY_ON_TITLE",
    "Game: Pause": "PAUSE_ON_TITLE",
    "Global Defaults": "DEFAULT_ON_START",
    "Playback Mode": "TUNE_MODE",
    "Normal": "TUNE_MODE_NORMAL",
    "Whitelist": "TUNE_MODE_WHITELIST",
    "Blacklist": "TUNE_MODE_BLACKLIST",
    "Audio Ducking": "AUDIO_DUCKING_ENABLED",
    "Ducking Volume": "AUDIO_DUCKING_VOLUME",
    "Language": "LANGUAGE",
    "Русский": "LANGUAGE_RUSSIAN",
    "English": "LANGUAGE_ENGLISH",
    "中文": "LANGUAGE_CHINESE",
    "Save": "SAVE_SETTINGS",
    "Exit": "EXIT",
    "Playlist": "PLAYLIST",
    "Browse": "BROWSE",
    "Settings": "SETTINGS",
    "Play": "PLAY",
    "Pause": "PAUSE",
    "Pass": "PASS",
    "Default Focus": "DEFAULT_FOCUS",
    "On": "ON",
    "Off": "OFF",
    "Custom Focus": "CUSTOM_FOCUS",
    "Miscellaneous": "MISCELLANEOUS",
    "Title Focus": "TITLE_FOCUS",
    "Home Focus": "HOME_FOCUS",
    "Remove Startup": "REMOVE_STARTUP",
    "Startup Path Removed": "STARTUP_PATH_REMOVED",
    "No startup path set": "NO_STARTUP_PATH_SET",
    "No startup path set in config.": "NO_STARTUP_PATH_SET",
    "Stop RyazhTune": "STOP_RYAZHATUNE",
    "Player": "PLAYER",
    "Couldn't open: ": "COULD_NOT_OPEN",
    "Maximum of ": "MAX_SCAN_HIT_PART1",
    " hit!": "MAX_SCAN_HIT_PART2",
    "Stopped Scanning Folder": "STOPPED_SCANNING_FOLDER",
    "Empty...": "EMPTY_FOLDER",
    "Startup Folder Set": "STARTUP_FOLDER_SET",
    "Add To Playlist": "ADD_TO_PLAYLIST",
    "Add to Playlist": "ADD_TO_PLAYLIST",
    "Set As Startup": "SET_AS_STARTUP",
    "Set as Startup": "SET_AS_STARTUP",
    "Tracks": "TRACKS",
    "Add All": "ADD_ALL",
    "Playlist is empty!": "PLAYLIST_IS_EMPTY",
    "Playlist is empty": "PLAYLIST_IS_EMPTY",
    "Remove": "REMOVE",
    "Remove All": "REMOVE_ALL",
    " by ": "BY_ARTIST_SEPARATOR",
    "Startup File Set": "STARTUP_FILE_SET",
    "Error": "ERROR_TITLE",
    "Music Library": "MUSIC_LIBRARY",
    "Toggle Mute": "TOGGLE_MUTE",
    "Music": "MUSIC",
    "Game": "GAME",
    "Title ID": "TITLE_ID",
    "Preset Volume": "PRESET_VOLUME",
    "1 track": "ONE_TRACK",
    " tracks": "N_TRACKS",
}
COMPOSED_ALIASES = {
    "Volume ⊘ \ue0e3 Toggle Mute": ("VOLUME", "TOGGLE_MUTE"),
}
CONSTANT_ALIASES = {
    "RyazhTune ♫": "RyazhTune ♫",
}
RUNTIME_ALIASES = set(ALIAS_SOURCE_KEYS) | set(COMPOSED_ALIASES) | set(CONSTANT_ALIASES)



def load_languages() -> dict[str, dict[str, str]]:
    languages: dict[str, dict[str, str]] = {}
    for path in sorted(LANG_DIR.glob("*.json")):
        languages[path.stem] = json.loads(path.read_text(encoding="utf-8"))
    if not languages:
        raise SystemExit("overlay/lang/*.json not found")
    return languages


def build_aliases(data: dict[str, str]) -> dict[str, str]:
    aliases: dict[str, str] = {}
    for alias, source_key in ALIAS_SOURCE_KEYS.items():
        aliases[alias] = data.get(source_key, alias)
    for alias, (left_key, right_key) in COMPOSED_ALIASES.items():
        left = data.get(left_key, "Volume")
        right = data.get(right_key, "Toggle Mute")
        aliases[alias] = f"{left} ⊘ \ue0e3 {right}"
    aliases.update(CONSTANT_ALIASES)
    return aliases


def normalize_languages(languages: dict[str, dict[str, str]]) -> list[str]:
    changed: list[str] = []
    for name, data in languages.items():
        before = len(data)
        for alias, value in build_aliases(data).items():
            data.setdefault(alias, value)
        if len(data) != before:
            changed.append(name)
    return changed


def write_languages(languages: dict[str, dict[str, str]], names: list[str]) -> None:
    for name in names:
        path = LANG_DIR / f"{name}.json"
        path.write_text(
            json.dumps(languages[name], ensure_ascii=False, indent=4) + "\n",
            encoding="utf-8",
        )


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument(
        "--fix",
        action="store_true",
        help="add missing runtime aliases before validating",
    )
    args = parser.parse_args()

    languages = load_languages()
    if args.fix:
        changed = normalize_languages(languages)
        write_languages(languages, changed)
        if changed:
            print(f"Added missing runtime aliases to: {', '.join(sorted(changed))}")

    missing_files = sorted(REQUIRED_LANGUAGES - set(languages))
    if missing_files:
        raise SystemExit(f"Missing language files: {missing_files}")

    base_keys = set(languages["ru"])
    missing_aliases = sorted(RUNTIME_ALIASES - base_keys)
    if missing_aliases:
        raise SystemExit(f"ru.json missing runtime aliases: {missing_aliases}")

    for name, data in languages.items():
        keys = set(data)
        missing = sorted(base_keys - keys)
        extra = sorted(keys - base_keys)
        if missing or extra:
            raise SystemExit(
                f"{name}.json key mismatch: missing={missing}, extra={extra}"
            )

        missing_runtime_aliases = sorted(RUNTIME_ALIASES - keys)
        if missing_runtime_aliases:
            raise SystemExit(
                f"{name}.json missing runtime aliases: {missing_runtime_aliases}"
            )

    print(f"Validated {len(languages)} language files with {len(base_keys)} keys each.")


if __name__ == "__main__":
    main()
