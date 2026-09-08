#pragma once

#include <array>
#include <cstdint>
#include <vector>

namespace neoslancer {

// The engine's `.fnt` asset format - NOT a StarLancer/Lancer.exe format at
// all. Lancer.exe hands the raw resource buffer straight to a third-party
// 2D library, WinVFX (winvfx8.dll/winvfx16.dll), as an opaque handle -
// ../StarLancer/reversing/reverse_engineered_functions.md Pass 39
// identifies this boundary but explicitly leaves the binary layout
// unrecovered ("genuinely unrecoverable without winvfx8.dll/winvfx16.dll
// (not present in this project)").
//
// This layout was recovered separately, directly from winvfx8.dll's own
// VFX_font_height/VFX_character_width/VFX_character_draw code (decompiled
// via Ghidra against the real WINVFX8.DLL in gamedata/StarLancer/) and
// empirically validated against a real font (handel.fnt): every one of
// its 96 populated glyphs decodes with the exact expected byte length and
// renders as a correct, legible character shape. Confidence 4 (direct
// disassembly + full empirical validation on one real file - not yet
// cross-checked against multiple font files).
//
// Layout (all fields little-endian, native x86 - unlike the big-endian
// BigFile/.hog container):
//   +0x00: unknown (not read by the functions this was recovered from)
//   +0x04: charCount (uint32) - number of entries in the offset table
//   +0x08: height (uint32) - fixed glyph height in pixels, shared by all
//          characters in this font
//   +0x0c: unknown (0 in the one sample checked)
//   +0x10: offset table, charCount x uint32, one entry per character code
//          (index = raw byte value, so charCount is usually 128 for
//          plain 7-bit ASCII) - each entry is a byte offset relative to
//          the font buffer's own start, or 0 for "no glyph"
//   at (base + offset[c]):
//     +0x00: width (uint32) - the glyph's pixel width AND its rendering
//            advance width
//     +0x04: width * height bytes, row-major, 8-bit palette-indexed
struct WinVfxGlyph {
    uint32_t width = 0;
    std::vector<uint8_t> pixels; // width * WinVfxFont::height bytes, empty if no glyph
};

struct WinVfxFont {
    uint32_t height = 0;
    std::array<WinVfxGlyph, 256> glyphs; // indexed by raw byte value

    bool hasGlyph(uint8_t c) const { return glyphs[c].width > 0; }
};

bool parseWinVfxFont(const std::vector<uint8_t>& data, WinVfxFont& out);

} // namespace neoslancer
