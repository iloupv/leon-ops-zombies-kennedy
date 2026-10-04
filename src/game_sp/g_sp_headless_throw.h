#pragma once

// w1 chunk 6 TEST SWITCHES (headless fighting test client only, off by default): offhand throws, claymore
// placement, the alternate grenade launcher, ballistic knife retrieval, freeze-gun shatter and the lure
// tracker. See g_sp_headless_throw.cpp. Harness only: usercmds and the scripts' own give functions.
#include <clientscript/cscr_main.h>

struct gentity_s;
struct client_t;
struct usercmd_s;

void G_SP_HeadlessThrowRegister();
void G_SP_HeadlessThrowReset();
// From G_SP_MeasureFields before Scr_EndLoadScripts: resolve bo1_testclient_scriptgive's functions.
void G_SP_HeadlessThrowFields(scriptInstance_t inst);
// At the fight's first target: run bo1_testclient_scriptgive's functions on the player once.
void G_SP_HeadlessThrowStart(gentity_s *player);
// L27 TEST SWITCH bo1_testclient_scriptat: each server frame; runs the level function once at its level time.
void G_SP_HeadlessScriptAtFrame();
// A bo1_testclient_fightweapons cycle name. "alt:<gun>" gives <gun> and holds its alternate weapon; an
// offhand or item weapon (frag, monkey, claymore) is thrown / placed while the previous gun is kept.
// Returns true for a thrown weapon (nothing to give or take); *give / *hold otherwise.
bool G_SP_HeadlessThrowCycle(const char *name, unsigned int *give, unsigned int *hold);
// The fight-weapons refill leaves an empty ballistic knife empty while its retrieval runs.
bool G_SP_HeadlessThrowKeepEmpty(gentity_s *player, unsigned int weapon);
// Before the target choice: knife retrieval, shatter shots and the lure tracker. True = owns this command.
bool G_SP_HeadlessThrowPreCommand(gentity_s *player, client_t *client, usercmd_s *cmd, unsigned int gun);
// With a target (the view already aims at it): throw / place. True = owns the fire buttons this command.
bool G_SP_HeadlessThrowCommand(gentity_s *player, client_t *client, usercmd_s *cmd, gentity_s *target, float distance,
    unsigned int gun);

// g_sp_client.cpp: the fight harness's pathnode walk toward goal (forward/right moves only).
bool G_SP_HeadlessWalkTo(gentity_s *player, const float *goal, int goalNum, usercmd_s *cmd);
