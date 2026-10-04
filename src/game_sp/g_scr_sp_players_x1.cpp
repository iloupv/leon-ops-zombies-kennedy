// Lane x1 (players): SP player / session builtins, ported from the SP exe (read-only copy,
// disassembled; the exported C where it exists). Rows are in g_scr_sp_players.cpp.
//
// SP -> KB layout notes used below (checked in the SP exe):
//  - playerState: SP and KB agree up to weaponstate (SP ps+0x158 == KB ps.weaponstate); SP ps.weapon is a
//    byte at +0x144, KB's is a u16 at the same offset; after +0x450 SP is 4 bytes ahead (linkFlags 0x454).
//  - gclient tail: SP flags +0x1c0c .. damage_fromWorld +0x1c4c == KB flags .. damage_fromWorld (+0xc90);
//    SP linkAnglesFrac +0x1c9c == KB linkAnglesFrac; SP revive +0x1d14, lastStand +0x1d1c, lastStandTime +0x1d20.
//  - SP clientState (sess.cs, via SP 0x005da470): lastDamageTime +0x3c, lastStandStartTime +0x40,
//    beingRevived +0x44 (the SP clientState netfield table at .rdata 0x00a5c0f8). KB has no beingRevived.
//  - SP weaponstates 11..17 are KB WEAPON_RELOADING..WEAPON_RELOAD_QUICK_EMPTY (KB = SP - 1 there).
//  - SP server command letters are sent unchanged: KB's cgame either does not use the letter or
//    (refreshhudammocounter's 'J') runs the same function for it.
#include "g_scr_sp_players_x1.h"
#include "scr_sp_tables.h"
#include "g_sp_levelstart.h"
#include "g_sp_ext.h"
#include "actor_sp_ext.h"
#include <cgame/cg_hudelem.h>
#include <game_mp/g_main_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_utils_mp.h>
#include <game/g_weapon.h>
#include <game/g_items.h>
#include <game/g_hudelem.h>
#include <game/actor.h>
#include <game/actor_script_cmd.h>
#include <bgame/bg_misc.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_actor_prone.h>
#include <server/sv_game.h>
#include <server/sv_world.h>
#include <server_mp/sv_main_mp.h>
#include <clientscript/cscr_vm.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/scr_const.h>
#include <aim_assist/aim_assist.h>
#include <qcommon/cm_trace.h>
#include <qcommon/common.h>
#include <universal/dvar.h>
#include <universal/q_shared.h>
#include <universal/com_math.h>
#include <cstdlib>
#include <cstring>

// Defined in aim_assist.cpp (no header declaration).
extern AimAssistGlobals aaGlobArray[1];

static SPPlayerX1State s_x1State[32];

// zombies: SP CS 0xc27, the per-client last-stand vision info string. In BO1Zombies 0xc27 is
// CS_HEAD_ICONS + 11 (MP head icons 0xc1c..0xc2a; a KB client registers any value there as a
// material), so the string is kept here and its one reader, the owning client, gets the value as a
// reliable server command instead (see PlayerCmdSP_visionsetlaststand).
static char s_lastStandVisionInfo[1024];
// zombies: SP 0x00b77400 (.data initial value 1), bumped n = n + 1 | 1 on every visionsetlaststand.
static int s_lastStandVisionCounter = 1;

SPPlayerX1State &G_SP_PlayerX1State(unsigned int clientNum)
{
    iassert(clientNum < ARRAY_COUNT(s_x1State));
    return s_x1State[clientNum];
}

void G_SP_ClearPlayerX1State(unsigned int clientNum)
{
    iassert(clientNum < ARRAY_COUNT(s_x1State));
    memset(&s_x1State[clientNum], 0, sizeof(s_x1State[clientNum]));
    // zombies: ClientSpawn writes client+0x1d10 = 0x3ff (SP 0x0048036c).
    s_x1State[clientNum].groundRefEnt = ENTITYNUM_NONE;
}

void G_SP_ClearLevelX1State()
{
    for (unsigned int i = 0; i < ARRAY_COUNT(s_x1State); ++i)
        G_SP_ClearPlayerX1State(i);
    s_lastStandVisionInfo[0] = 0;
}

bool G_SP_NoAutoPickup(const gentity_s *other, int touched)
{
    // zombies: Touch_Item_Auto (SP 0x00562e70): touched && other->client && client+0x1ccc -> return.
    return touched && other->client && s_x1State[other->client - level.clients].noAutoPickup;
}

// The SP player-method guard (e.g. SP 0x007d9cf0): "not an entity" / "entity %i is not a player".
static gentity_s *SP_Player(scr_entref_t entref)
{
    if (entref.classnum)
    {
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
        return nullptr;
    }
    iassert(entref.entnum < g_scrEntNumLimit);
    gentity_s *ent = &g_entities[entref.entnum];
    if (!ent->client)
    {
        Scr_ObjectError(va("entity %i is not a player", entref.entnum), SCRIPTINSTANCE_SERVER);
        return nullptr;
    }
    return ent;
}

// The SP "valid client" guard shared by the client-state setters (SP 0x007fa170, 0x007fee10):
// inuse and client, else Scr_Error("<name>() called on an invalid client entity.").
static int SP_ValidClient(scr_entref_t entref, const char *error)
{
    if (!entref.classnum)
    {
        iassert(entref.entnum < g_scrEntNumLimit);
        gentity_s *ent = &g_entities[entref.entnum];
        if (ent->r.inuse && ent->client)
            return ent->s.number;
    }
    else
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    Scr_Error(error, false);
    return -1;
}

// ---------------------------------------------------------------------------------------------
// revive / last stand

// zombies: startrevive (SP 0x007d9cf0). Exported C absent; read from exe bytes.
void PlayerCmdSP_startrevive(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    ent->client->revive = 1;               // SP client+0x1d14
    ent->client->ps.pm_type = 7;           // SP 7: the being-revived last-stand type
    G_SP_PlayerX1State(ent->s.number).beingRevived = true; // SP clientState+0x44 (SP 0x005da470)
}

// zombies: stoprevive (SP 0x007d9d80). Exported C absent; read from exe bytes.
void PlayerCmdSP_stoprevive(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    ent->client->revive = 0;
    ent->client->ps.pm_type = 6;           // SP 6: last stand
    G_SP_PlayerX1State(ent->s.number).beingRevived = false;
}

// zombies: visionsetlaststand (SP 0x007ff900, CScr_m_visionsetlaststand). The SP value is
// `"<name>" <durationMs> <counter as %c>` under the client number in CS 0xc27; see s_lastStandVisionInfo.
void PlayerCmdSP_visionsetlaststand(scr_entref_t entref)
{
    int clientNum = -1;
    gentity_s *ent = nullptr;
    if (!entref.classnum)
        ent = &g_entities[entref.entnum];
    else
        Scr_ObjectError("not an entity", SCRIPTINSTANCE_SERVER);
    char info[1024];
    I_strncpyz(info, s_lastStandVisionInfo, sizeof(info));
    if (!ent || !ent->r.inuse || !ent->client)
        Scr_Error("visionsetlaststand() called on an invalid client entity.\n", false);
    else
        clientNum = ent->s.number;
    int duration = 1000;
    int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (count != 1)
    {
        if (count != 2)
        {
            Scr_Error("USAGE: player visionsetlaststand( <visionset name>, <transition time> )\n", false);
            return;
        }
        duration = (int)((float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER) * 1000.0f + 9.313226e-10f);
    }
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    const char *value = va("\"%s\" %i %c", name, duration, s_lastStandVisionCounter);
    Info_SetValueForKey(info, va("%i", clientNum), value);
    s_lastStandVisionCounter = (s_lastStandVisionCounter + 1) | 1;
    I_strncpyz(s_lastStandVisionInfo, info, sizeof(s_lastStandVisionInfo));
    // zombies: SP networks the value through CS 0xc27, which collides with KB's CS_HEAD_ICONS + 11.
    // Only the keyed client reads it (CG_ConfigString_VisionSetLastStand, SP 0x005b5ce0: Com_Parse the
    // name, atoi the duration, CG_VisionSetStartLerp(localClientNum, 6, 3 = TO_SMOOTH, name, duration)),
    // so that client gets `F "<name>" <durationMs>` (a letter neither exe's CG_DeployServerCommand
    // handles). The counter only forces a configstring change and is not sent.
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, va("%c \"%s\" %i", 'F', name, duration));
}

// zombies: setmaxhealth (SP 0x007f5020): ent->health = ent->maxhealth = value, and the client's
// session max health (SP client+0x1ac0) when there is a client. An entity method in SP.
void PlayerCmdSP_setmaxhealth(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    int value = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    ent->health = value;
    ent->maxHealth = value;
    if (ent->client)
        ent->client->sess.maxHealth = value;
}

// ---------------------------------------------------------------------------------------------
// view model / buttons / pickups

// zombies: hideviewmodel (SP 0x007d6ff0). SP sends '}' to the client; KB's cgame has no handler for
// it yet (client side is outside this lane).
void PlayerCmdSP_hideviewmodel(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    playerState_s *ps = &ent->client->ps;
    // SP weaponstates 11..17 (the reload family) are KB's 10..16.
    const int weaponstate = ps->weaponstate;
    if (weaponstate == WEAPON_RELOADING_INTERUPT || weaponstate == WEAPON_RELOAD_START_INTERUPT
        || weaponstate == WEAPON_RELOAD_QUICK_EMPTY || weaponstate == WEAPON_RELOAD_QUICK
        || weaponstate == WEAPON_RELOAD_START || weaponstate == WEAPON_RELOAD_END
        || weaponstate == WEAPON_RELOADING)
    {
        BG_AddPredictableEventToPlayerstate(EV_STOP_WEAPON_SOUND, weaponstate, ps);
    }
    PM_Weapon_Idle(ps); // SP 0x0052a0e0 -> PM_Weapon_StateEnd 0x00767130
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c", '}'));
}

// zombies: showviewmodel (SP 0x007d6f90). SP sends '{'; KB's cgame has no handler for it yet.
void PlayerCmdSP_showviewmodel(scr_entref_t entref)
{
    if (!SP_Player(entref))
        return;
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c", '{'));
}

// zombies: reloadbuttonpressed (SP 0x00487ad0): button bits 4 (reload) and 5 (use-reload), this frame or
// since the last frame (SP stores the bitarray MSB-first: 0x08000000 / 0x04000000 of the first word).
void PlayerCmdSP_reloadbuttonpressed(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    gclient_s *client = ent->client;
    if (client->button_bitsSinceLastFrame.testBit(4u) || client->button_bits.testBit(4u)
        || client->button_bitsSinceLastFrame.testBit(5u) || client->button_bits.testBit(5u))
    {
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

// zombies: allowpickupweapons (SP 0x007da430): ps eFlags2 0x8000000 blocks WeaponEntCanBeGrabbed
// (SP 0x0075d170; its KB consumer is gated in bg_misc.cpp).
void PlayerCmdSP_allowpickupweapons(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    if (Scr_GetInt(0, SCRIPTINSTANCE_SERVER))
    {
        ent->client->ps.eFlags2 &= ~0x8000000;
        return;
    }
    ent->client->ps.eFlags2 |= 0x8000000;
}

// zombies: setautopickup (SP 0x007d7d50): client+0x1ccc = !value (read by Touch_Item_Auto).
void PlayerCmdSP_setautopickup(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    if (Scr_GetInt(0, SCRIPTINSTANCE_SERVER))
    {
        G_SP_PlayerX1State(ent->s.number).noAutoPickup = 0;
        return;
    }
    G_SP_PlayerX1State(ent->s.number).noAutoPickup = 1;
}

// ---------------------------------------------------------------------------------------------
// weapon queries

// zombies: getweaponmuzzlepoint (SP 0x005ba610): weaponParms for ent->s.weapon, G_CalcMuzzlePoints
// (SP 0x00670b70, shot count 1), return muzzleTrace. An entity method in SP.
void PlayerCmdSP_getweaponmuzzlepoint(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    weaponParms wp;
    Weapon_SetWeaponParamsWeapon(&wp, ent->s.weapon);
    G_CalcMuzzlePoints(ent, &wp, 1);
    Scr_AddVector(wp.muzzleTrace, SCRIPTINSTANCE_SERVER);
}

// zombies: getweaponforwarddir (SP 0x00510f50): as above, return forward.
void PlayerCmdSP_getweaponforwarddir(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    weaponParms wp;
    Weapon_SetWeaponParamsWeapon(&wp, ent->s.weapon);
    G_CalcMuzzlePoints(ent, &wp, 1);
    Scr_AddVector(wp.forward, SCRIPTINSTANCE_SERVER);
}

// zombies: getweaponrenderoptions (SP 0x007d8110): the held weapon's render options; nothing is
// returned when the player does not hold the weapon.
void PlayerCmdSP_getweaponrenderoptions(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    int weaponIndex = G_GetWeaponIndexForName((char *)name);
    Scr_VerifyWeaponIndex(weaponIndex, name);
    playerState_s *ps = &ent->client->ps;
    for (int i = 0; i < 15; ++i)
    {
        if (ps->heldWeapons[i].weapon && ps->heldWeapons[i].weapon == (unsigned int)weaponIndex)
        {
            Scr_AddInt(ps->heldWeapons[i].options.i, SCRIPTINSTANCE_SERVER);
            return;
        }
    }
}

// zombies: BG_SetWeaponOptions (SP 0x00418580, unnamed in the exe): copies the weapon-option bits of
// the held weapon and of its held alt weapon. The exe's alt loop (0x0041865f jmp 0x004185d2) never
// advances the alt index, so retail spins forever when the alt is held; this copies it once (deviation).
static void BG_SetWeaponOptions(playerState_s *ps, unsigned int weaponIndex, renderOptions_s options)
{
    if (!BG_PlayerHasWeapon(ps, weaponIndex))
        return;
    BG_GetWeaponDef(weaponIndex); // SP 0x00425770: result unused
    BG_GetHeldWeapon(ps, weaponIndex)->options.CopyWeaponOptions(&options);
    unsigned int alt = BG_GetWeaponVariantDef(weaponIndex)->altWeaponIndex;
    if (alt && BG_PlayerHasWeapon(ps, alt))
        BG_GetHeldWeapon(ps, alt)->options.CopyWeaponOptions(&options);
}

// zombies: updateweaponoptions (SP 0x007da4b0): <player> UpdateWeaponOptions(weaponName, options).
void PlayerCmdSP_updateweaponoptions(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    renderOptions_s options;
    options.i = Scr_GetInt(1, SCRIPTINSTANCE_SERVER);
    int weaponIndex = G_GetWeaponIndexForName((char *)name);
    Scr_VerifyWeaponIndex(weaponIndex, name);
    playerState_s *ps = &ent->client->ps;
    if (!BG_PlayerHasWeapon(ps, weaponIndex))
    {
        Scr_Error(va("updateWeaponOptions called on a weapon the player doesn't have: %s", name), 0);
        return;
    }
    BG_SetWeaponOptions(ps, weaponIndex, options);
}

// zombies: disableweaponfire (SP 0x007d8c00) / enableweaponfire (SP 0x007d8d50): weapFlags bit 0x1000000, read by
// PM_Weapon_FireDisabled (SP 0x00766f90) in PM_Weapon's fire path and PM_Weapon_CheckForMelee.
void PlayerCmdSP_disableweaponfire(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (ent)
        ent->client->ps.weapFlags |= 0x1000000u;
}

void PlayerCmdSP_enableweaponfire(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (ent)
        ent->client->ps.weapFlags &= ~0x1000000u;
}

// zombies: disableweaponreload (SP 0x007d8ce0) / enableweaponreload (SP 0x007d8c70): weapFlags bit 0x2000000, read by
// PM_Weapon_CheckFiringAmmo (SP 0x00766bb0) and PM_Weapon_CheckForReload (SP 0x00769090).
void PlayerCmdSP_disableweaponreload(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (ent)
        ent->client->ps.weapFlags |= 0x2000000u;
}

void PlayerCmdSP_enableweaponreload(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (ent)
        ent->client->ps.weapFlags &= ~0x2000000u;
}

// zombies: setloweredweapon (SP 0x007da2a0): <player> SetLoweredWeapon(bool). The int is read before the entity check.
// eFlags2 bit 0x800000; SP reads it only in the view-model offset (SP 0x0076ccf0, bg_weaponForcedLow* dvars), not ported.
void PlayerCmdSP_setloweredweapon(scr_entref_t entref)
{
    int lowered = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    if (lowered)
        ent->client->ps.eFlags2 |= 0x800000u;
    else
        ent->client->ps.eFlags2 &= ~0x800000u;
}

// zombies: getweaponclipmodel (SP 0x007f10b0).
void GScrSP_getweaponclipmodel()
{
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    int weaponIndex = G_GetWeaponIndexForName((char *)name);
    if (!weaponIndex)
    {
        if (*name && I_stricmp(name, "none"))
            Com_Printf(24, "unknown weapon '%s' in getWeaponClipModel\n", name);
        Scr_AddString("", SCRIPTINSTANCE_SERVER);
        return;
    }
    if (!BG_GetWeaponDef(weaponIndex)->worldClipModel)
    {
        Scr_AddString("", SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddString(BG_GetWeaponDef(weaponIndex)->worldClipModel->name, SCRIPTINSTANCE_SERVER);
}

// zombies: weaponfightdist / weaponmaxdist (SP 0x007fc870 / 0x007fc8c0): the weapon def's fightDist /
// maxDist (SP WeaponDef +0x70c / +0x710). SP raises "unknown weapon" for an unknown name (Scr_Error, not
// terminal) and then reads weapon 0's def.
static void GScrSP_WeaponDistField(bool maxDist)
{
    int weaponIndex = G_GetWeaponIndexForName((char *)Scr_GetString(0, SCRIPTINSTANCE_SERVER));
    if (!weaponIndex)
        Scr_Error("unknown weapon", 0);
    const WeaponDef *weapDef = BG_GetWeaponDef(weaponIndex);
    Scr_AddFloat(maxDist ? weapDef->maxDist : weapDef->fightDist, SCRIPTINSTANCE_SERVER);
}

void GScrSP_weaponfightdist()
{
    GScrSP_WeaponDistField(false);
}

void GScrSP_weaponmaxdist()
{
    GScrSP_WeaponDistField(true);
}

// ---------------------------------------------------------------------------------------------
// links, view

// zombies: playersetgroundreferenceent (SP 0x007f27c0): client+0x1d10 = entity number or 0x3ff.
// SP's consumer (SP 0x00667a70, from ClientEndFrame 0x0047f895) copies that entity's angles into an
// SP playerState vector (ps+0x464) that KB's playerState_s does not have; stored here only.
void PlayerCmdSP_playersetgroundreferenceent(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (!ent->client)
        Scr_ObjectError("not a player entity", SCRIPTINSTANCE_SERVER);
    if (!Scr_GetType(0, SCRIPTINSTANCE_SERVER))
    {
        G_SP_PlayerX1State(ent->s.number).groundRefEnt = ENTITYNUM_NONE;
        return;
    }
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) != VAR_POINTER || Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) != VAR_ENTITY)
        Scr_ParamError(0, "not an entity", SCRIPTINSTANCE_SERVER);
    G_SP_PlayerX1State(ent->s.number).groundRefEnt = Scr_GetEntity(0)->s.number;
}

// zombies: playerlinktoabsolute (SP 0x007f2f80).
void PlayerCmdSP_playerlinktoabsolute(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    if (Scr_GetType(0, SCRIPTINSTANCE_SERVER) != VAR_POINTER || Scr_GetPointerType(0, SCRIPTINSTANCE_SERVER) != VAR_ENTITY)
        Scr_ParamError(0, "Not an entity", SCRIPTINSTANCE_SERVER);
    if (!ent->client)
        Scr_ObjectError("Not a player entity", SCRIPTINSTANCE_SERVER);
    gentity_s *parent = Scr_GetEntity(0);
    unsigned int tag = 0;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) > 1 && Scr_GetType(1, SCRIPTINSTANCE_SERVER))
    {
        tag = Scr_GetConstLowercaseString(1, SCRIPTINSTANCE_SERVER);
        if (tag == scr_const._)
            tag = 0;
    }
    gclient_s *client = ent->client;
    client->linkAnglesFrac = 1.0f;
    client->linkAnglesLocked = true;
    // SP sets pm_flags 0x4000000 (absolute link): G_SetPlayerFixedLink then sets the view angles to the
    // tag's (SP 0x00441b7d), and linkAnglesLocked freezes the view (pm_flags 0x800, SP 0x0047f6b8).
    client->ps.pm_flags |= 0x4000000;
    Vec3Clear(client->ps.linkAngles);
    client->ps.linkFlags |= 1;
    if (!G_EntLinkTo(ent, parent, tag))
        Scr_Error("Failed to link entity", false);
}

// zombies: setdoublevision (SP 0x007fa170). SP sends '4' <ms> <value>; KB's cgame has no handler yet.
void PlayerCmdSP_setdoublevision(scr_entref_t entref)
{
    float value = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    float time = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    if (time < 0.0f)
        Scr_ParamError(1, "Time must be positive", SCRIPTINSTANCE_SERVER);
    int ms = (int)(time * 1000.0f);
    if (value < 0.0f)
        Scr_ParamError(0, "Double vision value must be greater than 0", SCRIPTINSTANCE_SERVER);
    int clientNum = SP_ValidClient(entref, "setdoublevision() called on an invalid client entity.\n");
    SV_GameSendServerCommand(clientNum, SV_CMD_RELIABLE, va("%c %i %f", '4', ms, value));
}

// zombies: getplayerviewheight (SP 0x007d6e70): ps+400, KB ps.viewHeightCurrent.
void PlayerCmdSP_getplayerviewheight(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    Scr_AddFloat(ent->client->ps.viewHeightCurrent, SCRIPTINSTANCE_SERVER);
}

// zombies: islookingat (SP 0x007d7950): the client's look-at entity (client+0x1c80, gclientSpExt::lookatent)
// is the argument. SP fills lookatent every frame in SP 0x00418d00 (called from 0x0047f8bf), ported as
// Player_UpdateLookAtEntity_SP (g_sp_lookat.cpp).
void PlayerCmdSP_islookingat(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    gclientSpExt &ext = G_ClientSpExt(ent->client);
    if (ext.lookatent.isDefined() && ext.lookatent.ent() == Scr_GetEntity(0))
    {
        Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

// zombies: resetadswidthandlerp (SP 0x007d68d0 -> 0x00405e50): clears the aim-assist override of the
// client's aaGlob entry (SP indexes aaGlobArray by ps.clientNum; KB has one local entry).
void PlayerCmdSP_resetadswidthandlerp(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    unsigned int clientNum = ent->client->ps.clientNum;
    if (clientNum >= ARRAY_COUNT(aaGlobArray))
        return;
    aaGlobArray[clientNum].overrideSnapWidthAndLerp = false;
    aaGlobArray[clientNum].overrideAutoaimLerpValue = 0.0f;
    aaGlobArray[clientNum].overrideAutoaimWidthValue = 0.0f;
}

// zombies: setadswidthandlerp (SP 0x007d6830 -> 0x00525a70(clientNum, width, lerp)).
void PlayerCmdSP_setadswidthandlerp(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    float width = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    float lerp = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    unsigned int clientNum = ent->client->ps.clientNum;
    if (clientNum >= ARRAY_COUNT(aaGlobArray))
        return;
    aaGlobArray[clientNum].overrideAutoaimLerpValue = lerp;   // SP aaGlob+0xe9c
    aaGlobArray[clientNum].overrideAutoaimWidthValue = width; // SP aaGlob+0xea0
    aaGlobArray[clientNum].overrideSnapWidthAndLerp = true;
}

// zombies: startcameratween (SP 0x007da570): EV_START_CAMERA_TWEEN (SP 180 = KB 182) seen by that
// client only; the event parm is the tween time in ms.
void PlayerCmdSP_startcameratween(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    float time = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    gentity_s *te = G_TempEntity(ent->r.currentOrigin, EV_START_CAMERA_TWEEN);
    te->r.clientMask[0] = -1;
    te->s.eventParm = (unsigned __int16)(int)(time * 1000.0f);
    unsigned int clientNum = ent->client->ps.clientNum;
    te->r.clientMask[clientNum >> 5] &= ~(1 << (clientNum & 0x1F));
}

// zombies: getnormalizedmovement (SP 0x0044aa80): the client's current usercmd forwardmove / rightmove
// (client+0x1a4b / +0x1a4c) scaled by 1/127, z 0. The SP guard reports "entity %i is not a player".
void PlayerCmdSP_getnormalizedmovement(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    float movement[3];
    movement[0] = (float)ent->client->sess.cmd.forwardmove * 0.007874016f;
    movement[1] = (float)ent->client->sess.cmd.rightmove * 0.007874016f;
    movement[2] = 0.0f;
    Scr_AddVector(movement, SCRIPTINSTANCE_SERVER);
}

// zombies: getnormalizedcameramovement (SP 0x005aac30): pitchmove / yawmove (client+0x1a4e / +0x1a4f)
// scaled by -1/127, z 0.
void PlayerCmdSP_getnormalizedcameramovement(scr_entref_t entref)
{
    gentity_s *ent = SP_Player(entref);
    if (!ent)
        return;
    float movement[3];
    movement[0] = (float)ent->client->sess.cmd.pitchmove * -0.007874016f;
    movement[1] = (float)ent->client->sess.cmd.yawmove * -0.007874016f;
    movement[2] = 0.0f;
    Scr_AddVector(movement, SCRIPTINSTANCE_SERVER);
}

// zombies: setvolfog, method form (SP 0x007fee10). SP's G_SetFog (0x007fe4d0) writes the fog under this
// client's key of an info string in CS 10 (G_SetFogForClient 0x00460040) and skips while a save loads
// (level state 2, never true here). KB's CS 10 and CG_ParseFog carry one global fog string, and
// KB's own setvolfog function (g_scr_main_mp.cpp) writes that form, so the method uses the same
// transport: the SP per-client key needs CG_ParseFog and the function form changed together.
void PlayerCmdSP_setvolfog(scr_entref_t entref)
{
    float sunColR = 0.5f;
    float sunColG = 0.5f;
    float sunColB = 0.5f;
    float sunDirX = 1.0f;
    float sunDirY = 0.0f;
    float sunDirZ = 0.0f;
    float sunStartAng = 0.0f;
    float sunEndAng = 0.0f;
    int clientNum = SP_ValidClient(entref, "setvolumetricfog() called on an invalid client entity.\n");
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 8 && Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 18)
        Scr_Error(
            "Incorrect number of parameters\n"
            "USAGE: setVolFog(<startDist>, <halfwayDist>, <halfwayHeight>, <baseHeight>, <red>, <green>, <blue>, <transition time>)\n"
            "OR:    SetVolFog(<startDist>, <halfwayDist>, <halfwayHeight>, <baseHeight>, <red>, <green>, <blue>, <fogColorScale>, "
            "<sunFogRed>, <sunFogGreen>, <sunFogBlue>, <sunFogDirX>, <sunFogDirY>, <sunFogDirZ>, <sunFogStartAng>, <sunFogEndAng>, "
            "<transition time>)\n",
            false);
    float startDist = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if (startDist < 0.0f)
        Scr_Error("setExpFog: startDist must be greater or equal to 0", false);
    float halfwayDist = (float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER);
    if (halfwayDist <= 0.0f)
        Scr_Error("setVolFog: halfwayDist must be greater than 0", false);
    float halfwayHeight = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    if (halfwayHeight < 0.0f)
        Scr_Error("setVolFog: halfwayHeight must be greater or equal to 0", false);
    float baseHeight = (float)Scr_GetFloat(3, SCRIPTINSTANCE_SERVER);
    float heightDensity = halfwayHeight < 1.0f ? 0.0f : 1.0f / halfwayHeight;
    float red = (float)Scr_GetFloat(4, SCRIPTINSTANCE_SERVER);
    float green = (float)Scr_GetFloat(5, SCRIPTINSTANCE_SERVER);
    float blue = (float)Scr_GetFloat(6, SCRIPTINSTANCE_SERVER);
    float colorScale;
    float time;
    float maxFogOpacity;
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) == 18)
    {
        colorScale = (float)Scr_GetFloat(7, SCRIPTINSTANCE_SERVER);
        sunColR = (float)Scr_GetFloat(8, SCRIPTINSTANCE_SERVER);
        sunColG = (float)Scr_GetFloat(9, SCRIPTINSTANCE_SERVER);
        sunColB = (float)Scr_GetFloat(10, SCRIPTINSTANCE_SERVER);
        sunDirX = (float)Scr_GetFloat(11, SCRIPTINSTANCE_SERVER);
        sunDirY = (float)Scr_GetFloat(12, SCRIPTINSTANCE_SERVER);
        sunDirZ = (float)Scr_GetFloat(13, SCRIPTINSTANCE_SERVER);
        sunStartAng = (float)Scr_GetFloat(14, SCRIPTINSTANCE_SERVER);
        sunEndAng = (float)Scr_GetFloat(15, SCRIPTINSTANCE_SERVER);
        time = (float)Scr_GetFloat(16, SCRIPTINSTANCE_SERVER);
        maxFogOpacity = (float)Scr_GetFloat(17, SCRIPTINSTANCE_SERVER);
    }
    else
    {
        Com_Printf(1, "setVolFog: Old syntax used. Please update script.\n");
        colorScale = red;
        if (red < green)
            colorScale = green;
        if (colorScale < blue)
            colorScale = blue;
        float inv = 1.0f / colorScale;
        red = inv * red;
        green = inv * green;
        blue = inv * blue;
        time = (float)Scr_GetFloat(7, SCRIPTINSTANCE_SERVER);
        maxFogOpacity = 1.0f;
    }
    if (clientNum < 0)
        return;
    Scr_SetFog("setVolFog", startDist, 1.0f / halfwayDist, heightDensity, baseHeight, red, green, blue, time,
        colorScale, sunColR, sunColG, sunColB, sunDirX, sunDirY, sunDirZ, sunStartAng, sunEndAng, maxFogOpacity);
}

// zombies: setshadowhint (SP 0x007f4c40): bits 13..15 of s.lerp.eFlags, read by KB's
// CG_GetShadowHintForRefEntity. An entity method in SP.
void PlayerCmdSP_setshadowhint(scr_entref_t entref)
{
    gentity_s *ent = GetEntity(entref);
    const char *hint = SL_ConvertToString(Scr_GetConstString(0, SCRIPTINSTANCE_SERVER), SCRIPTINSTANCE_SERVER);
    int bits = 0;
    if (!strcmp(hint, "normal"))
        bits = 0;
    else if (!strcmp(hint, "never"))
        bits = 0x2000;
    else if (!strcmp(hint, "high_priority"))
        bits = 0x6000;
    else if (!strcmp(hint, "low_priority"))
        bits = 0x4000;
    else if (!strcmp(hint, "always"))
        bits = 0x8000;
    else if (!strcmp(hint, "receiver"))
        bits = 0xA000;
    else
        Scr_Error("setshadowhint argument must be \"normal\", \"never\", \"high_priority\", \"low_priority\", \"always\", or \"receiver\".", false);
    ent->s.lerp.eFlags = (ent->s.lerp.eFlags & 0xFFFF1FFF) | bits;
}

// ---------------------------------------------------------------------------------------------
// session / profile

// zombies: uploadscore (SP 0x007d9590): '7' <leaderboard> followed by up to 9 integers.
// KB's cgame has no '7' handler (leaderboards are a client-side port).
void PlayerCmdSP_uploadscore(scr_entref_t entref)
{
    if (!SP_Player(entref))
        return;
    int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if ((unsigned int)(count - 2) > 8)
    {
        Scr_Error("Incorrect number of parameters\n", false);
        return;
    }
    int leaderboard = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    char values[100];
    memset(values, 0, sizeof(values));
    for (int i = 1; i < count; ++i)
    {
        char number[12];
        _itoa(Scr_GetInt(i, SCRIPTINSTANCE_SERVER), number, 10);
        I_strncat(values, sizeof(values), " ");
        I_strncat(values, sizeof(values), number);
    }
    SV_GameSendServerCommand(entref.entnum, SV_CMD_RELIABLE, va("%c %i%s", '7', leaderboard, values));
}

// zombies: refreshhudammocounter (SP 0x00804810) sends `J "1"` to every client. SP's 'J' is
// CG_SetHudFadeGroup (SP 0x004e2520), the same function as KB's CG_MenuShowNotify (group 1 = the
// weapon/ammo HUD), so the letter does not collide; the zombie menu name is on the client side.
void GScrSP_refreshhudammocounter()
{
    SV_GameSendServerCommand(-1, SV_CMD_RELIABLE, va("%c \"%i\"", 'J', 1));
}

// zombies: reportclientdisconnected (SP 0x00804d70): game message `<name> EXE_LEFTGAME` to all ('e' is
// the game-message command in both cgames).
void GScrSP_reportclientdisconnected()
{
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    SV_SendServerCommand(nullptr, SV_CMD_CAN_IGNORE, "%c \"\x15%s^7 \x14%s\"", 'e', name, "EXE_LEFTGAME");
}

// zombies: playerpositionvalid (SP 0x007f87f0): a (-15,-15,0)..(15,15,70) capsule at the point,
// mask 0x281c011, must be clear.
void GScrSP_playerpositionvalid()
{
    float pos[3];
    Scr_GetVector(0, pos, SCRIPTINSTANCE_SERVER);
    col_context_t context;
    trace_t trace;
    const float mins[3] = { -15.0f, -15.0f, 0.0f };
    const float maxs[3] = { 15.0f, 15.0f, 70.0f };
    G_TraceCapsule(&trace, pos, mins, maxs, pos, ENTITYNUM_NONE, 0x281C011, &context);
    if (1.0f > trace.fraction || trace.allsolid || trace.startsolid)
    {
        Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddInt(1, SCRIPTINSTANCE_SERVER);
}

// zombies: the eight profile dvars "mg0".."mg7" (SP 0x00591315: int 0..0x7fffffff, flags 0x4001).
// SP registers them at startup; KB has none, so they are registered on first use. SP flag 0x4000 has no
// KB meaning here; 0x1 (archive) is kept.
static const dvar_s *SP_ProfileVar(int index)
{
    static const dvar_s *vars[8];
    if (!vars[index])
        vars[index] = _Dvar_RegisterInt(va("mg%d", index), 0, 0, 0x7FFFFFFF, 1, "");
    return vars[index];
}

// zombies: setpersistentprofilevar (SP 0x007fa3a0).
void GScrSP_setpersistentprofilevar()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("SetPersistentProfileVar() called with wrong params.\n", false);
    int index = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    int value = Scr_GetInt(1, SCRIPTINSTANCE_SERVER);
    if (index < 0 || index >= 8)
        Scr_Error("SetPersistentProfileVar() called with wrong param range. Index must be 0 to MISSION_GLOBAL_MAX_DVARS.\n", false);
    if (value < 0 || value >= 256)
        Scr_Error("SetPersistentProfileVar() called with wrong param range. Value must be 0 to 255.\n", false);
    Dvar_SetInt((dvar_s *)SP_ProfileVar(index), value);
}

// zombies: getpersistentprofilevar (SP 0x007fa420). SP reads the dvar only when local client 0 is active
// and its controller has a loaded profile (SP 0x00667ee0 > 0), else returns the default. KB has no
// profile system: a dedicated server (no local client) counts as "no profile" - MEASURED by nothing,
// approximated.
void GScrSP_getpersistentprofilevar()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER) != 2)
        Scr_Error("GetPersistentProfileVar() called with wrong params.\n", false);
    int index = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    int defaultValue = Scr_GetInt(1, SCRIPTINSTANCE_SERVER);
    if ((unsigned int)index > 7)
    {
        Scr_Error("GetPersistentProfileVar() called with wrong param range. Index must be 0 to MISSION_GLOBAL_MAX_DVARS.\n", false);
        Scr_AddInt(defaultValue, SCRIPTINSTANCE_SERVER);
        return;
    }
    if (!Dvar_GetInt("dedicated"))
    {
        Scr_AddInt(SP_ProfileVar(index)->current.integer, SCRIPTINSTANCE_SERVER);
        return;
    }
    Scr_AddInt(defaultValue, SCRIPTINSTANCE_SERVER);
}

// zombies: hascollectible (SP 0x00804a30 -> 0x0058e040): bg_collectibles[n - 1] != '0'.
// bg_collectibles is registered by SP 0x0055c5b0 (49 '0's, flags 1); KB lacks it, so first use registers it.
void GScrSP_hascollectible()
{
    static const dvar_s *collectibles;
    if (!collectibles)
        collectibles = _Dvar_RegisterString("bg_collectibles", "0000000000000000000000000000000000000000000000000", 1, "");
    int n = Scr_GetInt(0, SCRIPTINSTANCE_SERVER);
    const char *value = collectibles->current.string;
    // SP indexes without a range check; keep reads inside the string.
    bool has = n >= 1 && (unsigned int)(n - 1) < strlen(value) && value[n - 1] != '0';
    Scr_AddInt(has, SCRIPTINSTANCE_SERVER);
}

// zombies: iscoopepd (SP 0x00804680).
void GScrSP_iscoopepd()
{
    if (Scr_GetNumParam(SCRIPTINSTANCE_SERVER))
        Scr_Error("USAGE: bool = IsCoopEPD()\n", false);
    Scr_AddInt(0, SCRIPTINSTANCE_SERVER);
}

// ---------------------------------------------------------------------------------------------
// actor methods (SP actor table 0x00a52058: "not an actor" guard)

// zombies: dropweapon (SP 0x007cae50).
void ActorCmdSP_dropweapon(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if (!I_stricmp(name, "none"))
        return;
    unsigned int hand = Scr_GetConstString(1, SCRIPTINSTANCE_SERVER);
    float speed = (float)Scr_GetFloat(2, SCRIPTINSTANCE_SERVER);
    int weaponIndex = G_GetWeaponIndexForName((char *)name);
    if (!weaponIndex)
        Scr_Error(va("unknown weapon '%s' in dropWeapon", name), false);
    unsigned int tag;
    if (hand == scr_const.left)
        tag = scr_const.tag_weapon_left;
    else if (hand == scr_const.right)
        tag = scr_const.tag_weapon_right;
    else if (hand == scr_const.back)
        tag = scr_const.tag_stowed_back;
    else
        tag = 0;
    gentity_s *item = Drop_Weapon(self->ent, weaponIndex, 0, tag);
    if (!item)
        return;
    if (speed == 0.0f)
    {
        Vec3Clear(item->s.lerp.pos.trDelta);
        item->s.lerp.pos.trTime = level.time;
    }
    else
    {
        gentity_s *ent = self->ent;
        float centerZ = (ent->r.maxs[2] - ent->r.mins[2]) * 0.5f + ent->r.currentOrigin[2];
        float dir[3];
        dir[0] = item->r.currentOrigin[0] - ent->r.currentOrigin[0];
        dir[1] = item->r.currentOrigin[1] - ent->r.currentOrigin[1];
        dir[2] = item->r.currentOrigin[2] - centerZ + 2.0f;
        float length = sqrtf(dir[0] * dir[0] + dir[1] * dir[1] + dir[2] * dir[2]);
        if (-length >= 0.0f)
            length = 1.0f;
        float inv = 1.0f / length;
        item->s.lerp.pos.trDelta[0] = inv * dir[0] * speed;
        item->s.lerp.pos.trDelta[1] = dir[1] * inv * speed;
        item->s.lerp.pos.trDelta[2] = inv * dir[2] * speed;
        item->s.lerp.pos.trTime = level.time;
    }
    // SP then sets item s.lerp.eFlags2 |= 0x200000 when the dropper's gentity+0x198 is set; that SP
    // gentity field has no identified KB member, so this step is not ported.
    Scr_AddEntity(item, SCRIPTINSTANCE_SERVER);
    Scr_Notify(self->ent, scr_const.dropweapon, 1);
    Scr_AddEntity(item, SCRIPTINSTANCE_SERVER);
}

// zombies: Actor_EnterProne (SP 0x0066cef0). SP actor+0xd18 is KB actor_s::ProneInfo, +0xd14 bProneOK,
// +0xe50 Physics.prone.
static void Actor_EnterProne_SP(actor_s *self, int timeMs)
{
    actor_prone_info_s *info = &self->ProneInfo;
    if (BG_ActorIsProne(info, level.time))
    {
        if (info->iProneTrans && timeMs != info->iProneTrans)
        {
            float fraction = (float)BG_GetActorProneFraction(info, level.time);
            if (1.0f > fraction)
                info->iProneTime = level.time - (int)((float)timeMs * fraction);
            info->iProneTrans = timeMs;
        }
        return;
    }
    gentity_s *ent = self->ent;
    info->bCorpseOrientation = false;
    info->iProneTime = level.time;
    info->prone = true;
    self->Physics.prone = true;
    info->iProneTrans = timeMs;
    self->bProneOK = BG_CheckProne(nullptr, ent->s.number, ent->r.currentOrigin, 15.0f, 48.0f, ent->r.currentAngles[1],
        &info->fTorsoPitch, &info->fWaistPitch, false, true, true, 1, PCT_ACTOR, 50.0f);
}

// zombies: Actor_ExitProne (SP 0x00588070).
static void Actor_ExitProne_SP(actor_s *self, int timeMs)
{
    actor_prone_info_s *info = &self->ProneInfo;
    if (!BG_ActorIsProne(info, level.time))
        return;
    if (info->iProneTrans && timeMs != info->iProneTrans)
    {
        float fraction = (float)BG_GetActorProneFraction(info, level.time);
        info->iProneTime = level.time - (int)((float)timeMs * fraction);
        info->iProneTrans = -timeMs;
        return;
    }
    info->iProneTrans = -timeMs;
    info->iProneTime = level.time;
}

// zombies: enterprone (SP 0x007ca610). The anim-node check reads what setproneanimnodes stored
// (Actor_SP_ProneAnimNodes, SP actor+0xd00..0xd04) and the reciprocal pitches (SP +0xd08 / +0xd0c).
void ActorCmdSP_enterprone(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    const unsigned short *nodes = Actor_SP_ProneAnimNodes(self);
    if ((self->fInvProneAnimLowPitch == 0.0f && self->fInvProneAnimHighPitch == 0.0f)
        || !nodes[0] || !nodes[1] || !nodes[2])
    {
        Scr_Error("Must call SetProneAnimNodes before calling EnterProne", false);
    }
    Actor_EnterProne_SP(self, (int)((float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0f));
}

// zombies: exitprone (SP 0x007ca6c0).
void ActorCmdSP_exitprone(scr_entref_t entref)
{
    actor_s *self = Actor_Get(entref);
    Actor_ExitProne_SP(self, (int)((float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER) * 1000.0f));
}

// zombies: updateprone (SP 0x007ca820). SP sets the two prone blend anims through 0x0040e1d0 and then
// runs Actor_UpdateProneInformation (SP 0x004897e0), which KB lacks (no Five caller: only the human
// animscripts use it). Not ported and not registered: the generated not-ported stub stays in place.

// ---------------------------------------------------------------------------------------------
// hud elements

struct SPHudFontScale
{
    float fromFontScale;    // SP hudelem +0x10
    int fontScaleStartTime; // SP hudelem +0x14
    __int16 fontScaleTime;  // SP hudelem +0x4e
};
// zombies: KB's networked hudelem_s (0x70 bytes) has no font-scale lerp fields; SP's do. Kept per hud
// element here. SP's client lerps (CG_HudElemSetupText 0x00779980 -> HudElem_CurrentFontScale
// 0x0047b0f0); KB can't send the lerp fields, so HudElem_UpdateClient sends the current value instead
// (HudElem_SP_WireFontScale below).
static SPHudFontScale s_hudFontScale[1024];

// zombies: HudElem_CurrentFontScale (SP 0x0047b0f0).
float HudElem_CurrentFontScale_SP(const game_hudelem_s *hud, int time)
{
    const SPHudFontScale &lerp = s_hudFontScale[hud - g_hudelems];
    int elapsed = time - lerp.fontScaleStartTime;
    if (lerp.fontScaleTime > 0 && elapsed < lerp.fontScaleTime)
    {
        if (elapsed < 0)
            elapsed = 0;
        return ((float)elapsed / (float)lerp.fontScaleTime) * (hud->elem.fontScale - lerp.fromFontScale) + lerp.fromFontScale;
    }
    return hud->elem.fontScale;
}

// zombies: changefontscaleovertime (SP 0x007ded10).
void HECmdSP_changefontscaleovertime(scr_entref_t entref)
{
    game_hudelem_s *hud = HECmd_GetHudElem(entref);
    float scaleTime = (float)Scr_GetFloat(0, SCRIPTINSTANCE_SERVER);
    if (scaleTime > 0.0f)
    {
        if (scaleTime > 60.0f)
            Scr_ParamError(0, va("scale time %g > 60", scaleTime), SCRIPTINSTANCE_SERVER);
    }
    else
        Scr_ParamError(0, va("scale time %g <= 0", scaleTime), SCRIPTINSTANCE_SERVER);
    SPHudFontScale &lerp = s_hudFontScale[hud - g_hudelems];
    lerp.fromFontScale = HudElem_CurrentFontScale_SP(hud, level.time);
    lerp.fontScaleStartTime = level.time;
    lerp.fontScaleTime = (__int16)(int)(scaleTime * 1000.0f + 9.313226e-10f);
}

// zombies: the font-scale part of SP HudElem_SetDefaults (SP 0x007dcee0: fromFontScale +0x10 = 0,
// fontScaleStartTime +0x14 = 0, fontScaleTime +0x4e = 0), so a reused slot carries no old lerp.
void HudElem_SP_ClearFontScaleLerp(const game_hudelem_s *hud)
{
    SPHudFontScale &lerp = s_hudFontScale[hud - g_hudelems];
    lerp.fromFontScale = 0.0f;
    lerp.fontScaleStartTime = 0;
    lerp.fontScaleTime = 0;
}

// zombies: APPROXIMATED client end of changefontscaleovertime. SP's client evaluates
// HudElem_CurrentFontScale (SP 0x0047b0f0) every render frame from the networked lerp fields; KB's
// hudelem_s has no room for them (layout is fixed), so the server writes the lerped scale into the
// client's hud copy at snapshot build. The animation is stepped at the server frame rate.
void HudElem_SP_WireFontScale(const game_hudelem_s *hud, hudelem_s *elem, int time)
{
    if (s_hudFontScale[hud - g_hudelems].fontScaleTime > 0)
        elem->fontScale = HudElem_CurrentFontScale_SP(hud, time);
}
