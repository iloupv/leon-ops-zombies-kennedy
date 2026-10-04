#pragma once

// k1: pinned script dice (TEST SWITCH bo1_testclient_dice, headless harness only, off by default).
struct gentity_s;

// Called by the RandomInt / RandomIntRange builtins after their G_irand draw. Returns true and replaces *value
// when the switch is on and this call site's outcome is pinned to the recording; otherwise leaves it alone.
bool G_SP_HeadlessDicePin(int lo, int hi, int *value);
// From G_SP_MeasureFields at the end of the script load: the canonical ids of the names the pins read.
void G_SP_HeadlessDiceFields(int inst);
// Actor spawn order (the n of the table), from SpawnActor after SP_actor succeeded.
void G_SP_HeadlessDiceActorSpawned(const gentity_s *ent);
// AnimScripted (G_ScriptedAnim_Begin): counts an actor's "tear_anim" cycles for the tear-phase pins.
void G_SP_HeadlessDiceAnimScripted(const gentity_s *ent, unsigned int notifyName);
// a1 c14: the player's melee damage roll (G_rand() % 5 in Melee_DoDamage): the k-th hit on an actor this level takes
// the recorded knife damage minus the weapon's base damage (a "melee k damage entnum" line). Replaces *roll when pinned.
bool G_SP_HeadlessDiceMelee(int baseDamage, int targetEnt, int *roll);
// k1 c16: G_InitGame's random seed. TEST SWITCH bo1_testclient_seed (headless only, 0 = off): a fixed seed so
// two runs draw the same unpinned dice (run-to-run envelope of the chase); otherwise the wall-clock seed.
unsigned int G_SP_HeadlessSeed(unsigned int wallClockSeed);
