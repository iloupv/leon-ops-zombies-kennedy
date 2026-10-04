#pragma once
#include <universal/memfile.h>
#include <universal/q_shared.h>
#include <cstddef>

// zombies: SP save-request queue (G_SaveGame 0x0043C850) and its per-server-loop consumer
// (0x004BC500, called by Com_ServerLoop at 0x0087E4D7), the writer 0x004CB930, the serializer
// 0x007EE240 and the memory commit 0x004C7F00. Parts of the serializer's content are still missing;
// notes/s1-plan.md lists them in exe order.

// G_SaveGame return codes (SP 0x0043C850).
enum SPSaveGameResult
{
    SP_SAVE_RELOADING = -1,       // g_reloading set: "savegame request ignored"
    SP_SAVE_SAME_FRAME = -2,      // a second auto save in one frame
    SP_SAVE_TOO_FREQUENT = -3,    // less than 3000 ms since the last request
    SP_SAVE_QUEUE_FULL = -4,      // three requests pending
    SP_SAVE_FRONTEND = -5,        // frontend map
    SP_SAVE_MOD_SKIPPED = -6,     // mod (L25): entities 1024+ in use, no save made; not a restart, so no script error
};

int G_SP_SaveGame(const char *filename, const char *description, const char *screenshot, int saveType,
    int flags, bool suppressPrint, int arg6, void (*callback)(), int callbackArg);
void G_SP_ClearSaveGameQueue();

// SP save header (0x498 bytes, filled by 0x0040C8F0; a SaveGame keeps it at +0x1004C). Offsets are the
// ones the exe writes and reads.
struct SPMemorySaveHeader
{
    int saveVersion;            // +0x000, SAVE_VERSION 0x134 (0x133 also accepted, SP 0x00484DC2)
    int mapChecksum;            // +0x004, the BSP checksum of the map (SP 0x00599F37)
    int saveCheckSum;           // +0x008, written 0
    int saveId;                 // +0x00C, the request id
    bool usesScriptChecksum;    // +0x010 (SP 0x0046BDB2), not written by 0x0040C8F0
    int scriptChecksum[3];      // +0x014 (SP 0x005EF4D0 / 0x0046BDCB)
    char mapName[256];          // +0x020 (SP 0x0087C830)
    char buildNumber[128];      // +0x120 (SP 0x0046BDE2)
    char campaign[256];         // +0x1A0, ui_campaign
    char screenShotName[64];    // +0x2A0
    char description[256];      // +0x2E0
    char filename[128];         // +0x3E0, "save\<name>.svg" (SP 0x0060B8F0; read by 0x00599F41)
    int health;                 // +0x460, player health in percent of max, 1..100
    int skill;                  // +0x464, g_gameskill
    int saveType;               // +0x468
    qtime_s time;               // +0x46C (SP 0x0040C120)
    int bodySize;               // +0x490, bytes used in the game state file (SP 0x0049C750)
    int pad;                    // +0x494, not written
};
static_assert(sizeof(SPMemorySaveHeader) == 0x498, "SP save header size");

// SP SaveGame (0x104F4 bytes; two of them at 0x01D04BA8 / 0x01D1509C, SP 0x008190B0).
enum SPSaveState
{
    SP_SAVE_STATE_FREE = 0,
    SP_SAVE_STATE_WRITING = 1,      // SP 0x00508D30
    SP_SAVE_STATE_VALID = 2,        // SP 0x00497520, "pending" for 0x004EF7C0
    SP_SAVE_STATE_COMMITTED = 3,    // SP 0x0045E030
    SP_SAVE_STATE_READING = 4,      // SP 0x00692310
};
struct SPSaveGame
{
    MemoryFile memFile[2];          // +0x00000 game state (0x200000 bytes), +0x08024 (0x140000 bytes)
    int state;                      // +0x10048, SPSaveState
    SPMemorySaveHeader header;      // +0x1004C
    void (*callback)(SPSaveGame *save, int arg); // +0x104E4 (SP 0x0068BE40)
    int callbackArg;                // +0x104E8
    bool isUsingGlobalBuffer;       // +0x104EC, set by 0x008190B0; 0 makes 0x00497520 drop the save
    bool unknown104ED;
    bool unknown104EE;              // +0x104EE, cleared by 0x0040C8F0
    bool unknown104EF;              // +0x104EF, cleared by 0x0040C8F0
    bool suppressPrint;             // +0x104F0: no 'S' server command on commit
    bool callbackNow;               // +0x104F1: 0x0068BE40 calls the callback at once
};
static_assert(sizeof(MemoryFile) == 0x8024, "SP MemoryFile size");
static_assert(offsetof(SPSaveGame, state) == 0x10048, "SP SaveGame layout");
static_assert(offsetof(SPSaveGame, header) == 0x1004C, "SP SaveGame layout");
static_assert(offsetof(SPSaveGame, isUsingGlobalBuffer) == 0x104EC, "SP SaveGame layout");
static_assert(sizeof(SPSaveGame) == 0x104F4, "SP SaveGame size");

// The committed memory save's header ([0x01D04BA0] + 0x1004C, SP 0x00599F10). SP reads it without a
// state check; an empty filename means nothing was ever committed.
const SPMemorySaveHeader *SP_GetCommittedMemorySave();
// SP 0x005411A0: 0 = the write buffer [0x01D04BA4], 1 = the committed buffer [0x01D04BA0].
SPSaveGame *SP_GetSaveBuffer(int which);
void G_SP_ProcessSaveQueue();
// G_LoadGame, SP 0x007ECB50: teardown, globals, VM and path nodes; returns false until complete.
bool G_SP_LoadGame(SPSaveGame *save);
// Memory-only load preparation (SP 0x00424530); scripts are retained on this restart.
SPSaveGame *G_SP_PrepareLoadGame(int mapChecksum);
bool G_SP_SaveScriptChecksumMatches(const SPSaveGame *save);
// Segment 1 readers used by G_InitGame(loadGame), SP 0x00668010 / 0x004E5E20.
bool G_SP_LoadWeapons(MemoryFile *memFile);
void G_SP_LoadRegisteredWeapons(MemoryFile *memFile);
