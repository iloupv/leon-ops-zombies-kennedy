#pragma once

// Runtime models: XModels built at level load from a model file (rtmodels/<name>.rlm) instead of a fastfile,
// for mods whose models are converted on the player's PC from another game the player owns (see
// r_runtime_xmodel.cpp). The model names come from the bo1_rtmodels dvar.
void __cdecl R_RuntimeXModel_RegisterDvars();
void __cdecl R_RuntimeXModel_LoadForLevel(const char *mapName);
bool __cdecl R_RuntimeXModel_IsAnimated(const struct XModel *model);
const struct XModel *__cdecl R_RuntimeXModel_Laser(bool dot);
// an upper-body action over the locomotion / aim (the local player's weapon): a shot, timed from it, or a reload,
// stretched over its duration
enum RtAction
{
    RT_ACTION_NONE,
    RT_ACTION_FIRE,
    RT_ACTION_RELOAD,
    RT_ACTION_TURN, // RE4's quick turn ("turn180"), on the whole body
};
// a baked motion's root yaw (degrees) at time seconds into it (held at its end), and its length (seconds); false when
// no runtime model has it (the converter keeps the root yaw of RE4's turns: ROOT_YAW_MOTIONS)
bool __cdecl R_RuntimeXModel_RootYaw(const char *name, float time, float *yaw, float *duration);
// returns the shot's (recoil's) weight on the pose this frame, 0..1 (0: no shot playing)
float __cdecl R_RuntimeXModel_PoseDObj(struct DObj *obj, const float *origin, const float *angles, int timeMs,
                                      bool isPlayer, float aimFrac, float aimPitch, RtAction action = RT_ACTION_NONE,
                                      float actionTime = 0.0f, float actionDuration = 0.0f,
                                      const float *aimCorrection = 0, bool turnToMotion = true);
