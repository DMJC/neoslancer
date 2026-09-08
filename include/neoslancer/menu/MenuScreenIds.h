#pragma once

namespace neoslancer::MenuScreenId {

// RunMenuScreenLoop's 12 real screen IDs (../StarLancer/reversing docs,
// confidence_db.md "RunMenuScreenLoop's 12 screen handlers — COMPLETE").
// 10/11 and 17/18 are each one real handler shared between two IDs via a
// mode flag (save/load, host/join) - kept as distinct IDs here too since
// that's how the dispatcher itself addresses them.
constexpr int MainMenu = 0;             // RunMainMenuScreen
constexpr int OptionsMenu = 1;          // RunOptionsMenuScreen
constexpr int SoundOptions = 3;         // RunSoundOptionsScreen
constexpr int MissionBriefing = 7;      // RunMissionBriefingScreen
constexpr int NetworkDisconnect = 8;    // RunNetworkDisconnectScreen
constexpr int SaveGame = 10;            // RunSaveLoadScreen, mode=save
constexpr int LoadGame = 11;            // RunSaveLoadScreen, mode=load
constexpr int NewGameSetup = 12;        // RunNewGameSetupScreen
constexpr int SaveGameBrowser = 13;     // RunSaveGameBrowserScreen
constexpr int MultiplayerSetup = 14;    // RunMultiplayerSetupScreen
constexpr int VideoOptions = 15;        // RunVideoOptionsScreen
constexpr int ControlsOptions = 16;     // RunControlsOptionsScreen
constexpr int MultiplayerLobbyHost = 17; // RunMultiplayerLobbyScreen, mode=host
constexpr int MultiplayerLobbyJoin = 18; // RunMultiplayerLobbyScreen, mode=join

} // namespace neoslancer::MenuScreenId
