#include "g_sp_savegame.h"
#include "g_sp_levelstart.h"
#include <game_mp/g_main_mp.h>
#include <server_mp/sv_main_mp.h>
#include <server/sv_game.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <win32/win_shared.h>
#include <cstring>
#include "g_sp_loadgame.h"
#include <clientscript/cscr_variable.h>
#include <clientscript/cscr_compiler.h>
#include <qcommon/common.h>
#include <universal/com_buildinfo.h>
#include <universal/com_shared.h>
#include <win32/win_common.h>
#include <win32/win_net.h>
#include <intrin.h>
#include <database/db_registry.h>
#include <game/g_hudelem.h>
#include "g_sp_level_exit.h"
#include "g_scr_sp_entity.h"
#include "g_sp_player_state.h"
#include <physics/phys_main.h>
#include <qcommon/cmd.h>
#include <bgame/bg_weapons_def.h>
#include <game/g_items.h>
#include <game/g_weapon.h>
#include <clientscript/cscr_save.h>
#include <clientscript/cscr_vm.h>
#include <game/pathnode.h>
#include <game/actor_badplace.h>
#include <game/g_vehicle_path.h>
#include "g_scr_sp_ai.h"
#include <game/g_bsp.h>
#include <qcommon/cm_load.h>
#include "g_anim_commands_sp.h"
#include "g_sp_ext.h"
#include "actor_sp_ext.h"
#include <game/g_scr_vehicle.h>
#include <game/turret.h>
#include <physics/destructible.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_animtree.h>
#include <clientscript/cscr_memorytree.h>

extern game_hudelem_s g_hudelems[1024];  // cgame/cg_hudelem.cpp (SP 0x01A565A8)
extern vehicle_node_t s_nodes[2000];   // game/g_vehicle_path.cpp (DERIVED KB vehicle node table)
extern __int16 s_numNodes;

extern dvar_s *dvarHashTable[1024];     // universal/dvar.cpp (SP 0x02620BF0)
extern FastCriticalSection g_dvarCritSect;  // universal/dvar.cpp (SP 0x0261CBE0)

// zombies: one queued save request, SP layout 0x19C bytes at 0x02899800 (filled by 0x0087E090).
struct SPSaveGameRequest
{
    char filename[64];      // +0x000
    char description[256];  // +0x040
    char screenshot[64];    // +0x140
    int id;                 // +0x180, from the counter at 0x023A5598 (0x004F1130)
    int saveType;           // +0x184, 0 = auto save
    int flags;              // +0x188, 2 = commit to memory, 4 = write to disk
    bool suppressPrint;     // +0x18C
    int arg6;               // +0x190
    void (*callback)();     // +0x194
    int callbackArg;        // +0x198
};

static SPSaveGameRequest s_saveQueue[3];    // 0x02899800
static int s_saveQueueCount;                // 0x02899CD4
static bool s_autoSaveQueuedThisFrame;      // 0x02899CD8
static int s_lastSaveRequestTime = -3000;   // 0x00B83D78, static data initialised to 0xFFFFF448
static int s_saveId;                        // 0x023A5598

// zombies: SP save memory (SaveMemory_InitializeSaveSystem 0x008190B0): two SaveGames, each with a
// 0x200000-byte and a 0x140000-byte MemoryFile buffer. SP runs the init once at startup; KB runs it
// on first use (nothing reads the buffers before that).
static SPSaveGame s_saveGames[2];                   // 0x01D04BA8, 0x01D1509C
static unsigned char s_saveBufferA0[0x200000];      // 0x01D25590
static unsigned char s_saveBufferA1[0x140000];      // 0x01F25590
static unsigned char s_saveBufferB0[0x200000];      // 0x02065590
static unsigned char s_saveBufferB1[0x140000];      // 0x02265590
static SPSaveGame *s_committedSave;                 // 0x01D04BA0
static SPSaveGame *s_writeSave;                     // 0x01D04BA4
static bool s_saveCommitted;                        // 0x023A5594, set by the memory commit 0x004C7F00

static void SP_SaveMemory_Init()
{
    // zombies: SP 0x008190B0.
    if (s_committedSave)
        return;
    memset(&s_saveGames[0], 0, sizeof(s_saveGames[0]));
    memset(&s_saveGames[1], 0, sizeof(s_saveGames[1]));
    s_saveGames[0].isUsingGlobalBuffer = true;
    s_saveGames[0].memFile[0].buffer = s_saveBufferA0;
    s_saveGames[0].memFile[0].bufferSize = sizeof(s_saveBufferA0);
    s_saveGames[0].memFile[1].buffer = s_saveBufferA1;
    s_saveGames[0].memFile[1].bufferSize = sizeof(s_saveBufferA1);
    s_saveGames[1].isUsingGlobalBuffer = true;
    s_saveGames[1].memFile[0].buffer = s_saveBufferB0;
    s_saveGames[1].memFile[0].bufferSize = sizeof(s_saveBufferB0);
    s_saveGames[1].memFile[1].buffer = s_saveBufferB1;
    s_saveGames[1].memFile[1].bufferSize = sizeof(s_saveBufferB1);
    s_committedSave = &s_saveGames[0];
    s_writeSave = &s_saveGames[1];
}

SPSaveGame *SP_GetSaveBuffer(int which)
{
    // zombies: SP 0x005411A0.
    SP_SaveMemory_Init();
    if (which == 0)
        return s_writeSave;
    if (which == 1)
        return s_committedSave;
    return nullptr;
}

const SPMemorySaveHeader *SP_GetCommittedMemorySave()
{
    // zombies: SP 0x00599F10 reads [0x01D04BA0] + 0x1004C (0x0066C910) with no state check.
    SPSaveGame *save = SP_GetSaveBuffer(1);
    return save ? &save->header : nullptr;
}

static bool SP_SaveMemory_IsAvailable(const SPSaveGame *save)
{
    // zombies: SP 0x005FD8B0.
    return save->state == SP_SAVE_STATE_FREE || save->state == SP_SAVE_STATE_COMMITTED;
}

static bool SP_SaveMemory_IsPending(const SPSaveGame *save)
{
    // zombies: SP 0x004EF7C0.
    return save->state == SP_SAVE_STATE_VALID;
}

static bool SP_SaveMemory_IsValid(const SPSaveGame *save)
{
    // zombies: SP 0x004F3730, no overflow in either memory file.
    return !save->memFile[0].memoryOverflow && !save->memFile[1].memoryOverflow;
}

static void SP_SaveMemory_Reset(SPSaveGame *save)
{
    // zombies: SP 0x00508D30 (MemFile_InitForWriting 0x00492550, no error on overflow, compressed).
    MemFile_InitForWriting(&save->memFile[0], 0x200000, save->memFile[0].buffer, false, true);
    MemFile_InitForWriting(&save->memFile[1], 0x140000, save->memFile[1].buffer, false, true);
    save->state = SP_SAVE_STATE_WRITING;
    // SP also clears the global 0x023A5590 here (not identified; nothing ported reads it).
    save->callbackNow = false;
}

static void SP_SaveMemory_Abort(SPSaveGame *save)
{
    // zombies: SP 0x00683030.
    if (save->state == SP_SAVE_STATE_VALID)
        save->state = SP_SAVE_STATE_FREE;
}

static void SP_SaveMemory_StartSegment(SPSaveGame *save, int index, bool gameState)
{
    // zombies: SP 0x004BF220 -> MemFile_StartSegment (0x006328B0) on memory file 0 or 1.
    MemFile_StartSegment(gameState ? &save->memFile[0] : &save->memFile[1], index);
}

static void SP_SaveMemory_MoveToSegment(SPSaveGame *save, int index, bool gameState)
{
    // zombies: SP 0x005B0D80 -> MemFile_MoveToSegment (0x004412D0) on memory file 0 or 1.
    MemFile_MoveToSegment(gameState ? &save->memFile[0] : &save->memFile[1], index);
}

static void SP_SaveMemory_Finish(SPSaveGame *save)
{
    // zombies: SP 0x00497520.
    save->state = SP_SAVE_STATE_VALID;
    if (save->isUsingGlobalBuffer)
        return;
    if (save->memFile[0].buffer)
        save->memFile[0].buffer = nullptr;  // SP 0x0041ED40
    if (save->memFile[1].buffer)
        save->memFile[1].buffer = nullptr;
    save->state = SP_SAVE_STATE_FREE;
}

static bool SP_SaveMemory_CommitSave(SPSaveGame *save)
{
    // zombies: SP 0x0045E030.
    if (save->state != SP_SAVE_STATE_VALID)
    {
        Com_Printf(10, "Attempting to commit an invalid save buffer\n");
        return false;
    }
    SPSaveGame *previous = s_committedSave;
    s_committedSave = s_writeSave;
    s_writeSave = previous;
    s_committedSave->state = SP_SAVE_STATE_COMMITTED;
    s_writeSave->state = SP_SAVE_STATE_FREE;
    // SP 0x00649FD0 stores the name in sv_lastSaveGame unless the dummy name is in use (0x004E9C10);
    // zombiemode always uses the dummy name, so that branch is not reached here.
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c", 58));
    if (!save->suppressPrint)
        SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c", 83));
    return true;
}

static void SP_SaveMemory_Commit(SPSaveGame *save)
{
    // zombies: SP 0x004C7F00.
    s_saveCommitted = true;
    SP_SaveMemory_CommitSave(save);
}

static bool SP_IsFrontendMapForSave()
{
    // SP 0x00684EB0 with a null name reads "mapname".
    const char *map = Dvar_GetString("mapname");
    return !I_strnicmp(map, "menu_", 5) || !I_stricmp(map, "frontend");
}

void G_SP_ClearSaveGameQueue()
{
    // zombies: SP 0x004EF140, called from the level start path at 0x00534B50.
    s_saveQueueCount = 0;
    s_autoSaveQueuedThisFrame = false;
}

int G_SP_SaveGame(const char *filename, const char *description, const char *screenshot, int saveType,
    int flags, bool suppressPrint, int arg6, void (*callback)(), int callbackArg)
{
    // zombies: G_SaveGame (SP 0x0043C850).
    if (SP_IsFrontendMapForSave())
        return SP_SAVE_FRONTEND;
    if (Dvar_GetInt("g_reloading"))
    {
        Com_Printf(15, "savegame request ignored\n");
        return SP_SAVE_RELOADING;
    }
    extern int g_numEntitiesHi;
    if (g_numEntitiesHi) // mod (L25): the save format holds entity numbers below 1024 only; the horde mod does not save
    {
        Com_Printf(15, "mod: savegame skipped (server-only entities 1024+ in use)\n");
        // mod (L48): was SP_SAVE_RELOADING, which the script builtin reports as "Attempting to save during a
        // restart" (_autosave.gsc:80), once per level load in every run with bo1_mod_maxactors above 32
        return SP_SAVE_MOD_SKIPPED;
    }
    if ((int)Sys_Milliseconds() - s_lastSaveRequestTime < 3000)
    {
        if (!suppressPrint)
            Com_Printf(15, "SAVEGAME Warning: Save frequency too high. Save %s ignored\n", filename);
        return SP_SAVE_TOO_FREQUENT;
    }
    s_lastSaveRequestTime = Sys_Milliseconds();
    if (!saveType)
    {
        if (s_autoSaveQueuedThisFrame)
        {
            Com_Printf(15, "Warning: Multiple Autosaves attempted in same frame. Save %s ignored\n", filename);
            return SP_SAVE_SAME_FRAME;
        }
        s_autoSaveQueuedThisFrame = true;
    }
    if (s_saveQueueCount >= 3)
    {
        Com_Printf(15, "Warning: Pending Saves limit exceeded. Save %s ignored\n", filename);
        return SP_SAVE_QUEUE_FULL;
    }

    // zombies: request fill (SP 0x0087E090).
    SPSaveGameRequest *request = &s_saveQueue[s_saveQueueCount++];
    I_strncpyz(request->filename, filename, sizeof(request->filename));
    I_strncpyz(request->description, description, sizeof(request->description));
    I_strncpyz(request->screenshot, screenshot, sizeof(request->screenshot));
    request->saveType = saveType;
    request->flags = flags;
    request->id = ++s_saveId;
    request->suppressPrint = suppressPrint;
    request->arg6 = arg6;
    request->callback = callback;
    request->callbackArg = callbackArg;
    return request->id;
}

static void SP_SaveNotPorted(const char *what)
{
    // One line per save for each serializer part that is not ported yet (notes/s1-plan.md section 3).
    Com_Printf(15, "SaveGame: %s not written (not ported)\n", what);
}

static void Scr_SaveDeveloperFlags(MemoryFile *memFile)
{
    // zombies: SP 0x00568F20(0, memFile): scrVarPub.developer and developer_script of the server
    // instance (SP 0x032C8686 / 0x032C8687).
    unsigned char value = gScrVarPub[SCRIPTINSTANCE_SERVER].developer;
    MemFile_WriteData(memFile, 1, &value);
    value = gScrVarPub[SCRIPTINSTANCE_SERVER].developer_script;
    MemFile_WriteData(memFile, 1, &value);
}

static void G_SaveWeaponName(MemoryFile *memFile, const char *name)
{
    // zombies: byte-length string case of SP 0x007E9A90 (maximum 0x100).
    const int length = (int)strlen(name);
    iassert(length < 0x100);
    unsigned char size = (unsigned char)length;
    MemFile_WriteData(memFile, 1, &size);
    MemFile_WriteData(memFile, length, (unsigned __int8 *)name);
}

static void G_LoadWeaponName(MemoryFile *memFile, char *name)
{
    // zombies: name reads in SP 0x00668010 / 0x007E9C40.
    unsigned char length;
    MemFile_ReadData(memFile, 1, &length);
    MemFile_ReadData(memFile, length, (unsigned __int8 *)name);
    name[length] = 0;
}

static void G_SaveRegisteredWeapons(MemoryFile *memFile)
{
    // zombies: SP 0x007EAF80 -> 0x007E9BB0. DERIVED: KB has 2048 base weapons (SP has 128).
    // These records contain base weapons only, so the item model byte is always zero in both.
    for (unsigned int i = 1; i < 2048; ++i)
    {
        if (!IsItemRegistered(i))
            continue;
        unsigned char more = 0;
        MemFile_WriteData(memFile, 1, &more);
        G_SaveWeaponName(memFile, BG_WeaponName(i));
        unsigned char model = 0;
        MemFile_WriteData(memFile, 1, &model);
    }
    unsigned char end = 1;
    MemFile_WriteData(memFile, 1, &end);
}

bool G_SP_LoadWeapons(MemoryFile *memFile)
{
    // zombies: SP 0x00668010, called by G_InitGame(loadGame) after moving to segment 1.
    // The caller retries from segment 1 with the weapon definitions reset on an index mismatch.
    unsigned int count;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&count);
    for (unsigned int i = 1; i < count; ++i)
    {
        char name[256];
        G_LoadWeaponName(memFile, name);
        if (BG_GetWeaponIndexForName(name, G_RegisterWeapon) != i)
        {
            Com_Printf(10, "Weapon index mismatch for '%s'\n", name);
            return false;
        }
    }
    return true;
}

static int G_LoadWeaponItem(MemoryFile *memFile)
{
    // zombies: SP 0x007E9C40 -> 0x00689F00. Empty names omit the model byte.
    char name[256];
    G_LoadWeaponName(memFile, name);
    if (!name[0])
        return 0;
    unsigned char model;
    MemFile_ReadData(memFile, 1, &model);
    const int weapon = G_GetWeaponIndexForName(name);
    // DERIVED: KB's item index stride is 2048 (SP 128); registered weapon records use model 0.
    return weapon ? weapon + 2048 * model : 0;
}

void G_SP_LoadRegisteredWeapons(MemoryFile *memFile)
{
    // zombies: SP 0x004E5E20 -> 0x007E9C40 -> 0x00689F00. The returned item index is unused;
    // resolving the name (G_GetWeaponIndexForName, SP 0x005C25C0) registers the weapon if needed.
    unsigned char end;
    MemFile_ReadData(memFile, 1, &end);
    while (!end)
    {
        G_LoadWeaponItem(memFile);
        MemFile_ReadData(memFile, 1, &end);
    }
}

static void G_SaveWeapons(MemoryFile *memFile)
{
    // zombies: weapon table prefix of SP 0x007EB0C0, excluding weapon zero.
    unsigned int count = BG_GetNumWeapons();
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&count);
    for (unsigned int i = 1; i < count; ++i)
        G_SaveWeaponName(memFile, BG_WeaponName(i));
    G_SaveRegisteredWeapons(memFile);
}

static void SP_SaveStrings(SPSaveGame *save)
{
    // zombies: SP 0x007EB0C0 (segment 1).
    SP_SaveMemory_StartSegment(save, 1, true);
    G_SaveWeapons(&save->memFile[0]);
    // Brief/notes said SP 0x142 might be player infos; the exe uses it for objectives (SP
    // 0x007F79FE, objective_current). Do not map it to KB CS_PLAYERINFOS when adding these ranges.
    SP_SaveNotPorted("configstring tables (SP 0x007EAD40)");
}

static void Dvar_SaveDvars(MemoryFile *memFile, unsigned int dvarFlags)
{
    // zombies: SP 0x00415920: every dvar with one of these flags, in hash table order, as
    // int length + name, int length + value string (0x00860E30), ended by -1.
    _InterlockedExchangeAdd(&g_dvarCritSect.readCount, 1u);
    while (g_dvarCritSect.writeCount)
        NET_Sleep(0);
    for (int bucket = 0; bucket < 1024; ++bucket)
    {
        for (dvar_s *dvar = dvarHashTable[bucket]; dvar; dvar = dvar->hashNext)
        {
            if (!(dvar->flags & dvarFlags))
                continue;
            int length = (int)strlen(dvar->name);
            if (length >= 0x400)
            {
                Com_PrintError(16, "ERROR: Truncating dvar name '%s' in save game\n", dvar->name);
                length = 0x3FF;
            }
            MemFile_WriteInt(memFile, length);
            MemFile_WriteData(memFile, length, (unsigned __int8 *)dvar->name);
            const char *value = Dvar_ValueToString(dvar, dvar->current);
            length = (int)strlen(value);
            if (length >= 0x400)
            {
                Com_PrintError(16, "ERROR: Truncating dvar value '%s' for dvar '%s' in save game\n", value, dvar->name);
                length = 0x3FF;
            }
            MemFile_WriteInt(memFile, length);
            MemFile_WriteData(memFile, length, (unsigned __int8 *)value);
        }
    }
    MemFile_WriteInt(memFile, -1);
    Sys_UnlockRead(&g_dvarCritSect);
}

// zombies: SP 0x01C08AEC. Written to and read from the save only: a raw scan of the SP exe finds no
// other access (G_SaveState 0x007ED5C9, G_LoadGame 0x007ECC12), so it is always 0.
static int s_spLevelUnused8AEC;
// zombies: SP 0x00BDF508, int[4][4]: one 128-bit weapon mask per client (bit = weapon index found in
// the weapon def table 0x00BE19A8 by 0x00553DF0). Set by 0x00609F90 (called from 0x00462674,
// 0x004C777E, 0x004CFDAB, 0x005340A8, 0x008108FA), tested by 0x00434430 (0x004CA097), cleared by
// 0x004F16D0. None of these is ported, so the mask stays 0 in KB.
static unsigned int s_spClientWeaponMask[4][4];
// zombies: SP 0x01C07074, set only by the drawcompassfriendlies builtin (0x007FC0D0), not registered in KB.
static int s_spDrawCompassFriendlies;

static void G_SaveState_Globals(MemoryFile *memFile)
{
    // zombies: SP 0x007ED4C4..0x007EDA40, G_SaveState's sections 1 to 3 up to g_player_maxhealth. A
    // helper only so the round-trip check (SP_SaveGame_CheckGlobals) can write them again; the exe has
    // them inline in G_SaveState.

    // Section 1 (SP 0x007ED4C4..0x007ED86C). SP level_locals_t fields are mapped to KB's by name
    // (level.framenum 0x01C0488C / level.time 0x01C04890 anchor the SP layout).
    char zoneName[64];
    memset(zoneName, 0, sizeof(zoneName));
    if (const char *name = DB_GetZoneNameByType(0x20000))    // SP 0x0041D1D0(0x20000)
        I_strncpyz(zoneName, name, sizeof(zoneName));
    MemFile_WriteData(memFile, sizeof(zoneName), (unsigned __int8 *)zoneName);
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spLoadGame.restartCount);   // 0x027F76A8
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&level.time);                  // 0x01C04890
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&level.framenum);              // 0x01C0488C
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&s_spLevelUnused8AEC);         // 0x01C08AEC
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spActorLimit);              // 0x01C08AE4 setailimit
    int grenadeSuicideDisabled = G_SP_GrenadeSuicideDisabled();                     // 0x01C08AE8
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&grenadeSuicideDisabled);
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.changeLevel);       // 0x01C07014
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.changeLevelDelay);  // 0x01C07020
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.savePersist);       // 0x01C0532C
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.missionSuccess);    // 0x01C07018
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.loadGameContinue);  // 0x01C0701C
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&level.scriptPrintChannel);    // 0x01C06BE8 (G_InitGame 25)
    MemFile_WriteData(memFile, 64, (unsigned __int8 *)g_spLevelExit.nextMap);       // 0x01C018B0
    // 0x01C06BEC / 0x01C06BF4 / 0x01C06BFC, set by setminimap (SP 0x00803E8F..0x00803EFB).
    MemFile_WriteData(memFile, 8, (unsigned __int8 *)level.compassMapUpperLeft);
    MemFile_WriteData(memFile, 8, (unsigned __int8 *)level.compassMapWorldSize);
    MemFile_WriteData(memFile, 8, (unsigned __int8 *)level.compassNorth);
    // 0x01C0533C = savepersist 0x01C0532C + 0x10, the builtin-set latch in KB's layout
    // (setplayerignoreradiusdamage writes it in both exes).
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&level.bPlayerIgnoreRadiusDamageLatched);
    MemFile_WriteData(memFile, 64, (unsigned __int8 *)s_spClientWeaponMask);        // 0x00BDF508

    Dvar_SaveDvars(memFile, 0x1000);    // SP 0x007ED87E, DVAR_SAVED

    // Section 3 (SP 0x007ED886..0x007EDA57). Hud elems (SP g_hudelems 0x01A565A8, 1024 x 0x84, in use
    // when elem.type, byte +0x6E, is set): short index + struct image, ended by short -1. KB's
    // game_hudelem_s is 0x7C and has no pointers; its image is written.
    for (short index = 0; index < 0x400; ++index)
    {
        if (g_hudelems[index].elem.type == HE_TYPE_FREE)
            continue;
        MemFile_WriteData(memFile, 2, (unsigned __int8 *)&index);
        MemFile_WriteData(memFile, sizeof(game_hudelem_s), (unsigned __int8 *)&g_hudelems[index]);
    }
    short end = -1;
    MemFile_WriteData(memFile, 2, (unsigned __int8 *)&end);
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&level.fFogOpaqueDist);        // 0x01C05330
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&level.fFogOpaqueDistSqrd);    // 0x01C05334
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&s_spDrawCompassFriendlies);   // 0x01C07074
    int value = Dvar_GetInt("g_gameskill");     // SP [0x02899CDC] + 0x18
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&value);
    value = Dvar_GetInt("g_player_maxhealth");  // SP [0x01BF13EC] + 0x18
    MemFile_WriteData(memFile, 4, (unsigned __int8 *)&value);
}

// DERIVED KB field table corresponding to SP 0x00A53FF0. Both path images are 0x20 bytes;
// the owner is a SentientHandle, not a pointer or a persistent handle-info index.
struct SPPathSaveField
{
    unsigned int offset;
    unsigned int size;
    int type;
};
static constexpr SPPathSaveField s_pathSaveFields[] = {
    { offsetof(pathnode_dynamic_t, pOwner), sizeof(SentientHandle), 7 }
};
static_assert(sizeof(pathnode_dynamic_t) == 0x20, "DERIVED path dynamic save layout");
static_assert(sizeof(pathnode_t) == 0x80 && sizeof(pathlink_s) == 0xC, "DERIVED path save layout");
static_assert(sizeof(SentientHandle) == 4 && offsetof(SentientHandle, number) == 0,
    "DERIVED sentient handle save layout");
static_assert(s_pathSaveFields[0].offset == 0 && s_pathSaveFields[0].size == 4
    && s_pathSaveFields[0].offset + s_pathSaveFields[0].size <= sizeof(pathnode_dynamic_t),
    "DERIVED path field table bounds/order");

static void G_SavePathNodes(MemoryFile *memFile)
{
    // zombies: SP 0x00496AD0. First/next node (-1) at 0x00566060 / 0x005D69C0 visits
    // every actual node in index order. WriteStruct 0x007EA1D0 fixes the copied image first.
    for (unsigned int i = 0; i < g_path.actualNodeCount; ++i)
    {
        const pathnode_t *node = &gameWorldCurrent->path.nodes[i];
        pathnode_dynamic_t image = node->dynamic;
        // WriteField1 type 7 (SP 0x007E9F0A): save the one-based sentient index, clear infoIndex.
        const unsigned int owner = node->dynamic.pOwner.number;
        if (owner > ARRAY_COUNT(g_sentients))
            Com_Error(ERR_DROP, "WriteField1: sentient out of range (%i)", owner);
        memcpy((unsigned char *)&image + s_pathSaveFields[0].offset, &owner, sizeof(owner));
        MemFile_WriteData(memFile, sizeof(image), (unsigned char *)&image);
        // WriteField2 has no payload for a sentient handle.
        if (node->dynamic.wLinkCount != node->constant.totalLinkCount)
        {
            for (int j = 0; j < node->dynamic.wLinkCount; ++j)
            {
                int nodeNum = node->constant.Links[j].nodeNum;
                MemFile_WriteData(memFile, 4, (unsigned char *)&nodeNum);
            }
        }
    }
}

static void G_LoadPathNodes(MemoryFile *memFile)
{
    // zombies: SP 0x004AA5C0. Restore dynamics, resolve the owner, then reorder the live
    // link array by searching backwards for each saved node number and swapping whole links.
    int owners = 0;
    int reordered = 0;
    for (unsigned int i = 0; i < g_path.actualNodeCount; ++i)
    {
        pathnode_t *node = &gameWorldCurrent->path.nodes[i];
        MemFile_ReadData(memFile, sizeof(node->dynamic), (unsigned char *)&node->dynamic);
        unsigned int owner;
        memcpy(&owner, (unsigned char *)&node->dynamic + s_pathSaveFields[0].offset, sizeof(owner));
        if (owner > ARRAY_COUNT(g_sentients))
            Com_Error(ERR_DROP, "ReadField: sentient out of range (%i)", owner);
        // ReadField type 7 (SP 0x00491CDB) registers the handle before sentients are read.
        node->dynamic.pOwner = {};
        if (owner)
        {
            node->dynamic.pOwner.setSentient(&g_sentients[owner - 1]);
            ++owners;
        }
        if (node->dynamic.wLinkCount != node->constant.totalLinkCount
            && !(node->constant.spawnflags & 0x4000))
        {
            for (int j = 0; j < node->dynamic.wLinkCount; ++j)
            {
                int nodeNum;
                MemFile_ReadData(memFile, 4, (unsigned char *)&nodeNum);
                int k = node->constant.totalLinkCount - 1;
                while (k >= 0 && node->constant.Links[k].nodeNum != nodeNum)
                    --k;
                // KB corruption check; SP assumes the saved link exists in this map's array.
                if (k < 0)
                    Com_Error(ERR_DROP, "ReadField: path link %i missing at node %u", nodeNum, i);
                const pathlink_s link = node->constant.Links[k];
                node->constant.Links[k] = node->constant.Links[j];
                node->constant.Links[j] = link;
                reordered += k != j;
            }
        }
    }
    Com_Printf(15, "loadgame: restored %u path nodes (%i owners, %i link swaps)\n",
        g_path.actualNodeCount, owners, reordered);
}

#include "g_sp_save_entity.inl"
#include "g_sp_save_badplace.inl"
#include "g_sp_save_client.inl"
#include "g_sp_save_actor.inl"
#include "g_sp_save_vehicle.inl"
#include "g_sp_save_level.inl"

static void G_GetClientSaveExt(unsigned int clientNum, SPClientSaveExt *ext)
{
    // DERIVED gather of the SP-only gclient fields KB keeps in side tables (write side only; the
    // live install scatters them back once handles can move into the live tables).
    memset(ext, 0, sizeof(*ext));
    ext->ext = g_clientSpExt[clientNum];
    ext->x1 = G_SP_PlayerX1State(clientNum);
    ext->builtin = G_SP_PlayerBuiltinState(clientNum);
    int hint;
    if (G_SP_GetScriptHintString(&level.clients[clientNum], &hint))
        ext->scriptHintString = hint + 1;
}

static void G_SaveClients(MemoryFile *memFile)
{
    // zombies: G_SaveState client loop (SP 0x007EDB71..0x007EDC2C): every client below
    // com_maxclients, index then record, -1 end. SP does not test connection state.
    for (int i = 0; i < com_maxclients->current.integer; ++i)
    {
        MemFile_WriteData(memFile, 4, (unsigned char *)&i);
        SPClientSaveExt ext;
        G_GetClientSaveExt(i, &ext);
        G_WriteClient(&level.clients[i], &ext, memFile);
    }
    int end = -1;
    MemFile_WriteData(memFile, 4, (unsigned char *)&end);
}

static void G_SaveEntities(MemoryFile *memFile)
{
    // zombies: G_SaveState entity loop (SP 0x007EDA75..0x007EDB65).
    MemFile_WriteData(memFile, 4, (unsigned char *)&level.num_entities);
    for (int i = 0; i < MAX_GENTITIES; ++i)
    {
        const gentity_s *ent = &g_entities[i];
        if (!ent->r.inuse)
            continue;
        MemFile_WriteData(memFile, 4, (unsigned char *)&i);
        G_WriteEntity(ent, &g_entSpExt[i], G_SP_GetScriptedAnim(ent), G_SP_GetAnimCommandSequence(i), memFile);
    }
    int end = -1;
    MemFile_WriteData(memFile, 4, (unsigned char *)&end);
}

static void G_LoadEntitiesForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: G_LoadGame entity loop (SP 0x007ECE60). TEMPORARY: stable diagnostic destinations
    // until clients/actors/sentients can also be loaded. No incomplete entity enters gameplay.
    // Keep link owners until all rings are read: two blockers can disconnect the same link.
    static gentity_s linkOwners[MAX_GENTITIES];
    int linkOwnerCount = 0;
    int numEntities, index, count = 0;
    MemFile_ReadData(memFile, 4, (unsigned char *)&numEntities);
    if (numEntities < 0 || numEntities > MAX_GENTITIES)
        Com_Error(ERR_DROP, "G_LoadGame: invalid entity count %i", numEntities);
    MemFile_WriteData(writer, 4, (unsigned char *)&numEntities);
    for (;;)
    {
        MemFile_ReadData(memFile, 4, (unsigned char *)&index);
        MemFile_WriteData(writer, 4, (unsigned char *)&index);
        if (index < 0)
            break;
        if (index >= MAX_GENTITIES || ++count > MAX_GENTITIES)
            Com_Error(ERR_DROP, "G_LoadGame: entitynum out of range (%i)", index);
        static gentity_s ent;
        static gentitySpExt ext;
        scripted_anim_sp_t scripted;
        int sequence;
        bool hasScripted = G_ReadEntity(&ent, &ext, &scripted, &sequence, memFile);
        iassert(ent.s.number == index);
        G_WriteEntity(&ent, &ext, hasScripted ? &scripted : nullptr, sequence, writer);
        ent.s.eventSequence = 0; // SP +0xCC: outer-loop reset, after the image diagnostic.
        if (ent.disconnectedLinks)
            linkOwners[linkOwnerCount++] = ent;
        SP_ReleaseEntityFields(s_entityFields, ARRAY_COUNT(s_entityFields), &ent);
        SP_ReleaseEntityFields(s_entityExtFields, ARRAY_COUNT(s_entityExtFields), &ext);
    }
    // TEMPORARY: undo diagnostic link registrations before the partial-load shutdown.
    for (int i = 0; i < linkOwnerCount; ++i)
        Path_ConnectPathsForEntity(&linkOwners[i]);
    Com_Printf(15, "loadgame: decoded %i entities (num_entities %i)\n", count, numEntities);
}

static void G_LoadBadPlacesForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: SP 0x007EAA40 records into a diagnostic array. TEMPORARY: the live reader
    // G_ReadBadPlaces updates path counts, and brush records need the restored volume entity,
    // so it runs only once entities are installed.
    static badplace_t places[ARRAY_COUNT(g_badplaces)];
    int used = 0, brushes = 0;
    for (unsigned i = 0; i < ARRAY_COUNT(places); ++i)
    {
        G_ReadBadPlace(&places[i], memFile);
        used += places[i].type != 0;
        brushes += places[i].type == 2;
    }
    G_WriteBadPlaces(places, writer);
    for (unsigned i = 0; i < ARRAY_COUNT(places); ++i)
        G_ReleaseBadPlace(&places[i]);
    Com_Printf(15, "loadgame: decoded %u bad places (%i in use, %i brush)\n", ARRAY_COUNT(places), used, brushes);
}

static void G_LoadClientsForCheck(MemoryFile *memFile, MemoryFile *writer)
{
    // zombies: G_LoadGame client loop (SP 0x007ECF38..0x007ECFA2) into diagnostic storage.
    // TEMPORARY: the live loop passes the restored entity (SP: its r.inuse) and installs the
    // record and side tables; here nothing enters the live client array.
    int index, count = 0;
    for (;;)
    {
        MemFile_ReadData(memFile, 4, (unsigned char *)&index);
        MemFile_WriteData(writer, 4, (unsigned char *)&index);
        if (index < 0)
            break;
        // SP compares index > level.maxclients; KB also bounds the side tables.
        if (index > level.maxclients || index >= (int)ARRAY_COUNT(g_clients))
            Com_Error(ERR_DROP, "G_LoadGame: clientnum out of range");
        static gclient_s client;
        SPClientSaveExt ext;
        G_ReadClient(&client, &ext, memFile, nullptr);
        iassert(client.ps.clientNum == index);
        G_WriteClient(&client, &ext, writer);
        SP_ReleaseEntityFields(s_clientFields, ARRAY_COUNT(s_clientFields), &client);
        SP_ReleaseEntityFields(s_clientExtFields, ARRAY_COUNT(s_clientExtFields), &ext);
        ++count;
    }
    Com_Printf(15, "loadgame: decoded %i clients\n", count);
}

static void G_SaveState(bool saving, SPSaveGame *save)
{
    // zombies: SP 0x007ED4A0(1, save) (segment 2). Sections in exe order; see notes/s1-plan.md.
    (void)saving;
    SP_SaveMemory_StartSegment(save, 2, true);
    G_SaveState_Globals(&save->memFile[0]);
    Sentient_WriteGlobals(&save->memFile[0]); // SP 0x007EDA53 -> 0x004AAEA0
    Scr_SavePre(SCRIPTINSTANCE_SERVER); // SP 0x007EDA57 -> 0x00642F70(0, 1)
    Scr_CheckSaveObjects(SCRIPTINSTANCE_SERVER);
    SP_SaveMemory_StartSegment(save, 3, true); // SP 0x007EDA64
    G_SavePathNodes(&save->memFile[0]); // SP 0x007EDA70 -> 0x00496AD0
    G_SaveEntities(&save->memFile[0]);
    G_WriteBadPlaces(g_badplaces, &save->memFile[0]); // SP 0x007EDB6C -> 0x007EA960
    G_SaveClients(&save->memFile[0]);
    G_SaveActors(&save->memFile[0]); // SP 0x007EDC3F
    G_SaveSentients(&save->memFile[0]); // SP 0x007EDCCE
    G_SaveVehicles(&save->memFile[0]); // SP 0x007EDD58
    G_SaveLevelSystems(&save->memFile[0]); // SP 0x007EDD81..0x007EE0F6
    SP_SaveMemory_StartSegment(save, 4, true); // SP 0x007EE0FD
    Scr_Save(SCRIPTINSTANCE_SERVER, &save->memFile[0]); // SP 0x007EE105 -> 0x0050DF00
    SP_SaveMemory_StartSegment(save, 5, true); // SP 0x007EE10F
    SP_SaveNotPorted("segment 5 (remaining animation/client state)");
}

static void Dvar_LoadDvars(MemoryFile *memFile, unsigned short dvarFlags)
{
    // zombies: SP 0x00862F60 (G_LoadGame calls it through 0x00595D40 with flags 0). The reader of
    // Dvar_SaveDvars: every saved value is set; the value it replaces is kept in dvar->saved for
    // Dvar_RestoreDvars (SP 0x00535F70), which runs first.
    char name[0x400];
    char string[0x400];
    Dvar_RestoreDvars();
    int length;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&length);
    while (length >= 0)
    {
        if (length >= 0x400)
            Com_Error(ERR_DROP, "SAVE_STRING_MAX_SIZE exceeded in save game");
        MemFile_ReadData(memFile, length, (unsigned __int8 *)name);
        name[length] = 0;
        MemFile_ReadData(memFile, 4, (unsigned __int8 *)&length);
        if (length >= 0x400)
            Com_Error(ERR_DROP, "SAVE_STRING_MAX_SIZE exceeded in save game");
        MemFile_ReadData(memFile, length, (unsigned __int8 *)string);
        string[length] = 0;
        // SP hashes the name (0x1505, *33 + tolower) and looks it up with 0x00862280.
        dvar_s *dvar = Dvar_FindMalleableVar(name);
        if (dvar)
        {
            if (dvar != Dvar_FindMalleableVar("sv_restoreDvars"))   // SP [0x0261CBD8]
            {
                dvar->loadedFromSaveGame = true;
                if (dvar->type == DVAR_TYPE_STRING)
                    Dvar_CopyString((char *)dvar->current.string, &dvar->saved);  // SP 0x00542140
                else
                    dvar->saved = dvar->current;
                DvarValue value;
                Dvar_StringToValue(&value, dvar->type, dvar->domain, string);  // SP 0x008612A0
                Dvar_SetVariant(dvar, value, DVAR_SOURCE_INTERNAL);             // SP 0x00861DE0
                dvar->flags |= dvarFlags;
            }
        }
        else
        {
            // SP 0x00862D70: an unknown saved dvar is registered as a string dvar.
            DvarValue value;
            DvarLimits domain;
            memset(&value, 0, sizeof(value));
            memset(&domain, 0, sizeof(domain));
            value.string = string;
            Dvar_RegisterVariant(name, DVAR_TYPE_STRING, dvarFlags | 0x4000, value, domain, "");
        }
        MemFile_ReadData(memFile, 4, (unsigned __int8 *)&length);
    }
}

static void G_LoadGame_Globals(MemoryFile *memFile)
{
    // zombies: SP 0x007ECBC5..0x007ECE2A, the reader of G_SaveState_Globals (G_LoadGame's sections 1
    // to 3). Reads mirror the writes one to one: MemFile_ReadData never crosses a compressed chunk
    // that one write did not cross (SP 0x00473420 has the same rule).
    char zoneName[64];
    MemFile_ReadData(memFile, sizeof(zoneName), (unsigned __int8 *)zoneName);
    // SP queues "loadGump <zone>" (0x0049B930 = Cbuf_AddText). KB registers no loadGump command; a
    // memory save restores in the process that wrote it, where that gump zone is still loaded.
    if (Cmd_FindCommand("loadGump"))
        Cbuf_AddText(0, va("loadGump %s\n", zoneName));
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spLoadGame.restartCount);  // 0x027F76A8
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&level.time);                 // 0x01C04890
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&level.framenum);             // 0x01C0488C
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&s_spLevelUnused8AEC);        // 0x01C08AEC
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spActorLimit);             // 0x01C08AE4
    int grenadeSuicideDisabled;                                                   // 0x01C08AE8
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&grenadeSuicideDisabled);
    G_SP_SetGrenadeSuicideDisabled(grenadeSuicideDisabled != 0);
    svs.time = level.time;                                  // SP 0x0286D014
    // SP 0x00658ED0: both physics clocks (0x00B79D18 / 0x00B79D1C) to the level time.
    physGlob.timeLastSnapshot = level.time;
    physGlob.timeLastUpdate = level.time;
    // Every client's last usercmd time (SP client_t +0x10F38) to 0, so the next usercmds run.
    for (int i = 0; i < com_maxclients->current.integer; ++i)
        svs.clients[i].lastUsercmd.serverTime = 0;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.changeLevel);      // 0x01C07014
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.changeLevelDelay); // 0x01C07020
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.savePersist);      // 0x01C0532C
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.missionSuccess);   // 0x01C07018
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&g_spLevelExit.loadGameContinue); // 0x01C0701C
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&level.scriptPrintChannel);   // 0x01C06BE8
    MemFile_ReadData(memFile, 64, (unsigned __int8 *)g_spLevelExit.nextMap);      // 0x01C018B0
    MemFile_ReadData(memFile, 8, (unsigned __int8 *)level.compassMapUpperLeft);   // 0x01C06BEC
    MemFile_ReadData(memFile, 8, (unsigned __int8 *)level.compassMapWorldSize);   // 0x01C06BF4
    MemFile_ReadData(memFile, 8, (unsigned __int8 *)level.compassNorth);          // 0x01C06BFC
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&level.bPlayerIgnoreRadiusDamageLatched); // 0x01C0533C
    MemFile_ReadData(memFile, 64, (unsigned __int8 *)s_spClientWeaponMask);       // 0x00BDF508

    Dvar_LoadDvars(memFile, 0);     // SP 0x00595D40

    memset(g_hudelems, 0, sizeof(g_hudelems));  // SP 0x007ECD37 (0x21000 = 1024 x SP 0x84)
    short index;
    MemFile_ReadData(memFile, 2, (unsigned __int8 *)&index);
    while (index != -1)
    {
        MemFile_ReadData(memFile, sizeof(game_hudelem_s), (unsigned __int8 *)&g_hudelems[index]);
        MemFile_ReadData(memFile, 2, (unsigned __int8 *)&index);
    }
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&level.fFogOpaqueDist);       // 0x01C05330
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&level.fFogOpaqueDistSqrd);   // 0x01C05334
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&s_spDrawCompassFriendlies);  // 0x01C07074
    int value;
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&value);
    Dvar_SetInt((dvar_s *)Dvar_FindVar("g_gameskill"), value);     // SP 0x0045A170
    if (const dvar_s *savedGameskill = Dvar_FindVar("saved_gameskill"))    // SP 0x005AE810
        Dvar_SetFromString((dvar_s *)savedGameskill, va("%d", value)); // SP 0x0044A2A0
    G_SP_UpdateSkillDvars();        // SP 0x00640590
    MemFile_ReadData(memFile, 4, (unsigned __int8 *)&value);
    Dvar_SetInt((dvar_s *)Dvar_FindVar("g_player_maxhealth"), value);
}

bool G_SP_SaveScriptChecksumMatches(const SPSaveGame *save)
{
    // zombies: Scr_GetChecksum (SP 0x005EF4D0), compared at 0x0051EC7F and 0x0046BDCB.
    const scrVarPub_t *pub = &gScrVarPub[SCRIPTINSTANCE_SERVER];
    return save->header.scriptChecksum[0] == pub->checksum
        && save->header.scriptChecksum[1] == gScrCompilePub[SCRIPTINSTANCE_SERVER].programLen
        && save->header.scriptChecksum[2] == pub->endScriptBuffer - pub->programBuffer;
}

SPSaveGame *G_SP_PrepareLoadGame(int mapChecksum)
{
    // zombies: memory-save/retained-scripts branch of SP 0x00424530. The disk/source-recompile
    // branches are not used by loadgame_continue in zombies and are not ported here.
    SPSaveGame *save = SP_GetSaveBuffer(1);  // SP 0x00663970
    if (!SP_MemorySaveMatches(save->header.filename, mapChecksum))
    {
        Com_Error(ERR_DROP, "\x15Unable to find save.");
        return nullptr;
    }
    // SP 0x00692310 initializes both files and sets state 4. The secondary payload is not
    // written yet (header +0x494 is zero); KB MemFile rejects a zero-size reader.
    MemFile_InitForReading(&save->memFile[0], save->header.bodySize, save->memFile[0].buffer, true);
    if (save->header.pad)
        MemFile_InitForReading(&save->memFile[1], save->header.pad, save->memFile[1].buffer, true);
    save->state = SP_SAVE_STATE_READING;
    // SP 0x00443120(memFile, fileHandle=0, server): read/ignore developer, restore developer_script.
    unsigned char developer;
    MemFile_ReadData(&save->memFile[0], 1, &developer);
    MemFile_ReadData(&save->memFile[0], 1, (unsigned __int8 *)&gScrVarPub[SCRIPTINSTANCE_SERVER].developer_script);
    SP_SaveMemory_MoveToSegment(save, -1, true);
    if (!SP_SaveMemory_IsValid(save))
        Com_Error(ERR_DROP, "\x15The save file has become corrupted.\n");
    Com_Printf(15, "loadgame: prepared memory reader (state %i, %i bytes)\n", save->state, save->header.bodySize);
    return save;
}

static void SP_SaveGame_CheckRestoredScript(SPSaveGame *save)
{
    // KB diagnostic: reserialize the decoded graph with its original save IDs. Segment-local
    // compression is deterministic; compare every byte, including object/child order and roots.
    static unsigned char buffer[0x200000];
    static MemoryFile writer;
    MemFile_InitForWriting(&writer, sizeof(buffer), buffer, true, true);
    Scr_Save(SCRIPTINSTANCE_SERVER, &writer);
    MemFile_StartSegment(&writer, -1);
    const unsigned char *original = MemFile_GetSegmentAddess(&save->memFile[0], 4);
    const int length = MemFile_GetSegmentAddess(&save->memFile[0], 5) - original;
    const bool same = !writer.memoryOverflow && length == writer.bytesUsed
        && !memcmp(original, buffer, length);
    Com_Printf(15, "loadgame: VM reserialization %s (%i saved bytes, %i restored bytes)\n",
        same ? "identical" : "DIFFERENT", length, writer.bytesUsed);
    iassert(same);
}

bool G_SP_LoadGame(SPSaveGame *save)
{
    // zombies: G_LoadGame, SP 0x007ECB50 (called by the 0x0046BD80 checks). Partial restore:
    // entity/VM teardown, globals, VM, path payload and entity codec diagnostic.
    // Brief said path transient resets; exe does ShutdownRopes / Rope_InitRopes here.
    ShutdownRopes();           // SP 0x0046A5D0
    Rope_InitRopes();           // SP 0x0067D6F0
    // zombies: SP 0x007ECB6C..0x007ECBA4. Free the map-spawned entities before replacing
    // their script graph.
    const dvar_s *movingPaths = Dvar_FindVar("enable_moving_paths");
    if (movingPaths)
        Dvar_SetInt((dvar_s *)movingPaths, 0);
    G_FreeEntities(true);       // SP 0x0042C840
    if (movingPaths)
        Dvar_SetInt((dvar_s *)movingPaths, 1);
    HudElem_DestroyAll();       // SP 0x005188E0
    Path_ShutdownBadPlaces();   // SP 0x00435030 -> 0x00692A70
    for (unsigned int i = 0; i < g_path.actualNodeCount; ++i)
        Scr_FreeEntityNum(i, 2, SCRIPTINSTANCE_SERVER);
    // SP 0x005420B0 frees only script entities; G_FreeVehiclePaths also clears constant data.
    for (int i = 0; i < s_numNodes; ++i)
        Scr_FreeEntityNum(s_nodes[i].index, 3, SCRIPTINSTANCE_SERVER);
    // 0x00512670 clears threat group strings and the table before the VM is shut down.
    Actor_FreeThreatBiasGroups();
    Scr_ShutdownSystem(SCRIPTINSTANCE_SERVER, 1, 1); // SP 0x00596D40
    // zombies: animation command table initialization (SP 0x005579D0), using KB's command pool.
    G_SP_ResetAnimCommands();
    MemoryFile *memFile = &save->memFile[0];    // SP 0x0056BFC0
    level.initializing = 1;     // SP [0x01C04180] = 1
    SP_SaveMemory_MoveToSegment(save, 2, true);
    G_LoadGame_Globals(memFile);
    Com_Printf(15, "loadgame: restored globals at level.time %i (script objects %u, threads %u)\n",
        level.time, gScrVarPub[SCRIPTINSTANCE_SERVER].numScriptObjects,
        gScrVarPub[SCRIPTINSTANCE_SERVER].numScriptThreads);
    Sentient_ReadGlobals(memFile); // SP 0x007ECE30 -> 0x0047E6E0
    Com_Printf(15, "loadgame: restored sentient globals (40 bytes)\n");
    SP_SaveMemory_MoveToSegment(save, 4, true);
    // SP 0x006441C0 -> Scr_RemoveClassMap 0x0048A1B0, before installing the saved class maps.
    for (unsigned int i = 0; i < CLASS_NUM_COUNT; ++i)
        Scr_RemoveClassMap(SCRIPTINSTANCE_SERVER, i);
    Scr_Load(SCRIPTINSTANCE_SERVER, memFile); // SP 0x0042AEB0
    Com_Printf(15, "loadgame: restored VM (%u saved objects, %u live objects, %u archived threads, time %u)\n",
        gScrVarPub[SCRIPTINSTANCE_SERVER].savecount, gScrVarPub[SCRIPTINSTANCE_SERVER].numScriptObjects,
        gScrVarPub[SCRIPTINSTANCE_SERVER].numScriptThreads, gScrVarPub[SCRIPTINSTANCE_SERVER].time);
    SP_SaveGame_CheckRestoredScript(save);
    SP_SaveMemory_MoveToSegment(save, 3, true); // SP 0x007ECE52
    G_LoadPathNodes(memFile);                  // SP 0x007ECE58
    // KB diagnostic: compare all of segment 3, now paths plus entities, after the real readers.
    static unsigned char pathBuffer[0x200000];
    static MemoryFile pathWriter;
    MemFile_InitForWriting(&pathWriter, sizeof(pathBuffer), pathBuffer, true, true);
    G_SavePathNodes(&pathWriter);
    G_LoadEntitiesForCheck(memFile, &pathWriter);
    G_LoadBadPlacesForCheck(memFile, &pathWriter); // SP 0x007ECED8 -> 0x007EAA40
    // Not ported here: SP 0x007ECEDD rebuilds the free-entity list from the restored entities;
    // that belongs with the live entity install.
    G_LoadClientsForCheck(memFile, &pathWriter); // SP 0x007ECF87 -> 0x007EC530
    G_LoadActorsForCheck(memFile, &pathWriter); // SP 0x007ECFA6 -> 0x007EC760
    G_LoadSentientsForCheck(memFile, &pathWriter); // SP 0x007ECFD0
    G_LoadVehiclesForCheck(memFile, &pathWriter); // SP 0x007ED030 -> 0x007EC930
    G_LoadLevelSystemsForCheck(memFile, &pathWriter); // SP 0x007ED04D..0x007ED278
    MemFile_StartSegment(&pathWriter, -1);
    const unsigned char *pathOriginal = MemFile_GetSegmentAddess(memFile, 3);
    const int pathBytes = MemFile_GetSegmentAddess(memFile, 4) - pathOriginal;
    const bool pathsSame = !pathWriter.memoryOverflow && pathBytes == pathWriter.bytesUsed
        && !memcmp(pathOriginal, pathBuffer, pathBytes);
    Com_Printf(15, "loadgame: path/entity reserialization %s (%i saved bytes, %i restored bytes)\n",
        pathsSame ? "identical" : "DIFFERENT", pathBytes, pathWriter.bytesUsed);
    iassert(pathsSame);
    // TEMPORARY: no sentient reader yet. Release the restored path handles by registry index;
    // setSentient(nullptr) would dereference the absent game-side sentient owner during abort.
    extern EntHandleList g_sentientsHandleList[MAX_SENTIENTS_CAP];
    for (unsigned int i = 0; i < g_path.actualNodeCount; ++i)
    {
        SentientHandle *owner = &gameWorldCurrent->path.nodes[i].dynamic.pOwner;
        if (owner->number)
            RemoveEntHandleInfo(&g_sentientsHandleList[owner->number - 1], owner->infoIndex);
        *owner = {};
    }
    // TEMPORARY diagnostic cleanup: no game entities have been restored yet, so ERR_DROP's
    // G_FreeEntities cannot release the saved entity IDs. Do the script half while the loader
    // still holds every object. Remove this when the entity reader populates the game tables.
    for (unsigned int i = 1; i <= gScrVarPub[SCRIPTINSTANCE_SERVER].savecount; ++i)
    {
        const unsigned int id = gScrVarPub[SCRIPTINSTANCE_SERVER].saveIdMapRev[i];
        const VariableValueInternal *object = &gScrVarGlob[SCRIPTINSTANCE_SERVER].variableList[id + 1];
        if ((object->w.type & VAR_MASK) == VAR_ENTITY)
            Scr_FreeEntityNum(object->u.o.u.entnum, object->w.classnum >> VAR_NAME_BITS, SCRIPTINSTANCE_SERVER);
    }
    // TEMPORARY diagnostic: release the loader's object holds now, so ERR_DROP can free the VM.
    // The complete restore calls this after game-side fields acquire references (SP 0x007ED3C2).
    Scr_LoadPost(SCRIPTINSTANCE_SERVER);
    // Not ported: installing decoded entities into the live tables, then the client,
    // level, actor and sentient payloads (notes/s1-plan.md section 3).
    return false; // TEMPORARY: an incomplete payload must never resume a server frame.
}

static void SP_SaveGame_CheckGlobals(const SPSaveGame *save)
{
    // KB diagnostic (not in SP): after the commit, read segment 2 of the committed save with the
    // reader's read sizes (G_LoadGame_Globals / Dvar_LoadDvars / Sentient_ReadGlobals) and compare
    // the bytes with the same sections written again uncompressed, including the 40-byte tail.
    static MemoryFile reader;
    static MemoryFile writer;
    static unsigned char readBytes[0x40000];
    static unsigned char writeBytes[0x40000];
    MemFile_InitForReading(&reader, save->header.bodySize, save->memFile[0].buffer, true);
    MemFile_MoveToSegment(&reader, 2);
    int used = 0;
    auto read = [&](int size) -> unsigned char * {
        if (used + size > (int)sizeof(readBytes))
            return nullptr;
        MemFile_ReadData(&reader, size, readBytes + used);
        used += size;
        return readBytes + used - size;
    };
    static const int section1[] = { 64, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 4, 64, 8, 8, 8, 4, 64 };
    bool ok = true;
    for (int size : section1)
        ok = ok && read(size);
    int dvars = 0;
    int *length = ok ? (int *)read(4) : nullptr;
    while (ok && length && *length >= 0)
    {
        ok = *length < 0x400 && read(*length) && (length = (int *)read(4)) && *length >= 0 && *length < 0x400
            && read(*length);
        ++dvars;
        length = ok ? (int *)read(4) : nullptr;
    }
    ok = ok && length;
    int hudElems = 0;
    short *index = ok ? (short *)read(2) : nullptr;
    while (ok && index && *index != -1)
    {
        ok = *index >= 0 && *index < 0x400 && read(sizeof(game_hudelem_s));
        ++hudElems;
        index = ok ? (short *)read(2) : nullptr;
    }
    ok = ok && index;
    for (int i = 0; ok && i < 5; ++i)
        ok = read(4) != nullptr;
    ok = ok && read(0x28); // SP 0x004AAEA0 / 0x0047E6E0, SentientGlobals.
    MemFile_InitForWriting(&writer, sizeof(writeBytes), writeBytes, false, false);
    G_SaveState_Globals(&writer);
    Sentient_WriteGlobals(&writer);
    MemFile_StartSegment(&writer, -1);
    const int written = writer.bytesUsed - 4;   // segment 0 starts with its 4-byte length
    const bool same = ok && !writer.memoryOverflow && written == used && !memcmp(readBytes, writeBytes + 4, used);
    Com_Printf(15, "SaveGame check: segment 2 sections 1-3 read back %s (%i bytes read, %i written, %i dvars, %i hud elems)\n",
        same ? "identical" : "DIFFERENT", used, written, dvars, hudElems);
}

static void SP_SaveGame_CheckWeapons(const SPSaveGame *save)
{
    // KB diagnostic, not SP: exercise the real segment-1 readers against the live definitions.
    // All names already exist, so the weapon registration callbacks do not run in this check.
    static MemoryFile reader;
    MemFile_InitForReading(&reader, save->header.bodySize, save->memFile[0].buffer, true);
    MemFile_MoveToSegment(&reader, 1);
    bool same = G_SP_LoadWeapons(&reader);
    int registered = 0;
    int expected = 0;
    for (unsigned int i = 1; i < 2048; ++i)
        expected += IsItemRegistered(i) != 0;
    if (same)
    {
        unsigned char end;
        MemFile_ReadData(&reader, 1, &end);
        while (!end)
        {
            const int item = G_LoadWeaponItem(&reader);
            same = same && item > 0 && item < 2048 && IsItemRegistered(item);
            ++registered;
            MemFile_ReadData(&reader, 1, &end);
        }
    }
    same = same && registered == expected && !reader.memoryOverflow;
    Com_Printf(15, "SaveGame check: segment 1 weapons %s (%u names, %i registered records, %i expected)\n",
        same ? "resolved" : "DIFFERENT", BG_GetNumWeapons() - 1, registered, expected);
}

static void SP_SaveGame_CheckScriptHeader(const SPSaveGame *save)
{
    // KB diagnostic, not the VM restore: verify segment addressing/compression and the two
    // leading fields using a separate reader. Full decoding is tested by the opt-in restore.
    static MemoryFile reader;
    MemFile_InitForReading(&reader, save->header.bodySize, save->memFile[0].buffer, true);
    MemFile_MoveToSegment(&reader, 4);
    unsigned int time;
    unsigned short count;
    MemFile_ReadData(&reader, 4, (unsigned __int8 *)&time);
    MemFile_ReadData(&reader, 2, (unsigned __int8 *)&count);
    const scrVarPub_t *pub = &gScrVarPub[SCRIPTINSTANCE_SERVER];
    const bool same = time == pub->time && count == pub->savecount && !reader.memoryOverflow;
    const int bytes = MemFile_GetSegmentAddess(&reader, 5) - MemFile_GetSegmentAddess(&reader, 4);
    Com_Printf(15, "SaveGame check: segment 4 header %s (%u objects, script time %u, %i compressed bytes)\n",
        same ? "identical" : "DIFFERENT", count, time, bytes);
    iassert(same);
}

static bool SP_SaveGame_BuildFilename(char *filename, int size, const char *name)
{
    // zombies: SP 0x0060B8F0 (the saveType argument is not read).
    char clean[128];
    const int length = (int)strlen(name);
    if (length >= size || length >= 0x80)
    {
        Com_Printf(10, "filename '%s' is too long.\n", name);
        return false;
    }
    for (int i = 0; i < length; ++i)
    {
        const char c = name[i];
        if (c == '/' || c == '\\')
        {
            clean[i] = (i == 8 && !I_strnicmp(name, "autosave", 8)) ? '/' : '-';
            continue;
        }
        // SP 0x006259A0: letters, digits, '_' and '-'.
        if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c <= '9') || c == '_' || c == '-'))
        {
            Com_Printf(10, "filename '%s' has invalid character (%c) in filename.  Must use alphanumeric characters only.\n",
                name, c);
            return false;
        }
        clean[i] = c;
    }
    clean[length] = 0;
    Com_sprintf(filename, size, "save\\%s.svg", clean);
    // SP then calls 0x004DFD60 (a platform check whose result is not used here).
    return true;
}

static void SP_SaveGame_FillHeader(const char *filename, const char *description, const char *screenshot,
    int mapChecksum, bool suppressPrint, int saveType, int saveId, SPSaveGame *save)
{
    // zombies: SP 0x0040C8F0.
    SPMemorySaveHeader *header = &save->header;
    memset(header, 0, sizeof(*header));
    header->saveType = saveType;
    save->suppressPrint = suppressPrint;
    header->saveVersion = 0x134;
    const dvar_s *mapname = _Dvar_RegisterString("mapname", "", 0x44, "");
    header->saveId = saveId;
    I_strncpyz(header->mapName, mapname->current.string, sizeof(header->mapName));
    Com_sprintf(header->buildNumber, sizeof(header->buildNumber), "%d", Com_GetBuildNumber());
    header->mapChecksum = mapChecksum;
    header->saveCheckSum = 0;
    // SP 0x005EF4D0(header + 0x14, 0): script checksum, program length, program buffer size.
    header->scriptChecksum[0] = gScrVarPub[SCRIPTINSTANCE_SERVER].checksum;
    header->scriptChecksum[1] = gScrCompilePub[SCRIPTINSTANCE_SERVER].programLen;
    header->scriptChecksum[2] = gScrVarPub[SCRIPTINSTANCE_SERVER].endScriptBuffer
        - gScrVarPub[SCRIPTINSTANCE_SERVER].programBuffer;
    const dvar_s *campaign = _Dvar_RegisterString("ui_campaign", "american", 0x1000, "");
    I_strncpyz(header->campaign, campaign->current.string, sizeof(header->campaign));
    if (screenshot)
        I_strncpyz(header->screenShotName, screenshot, sizeof(header->screenShotName));
    else
        header->screenShotName[0] = 0;
    I_strncpyz(header->filename, filename, sizeof(header->filename));
    if (description)
        I_strncpyz(header->description, description, sizeof(header->description));
    else
        header->description[0] = 0;
    // Health in percent of the session max health (SP gclient +0x1AC0), 1..100.
    const int maxHealth = g_entities[0].client ? g_entities[0].client->sess.maxHealth : 0;
    int health = 1;
    if (g_entities[0].health && maxHealth)
    {
        health = (int)((float)(g_entities[0].health * 100) / (float)maxHealth);
        if (health < 1)
            health = 1;
        else if (health > 100)
            health = 100;
    }
    header->health = health;
    header->skill = Dvar_GetInt("g_gameskill");
    Com_RealTime(&header->time, true);  // SP 0x0040C120(&time, 1)
    header->bodySize = save->memFile[0].bufferSize; // SP 0x0049C750(save, 0, 0)
    save->unknown104EE = false;
    save->unknown104EF = false;
}

static bool SP_SaveGame_Serialize(const SPSaveGameRequest *request, int mapChecksum, SPSaveGame *save)
{
    // zombies: SP 0x007EE240 (request in edi, save buffer in esi).
    SP_SaveMemory_Reset(save);
    Scr_SaveDeveloperFlags(&save->memFile[0]);  // SP 0x00568F20(0, 0x0056BFC0(save))
    SP_SaveStrings(save);
    G_SaveState(true, save);
    SP_SaveMemory_StartSegment(save, -1, true);
    if (!SP_SaveMemory_IsValid(save))
    {
        SP_SaveMemory_Abort(save);
        return false;
    }
    char filename[128];
    if (!SP_SaveGame_BuildFilename(filename, sizeof(filename), request->filename))
    {
        SP_SaveMemory_Abort(save);
        return false;
    }
    SP_SaveGame_FillHeader(filename, request->description, request->screenshot, mapChecksum,
        request->suppressPrint, request->saveType, request->id, save);
    SP_SaveMemory_Finish(save);
    return true;
}

static void SP_SaveGame_SetCallback(const SPSaveGameRequest *request, SPSaveGame *save)
{
    // zombies: SP 0x0068BE40. KB's requests never carry a callback (the savegame builtin passes SP's
    // no-op), so only the first branch is reached.
    if (!request->callback)
        save->callback = nullptr;
}

static void SP_SaveGame_CommitPending(int flags)
{
    // zombies: SP 0x007EBFA0: a save still pending in the write buffer is committed first (flags 2 / 4)
    // unless the player is dead. The disk half (0x00531780 / 0x005FDA60 / 0x004B5EE0) is not ported:
    // zombiemode saves use flags 2 only.
    SPSaveGame *save = SP_GetSaveBuffer(0);
    const bool dead = g_entities[0].health <= 0;
    if (SP_SaveMemory_IsPending(save) && !dead && (flags & 6))
        SP_SaveMemory_Commit(save);
}

static int SP_SaveGame_Write(const SPSaveGameRequest *request, int mapChecksum)
{
    // zombies: SP 0x004CB930. It returns 0 on every path, so the consumer handles one request per call.
    if (!SP_IsFrontendMapForSave() && g_entities[0].health <= 0)
        return 0;
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c", 53));
    SP_SaveGame_CommitPending(request->flags);
    SPSaveGame *save = SP_GetSaveBuffer(0);
    if (!SP_SaveGame_Serialize(request, mapChecksum, save))
        return 0;
    SP_SaveGame_SetCallback(request, save);
    if (request->flags & 2)
    {
        SP_SaveMemory_Commit(save);
        // KB diagnostic line (not in SP), for the run logs.
        Com_Printf(15, "SaveGame %s committed to memory (%i bytes of game state)\n", save->header.filename,
            save->header.bodySize);
        SP_SaveGame_CheckGlobals(SP_GetSaveBuffer(1));
        SP_SaveGame_CheckWeapons(SP_GetSaveBuffer(1));
        SP_SaveGame_CheckScriptHeader(SP_GetSaveBuffer(1));
    }
    // flags & 4 -> disk write 0x004B5EE0, not ported (zombiemode never sets it).
    return 0;
}

void G_SP_ProcessSaveQueue()
{
    // zombies: SP 0x004BC500. 0x005CEAC0 / 0x005E7A70 service an asynchronous disk write; KB issues
    // none, so their null-handle branches apply (nothing to finish, not busy).
    if (!G_SP_IsSPLevel())
        return;
    // KB diagnostic (not in SP): bo1_resave_ms queues one more level-start save (same name as
    // the script's savegame("levelstart")) once level time reaches it, so the restore diagnostic can
    // decode a payload with live actors. 0 (default) = off.
    static bool resaveDone;
    const int resaveMs = Dvar_GetInt("bo1_resave_ms");
    if (resaveMs <= 0 || level.time < resaveMs)
        resaveDone = false; // re-armed by each new level
    else if (!resaveDone && g_entities[0].client && g_entities[0].health > 0)
    {
        resaveDone = true;
        int actors = 0;
        for (int i = 0; i < MAX_ACTORS; ++i)
            actors += level.actors[i].inuse != 0;
        Com_Printf(15, "savegame: bo1_resave_ms %i: queueing a level-start save at level.time %i (%i actors in use)\n",
            resaveMs, level.time, actors);
        G_SP_SaveGame(Dvar_GetString("mapname"), "", "$default", 0, 2, false, 0, nullptr, 0);
    }
    if (!s_saveQueueCount)
        return;
    if (Dvar_GetInt("g_reloading"))
    {
        Com_Printf(15, "savegame request ignored\n");
        return;
    }
    int result;
    // SP 0x005FD8B0(0x005411A0(0)): the write buffer is free or committed.
    if (!SP_SaveMemory_IsAvailable(SP_GetSaveBuffer(0)))
        return;     // the queue is not empty here, so SP's end-of-frame reset (0x004BC5D5) does nothing
    do
    {
        // SP calls the pre-save hook at 0x02899990 here when one is installed; KB installs none.
        result = SP_SaveGame_Write(&s_saveQueue[0], g_spSaveMapChecksum);  // checksum: SP 0x0050D030
        // SP 0x0087DFF0 then handles the internal vid_restart / snd_restart saves, which only the
        // SP client's restart commands request.
        for (int i = 1; i < s_saveQueueCount; ++i)
            s_saveQueue[i - 1] = s_saveQueue[i];
        --s_saveQueueCount;
    } while (result && s_saveQueueCount);
    if (!s_saveQueueCount)
        s_autoSaveQueuedThisFrame = false;
}
