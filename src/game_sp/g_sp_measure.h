#pragma once

#include <clientscript/cscr_variable.h>

struct gentity_s;

void G_SP_MeasureInit();
void G_SP_MeasureFields(scriptInstance_t inst);
void G_SP_MeasureFrame();
bool G_SP_MeasureEnabled();
// L12 (TEST): bo1_measure_animents - animated non-actor entities, server tree / client DObj / client notetracks.
bool G_SP_MeasureAnimEntsOn();
struct XAnimTree_s;
void G_SP_FormatWeightedAnims(const XAnimTree_s *tree, char *out, int size);
// L15 (TEST): an anim's share of the final pose (weight over its siblings' sum, multiplied up to the root); -1 bad index.
float G_SP_MeasureEffectiveWeight(const XAnimTree_s *tree, unsigned int anim);
void G_SP_MeasureActorTrace(const struct actor_s *actor); // a1 c15 TEST: bo1_measure_actortrace_from / _to
bool G_SP_MeasureActorTraceOn(); // a1 c15 TEST: level.time inside that window
void G_SP_MeasureServerFrame(unsigned int totalMs, unsigned int gameMs, unsigned int beforeGameMs);
void G_SP_MeasureActor(const char *event, const gentity_s *ent, const gentity_s *spawner);
void G_SP_MeasureNotify(scriptInstance_t inst, unsigned int owner, unsigned int name, const VariableValue *top);
// j1: one "animscripted" line per G_ScriptedAnim_Begin (anim asset, notetracks, requested placement).
struct XAnim_s;
void G_SP_MeasureAnimScripted(const gentity_s *ent, const XAnim_s *anims, unsigned int anim, unsigned int notifyName,
    const float *origin, const float *angles, float rate, float blendTime);

// k1: headless long-game measurement only. Slow-frame phase profile of G_RunFrame (emitted with
// server_frame when a frame takes >= 50 ms) and a periodic "resources" record (process memory, script
// variables/threads, entity/actor/sentient pool use, client FX pool use).
enum SPProfilePhase
{
    SP_PROFILE_CLIENTS,     // per-client touch triggers / weapons / timed damage / IK
    SP_PROFILE_PRE,         // flame age, IK time, DObj server time
    SP_PROFILE_TRIGGERS,    // pending trigger notifies (+ the script threads they wake)
    SP_PROFILE_NOTIFIES,    // G_ClientDoPerFrameNotifies
    SP_PROFILE_XANIM,       // G_XAnimUpdateEnt
    SP_PROFILE_SCRIPT_TIME, // Scr_IncTime: time-waiting script threads
    SP_PROFILE_ENTITIES,    // G_RunFrameForEntity (actor think etc.)
    SP_PROFILE_CLIENT_END,  // objectives, hud elems, ClientEndFrame
    SP_PROFILE_TAIL,        // spawn system, corpses, Path_Update, glass, measurement
    SP_PROFILE_COUNT
};
void G_SP_ProfileBegin();
void G_SP_ProfileMark(int phase);
long long G_SP_ProfileNow();
void G_SP_ProfileEntity(const gentity_s *ent, long long start);
// A* search timing (actor_navigation.cpp): one guard per search function.
struct SPPathProfile
{
    long long start;
    SPPathProfile();
    ~SPPathProfile();
};

// k1: headless -Client frame cost (measurement only): main-thread CL_Frame and CG_DrawActiveFrame
// durations, summarised as a BO1_CLIENTPERF line every 5 s of real time.
long long G_SP_MeasureTicks();
long long G_SP_MeasureClientCgBegin();
void G_SP_MeasureClientCg(long long start);
void G_SP_MeasureClientFrame(long long start);
// p1: '+set bo1_frameperf <ms>' (headless): whole Com_Frame timing. BO1_FRAMEPERF every 5 s (frame interval
// p50/p99/max), BO1_FRAMEHITCH for each interval >= <ms> with the phase split and the PROF_SCOPED zones of that frame.
enum { FRAMEPERF_BEGIN, FRAMEPERF_SV, FRAMEPERF_PRE, FRAMEPERF_CL, FRAMEPERF_SCR, FRAMEPERF_END, FRAMEPERF_MARKS };
void G_SP_FramePerfMark(int mark);
void G_SP_FramePerfServer(long long ticks); // server thread: one SV_PreFrame..SV_PostFrame, BO1_SVHITCH when >= <ms>
// L19: '+set bo1_frameperf <ms>' (headless): BO1_LOADMARK line with the ms since the previous mark, to split the
// end-of-load frame (CL_InitCGame and what follows it). Measurement only.
void G_SP_LoadMark(const char *label);
