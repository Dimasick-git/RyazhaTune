# RyazhaTune

RyazhaTune is a custom background music module for the Nintendo Switch. It allows users to play custom audio files during gameplay and system navigation. This project is a fork of the original sys-tune implementation, extended with additional features for improved user experience and configuration management.

## Features

- **Persistent Playlists**: Playlist state is preserved across system reboots.
- **Autoplay Capability**: Configurable automatic playback initialization upon system startup.
- **Whitelist Mode**: Configurable title ID whitelisting to restrict background music playback to specific applications.
- **Audio Format Support**: Native decoding for MP3, FLAC, WAV, and WAVE formats.
- **Overlay Integration**: Seamless control via the Tesla overlay interface.

## Installation

1. Download the latest release archive (`.zip`) from the repository releases page.
2. Extract the contents of the archive to the root directory of the Nintendo Switch SD card.
3. Place supported audio files (MP3, FLAC, WAV) onto the SD card.
4. Launch the Tesla overlay menu to control playback and manage configurations.

## Architecture and Components

The project consists of several core components:

- **sys-tune**: The main background system module (sysmodule) responsible for audio decoding and playback via the `audren` service.
- **overlay**: The Tesla overlay plugin (`.ovl`) providing the graphical user interface for playback control.
- **ipc**: Inter-process communication definitions facilitating interaction between the overlay and the sysmodule.
- **libtesla**: A bundled UI library for rendering the overlay interface.
- **common**: Shared utilities for configuration management (`minIni`), SD card access, and process management.

## Configuration

Configuration files are automatically generated upon first use and are stored in `/config/RyazhTune/`. The module utilizes `minIni` for configuration parsing.

- `config.ini`: Main configuration parameters (autoplay, volume, shuffle, repeat).
- `whitelist.ini`: Title IDs configured for whitelist mode.
- `blacklist.ini`: Title IDs configured for blacklist mode.
- `playlist.txt`: Persistent playlist state.

## Building from Source

### Prerequisites

- [devkitPro](https://devkitpro.org/) with the `switch-dev` package group installed.
- Ensure the `DEVKITPRO` environment variable is correctly configured.

### Build Process

To compile the entire project (sysmodule and overlay), execute the following command in the root directory:

```bash
make all
```

To generate a distributable release archive:

```bash
make dist
```

## Acknowledgments

This project builds upon the foundational work of the original sys-tune developers. We extend our gratitude to the contributors of the original repository and the broader Nintendo Switch homebrew community.

## Version Information

- **Current Version:** 4.7.0
- **Status:** Stable

## License

This project is licensed under the GNU General Public License Version 2 (GPLv2). See the `LICENSE` file for full details. The bundled `libtesla` component is also distributed under the GPLv2.
