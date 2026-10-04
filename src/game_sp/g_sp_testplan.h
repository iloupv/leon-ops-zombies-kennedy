#pragma once

// Headless test-client feature plan (bo1_testclient_plan). HARNESS ONLY: every entry point returns
// immediately unless Sys_IsHeadless(), zombie mode and the test-client dvars are set. The plan walks the
// test client to the scripts' own triggers and presses use through its usercmd; the scripts decide
// what a press does. The one state write is the TEST SWITCH bo1_testclient_score (see the .cpp).

#include <clientscript/cscr_variable.h>

struct gentity_s;
struct client_t;
struct usercmd_s;

void G_SP_TestPlanRegister();
void G_SP_TestPlanReset();
void G_SP_TestPlanFields(scriptInstance_t inst);
void G_SP_TestPlanNotify(scriptInstance_t inst, unsigned int owner, unsigned int name, const VariableValue *top);
void G_SP_TestPlanObserve(const char *name);
bool G_SP_TestPlanActive();
// r1: the Pentagon thief while the plan's thief round policy runs (the fight code shoots it first), else null.
gentity_s *G_SP_TestPlanThief(gentity_s *player);
bool G_SP_TestPlanComplete();
void G_SP_TestPlanQuitCheck();
// j1: true while a grenade step holds the frag button (the fighting policy must not interrupt it).
bool G_SP_TestPlanHoldingGrenade();
// q1 c15: the actor's script has_legs is false (crawler); harness evidence only.
bool G_SP_TestPlanActorLegless(int entnum);
// r1: the gun the plan keeps raised though it may be empty (box / wall buy replacement), else 0.
unsigned int G_SP_TestPlanHoldWeapon();
// L20: may the fighting harness back the player away from a close zombie over the plan's movement? 0 = no (the
// step holds a spot), 1 = only while the held gun cannot fire (reload / empty clip), 2 = as without a plan.
int G_SP_TestPlanMayRetreat(const gentity_s *player);
// Called by the fighting harness each command. target is the actor being shot (or null); returns true
// when the plan wrote movement/view/buttons that override the fighting policy's.
bool G_SP_TestPlanCommand(gentity_s *player, client_t *client, usercmd_s *cmd, gentity_s *target,
    float targetDistance, bool melee);
