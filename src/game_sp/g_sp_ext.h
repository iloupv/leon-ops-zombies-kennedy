#pragma once

#include <game/enthandle.h>
#include <universal/q_shared.h>

struct gclient_s;

// Keep SP-only storage out of KB's fixed-stride entity and client arrays.
struct gentitySpExt
{
    // zombies: gentity +0x15e (SP 0x00a55744).
    unsigned __int16 script_linkname;
    // zombies: gentity +0x328 (SP 0x00a55780).
    float anglelerprate;
    // zombies: gentity pointer +0x210, tracked with KB's handle lifetime (SP 0x00a55794).
    EntHandle activator;
    unsigned __int16 soundNotify; // zombies: SP gentity +0x284.
    unsigned int soundAlias;
    int soundNotifyTime;
    int soundNotifyLength;
    bool soundNotifyPending;
    bool scriptedAnimSupported; // zombies: SP entity flags 0x2000, not KB FL_NO_AUTO_ANIM_UPDATE.
    bool actorTurret; // zombies: SP entity flags 0x4000000 (set by useturret), not KB FL_OBSTACLE.
};

struct gclientSpExt
{
    unsigned int sessionPerks; // zombies: SP client session +0x1b64; survives ClientSpawn.
    // zombies: client +0x1ba4 / +0x1ba8 (SP 0x00a527fc / 0x00a52818).
    int downs;
    int revives;
    // zombies: client entity handle +0x1c80 (SP 0x00a52a48).
    EntHandle lookatent;
};

// zombies: setplayergravity / clearplayergravity (SP 0x00806dd0 / 0x00806e60): SP client array 0x01a53770
// (0x918 bytes each) +0x910 flag, +0x914 value; read by ClientThink_real / ClientEndFrame / G_InitGrenadeMovement.
// Not in gclientSpExt: that is part of the SP save image (g_sp_save_client.inl).
struct SPPlayerGravity
{
    bool gravityOverride;
    int gravityOverrideValue;
};
extern SPPlayerGravity g_spPlayerGravity[32];
SPPlayerGravity &G_SP_PlayerGravityState(const gclient_s *client);

extern gentitySpExt g_entSpExt[MAX_GENTITIES_SV];
// KB's g_clients array has 32 entries; there is no shared MAX_CLIENTS constant.
extern gclientSpExt g_clientSpExt[32];

gentitySpExt &G_EntSpExt(const gentity_s *ent);
gclientSpExt &G_ClientSpExt(const gclient_s *client);
void G_ClearEntSpExt(gentity_s *ent);
void G_ClearClientSpExt(gclient_s *client);
// zombies: ps.gravity for a player (SP ClientThink_real 0x0069d5a3 rounds, ClientEndFrame 0x0047f67e truncates).
int G_SP_PlayerGravity(const gclient_s *client, bool round);
bool G_SP_ZombietronOff();
void G_SP_SetSoundNotify(gentity_s *ent, unsigned int alias, unsigned int notify);
void G_SP_UpdateSoundNotify(gentity_s *ent);
void G_SP_Cmd_SoundLength_f(); // ClientCommand "sl" (SP 0x0053B6B0)
// zombies: lane x3 - SP G_FreeEntity 0x00438bca turret release (g_scr_sp_ai_cmds.cpp).
void Actor_SP_ReleaseFreedTurret(gentity_s *turret);
