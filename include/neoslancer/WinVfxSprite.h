#pragma once

#include "neoslancer/WinVfxPalette.h"

#include <cstdint>
#include <vector>

namespace neoslancer {

// The engine's `.spr` asset format - like `.fnt` (see WinVfxFont.h), an
// opaque WinVFX resource Lancer.exe never parses itself
// (../StarLancer/reversing docs Pass 39). Recovered directly from
// winvfx8.dll's own VFX_shape_bounds/VFX_shape_draw code and empirically
// validated: every shape in a real CURSOR.SPR decodes with EXACT byte
// consumption landing precisely on the next shape's start (zero slack,
// across all 5 shapes), and 29/31 shapes in a much larger, real
// FRONTEND.SPR decode cleanly with the final shape's RLE stream ending
// exactly at end-of-file. Confidence 4.
//
// Container layout (little-endian, native x86):
//   +0x00: unknown
//   +0x04: shapeCount (uint32)
//   +0x08: shapeCount x 8-byte entries {
//              descOffset (uint32) - relative to container base, 0x18
//                  bytes before the shape's RLE pixel stream (see below)
//              paletteOffset (uint32) - relative to container base, 0 if
//                  none; points at a local color table:
//                  +0x00: count (uint32)
//                  +0x04 + i*4: {paletteSlot:u8, r6:u8, g6:u8, b6:u8}
//                      (6-bit-per-channel VGA-style triples, <<2 for 8-bit)
//          }
//
// Whole-file embedded palette (../StarLancer/reversing docs Pass 59,
// confidence 5, verified pixel-exact against a real screenshot of the
// medal-case UI): index 0's "shape" often isn't a real shape at all -
// when its record fails to parse as a plausible ShapeRecord AND is
// exactly 768 bytes before shape 1's descOffset, those 768 bytes are a
// private 256-entry {R,G,B} palette (6-bit VGA precision, same <<2
// scaling as WinVfxPalette) for every other shape in the file. This is
// a DIFFERENT mechanism from the (confirmed-unused, Pass 53)
// per-shape paletteOffset field above - the palette is per-FILE, not
// per-shape, conventionally stored as if it were shape 0. Confirmed for
// MEDAL1-6.SPR; NOT confirmed to hold for every .spr file (e.g.
// FRONTEND.SPR also has this exact 768-byte gap shape, but using it
// doesn't produce a coherent image for its large shapes - those are
// understood to be hover-highlight overlays never meant to render
// standalone, not a counterexample to this mechanism itself).
// parseWinVfxSprite detects and exposes it via hasEmbeddedPalette/
// embeddedPalette below when present, but still leaves shape 0's own
// entry in `shapes` as an empty placeholder (as for any other
// unparseable shape) so every other shape keeps its original index.
//
// Shape descriptor (at container base + descOffset):
//   +0x00: "bounds" (opaque to this port - not needed to rasterize)
//   +0x04: "origin" (opaque to this port - likely a draw hotspot/pivot)
//   +0x08: minX (int32)
//   +0x0c: minY (int32)
//   +0x10: maxX (int32)
//   +0x14: maxY (int32)
//   +0x18: RLE-encoded pixel stream, one row at a time, width = maxX-minX+1,
//          height = maxY-minY+1. Each row is a sequence of opcodes read
//          from a control byte (bit0 = type, bits1-7 = a 7-bit count):
//            count==0, bit0==0: end of row
//            count==0, bit0==1: "skip" - next byte = number of transparent
//                pixels to skip (advance x, write nothing)
//            count>0,  bit0==0: "solid run" - next byte = one color value,
//                repeated `count` times
//            count>0,  bit0==1: "literal run" - next `count` bytes are
//                literal pixel values, copied verbatim
struct WinVfxShape {
    int32_t minX = 0, minY = 0, maxX = 0, maxY = 0;
    int width = 0;
    int height = 0;
    std::vector<uint8_t> pixels; // width*height, 8-bit palette index, only where opaque[i] is true
    std::vector<uint8_t> opaque; // width*height, 1 = drawn, 0 = transparent
};

struct WinVfxSprite {
    std::vector<WinVfxShape> shapes;
    bool hasEmbeddedPalette = false;
    WinVfxPalette embeddedPalette; // valid only if hasEmbeddedPalette
};

bool parseWinVfxSprite(const std::vector<uint8_t>& data, WinVfxSprite& out);

} // namespace neoslancer
