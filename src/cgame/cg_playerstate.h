#pragma once
#include <bgame/bg_local.h>

struct transPlayerState_t // sizeof=0x30
{                                       // XREF: cg_s/r
    int otherFlags;
    int damageEvent;
    __int16 predictableEventSequence;
    // padding byte
    // padding byte
    int predictableEvents[4];
    __int16 unpredictableEventSequence;
    // padding byte
    // padding byte
    int unpredictableEvents[4];
};

void __cdecl CG_Respawn(int localClientNum, int spectate);
void __cdecl CG_ResetSpectatorViewAngles(int localClientNum);
void __cdecl CG_TransitionPlayerState(int localClientNum, playerState_s *ps, const transPlayerState_t *ops);
void __cdecl CG_DamageFeedback(int localClientNum, int yawByte, int pitchByte, int damage);
void __cdecl CG_CheckPlayerstateEvents(int localClientNum, playerState_s *ps, const transPlayerState_t *ops);
void __cdecl CG_ClearTransPlayerState(transPlayerState_t *transPs);
void __cdecl CG_ExtractTransPlayerState(const playerState_s *ps, transPlayerState_t *transPs);
