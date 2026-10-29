#pragma once

// Runtime models: XModels built at level load from a model file (rtmodels/<name>.rlm) instead of a fastfile,
// for mods whose models are converted on the player's PC from another game the player owns (see
// r_runtime_xmodel.cpp). The model names come from the bo1_rtmodels dvar.
void __cdecl R_RuntimeXModel_RegisterDvars();
void __cdecl R_RuntimeXModel_LoadForLevel(const char *mapName);
bool __cdecl R_RuntimeXModel_IsAnimated(const struct XModel *model);
// the laser sight's beam and dot, red or blue (the wonder weapons': cg_view_mp.cpp CG_DrawLaser)
const struct XModel *__cdecl R_RuntimeXModel_Laser(bool dot, bool blue);
const struct XModel *__cdecl R_RuntimeXModel_LaserPiece(int piece, bool blue); // the beam's 8 / 4 / 2 / 1 unit pieces (0..3)
// mod: the kick's halo on a staggered zombie (cg_view_mp.cpp CG_LeonKickHalos), or 0
const struct XModel *__cdecl R_RuntimeXModel_Halo();
// mod: RE4's HUD's image `name` as a 2D material (rtmodels/re4hud.rlh: cgame_mp/cg_re4hud_mp.cpp), or 0
struct Material *__cdecl R_RuntimeXModel_HudMaterial(const char *name);
// an upper-body action over the locomotion / aim (the local player's weapon): a shot, timed from it, or a reload,
// stretched over its duration
enum RtAction
{
    RT_ACTION_NONE,
    RT_ACTION_FIRE,
    RT_ACTION_RELOAD,
    RT_ACTION_TURN, // RE4's quick turn ("turn180"), on the whole body
    RT_ACTION_KNIFE, // RE4's knife slash ("knife", up / down by the pitch)
    RT_ACTION_KICK, // RE4's kick ("kick"), on the whole body
    RT_ACTION_THROW, // RE4's grenade throw ("throw")
    RT_ACTION_THROW_READY, // RE4's grenade ready pose ("throw_ready", looped), before the throw
    RT_ACTION_HIT_FRONT, // RE4's hit reactions ("hit_front" / "hit_back"), on the whole body
    RT_ACTION_HIT_BACK,
    RT_ACTION_DEATH, // RE4's death ("death"), its last frame held, on the whole body
    // a round-by-round reload (BO1's bSegmentedReload) in its stages, each over its own time: the open
    // ("reload_start"), one round ("reload_loop", again for each) and the close ("reload_end")
    RT_ACTION_RELOAD_START,
    RT_ACTION_RELOAD_LOOP,
    RT_ACTION_RELOAD_END,
    RT_ACTION_MOTION, // a motion named for the pose (R_RuntimeXModel_SetActionMotion) played once, on the whole body
    // the bolt worked after a shot (BO1's rechamber) over its duration: the bolt cycle of the set's reload (the bolt
    // rifle's: RtBoltFrames)
    RT_ACTION_BOLT,
};
// a baked motion's root yaw (degrees) at time seconds into it (held at its end), and its length (seconds); false when
// no runtime model has it (the converter keeps the root yaw of RE4's turns: ROOT_YAW_MOTIONS)
bool __cdecl R_RuntimeXModel_RootYaw(const char *name, float time, float *yaw, float *duration);
// whether a motion of that exact name ("<set>:<name>" for a set's own) was loaded
bool __cdecl R_RuntimeXModel_HasAnim(const char *name);
// the head's look for the next R_RuntimeXModel_PoseDObj (a player's): degrees about up, + to his left
void __cdecl R_RuntimeXModel_SetLookYaw(float degrees);
void __cdecl R_RuntimeXModel_SetFireWeight(float weight); // mod: how strongly a shot plays RE4's fire motion (0..1)
void __cdecl R_RuntimeXModel_SetFireRates(float kick, float settle); // mod: a shot's motion's rates: its kick, its settle
void __cdecl R_RuntimeXModel_SetReloadSet(const char *set); // mod: the next pose's reload from this set (0: the base)
void __cdecl R_RuntimeXModel_SetKickVariant(int variant); // mod: the next pose's kick: 1 the character's second roundhouse
void __cdecl R_RuntimeXModel_SetFirePrev(float seconds); // mod: the next pose's previous shot, seconds ago (-1: none)
void __cdecl R_RuntimeXModel_SetActionMotion(const char *name); // mod: the next pose's RT_ACTION_MOTION (an enemy's)
// mod: the local player's locomotion for the next R_RuntimeXModel_PoseDObj: legs turned toward an aimed strafe, RE4's
// turn in place when he turns standing; and the last such pose's turn in place (1 left, -1 right, 0 none)
void __cdecl R_RuntimeXModel_SetPlayerMotion(bool strafeLegs, bool turnInPlace, int moveIntent);
// mod: a player's run under RE4's controls for the next R_RuntimeXModel_PoseDObj (bgame/bg_leon.h LEON_EF2_RUNNING): 1 his
// cycle the run's, 0 the walk's; not told (-1), by his speed
void __cdecl R_RuntimeXModel_SetRunning(int running);
// mod: where in RE4's reload the next R_RuntimeXModel_PoseDObj's reload action starts (0..1; a new gun's first raise)
void __cdecl R_RuntimeXModel_SetReloadFrom(float from);
int __cdecl R_RuntimeXModel_PlayerTurn();
// mod: whether the last such pose still played his quick turn or kick (the laser waits for it: CG_ShoulderLaser)
bool __cdecl R_RuntimeXModel_PlayerBodyAction();
// mod: whether the last such pose was easing from another (RtPoseFade: the aim correction holds meanwhile)
bool __cdecl R_RuntimeXModel_PlayerEasing();
// mod: the local player's footsteps (his cycle's foot plants) since the last call, and whether they were a run's
int __cdecl R_RuntimeXModel_TakePlayerSteps(bool *run);
// mod: a dvar set non-zero (bool, int, or a string set from the command line or a script); false when it does not exist
bool __cdecl R_RuntimeXModel_DvarOn(const char *name);
// returns the shot's (recoil's) weight on the pose this frame, 0..1 (0: no shot playing; the bolt worked after it too)
float __cdecl R_RuntimeXModel_PoseDObj(struct DObj *obj, const float *origin, const float *angles, int timeMs,
                                      bool isPlayer, float aimFrac, float aimPitch, RtAction action = RT_ACTION_NONE,
                                      float actionTime = 0.0f, float actionDuration = 0.0f,
                                      const float *aimCorrection = 0, bool turnToMotion = true,
                                      const char *animSet = 0);
