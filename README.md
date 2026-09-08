# neoslancer

StarLancer Linux Port

A cross-platform reimplementation of Digital Anvil's *Star Lancer* (the
RealSpace-engine `Lancer.exe`), targeting Linux via SDL2 + OpenGL.

Behavior is derived from reverse-engineering the original binary. The
sibling [`StarLancer`](https://github.com/DMJC/StarLancer) repository holds
that reverse-engineering work (`reversing/`) and is the source of truth for
how the original engine behaves.

## Status

- Window/GL-context bring-up, config (`starlancer.ini`), and command-line
  parsing, ported from `WinMain`'s bootstrap sequence.
- A `.hog`/BigFile archive reader (container, TOC, RefPack decompression),
  validated against a real `RESOURCE.HOG`.
- The front-end menu system (`RunMenuScreenLoop`'s full 12-screen dispatch
  table): a real, navigable Main Menu and Options hub, with Sound/Video/
  Controls options genuinely reading and writing `starlancer.ini`. Screens
  needing systems this project doesn't have yet (save files, multiplayer)
  are honest "not implemented" placeholders rather than guesses.
- Parsers for the WinVFX `.fnt`/`.spr` asset formats (menu fonts and UI
  sprites), recovered from `winvfx8.dll` directly (a third-party 2D
  middleware library the original engine never parses these itself -
  see `WinVfxFont.h`/`WinVfxSprite.h`) and validated against real game
  data. Not yet wired into rendering - menu text still uses a placeholder
  system font via SDL_ttf.
- The ship-interior VR loop (`RunShipInteriorVRLoop`): real hotspot
  navigation over a real (partial) slice of the actual room graph,
  re-derived from `Lancer.exe`'s own data, with real `.bik` video
  playback via FFmpeg's independent Bink decoder. Reachable from the
  Main Menu's "SHIP INTERIOR (DEMO)" button. Plus the logo/splash intro
  movie sequence WinMain plays before the menu ever appears.

No actual gameplay (flight/combat, mission loading, mission briefing)
has been ported yet.

## Building

Dependencies: CMake >= 3.20, a C++20 compiler, SDL2, SDL2_ttf, OpenGL, GLEW,
and FFmpeg (libavformat/libavcodec/libavutil/libswscale, found via
pkg-config).

```sh
cmake -S . -B build
cmake --build build
./build/src/neoslancer
```

## Layout

```
src/                Implementation (.cpp)
include/neoslancer/  Public headers (.h)
cmake/               CMake helper modules
assets/              Game/runtime assets (not the original's proprietary data)
docs/                Project documentation
```
