#pragma once

struct playerState_s;
struct weaponState_t;
struct WeaponDef;

struct viewState_t // sizeof=0x24
{                                       // XREF: ?ClientThink_real@@YAXPAUgentity_s@@PAUusercmd_s@@@Z/r
                                        // CG_OffsetFirstPersonView/r
    playerState_s *ps;                  // XREF: CG_OffsetFirstPersonView+73/w
    // CG_OffsetFirstPersonView+85/r ...
    int damageTime;                     // XREF: CG_OffsetFirstPersonView+A3/w
    // ClientThink_real(gentity_s *,usercmd_s *)+856/w
    int time;                           // XREF: CG_OffsetFirstPersonView+B8/w
    // ClientThink_real(gentity_s *,usercmd_s *)+85E/w ...
    float v_dmg_pitch;                  // XREF: CG_OffsetFirstPersonView+C6/w
    // ClientThink_real(gentity_s *,usercmd_s *)+86F/w
    float v_dmg_roll;                   // XREF: CG_OffsetFirstPersonView+D6/w
    // ClientThink_real(gentity_s *,usercmd_s *)+882/w
    float xyspeed;                      // XREF: CG_OffsetFirstPersonView+E6/w
    // ClientThink_real(gentity_s *,usercmd_s *)+903/w
    float frametime;                    // XREF: CG_OffsetFirstPersonView+FE/w
    // ClientThink_real(gentity_s *,usercmd_s *)+918/w
    float fLastIdleFactor;              // XREF: CG_OffsetFirstPersonView+10B/w
    // ClientThink_real(gentity_s *,usercmd_s *)+92B/w
    int *weapIdleTime;                  // XREF: CG_OffsetFirstPersonView+119/w
    // ClientThink_real(gentity_s *,usercmd_s *)+93C/w
};

double __cdecl BG_GetBobCycle(const playerState_s *ps);
double __cdecl BG_CalculateWeaponMovement_VerticalBobFactor(const playerState_s *ps, float cycle, float speed);
double __cdecl BG_CalculateWeaponMovement_HorizontalBobFactor(const playerState_s *ps, float cycle, float speed);
void __cdecl BG_CalculateWeaponMovement(weaponState_t *ws, float *origin, float *angles);
void __cdecl BG_CalculateWeaponMovement_Base(weaponState_t *ws);
void __cdecl BG_CalculateWeaponMovement_Base_Internal_WeaponState(
                const weaponState_t *ws,
                float *targetPos,
                float *targetAngles);
double __cdecl BG_CalculateWeaponMovement_GetSprintLerp(const weaponState_t *ws, const WeaponDef *weapDef);
double __cdecl BG_CalculateWeaponMovement_GetLowReadyLerp(const weaponState_t *ws, const WeaponDef *weapDef);
double __cdecl BG_CalculateWeaponMovement_GetDTPLerp(const weaponState_t *ws, const WeaponDef *weapDef);
double __cdecl BG_CalculateWeaponMovement_GetSlideLerp(const weaponState_t *ws, const WeaponDef *weapDef);
void __cdecl BG_CalculateWeaponMovement_Base_Internal_Stance(
                const weaponState_t *ws,
                float *targetPos,
                float *targetAngles);
void __cdecl BG_CalculateWeaponMovement_Base_Internal_Movement(
                const weaponState_t *ws,
                float minSpeed,
                float *targetPos,
                float *targetAngles);
void __cdecl BG_CalculateWeaponMovement_Base_Internal_Save(
                weaponState_t *ws,
                const float *targetPos,
                const float *targetAngles);
void __cdecl BG_CalculateWeaponMovement_Recoil(weaponState_t *ws);
int __cdecl BG_CalculateWeaponMovement_GunRecoil_SingleAngle(
                float *fOffset,
                float *speed,
                float fTimeStep,
                float fOfsCap,
                float fGunKickAccel,
                float fGunKickSpeedMax,
                float fGunKickSpeedDecay,
                float fGunKickStaticDecay);
void __cdecl BG_CalculateWeaponMovement_Sway(weaponState_t *ws);
void __cdecl BG_CalculateWeaponMovement_Bob(weaponState_t *ws);
void __cdecl BG_SetWeaponMovementAngles(weaponState_t *ws, float *angles);
void __cdecl BG_ComputeAndApplyWeaponMovement_IdleAngles(weaponState_t *ws, float *angles);
void __cdecl BG_ComputeAndApplyWeaponMovement_DamageKickAngles(weaponState_t *ws, float *angles);
void __cdecl BG_ApplyWeaponMovement_BaseAngles(weaponState_t *ws, float *angles);
void __cdecl BG_ApplyWeaponMovement_SwayAngles(weaponState_t *ws, float *angles);
void __cdecl BG_ApplyWeaponMovement_BobAngles(weaponState_t *ws, float *angles);
void __cdecl BG_SetWeaponMovementOrigin(weaponState_t *ws, float *origin);
void __cdecl BG_ApplyWeaponMovement_BaseOrigin(weaponState_t *ws, float *origin);
void __cdecl BG_ApplyWeaponMovement_LeanOrigin(weaponState_t *ws, float *origin);
void __cdecl BG_ApplyWeaponMovement_BobOrigin(weaponState_t *ws, float *origin);
double __cdecl BG_CalculateViewMovement_VerticalBobFactor(const playerState_s *ps, float cycle, float speed);
double __cdecl BG_CalculateViewMovement_HorizontalBobFactor(const playerState_s *ps, float cycle, float speed);
void __cdecl BG_CalculateViewMovementAngles(viewState_t *vs, float *angles);
void __cdecl BG_CalculateViewMovement_DamageKick(viewState_t *vs, float *angles);
void __cdecl BG_CalculateViewMovement_IdleAngles(viewState_t *vs, float *angles);
void __cdecl BG_CalculateViewMovement_BobAngles(viewState_t *vs, float *angles);
void __cdecl BG_CalculateViewMovement_AdsBob(viewState_t *vs, float *angles);
