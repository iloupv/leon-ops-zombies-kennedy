#pragma once
#include <game/teams.h>

struct centity_s;
struct Material;
struct entityState_s;
struct clientInfo_t;
struct ShaderConstantSet;
struct cg_s;
struct XModel;

void __cdecl CG_AddAllPlayerSpriteDrawSurfs(int localClientNum);
void __cdecl CG_AddPlayerSpriteDrawSurfs(int localClientNum, const centity_s *cent);
void    CG_AddPlayerSpriteDrawSurf(
                int localClientNum,
                const centity_s *cent,
                Material *material,
                float additionalRadiusSize,
                int height,
                bool fixedScreenSize);
char __cdecl CG_Player_ApplyVehicleAnimOffsets(
                int localClientNum,
                const entityState_s *es,
                centity_s *cent,
                clientInfo_t *ci);
void __cdecl CG_HighlightPlayer(int localClientNum, centity_s *cent, ShaderConstantSet *constantSet, bool isExtraCam);
void __cdecl CG_Player(int localClientNum, centity_s *cent);
void __cdecl CG_PlayerTurretPositionAndBlend(int localClientNum, centity_s *cent);
void __cdecl CG_PlayerUpdateUserRigidBody(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePerkSounds(int localClientNum, const cg_s *cgameGlob);
void __cdecl CG_Corpse(int localClientNum, centity_s *cent);
void __cdecl CG_UpdatePlayerDObj(int localClientNum, centity_s *cent);
void __cdecl CG_ResetPlayerEntity(int localClientNum, cg_s *cgameGlob, centity_s *cent, int resetAnimation);
const char *__cdecl CG_GetTeamName(team_t team);
const char *__cdecl CG_GetFFATeamName(ffa_team_t team);
const char *__cdecl CG_GetOpposingTeamName(team_t team);
const char *__cdecl CG_GetPlayerTeamName(int localClientNum);
const char *__cdecl CG_GetPlayerOpposingTeamName(int localClientNum);
bool __cdecl CG_IsPlayerDead(int localClientNum);
int __cdecl CG_GetPlayerClipAmmoCount(int localClientNum);
void __cdecl CG_UpdateWeaponVisibilityImmediate(int localClientNum, centity_s *cent);
void __cdecl CG_UpdateWeaponVisibilityInternal(int localClientNum, centity_s *cent);
bool __cdecl CG_IsWeaponVisible(int localClientNum, centity_s *cent, XModel *weapModel, float *origin, float *forward);
void __cdecl CG_CalcWeaponVisTrace(
                XModel *weapModel,
                float *origin,
                float *forward,
                float *start,
                float *end,
                float *modelLen);
bool __cdecl CG_PlayerInViewFrustum(int localClientNum, const centity_s *cent);
