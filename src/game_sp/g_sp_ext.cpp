#include <game_mp/g_spawn_mp.h>
#include <game_mp/g_scr_main_mp.h>
#include "g_sp_ext.h"
#include <bgame/bg_misc.h>
#include "actor_sp_ext.h"
#include <game_mp/g_main_mp.h>
#include <clientscript/cscr_stringlist.h>
#include <clientscript/cscr_vm.h>
#include <server_mp/sv_main_mp.h>
#include <game_mp/g_cmds_mp.h>
#include <qcommon/cmd.h>

gentitySpExt g_entSpExt[MAX_GENTITIES_SV];
gclientSpExt g_clientSpExt[32];

gentitySpExt &G_EntSpExt(const gentity_s *ent)
{
    const int index = ent - g_entities;
    iassert(index >= 0 && index < ARRAY_COUNT(g_entSpExt));
    return g_entSpExt[index];
}

gclientSpExt &G_ClientSpExt(const gclient_s *client)
{
    const int index = client - level.clients;
    iassert(index >= 0 && index < ARRAY_COUNT(g_clientSpExt));
    return g_clientSpExt[index];
}

void G_ClearEntSpExt(gentity_s *ent)
{
    G_SP_ClearScriptedAnim(ent);
    gentitySpExt &ext = G_EntSpExt(ent);
    Scr_SetString(&ext.script_linkname, 0, SCRIPTINSTANCE_SERVER);
    ext.anglelerprate = 0.0f;
    ext.activator.setEnt(NULL);
    Scr_SetString(&ext.soundNotify, 0, SCRIPTINSTANCE_SERVER);
    ext.soundAlias = 0;
    ext.soundNotifyTime = 0;
    ext.soundNotifyLength = -1;
    ext.soundNotifyPending = false;
    ext.scriptedAnimSupported = false;
    ext.actorTurret = false;
}

void G_SP_SetSoundNotify(gentity_s *ent, unsigned int alias, unsigned int notify)
{
    // zombies: retain the new string across notification of the old one (SP 0x00813020).
    unsigned __int16 name = 0;
    Scr_SetString(&name, notify, SCRIPTINSTANCE_SERVER);
    gentitySpExt &ext = G_EntSpExt(ent);
    if (ext.soundNotify)
    {
        Scr_Notify(ent, ext.soundNotify, 0);
        if (!ext.soundNotifyPending)
        {
            Scr_SetString(&name, 0, SCRIPTINSTANCE_SERVER);
            Scr_Error("issued a second playsound with notification string before the first finished\n", false);
        }
    }
    Scr_SetString(&ext.soundNotify, name, SCRIPTINSTANCE_SERVER);
    Scr_SetString(&name, 0, SCRIPTINSTANCE_SERVER);
    ext.soundAlias = alias;
    ext.soundNotifyTime = svs.time;
    ext.soundNotifyLength = -1;
    ext.soundNotifyPending = true;
}

void G_SP_Cmd_SoundLength_f()
{
    // zombies (a1): ClientCommand "sl <entnum> <msec>" (SP 0x0053B6B0, dispatched from SP ClientCommand 0x004AF8C0
    // after "mlvl"). The client sends it when its sound system knows the length of a notify playsound
    // (CG_ScriptSndLengthNotify, SP 0x004534C0), so "sounddone" fires when the sound ends instead of on the 5000 ms
    // fallback. Only a longer length replaces the stored one, and the timer restarts from the report.
    char buf[256];
    if (SV_Cmd_Argc() != 3)
    {
        Com_Printf(1, "\x15'sl' client command received with %i parameters instead of 3\n", SV_Cmd_Argc());
        return;
    }
    SV_Cmd_ArgvBuffer(1, buf, sizeof(buf));
    const unsigned int entnum = atoi(buf);
    SV_Cmd_ArgvBuffer(2, buf, sizeof(buf));
    const int msec = atoi(buf);
    if (entnum >= MAX_GENTITIES_SV)
    {
        Com_Printf(1, "\x15'sl' client command received with entnum %i\n", entnum);
        return;
    }
    gentitySpExt &ext = G_EntSpExt(&g_entities[entnum]);
    if (ext.soundNotifyLength < msec)
    {
        ext.soundNotifyLength = msec;
        ext.soundNotifyTime = svs.time; // SP: level.time; KB's soundNotify timer runs on svs.time (G_SP_SetSoundNotify)
    }
}

void G_SP_UpdateSoundNotify(gentity_s *ent)
{
    // zombies: G_UpdateEntity uses a 5000 ms fallback for unknown lengths (SP 0x007E36A0).
    gentitySpExt &ext = G_EntSpExt(ent);
    if (!ext.soundNotify)
        return;
    const int length = ext.soundNotifyLength < 0 ? 5000 : ext.soundNotifyLength;
    if (svs.time - ext.soundNotifyTime >= length)
    {
        Scr_Notify(ent, ext.soundNotify, 0);
        Scr_SetString(&ext.soundNotify, 0, SCRIPTINSTANCE_SERVER);
    }
}

void G_ClearClientSpExt(gclient_s *client)
{
    gclientSpExt &ext = G_ClientSpExt(client);
    ext.sessionPerks = 0;
    ext.downs = 0;
    ext.revives = 0;
    ext.lookatent.setEnt(NULL);
    G_SP_PlayerGravityState(client) = SPPlayerGravity{};
}

SPPlayerGravity g_spPlayerGravity[32];

SPPlayerGravity &G_SP_PlayerGravityState(const gclient_s *client)
{
    return g_spPlayerGravity[client - g_clients];
}

bool G_SP_ZombietronOff()
{
    const dvar_s *zombietron = Dvar_FindVar("zombietron");
    return !zombietron || !zombietron->current.enabled;
}

int G_SP_PlayerGravity(const gclient_s *client, bool round)
{
    if (zombiemode && zombiemode->current.enabled && G_SP_PlayerGravityState(client).gravityOverride)
        return G_SP_PlayerGravityState(client).gravityOverrideValue;
    // SP adds 9.3e-10 and stores with fistp in ClientThink_real; ClientEndFrame uses cvttss2si.
    return round ? (int)lrintf(bg_gravity->current.value + 9.3e-10f) : (int)bg_gravity->current.value;
}
