# neoslancer

StarLancer Linux Port

A cross-platform reimplementation of Digital Anvil's *Star Lancer* (the
RealSpace-engine `Lancer.exe`), targeting Linux via SDL2 + OpenGL.

Behavior is derived from reverse-engineering the original binary. The
sibling [`StarLancer`](https://github.com/DMJC/StarLancer) repository holds
that reverse-engineering work (`reversing/`) and is the source of truth for
how the original engine behaves.

## Status

Early scaffolding: window/GL-context bring-up and a bare main loop. No
gameplay or original-engine behavior has been ported yet.

## Building

Dependencies: CMake >= 3.20, a C++20 compiler, SDL2, OpenGL, GLEW.

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
