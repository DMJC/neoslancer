#include "neoslancer/vr/VRRoomGraph.h"

#include <unordered_map>

namespace neoslancer {

// Generated from real Lancer.exe memory reads (Ghidra read_memory against
// the shared 'Starlancer' Ghidra project, program /Lancer.exe) - see
// VRRoomGraph.h for the struct layout and citation. Columns:
// {address, hotspotX, hotspotY, hotspotW, hotspotH, roomType, soundFlag, moviePath, targets}
const std::vector<VRRoomNodeData>& vrRoomGraphNodes() {
    static const std::vector<VRRoomNodeData> nodes = {
        {0x506ad0, 0, 0, 0, 0, 0, -1, "rel_ladd_bunk.bik", {0x506b30, 0x506b60, 0x506b00}},
        {0x506b00, 170, 120, 296, 290, 1, -1, "", {0x506f80}},
        {0x506b30, 0, 0, 40, 479, 0, -1, "rel_t2itac.bik", {0x506c50, 0x506e60, 0x506cb0}},
        {0x506b60, 562, 0, 78, 480, 0, -1, "rel_t2l.bik", {0x506b90, 0x506bc0, 0x506e00, 0x506e90}},
        {0x506b90, 0, 0, 40, 479, 0, -1, "rel_l2t.bik", {0x506b30, 0x506b60, 0x506b00}},
        {0x506bc0, 562, 0, 78, 480, 0, -1, "rel_l2cd.bik", {0x506d70, 0x506bf0, 0x506da0}},
        {0x506c80, 181, 150, 340, 150, 0, -1, "rel_itac2bunk.bik", {0x506b30, 0x506b60, 0x506b00}},
        {0x506d10, 181, 150, 340, 150, 0, -1, "rel_pod2c.bik", {0x506b30, 0x506b60, 0x506b00}},
        {0x506dd0, 181, 150, 340, 150, 0, -1, "rel_cd2bunk.bik", {0x506b30, 0x506b60, 0x506b00}},
        {0x506e00, 300, 0, 100, 100, 7, -1, "", {0x506e30}},
        {0x506e30, 0, 400, 400, 480, 0, -1, "rel_tv_c.bik", {0x506b90, 0x506bc0, 0x506e00, 0x506e90}},
        {0x506e90, 200, 200, 400, 400, 6, -1, "", {0x506f20, 0x506ec0}},
        {0x506f20, 0, 0, 80, 400, 0, -1, "rel_lock2c.bik", {0x506b30, 0x506b60, 0x506b00}},
        {0x50a988, 0, 0, 40, 479, 0, -1, "brd_p2i.bik", {0x50aef8, 0x50b288, 0x50b348}},
        {0x50a9b8, 562, 101, 78, 380, 0, -1, "brd_p2d.bik", {0x50a958, 0x50a9e8, 0x50aad8}},
        {0x50ab08, 220, 0, 200, 480, 0, -1, "itac2dor.bik", {0x50a958, 0x50a9e8, 0x50aad8}},
        {0x50ac58, 562, 0, 78, 480, 0, -1, "ir_d2cd.bik", {0x50abf8, 0x50abc8, 0x50ac28}},
        {0x50ae98, 0, 0, 40, 479, 0, -1, "ir_d2p.bik", {0x50ae68, 0x50ac88, 0x50acb8}},
        {0x50aec8, 181, 150, 340, 150, 0, -1, "itac2rot.bik", {0x50ae98, 0x50ab08, 0x50ac58}},
        {0x50aef8, 181, 150, 340, 150, 2, -1, "", {0x50aec8}},
        {0x50af88, 562, 0, 78, 480, 0, -1, "cd_d2cd.bik", {0x50b168, 0x50b1f8, 0x50aa78}},
        {0x50afb8, 181, 150, 340, 150, 0, -1, "cd2door.bik", {0x50a958, 0x50a9e8, 0x50aad8}},
        {0x50b138, 0, 208, 40, 90, 0, -1, "cd_d2p.bik", {0x50afe8, 0x50b108, 0x50b0a8}},
        {0x50b168, 0, 208, 40, 90, 0, -1, "cd_cd2d.bik", {0x50af88, 0x50afb8, 0x50b138}},
        {0x50b288, 0, 208, 40, 90, 0, -1, "brd_i2l.bik", {0x50b948, 0x50af28, 0x50b258}},
        {0x50b2b8, 0, 0, 0, 0, 0, -1, "b2iloop.bik", {0x50aef8, 0x50b288, 0x50b348}},
        {0x50b2e8, 462, 0, 178, 100, 7, -1, "", {0x50a988}},
        {0x50b318, 562, 0, 78, 480, 0, -1, "tv2brd.bik", {0x50a988, 0x50a9b8, 0x50b378, 0x50b2e8}},
        {0x50b348, 562, 0, 78, 480, 0, -1, "brd_i2p.bik", {0x50a988, 0x50a9b8, 0x50b378, 0x50b2e8}},
        {0x50b378, 181, 150, 340, 150, 5, 3, "", {0x50b678}},
        {0x50b3a8, 220, 0, 200, 480, 0, -1, "lockzomo.bik", {0x50b468, 0x50b6a8, 0x50b8b8}},
        {0x50b468, 0, 0, 160, 480, 0, -1, "lock_p2i.bik", {0x50b408, 0x50b438, 0x50b8e8}},
        {0x50b498, 220, 0, 200, 480, 2, -1, "", {0x50b498}},
        {0x50b648, 0, 0, 160, 480, 0, -1, "pod_i2l.bik", {0x50b5e8, 0x50b4c8, 0x50b618}},
        {0x50b678, 181, 150, 340, 150, 0, 2, "pod2rot2.bik", {0x50b498, 0x50b648}},
        {0x50b6a8, 181, 150, 340, 150, 5, 3, "", {0x50b678}},
        {0x50b8b8, 562, 0, 78, 480, 0, -1, "lock_p2d.bik", {0x50b6d8, 0x50b708, 0x50b888}},
    };
    return nodes;
}

const VRRoomNodeData* findVRRoomNode(uint32_t address) {
    static const std::unordered_map<uint32_t, const VRRoomNodeData*> byAddress = [] {
        std::unordered_map<uint32_t, const VRRoomNodeData*> map;
        for (const auto& node : vrRoomGraphNodes()) {
            map[node.address] = &node;
        }
        return map;
    }();
    const auto it = byAddress.find(address);
    return it == byAddress.end() ? nullptr : it->second;
}

const VRHubAddresses& vrLateCampaignHubs() {
    static const VRHubAddresses hubs{0x0050aec8, 0x0050b678, 0x0050b3a8, 0x0050b318, 0x0050b168};
    return hubs;
}

const VRHubAddresses& vrEarlyCampaignHubs() {
    static const VRHubAddresses hubs{0x00506c80, 0x00506d10, 0x00506f20, 0x00506e30, 0x00506dd0};
    return hubs;
}

uint32_t resolveVRRoomTransition(uint32_t nodeAddress, bool lateCampaign, bool& exitToMenu) {
    exitToMenu = false;
    const VRRoomNodeData* node = findVRRoomNode(nodeAddress);
    if (!node) {
        return nodeAddress;
    }

    const VRHubAddresses& hubs = lateCampaign ? vrLateCampaignHubs() : vrEarlyCampaignHubs();
    switch (node->roomType) {
        case 1:
            exitToMenu = true;
            return 0;
        case 2:
            return hubs.itacRot;
        case 5:
            return hubs.pod;
        case 6:
            return hubs.lock;
        case 7:
            return hubs.briefing;
        case 9:
            return hubs.corridor;
        default:
            return nodeAddress;
    }
}

} // namespace neoslancer
