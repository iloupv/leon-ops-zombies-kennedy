#include "sentient_fields.h"
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <clientscript/cscr_vm.h>
#include "sentient.h"
#include <game_mp/g_spawn_mp.h>
#include <qcommon/common.h>

static void __cdecl SentientScr_SetTeamSP(sentient_s *pSelf, const sentient_fields_s *pField);
static void __cdecl SentientScr_GetTeamSP(sentient_s *pSelf, const sentient_fields_s *pField);

const sentient_fields_s fields_2[] =
{
  { "aiteam", 4, { 4 }, F_INT, SentientScr_SetTeam, SentientScr_GetTeam },
  { "script_owner", 8, { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "threatbias", 12, { 4 }, F_INT, NULL, NULL },
  { "threatbiasgroup", 16, { 4 }, F_INT, &SentientScr_ReadOnly, NULL },
  { "attacker", 48, { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "node", 96, { 4 }, F_PATHNODE, &SentientScr_ReadOnly, NULL },
  { "prevnode", 100, { 4 }, F_PATHNODE, &SentientScr_ReadOnly, NULL },
  { "enemy", 56, { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "scriptenemy", 60, { 4 }, F_ENTHANDLE, &SentientScr_ReadOnly, NULL },
  { "scriptenemytag", 64, { 2 }, F_STRING, &SentientScr_ReadOnly, NULL },
  { "syncedmeleetarget", 52, { 4 }, F_ENTHANDLE, NULL, NULL },
  { "ignoreme", 20, { 1 }, F_BYTE, NULL, NULL },
  { "ignoreall", 21, { 1 }, F_BYTE, NULL, NULL },
  { "ignoreforfriendlyfire", 22, { 1 }, F_BYTE, NULL, NULL },
  { "maxvisibledist", 36, { 4 }, F_FLOAT, NULL, NULL },
  { "attackeraccuracy", 88, { 4 }, F_FLOAT, NULL, NULL },
  { "ignorerandombulletdamage", 92, { 1 }, F_BYTE, NULL, NULL },
  { "turretinvulnerability", 93, { 1 }, F_BYTE, NULL, NULL },
  { "inmeleecharge", 140, { 1 }, F_BYTE, NULL, NULL },
  // zombies: SP +0x4 -> sentient_s::eTeam (SP 0x00a55c78).
  { "team", offsetof(sentient_s, eTeam), { 4 }, F_INT, SentientScr_SetTeamSP, SentientScr_GetTeamSP },
  { NULL, 0, { 0 }, F_INT, NULL, NULL }
};

// zombies: only axis, allies and neutral are writable; dead is getter-only (SP 0x00819180).
static void __cdecl SentientScr_SetTeamSP(sentient_s *pSelf, const sentient_fields_s *pField)
{
    iassert(pSelf);
    const char *pszTeam = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( !I_stricmp(pszTeam, "axis") )
    {
        Sentient_SetTeam(pSelf, TEAM_AXIS);
        return;
    }
    if ( !I_stricmp(pszTeam, "allies") )
    {
        Sentient_SetTeam(pSelf, TEAM_ALLIES);
        return;
    }
    if ( !I_stricmp(pszTeam, "neutral") )
    {
        // SP's neutral value 3 occupies KB's spectator enum slot.
        Sentient_SetTeam(pSelf, (team_t)3);
        return;
    }
    Scr_Error(va("unknown team '%s', should be axis, allies, or neutral\n", pszTeam), 0);
}

// zombies: jump table pushes nothing for 0 and 4 (SP 0x00819210).
static void __cdecl SentientScr_GetTeamSP(sentient_s *pSelf, const sentient_fields_s *pField)
{
    iassert(pSelf);
    switch ( (int)pSelf->eTeam )
    {
        case 1:
            Scr_AddString("axis", SCRIPTINSTANCE_SERVER);
            return;
        case 2:
            Scr_AddString("allies", SCRIPTINSTANCE_SERVER);
            return;
        case 3:
            Scr_AddString("neutral", SCRIPTINSTANCE_SERVER);
            return;
        case 5:
            Scr_AddString("dead", SCRIPTINSTANCE_SERVER);
            return;
        default:
            return;
    }
}

void __cdecl SentientScr_ReadOnly(sentient_s *pSelf, const sentient_fields_s *pField)
{
    const char *v2; // eax

    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 67, 0, "%s", "pSelf") )
        __debugbreak();
    v2 = va("sentient property '%s' is read-only", pField->name);
    Scr_Error(v2, 0);
}

void __cdecl SentientScr_SetTeam(sentient_s *pSelf, const sentient_fields_s *pField)
{
    const char *v1; // eax
    char *pszTeam; // [esp+0h] [ebp-4h]

    if ( Com_IsSPLevel() ) // zombies: SP team field on every SP level
    {
        SentientScr_SetTeamSP(pSelf, pField);
        return;
    }
    if ( !pSelf && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 79, 0, "%s", "pSelf") )
        __debugbreak();
    pszTeam = Scr_GetString(0, SCRIPTINSTANCE_SERVER);
    if ( I_stricmp(pszTeam, "axis") )
    {
        if ( I_stricmp(pszTeam, "allies") )
        {
            if ( I_stricmp(pszTeam, "spectator") )
            {
                if ( I_stricmp(pszTeam, "free") )
                {
                    v1 = va("unknown team '%s', should be axis, allies, or neutral\n", pszTeam);
                    Scr_Error(v1, 0);
                }
                else
                {
                    Sentient_SetTeam(pSelf, TEAM_FREE);
                }
            }
            else
            {
                Sentient_SetTeam(pSelf, TEAM_SPECTATOR);
            }
        }
        else
        {
            Sentient_SetTeam(pSelf, TEAM_ALLIES);
        }
    }
    else
    {
        Sentient_SetTeam(pSelf, TEAM_AXIS);
    }
}

void __cdecl SentientScr_GetTeam(sentient_s *pSelf, const sentient_fields_s *pField)
{
    if ( Com_IsSPLevel() ) // zombies: SP team field on every SP level
    {
        SentientScr_GetTeamSP(pSelf, pField);
        return;
    }
    if ( !pSelf
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 108, 0, "%s", "pSelf") )
    {
        __debugbreak();
    }
    if ( pSelf->eTeam >= (unsigned int)TEAM_NUM_TEAMS
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                    110,
                    0,
                    "%s",
                    "pSelf->eTeam >= TEAM_FREE && pSelf->eTeam < TEAM_NUM_TEAMS") )
    {
        __debugbreak();
    }
    switch ( pSelf->eTeam )
    {
        case TEAM_FREE:
            Scr_AddString("free", SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_AXIS:
            Scr_AddString("axis", SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_ALLIES:
            Scr_AddString("allies", SCRIPTINSTANCE_SERVER);
            break;
        case TEAM_SPECTATOR:
            Scr_AddString("spectator", SCRIPTINSTANCE_SERVER);
            break;
        default:
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                            139,
                            0,
                            "SentientScr_GetTeam: default case (shouldn't happen") )
                __debugbreak();
            break;
    }
}

void __cdecl GScr_AddFieldsForSentient()
{
    const sentient_fields_s *f; // [esp+4h] [ebp-4h]

    for ( f = fields_2; f->name; ++f )
    {
        if ( f->setter == SentientScr_SetTeamSP && !Com_IsSPLevel() )
            continue;
        if ( ((f - fields_2) & 0xC000) != 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                        158,
                        0,
                        "%s",
                        "!((f - fields) & ENTFIELD_MASK)") )
        {
            __debugbreak();
        }
        if ( f - fields_2 != (unsigned __int16)(f - fields_2)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                        159,
                        0,
                        "%s",
                        "(f - fields) == (unsigned short)( f - fields )") )
        {
            __debugbreak();
        }
        Scr_AddClassField(0, (char *)f->name, (unsigned __int16)(f - fields_2) | 0x4000, SCRIPTINSTANCE_SERVER);
    }
}

void __cdecl Scr_SetSentientField(sentient_s *sentient, unsigned int offset)
{
    const sentient_fields_s *f; // [esp+0h] [ebp-4h]

    if ( !sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 175, 0, "%s", "sentient") )
    {
        __debugbreak();
    }
    if ( offset >= ARRAY_COUNT(fields_2) - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                    176,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    f = &fields_2[offset];
    if ( f->setter )
        f->setter(sentient, f);
    else
        GScr_SetGenericField((unsigned __int8 *)sentient, f->type, f->ofs, 0);
}

void __cdecl Scr_GetSentientField(sentient_s *sentient, unsigned int offset)
{
    const sentient_fields_s *f; // [esp+0h] [ebp-4h]

    if ( !sentient
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp", 199, 0, "%s", "sentient") )
    {
        __debugbreak();
    }
    if ( offset >= ARRAY_COUNT(fields_2) - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\game\\sentient_fields.cpp",
                    200,
                    0,
                    "%s",
                    "(unsigned)offset < ARRAY_COUNT( fields ) - 1") )
    {
        __debugbreak();
    }
    f = &fields_2[offset];
    if ( f->getter )
        f->getter(sentient, f);
    else
        GScr_GetGenericField((unsigned __int8 *)sentient, f->type, f->ofs, 0);
}

