#pragma once

#include <universal/q_shared.h>
#include <qcommon/bitarray.h>

struct playerState_s;
struct centity_s;
struct cg_s;

struct AimTarget // sizeof=0x30
{                                                                             // XREF: AimTargetGlob/r
                                                                                // ?AimTarget_ProcessEntityInternal@@YAXHPBUcentity_s@@@Z/r
        int entIndex;
        float worldDistSqr;
        float heightChange;
        float mins[3];
        float maxs[3];
        float velocity[3];
};

struct AimTargetGlob // sizeof=0xC04
{                                                                             // XREF: .data:atGlobArray/r
        AimTarget targets[64];
        int targetCount;
};

struct __declspec(align(2)) AimAssistPlayerState // sizeof=0x4C
{                                                                             // XREF: AimAssistGlobals/r
        float origin[3];
        float viewangles[3];
        float velocity[3];
        int eFlags;
        int linkFlags;
        int pm_flags;
        int weapFlags;
        int weaponstate;
        float fWeaponPosFrac;
        float viewHeightCurrent;
        int weapIndex;
        int meleeWeapIndex;
        bool hasAmmo;
        bool isDualWielding;
        bool targetAssistDisabled;
        // padding byte
};

struct AimTweakables // sizeof=0x20
{                                                                             // XREF: AimAssistGlobals/r
        float slowdownRegionWidth;
        float slowdownRegionHeight;
        float autoAimRegionWidth;
        float autoAimRegionHeight;
        float autoMeleeRegionWidth;
        float autoMeleeRegionHeight;
        float lockOnRegionWidth;
        float lockOnRegionHeight;
};

struct AimScreenTarget // sizeof=0x34
{                                                                             // XREF: AimAssistGlobals/r
                                                                                // ?AimAssist_UpdateScreenTargets@@YAXHQBM0MM@Z/r
        int entIndex;                                             // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+27C/w
        float clipMins[2];                                    // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+287/w
                                                                                // AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+294/w
        float clipMaxs[2];                                    // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+2AF/w
                                                                                // AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+2C2/w
        float aimPos[3];                                        // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+30E/o
        float velocity[3];                                    // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+2E0/w
                                                                                // AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+2F3/w ...
        float distSqr;                                            // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+33A/w
        float crosshairDistSqr;                         // XREF: AimAssist_UpdateScreenTargets(int,float const * const,float const * const,float,float)+34F/w
};

enum eAutoMeleeState : __int32
{                                                                             // XREF: AimAssistGlobals/r
        AMS_NOT_ACTIVE = 0x0,
        AMS_TARGET_AQUIRED = 0x1,
        AMS_TARGETING = 0x2,
};

struct AimAssistGlobals // sizeof=0xEB0
{                                                                             // XREF: .data:aaGlobArray/r
                                                                                // TopDownState/r
        AimAssistPlayerState ps;
        bool initialized;
        // padding byte
        // padding byte
        // padding byte
        AimTweakables tweakables;
        float viewOrigin[3];
        float viewAngles[3];
        float viewAxis[3][3];
        float fovTurnRateScale;
        float fovScaleInv;
        float adsLerp;
        float pitchDelta;
        float yawDelta;
        float screenWidth;                                    // XREF: AimAssist_UpdateTweakables+A0/r
                                                                                // AimAssist_UpdateTweakables+145/r ...
        float screenHeight;                                 // XREF: AimAssist_UpdateTweakables+F0/r
                                                                                // AimAssist_UpdateTweakables+195/r ...
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        float screenMtx[4][4];
        float invScreenMtx[4][4];
        AimScreenTarget screenTargets[64];
        int screenTargetCount;
        int autoAimTargetEnt;
        bool autoAimPressed;
        bool autoAimActive;
        // padding byte
        // padding byte
        float autoAimPitch;
        float autoAimPitchTarget;
        float autoAimYaw;
        float autoAimYawTarget;
        bool autoAimJustGotTarget;
        bool autoAimHasRealTarget;
        bool aimSlowdownActive;
        // padding byte
        int aimSlowdownTargetEnt;
        int autoMeleeTargetEnt;
        eAutoMeleeState autoMeleeState;
        float autoMeleePitch;
        float autoMeleePitchTarget;
        float autoMeleeYaw;
        float autoMeleeYawTarget;
        int lockOnTargetEnt;
        bitarray<51> prev_button_bits;
        bool overrideSnapWidthAndLerp;
        // padding byte
        // padding byte
        // padding byte
        float overrideAutoaimLerpValue;
        float overrideAutoaimWidthValue;
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
        // padding byte
};

struct AimInput // sizeof=0x30
{                                                                             // XREF: CL_GamepadMove/r
                                                                                // CL_MouseMove/r
        float deltaTime;                                        // XREF: CL_GamepadMove+3A4/w
        float pitch;                                                // XREF: CL_GamepadMove+3B4/w
        float pitchAxis;                                        // XREF: CL_GamepadMove+3BE/w
        float pitchMax;                                         // XREF: CL_GamepadMove+3CE/w
        float yaw;                                                    // XREF: CL_GamepadMove+3DE/w
        float yawAxis;                                            // XREF: CL_GamepadMove+3E8/w
        float yawMax;                                             // XREF: CL_GamepadMove+3F8/w
        float forwardAxis;                                    // XREF: CL_GamepadMove+402/w
        float rightAxis;                                        // XREF: CL_GamepadMove+40C/w
        bitarray<51> button_bits;                     // XREF: CL_GamepadMove:loc_553914/o
        int localClientNum;                                 // XREF: CL_GamepadMove+451/w
};

struct __declspec(align(4)) AimOutput // sizeof=0x10
{                                                                             // XREF: CL_GamepadMove/r
                                                                                // CL_MouseMove/r
        float pitch;                                                // XREF: CL_GamepadMove+467/r
                                                                                // CL_MouseMove+669/r
        float yaw;                                                    // XREF: CL_GamepadMove+477/r
                                                                                // CL_MouseMove+679/r
        float meleeChargeYaw;                             // XREF: CL_GamepadMove+487/r
                                                                                // CL_MouseMove+689/r
        unsigned __int8 meleeChargeDist;        // XREF: CL_GamepadMove+494/r
                                                                                // CL_MouseMove+696/r
        // padding byte
        // padding byte
        // padding byte
};

void __cdecl AimAssist_Init(int localClientNum);
void AimAssist_RegisterDvars();
void AimAssist_UpdateMeleeRangeDefault();
void __cdecl AimAssist_Setup(int localClientNum, const playerState_s *ps);
void __cdecl AimAssist_BackupPlayerState(int localClientNum, const playerState_s *ps);
char __cdecl AimAssist_PlayerDisabledAutoAim();
void __cdecl AimAssist_UpdateScreenTargets(
                int localClientNum,
                const float *viewOrg,
                const float *viewAngles,
                float tanHalfFovX,
                float tanHalfFovY);
void __cdecl AimAssist_FovScale(AimAssistGlobals *aaGlob, float tanHalfFovY);
void    AimAssist_CreateScreenMatrix(
                AimAssistGlobals *aaGlob,
                float tanHalfFovX,
                float tanHalfFovY);
char __cdecl AimAssist_ConvertToClipBounds(
                const AimAssistGlobals *aaGlob,
                const float (*bounds)[3],
                const float (*mtx)[3],
                float (*clipBounds)[3]);
char __cdecl AimAssist_XfmWorldPointToClipSpace(const AimAssistGlobals *aaGlob, float *in, float *out);
double __cdecl AimAssist_GetCrosshairDistSqr(const float *clipMins, const float *clipMaxs);
void __cdecl AimAssist_AddToTargetList(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget);
int __cdecl AimAssist_CompareTargets(const AimScreenTarget *screenTargetA, const AimScreenTarget *screenTargetB);
int __cdecl AimAssist_CalcAimPos(
                int localClientNum,
                const centity_s *targetEnt,
                const AimTarget *target,
                float *aimPos);
int __cdecl AimTarget_GetTagPos(int localClientNum, const centity_s *cent, unsigned int tagName, float *pos);
int __cdecl AimAssist_GetScreenTargetCount(int localClientNum);
int __cdecl AimAssist_GetScreenTargetEntity(int localClientNum, unsigned int targetIndex);
void __cdecl AimAssist_ClearEntityReference(int localClientNum, int entIndex);
bool __cdecl AimAssist_IsPrevTargetEntity(int localClientNum, int entIndex);
void __cdecl AimAssist_UpdateGamePadInput(const AimInput *input, AimOutput *output);
void __cdecl AimAssist_UpdateTweakables(int localClientNum);
void __cdecl AimAssist_UpdateAdsLerp(const AimInput *input);
void __cdecl AimAssist_ApplyTurnRates(const AimInput *input, AimOutput *output);
double __cdecl AimAssist_LerpDvars(const dvar_s *from, const dvar_s *to, float frac);
void __cdecl AimAssist_CalcAdjustedAxis(const AimInput *input, float *pitchAxis, float *yawAxis);
void __cdecl AimAssist_CalcSlowdown(const AimInput *input, float *pitchScale, float *yawScale);
double __cdecl AimAssist_GetAimAssistRange(unsigned int weapIndex, float adsLerp);
const AimScreenTarget *__cdecl AimAssist_GetBestTarget(
                const AimAssistGlobals *aaGlob,
                float range,
                float regionWidth,
                float regionHeight);
bool __cdecl AimAssist_IsPlayerUsingOffhand(const AimAssistPlayerState *ps);
bool __cdecl AimAssist_IsSlowdownActive(const AimAssistPlayerState *ps);
double __cdecl AimAssist_GetProfileSensitivity();
void __cdecl AimAssist_ApplyAutoAim(const AimInput *input, AimOutput *output);
double __cdecl AimAssist_GetAutoAimRange(unsigned int weapIndex);
void __cdecl AimAssist_ClearAutoAimTarget(AimAssistGlobals *aaGlob);
char __cdecl AimAssist_UpdateAutoAimTarget(AimAssistGlobals *aaGlob);
const AimScreenTarget *__cdecl AimAssist_GetTargetFromEntity(const AimAssistGlobals *aaGlob, int entIndex);
void __cdecl AimAssist_SetAutoAimTarget(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget);
void __cdecl AimAssist_ApplyLockOn(const AimInput *input, AimOutput *output);
const AimScreenTarget *__cdecl AimAssist_GetPrevOrBestTarget(
                const AimAssistGlobals *aaGlob,
                float range,
                float regionWidth,
                float regionHeight,
                int prevTargetEnt);
void __cdecl AimAssist_ApplyAutoMelee(const AimInput *input, AimOutput *output);
void __cdecl AimAssist_ClearAutoMeleeTarget(AimAssistGlobals *aaGlob);
char __cdecl AimAssist_UpdateAutoMeleeTarget(AimAssistGlobals *aaGlob, int localClientNum);
void __cdecl AimAssist_SetAutoMeleeTarget(AimAssistGlobals *aaGlob, const AimScreenTarget *screenTarget);
void __cdecl AimAssist_UpdateMouseInput(const AimInput *input, AimOutput *output);
void __cdecl AimAssist_DrawDebugOverlay(int localClientNum);
void __cdecl AimAssist_DrawCenterBox(
                const AimAssistGlobals *aaGlob,
                float clipHalfWidth,
                float clipHalfHeight,
                const float *color);
void __cdecl AimAssist_DrawTargets(int localClientNum, const playerState_s *ps, const float *color);
