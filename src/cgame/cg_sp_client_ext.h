#pragma once
// zombies: SP cg_s state that BO1Zombies's MP cg_s has no room for, kept per local client beside
// cg_s (the decompiled code hard-codes cg_s's layout, so it is never resized). Cleared where SP's
// CG_Init clears cg_s, i.e. before CG_InitVisionSets (the hook in cg_visionsets.cpp).
#include <cgame/cg_visionsets.h>

// Opt-in client visual evidence; no gameplay changes.
bool CG_SP_VisualsEnabled();
void CG_SP_VisualShot(int localClientNum, const char *name);
int CG_SP_VisualsFrameMs();
void CG_SP_VisionDumpFrame(int localClientNum);
void CG_SP_DumpVisionSets(int localClientNum, int requestedTime); // cg_visionsets.cpp

// SP runs ten vision channels (CG_SelectVisionSet 0x0045ea30); 0..5 are KB's visionSetMode_t,
// these four exist only in SP. Their default names come from SP CG_InitVisionSets 0x0047c180.
enum SpVisionSetSlot
{
    SP_VISIONSET_LASTSTAND  = 6, // "laststand", cg+0xcc720
    SP_VISIONSET_DEATH      = 7, // "death", cg+0xcc760 (the damage channel's timer, cg+0xccd34)
    SP_VISIONSET_LOWHEALTH  = 8, // "low_health", cg+0xcc7a0
    SP_VISIONSET_UNDERWATER = 9, // "creek_1_water", cg+0xcc7e0
    SP_VISIONSET_FIRST      = 6,
    SP_VISIONSET_COUNT      = 10,
};

struct CgSPClientExt
{
    // SP keeps each channel as (from 0xc7668, to 0xc90a8, current 0xcaae8) + slot * 0x2a0 and its
    // lerp record at 0xcc528 + slot * 0xc; index here is slot - SP_VISIONSET_FIRST.
    visionSetVars_t visionSetFrom[SP_VISIONSET_COUNT - SP_VISIONSET_FIRST];
    visionSetVars_t visionSetTo[SP_VISIONSET_COUNT - SP_VISIONSET_FIRST];
    visionSetVars_t visionSetCurrent[SP_VISIONSET_COUNT - SP_VISIONSET_FIRST];
    visionSetLerpData_t visionSetLerpData[SP_VISIONSET_COUNT - SP_VISIONSET_FIRST];
    char visionName[SP_VISIONSET_COUNT - SP_VISIONSET_FIRST][64]; // cg+0xcc5a0 + slot * 64
    bool underwater;             // cg+0xccd30 (written by SP CG_CalcViewValues 0x00793fb0)
    bool vsVision;               // cg+0xccd31 (only set for the "vs" game type; not ported)
    int damageVisionEndTime;     // cg+0xccd34
    int useAlternateAimParams;   // cg+0xce534 (usealternateaimparams / clearalternateaimparams)
    int hideViewModel;           // cg+0xa9d44 (server commands '}' / '{', read by CG_AddViewWeapon)
    int timeScaleLerpStart;      // cg+0xa9b58 (server command 'X', SP 0x005da340; read by SP 0x00772540)
    int timeScaleLerpEnd;        // cg+0xa9b5c
    float timeScaleLerpFrom;     // cg+0xa9b60
    float timeScaleLerpTo;       // cg+0xa9b64
    int deadQuoteTime;           // cg+0xa9b78 (server command 'Y', not ported; 0 until it is)
    float lastStandSway[3];      // cg+0xa4720: the downed view sway offset added to the view angles
    float lastStandSwayVel[3];   // cg+0xa472c
    float lastStandSwayGoal[3];  // cg+0xa4738 (SP 0x00791d20 picks a new goal when the sway stops closing in)
};

struct CgSPVisionSwap
{
    bool active;
    visionSetMode_t channel;
    visionSetLerpStyle_t style;
    visionSetVars_t saved;
};

CgSPClientExt *CG_SP_GetClientExt(int localClientNum);
void CG_SP_InitVisionSets(int localClientNum);
bool CG_SP_VisionSetStartLerp(int localClientNum, int mode, visionSetLerpStyle_t style, const char *name, int duration);
void CG_SP_VisionSetsUpdate(int localClientNum);
bool CG_SP_VisionSetApplyBegin(int localClientNum, visionSetMode_t channel, CgSPVisionSwap *swap);
void CG_SP_VisionSetApplyEnd(int localClientNum, CgSPVisionSwap *swap);

// zombies: SP server commands KB's CG_DeployServerCommand did not handle (cg_sp_servercmds.cpp).
// Every one is called from a zombiemode-gated case.
void CG_SP_VisionSetLastStandCommand(int localClientNum);
void CG_SP_HideViewModelCommand(int localClientNum);
void CG_SP_ShowViewModelCommand(int localClientNum);
void CG_SP_DoubleVisionCommand(int localClientNum);
void CG_SP_UploadScoreCommand(int localClientNum);
void CG_SP_CleanupSpawnedDynEntsCommand(int localClientNum);
void CG_SP_Start3DCinematicCommand(int localClientNum);
void CG_SP_Stop3DCinematicCommand(int localClientNum);
void CG_SP_Pause3DCinematicCommand(int localClientNum);
void CG_SP_SoundFadeCommand(int localClientNum);
void CG_SP_TimeScaleLerpCommand(int localClientNum);
void CG_SP_ClientExploderCommand(int localClientNum);
void CG_SP_TransportedCommand(int localClientNum);
void CG_SP_RopeCommand(int localClientNum);
void CG_SP_WaterSheetingCommand(int localClientNum);
void CG_SP_OpenMainMenuCommand(int localClientNum);
void CG_SP_CloseMainMenuCommand(int localClientNum);
void CG_SP_UpdateTimeScaleLerp(int localClientNum);
void CG_SP_SetDoubleVision(int localClientNum, int duration, float value);
void CG_SP_UpdateDoubleVision(int localClientNum);

// zombies: entity events whose SP client end is a client-script callback (cg_sp_servercmds.cpp).
struct centity_s;
extern int cg_spScrPlayWeaponDeathEffects;
extern int cg_spScrGibEvent;
void CG_SP_PlayWeaponDeathEffects(int localClientNum, int entityNum, unsigned int weapon, unsigned int parm);
void CG_SP_GibEntity(int localClientNum, centity_s *cent, unsigned int parm);
