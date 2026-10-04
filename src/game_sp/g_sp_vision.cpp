#include <server_mp/sv_init_mp.h>
#include "g_sp_vision.h"
#include <clientscript/cscr_vm.h>
#include <game_mp/g_scr_main_mp.h>
#include <game_mp/g_main_mp.h>
#include <server/sv_game.h>
#include <universal/q_shared.h>

// zombies: SP uses CS 0x5ed, an info string keyed by client number. Carry that
// format in KB's existing naked-vision slot (1550), with a matching CG reader.
// No collision with KB's model / head-icon configstring ranges.
static const int SP_VISION_CONFIGSTRING = 1550;

static void SP_WriteVision(bool all, int clientNum)
{
    char info[1024];
    SV_GetConfigstring(SP_VISION_CONFIGSTRING, info, sizeof(info));
    int duration = 1000;
    int count = Scr_GetNumParam(SCRIPTINSTANCE_SERVER);
    if (count != 1)
    {
        if (count != 2)
        {
            Scr_Error("USAGE: VisionSetNaked( <visionset name>, <transition time> )\n", false);
            return;
        }
        duration = (int)((float)Scr_GetFloat(1, SCRIPTINSTANCE_SERVER) * 1000.0f + 9.313226e-10f);
    }
    const char *name = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    char value[1024];
    Com_sprintf(value, sizeof(value), "\"%s\" %i", name, duration);
    const int end = all ? level.maxclients : clientNum + 1;
    for (int i = all ? 0 : clientNum; i < end; ++i)
        Info_SetValueForKey(info, va("%i", i), value);
    SV_SetConfigstring(SP_VISION_CONFIGSTRING, info);
}

void G_SP_VisionSetNaked()
{
    // zombies: SP 0x007FF320 updates every client's entry.
    SP_WriteVision(true, 0);
}

static int SP_VisionClient(scr_entref_t entref, const char *function)
{
    gentity_s *ent = GetEntity(entref);
    if (!ent->r.inuse || !ent->client)
        Scr_Error(va("%s() called on an invalid client entity.\n", function), false);
    return ent->s.number;
}

void G_SP_PlayerVisionSetNaked(scr_entref_t entref)
{
    // zombies: SP 0x007FF420 replaces only the addressed client's entry.
    SP_WriteVision(false, SP_VisionClient(entref, "visionsetnaked"));
}

void G_SP_GetVisionSetNaked(scr_entref_t entref)
{
    // zombies: SP 0x007FF550 truncates at the first non-name character;
    // 0x006259A0 accepts exactly ASCII letters, digits, '_' and '-'.
    int clientNum = SP_VisionClient(entref, "getvisionsetnaked");
    char info[1024], name[1024];
    SV_GetConfigstring(SP_VISION_CONFIGSTRING, info, sizeof(info));
    I_strncpyz(name, Info_ValueForKey(info, va("%i", clientNum)), sizeof(name));
    for (char *p = name; *p; ++p)
        if (!((*p >= 'a' && *p <= 'z') || (*p >= 'A' && *p <= 'Z')
            || (*p >= '0' && *p <= '9') || *p == '_' || *p == '-'))
        {
            *p = 0;
            break;
        }
    Scr_AddString(name, SCRIPTINSTANCE_SERVER);
}
