#pragma once

// j1: play-session recorder (TOOL, not game behaviour; off by default). `+set bo1_playtrace 1` writes one
// JSON line per bo1_playtrace_ms (default 1000) of level time to <fs_homepath>\playtrace\playtrace-<date>.jsonl:
// player state, the zombies near each player (animscript, AI state, enemy, distance, time since their last
// melee), plus event lines (player hurt, zombie melee() calls, client snapshot / ping / prediction samples)
// and "mark" lines from the F8 key (only while F8 is unbound) or the `playtrace_mark` command.
// See notes/playtrace.md. When off every hook is one pointer test and a bool read.

struct gentity_s;
struct actor_s;
struct sentient_s;

void G_SP_PlayTraceRegister(); // dvars + command; idempotent (G_InitGame and CL_InitOnceForAllClients)
bool G_SP_PlayTraceOn();
void G_SP_PlayTraceLine(const char *line); // appends line + '\n' (any thread; opens the file on first use)
void G_SP_PlayTraceServerFrame();          // end of G_RunFrame (server thread)
void G_SP_PlayTraceMelee(const actor_s *self, const gentity_s *hitEnt); // ActorCmd_Melee
void G_SP_PlayTraceMark(const char *source);                            // main thread (key / command)
int G_SP_PlayTraceIntervalMs();
// j1: per-actor team-move notes for the zombie samples: last Actor_GetTeamMoveStatus result ("tm") and the
// last path clear in exposed combat ("pc" reason: FUN_007d30d0 1 closeEnt, 2 pileUp, 3 in goal;
// 4x Actor_HandleInvalidPath (SP 0x007bd830) in Actor_UpdateGoalPath, x = branch 1 lookahead past the enemy, 2 in
// goal, 3 enemy goal (14x: path found this frame, hadPath 0); team move 5 dodge-count clear, 6 enemy under 37.5 u,
// 7 MoveAlongPathWithTeam; "pca" ms since; "pl"/"pla" the same for the last clear of a path the actor already had). Samples also carry cl/pu (pCloseEnt /
// pPileUpEnt entnum or -1), mm moveMode, ss eSubState, tw iTeamMoveWaitTime - level.time, hit Physics.iHitEntnum.
void G_SP_PlayTraceTeamMove(const actor_s *self, int result);
void G_SP_PlayTracePathClear(const actor_s *self, int reason);
// j1: "hip" event line when Actor_HandleInvalidPath (SP 0x007bd830) rejects a path: branch, lookahead (la dist, lad dir,
// le = lookahead end - enemy, dot), fg vFinalGoal / p0 pts[0] / cg codeGoal / cp vCurrPoint / es self, all minus the enemy.
void G_SP_PlayTraceInvalidPath(const actor_s *self, const sentient_s *enemy, int branch, bool hadPath);
// L9: the animscript an actor runs ("move", "death", "specific", ...; "none" when none).
const char *G_SP_PlayTraceScriptName(const actor_s *actor);
// L9: one JSON object (no braces) describing a DEAD actor for death-state evidence: animscript, AI state, ms since
// the recorder first saw it dead, trType of lerp.pos (14/15 ragdoll), the weighted leaf anims (name:time:weight, at
// most 4), origin. `deadSince` is the level time it was first seen dead (caller-owned bookkeeping).
void G_SP_PlayTraceDeadActor(const actor_s *actor, int deadSince, char *buf, int size);
