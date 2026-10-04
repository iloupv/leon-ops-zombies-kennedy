#pragma once
#include "cg_local_mp.h"
#include <qcommon/msg_mp.h>

struct pmove_t // sizeof=0x910
{                                                                             // XREF: .data:pmove_t * g_pmove/r
    struct playerState_s *ps;
    usercmd_s cmd;                                            // XREF: PlayerCmd_Revive(scr_entref_t)+149/o
    // PlayerCmd_Revive(scr_entref_t)+18D/o
    usercmd_s oldcmd;
    int tracemask;
    int numtouch;
    int touchents[32];
    int numGlassTouch;
    int touchGlasses[32];
    float mins[3];
    float maxs[3];
    float xyspeed;
    float currentPitch;
    float averagePitch;
    int nextPitch;
    float pitchHistory[30];
    int proneChange;
    float maxSprintTimeMultiplier;
    bool mantleStarted;
    // padding byte
    // padding byte
    // padding byte
    float mantleEndPos[3];
    int mantleDuration;
    int viewChangeTime;
    float viewChange;
    float vehicleAngles[3];
    pmoveVehAnimState_t vehAnimState;
    unsigned __int8 handler;
    // padding byte
    // padding byte
    // padding byte
    int localClientNum;
    const struct gjkcc_input_t *m_gjkcc_input;
    colgeom_visitor_inlined_t<200> proximity_data;
    // zombies: SP keeps AllowStand/AllowCrouch/AllowProne in pm_flags 0x400000/0x800000/0x1000000
    // (SPPlayerPermission); KB's pm_flags bits mean other things, so the mover gets them here (0 = all allowed).
    unsigned int spDisabledStances;

    pmove_t();
    pmove_t(const pmove_t *__that);
};


int __cdecl CG_ItemListLocalClientNum();
void __cdecl CG_ClearItemList();
void __cdecl CG_BuildItemList(int localClientNum, const snapshot_s *nextSnap);
void __cdecl MovieCameraMove(int localClientNum, usercmd_s *cmd);
void __cdecl FollowCameraMove(cg_s *cgameGlob, centity_s *cent, usercmd_s *cmd);
void __cdecl CG_ClearSavedPlayerState(int localClientNum);
void __cdecl CG_SavePlayerState(int localClientNum);
void __cdecl CG_PredictPlayerState(int localClientNum);
void __cdecl CG_PredictPlayerState_Internal(int localClientNum);
void __cdecl CG_TouchItemPrediction(int localClientNum);
void __cdecl CG_TouchItem(cg_s *cgameGlob, centity_s *cent);
void __cdecl CG_InterpolatePlayerStateViewAngles(int localClientNum, playerState_s *out, usercmd_s *cmd);
char __cdecl CG_ShouldInterpolatePlayerStateViewClamp(int localClientNum, const snapshot_s *prevSnap);
void __cdecl CG_InterpolatePlayerState(int localClientNum, int grabAngles);
void __cdecl CG_CopyFlagsFromSnapshotEntity(int localClientNum);

extern pmove_t cg_pmove[1];
