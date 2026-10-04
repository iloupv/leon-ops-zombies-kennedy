#include "ui_commands.h"
#include <universal/dvar.h>
#include <client_mp/cl_main_mp.h>
#include <win32/win_gamerprofile.h>

void __cdecl UI_SetClanName_f()
{
    char *String; // eax

    String = (char *)Dvar_GetString("ui_clantag_new");
    Dvar_SetStringByName("clanName", String);
    Dvar_SetStringByName("ui_clantag_new", (char *)"");
    CL_SanitizeClanName();
    GamerProfile_UpdateProfileFromDvars(0, PROFILE_WRITE_IF_CHANGED);
    dvar_modifiedFlags |= 2u;
}

