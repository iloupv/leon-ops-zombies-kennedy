#pragma once

struct gclient_s;

// SP permissions cannot occupy KB's pm_flags: those bits already mean different things in MP.
// Consumers must gate on zombiemode and use this server-side state; client prediction needs a wire port.
enum SPPlayerPermission
{
    SP_DISABLE_STAND = 0x400000,
    SP_DISABLE_CROUCH = 0x800000,
    SP_DISABLE_PRONE = 0x1000000,
    SP_DISABLE_LEAN = 0x2000000,
    SP_DISABLE_MELEE = 0x8000000
};

struct SPPlayerBuiltinState
{
    unsigned int disabledActions;
    bool healthShield;
    unsigned int perks; // SP playerState +0x4fc, two bits per perk.
    bool lowReady; // zombies: setlowready (SP ps.pm_flags 0x200000, SP 0x007da1e0)
    bool weaponChangeRaising; // SP gclient +0x1d0e: a raise was seen, weapon_change_complete pending.
};

SPPlayerBuiltinState &G_SP_PlayerBuiltinState(unsigned int clientNum);
void G_SP_ClearPlayerBuiltinState(gclient_s *client);
void G_SP_ClearLevelBuiltinState();
void G_SP_ResetMissionFailed();
struct gentity_s;
enum hitLocation_t : __int32;
bool G_SP_ShouldEnterLastStand();
void G_SP_PlayerEnterLastStand(struct gentity_s *ent, struct gentity_s *inflictor, struct gentity_s *attacker, int damage,
    unsigned int meansOfDeath, unsigned int weapon, float *dir, hitLocation_t hitLoc, int timeOffset);
bool G_SP_GrenadeSuicideDisabled();
void G_SP_SetGrenadeSuicideDisabled(bool disabled);
void G_SP_DisableGrenadeSuicide();
bool G_SP_HasPerk(gclient_s *client, const char *name);
void G_SP_SetPerk(gclient_s *client, const char *name, bool enabled);
void G_SP_GetPerks(gclient_s *client);
