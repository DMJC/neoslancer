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
- The WinVFX `.fnt`/`.spr`/`.ccb` asset formats (menu fonts, UI sprites,
  and the master color palette), recovered from `winvfx8.dll` directly
  (a third-party 2D middleware library the original engine never parses
  these itself - see `WinVfxFont.h`/`WinVfxSprite.h`/`WinVfxPalette.h`)
  and rendered as real GL textures (`WinVfxRenderer.h`). The Main Menu's
  title and every button label render with the real `handel.fnt` bitmap
  font; other screens still use a placeholder system font via SDL_ttf.
- The ship-interior VR loop (`RunShipInteriorVRLoop`): real hotspot
  navigation over a real (partial) slice of the actual room graph,
  re-derived from `Lancer.exe`'s own data, with real `.bik` video
  playback via FFmpeg's independent Bink decoder. Registered as screen
  ID 7 (`MissionBriefing`) but not currently wired to a Main Menu
  button. Plus the logo/splash intro movie sequence
  WinMain plays before the menu ever appears - the Main Menu itself then
  reuses that sequence's last clip (`SPLASH TO MM.BIK`), frozen on its
  final frame, as its own background (confirmed against a real, Wine-run
  copy of the game). `RunOptionsMenuScreen`/`RunControlsOptionsScreen`
  never open a `.bik` for a *persistent* background (both are static
  sprite sheets, `frntend4.spr`/`frntend6.spr`, each with their own
  embedded palette - see `WinVfxSprite.h`) - but real StarLancer plays a
  short real `.bik` fade/wipe clip at almost every menu navigation edge
  (Main Menu -> Options, Options -> a sub-screen, a sub-screen back to
  Options, etc.), whose frozen final frame becomes the new backdrop -
  `MenuAssets::playMenuTransition` ports that mechanism, using the real,
  per-edge clip name from a full call map recovered directly from the
  binary (`INTERFACE\*.bik`, 18 clips actually used, another 8
  referenced-but-dead). Real, decompiled-not-guessed hover-highlight
  shapes (`MenuAssets.h`, `OptionsMenuScreen.h`, `ControlsOptionsScreen.h`)
  are ported on the Main Menu and Options hub; Controls Options gets the
  same real checkbox-frame/checkmark shapes but positioned against this
  port's own (not pixel-identical) list layout, since it doesn't yet
  have a data model for the real screen's full scrollable key-rebind
  list.

No actual gameplay (flight/combat, mission loading, mission briefing)
has been ported yet.

## Building

Dependencies: CMake >= 3.20, a C++20 compiler, SDL2, SDL2_ttf, OpenGL, GLEW,
and FFmpeg (libavformat/libavcodec/libavutil/libswscale, found via
pkg-config).

```sh
cmake -S . -B build
cmake --build build
```

## Running

This repository contains no game data - only Digital Anvil's original,
proprietary `RESOURCE.HOG` and friends make anything appear on screen.
You need a copy of an actual StarLancer install (retail CD, GOG, etc.) with
at least the following present directly inside its top-level folder:

- `RESOURCE.HOG` - the main asset archive (menus, fonts, sprites, palette).
- `starlancer.ini` - device/video config; created if missing.
- The intro/menu movies: `NEW_NMS.BIK`, `NEW_DALOGO_FS_UNCMPR.BIK`,
  `WARTY_.BIK`, `SPLASH TO MM.BIK` (its final frame becomes the Main
  Menu's own background).
- `SHIPS/`, `MISSILES/`, `GUNS/`, `ADD_ONS/` - not yet consumed by
  anything ported so far, but resolved up front regardless.

Point neoslancer at that directory with the `NEOSLANCER_DATA` environment
variable:

```sh
NEOSLANCER_DATA=/path/to/StarLancer ./build/src/neoslancer
```

If `NEOSLANCER_DATA` isn't set, it falls back to the directory the
`neoslancer` binary itself lives in - so copying (or symlinking) a real
install's files next to `build/src/neoslancer` works too. A missing/wrong
data root isn't fatal: menus fall back to a placeholder SDL_ttf font
instead of the real game's assets, and a missing `RESOURCE.HOG` is only
logged, not treated as an error, so partial/incomplete data still boots.

## Dev tools

- `hogdump <archive.hog> --list|--extract|--decode-fnt|--decode-spr` -
  inspect/extract entries from a BigFile archive; see `--help` (no args)
  for the full set of subcommands.
- `sprviewer <file.spr> [palette.ccb]` - an interactive SDL2/OpenGL
  viewer for a single WinVFX `.spr` sprite sheet (loose `.spr`/`.ccb`
  files on disk are transparently RefPack-decompressed if needed, same
  as archive entries). Steps through every shape with Left/Right
  (Home/End for first/last). If the file carries its own embedded
  palette (many do - see `WinVfxSprite.h`; confirmed for the medal-case
  UI's `MEDAL1-6.SPR`, each with its own distinct color scheme baked
  in, not shared via any global `.ccb`) that's used automatically;
  otherwise a `[palette.ccb]` argument or greyscale-by-index. R swaps
  the red/blue channels, for sanity-checking a palette that looks like
  it might be parsed in the wrong byte order. Useful for checking a raw
  `.spr` file's shape count/bounds/decode without launching the full
  game, e.g. `./build/src/sprviewer /path/to/StarLancer/cd1/YOVB.SPR
  /path/to/StarLancer/SOFTPAL.CCB` for a loose file, or
  `hogdump RESOURCE.HOG --extract FRONTEND.SPR /tmp/frontend.spr` first
  for one that only exists packed inside the main archive.
- `tgaviewer <file.tga> [more.tga ...]` - an interactive SDL2/OpenGL
  viewer for genuine, unmodified Targa images (`TgaImage.h`; loose
  `.tga` files under `RESOURCE/` are transparently RefPack-decompressed
  if needed, same as `.spr`/`.fnt`; `cd1`/`cd2` disc-extracted copies
  are already plain TGA). Handles color-mapped, truecolor, and
  grayscale images, RLE packets, and both storage orders. Pass several
  files to step through them with Left/Right; R swaps red/blue, for
  sanity-checking a color-mapped image's BGR-vs-RGB byte order. Notably
  useful for `RESOURCE/palette.tga`/`softpal.tga`/`palette3.tga` -
  reversing docs Pass 61 found these (not `.ccb`, contrary to this
  project's own earlier assumption) are the real master 256-color
  palette source `InitializeGraphicsDevice` loads at startup, though
  `neoslancer` itself hasn't been migrated to read the palette from
  here yet (still reads `.ccb`, whose actual real purpose Pass 61 also
  reopened as an unresolved question).

## Layout

```
src/                Implementation (.cpp)
include/neoslancer/  Public headers (.h)
cmake/               CMake helper modules
assets/              Game/runtime assets (not the original's proprietary data)
docs/                Project documentation
```
