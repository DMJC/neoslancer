#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace neoslancer {

// The ship-interior "VR" room graph consumed by RunShipInteriorVRLoop
// (0x439fb0, ../StarLancer/reversing/reverse_engineered_functions.md +
// confidence_db.md). Lancer.exe never loads this from an asset file - it's
// static VRRoomNode data baked directly into the executable's own data
// segment, so "porting" it means reading the real struct instances out of
// the binary rather than parsing a resource format.
//
// VRRoomNode (44 bytes), confirmed by reading real instances from the
// binary and cross-checking every field against how the decompiled loop
// dereferences them:
//   +0x00: int16 hotspotX, hotspotY, hotspotW, hotspotH - this node's OWN
//          clickable rect, as read by its PARENT when hit-testing the
//          mouse against it as a destination (root/entry nodes: all zero)
//   +0x08: char *moviePath   - primary .bik filename for this room
//   +0x0c: char *moviePathAlt - nullable one-time arrival clip (not ported
//          here - this port always plays moviePath)
//   +0x10: int16 unknown (confirmed unused by the loop itself)
//   +0x12: int16 numTargets
//   +0x14: VRRoomNode *target[5] - FIXED 5-slot array regardless of
//          numTargets; trailing slots NULL
//   +0x28: int16 roomType - 0 = plain room; 1 = exit to the front-end
//          menu; 2/5/6/7/9 = unconditional jump to a fixed hub-pair root
//          for that type (this node's own target[] becomes dead data);
//          3/4 = fixed-screen-rect hover-prop triggers (NOT room
//          transitions - not ported, see confidence_db.md)
//   +0x2a: int16 soundFlag (-1 in every sample seen; meaning not pinned
//          down beyond "gates a transition sound effect")
//
// This session's own re-derivation (independent of, but consistent with,
// the reversing project's prior passes): imported nothing new - read the
// SAME Lancer.exe already open in the shared Ghidra project, via
// read_memory, directly at the entry node + all 6 late/early hub-pair
// roots + their full immediate children (37 real nodes total, Confidence
// 5 - exact bytes, not inferred). This is a REAL but PARTIAL slice of the
// full graph: the source docs themselves only fully catalogued ~30-40 of
// the ~145 total nodes found across the whole campaign (the rest were
// read but not individually recorded) - a target address absent from
// vrRoomGraphNodes() means "not walked yet", not "doesn't exist". See
// dependency notes in VRRoomScreen.h for how an unresolved target is
// handled (the hotspot is simply not offered as clickable).
struct VRRoomNodeData {
    uint32_t address; // the node's own address in Lancer.exe - used as a stable ID
    int16_t hotspotX, hotspotY, hotspotW, hotspotH;
    int16_t roomType;
    int16_t soundFlag;
    std::string moviePath; // empty for roomType!=0 doorway nodes this port doesn't play (see VRRoomScreen)
    std::vector<uint32_t> targets;
};

const std::vector<VRRoomNodeData>& vrRoomGraphNodes();
const VRRoomNodeData* findVRRoomNode(uint32_t address);

// roomType 2/5/6/7/9's hub-pair jump targets, Confidence 3 (direct code
// fact per the reversing docs); English hub names are Confidence 1
// (abbreviation guesses from movie filenames, not textually confirmed).
struct VRHubAddresses {
    uint32_t itacRot;   // roomType 2
    uint32_t pod;        // roomType 5
    uint32_t lock;        // roomType 6
    uint32_t briefing;    // roomType 7
    uint32_t corridor;    // roomType 9
};
const VRHubAddresses& vrLateCampaignHubs();
const VRHubAddresses& vrEarlyCampaignHubs();

constexpr uint32_t kVRLateCampaignEntry = 0x0050b2b8;
constexpr uint32_t kVREarlyCampaignEntry = 0x00506ad0;

// Applies the roomType-driven redirection rule: for roomType in
// {2,5,6,7,9}, returns the campaign-appropriate hub address and leaves
// exitToMenu false; for roomType==1, sets exitToMenu=true and returns 0;
// otherwise (0, or the unhandled hover-prop types 3/4) returns
// nodeAddress unchanged.
uint32_t resolveVRRoomTransition(uint32_t nodeAddress, bool lateCampaign, bool& exitToMenu);

} // namespace neoslancer
