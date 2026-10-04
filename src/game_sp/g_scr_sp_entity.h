#pragma once

// zombies: haseyes (SP 0x008062e0) uses gentity+4, eFlags bit 17 (0x20000).
// KB uses that bit for destructibles. Its eFlags2 bit 31 is free and is sent as
// part of the existing 32-bit netfield. Client haseyes must read this bit instead.
static const unsigned int SP_EFLAGS2_HAS_EYES = 0x80000000u;

// zombies: setailimit (SP 0x00804ad0), storage at SP 0x01c08ae4. Actor allocation must
// consult this in zombiemode; the game-init owner must reset it to 32 per level.
extern int g_spActorLimit;

struct gentity_s;
// SP's code-spawn byte and spawn statistic do not exist in KB's fixed structs.
// Call the level reset from G_InitGame and the entity reset from G_InitGentity /
// G_FreeEntity. The getter also rejects entries from an older entity useCount.
extern unsigned int g_spCodeSpawnCount;
void G_SP_ResetEntityBuiltins();
void G_SP_InitGravityVolumes(); // zombies: SP 0x00561830
// zombies: G_InitGame node link reset (SP 0x004587d0).
void Path_SP_ResetLinkOverrides(int restart);
void G_SP_ClearEntityBuiltinState(unsigned int entnum);
bool G_SP_IsCodeSpawned(const gentity_s *ent);

struct gclient_s;
// zombies: lane x2 side state (SP fields KB's structs do not have). Level reset is part of
// G_SP_ResetEntityBuiltins.
// SP 0x01C88DE4: the frame's peak compressed snapshot size, fed by SV_SendMessageToClient
// (SP 0x0049E716), read by oktospawn.
void G_SP_NoteSnapshotSize(bool firstClient, int size);
// SP gclient+0x584: setscripthintstring's hint, read by Player_UpdateCursorHints (SP 0x00603889).
bool G_SP_GetScriptHintString(const gclient_s *client, int *hintString);
// SP svFlags 0x100 set by transmittargetname (no reader ported yet).
bool G_SP_TransmitsTargetname(const gentity_s *ent);
