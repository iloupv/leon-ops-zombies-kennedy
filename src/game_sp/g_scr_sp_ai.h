#pragma once

// SP 0x00512670, called by G_ShutdownGame before the script system shuts down.
void Actor_FreeThreatBiasGroups();

// SP sentient method receiver check (SP 0x0067b6a0 / 0x008195a0 / 0x00511750): "not a sentient".
struct sentient_s;
sentient_s *GScr_SP_GetSentient(scr_entref_t entref);

// mod (L43) bo1_mod_scriptperf: canonical field ids for the rewritten loops' natives (from Scr_EndLoadScripts).
void GScr_ModScriptPerfFields(scriptInstance_t inst);

// SP 0x007f08d0: OR of the team flags named by parameters firstParam.. (0 if none).
int GScr_SP_TeamFlagsFrom(unsigned int firstParam, const char *function);
