#pragma once

#include <clientscript/cscr_variable.h>

// TEST SWITCHES (headless only): bo1_testclient_labs, bo1_testclient_round - see g_sp_headless_labs.cpp.
struct gentity_s;
void G_SP_HeadlessLabsInit();
bool G_SP_HeadlessLabsActive();
gentity_s *G_SP_HeadlessLabsGoal(gentity_s *player);
gentity_s *G_SP_HeadlessLabsThief(gentity_s *player);
void G_SP_HeadlessRoundFields(scriptInstance_t inst);
void G_SP_HeadlessRoundNotify(scriptInstance_t inst, unsigned int owner, unsigned int name);
