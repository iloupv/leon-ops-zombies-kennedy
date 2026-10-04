#include "live_clans.h"
#include <win32/win_gamerprofile.h>

char *__cdecl Clan_GetName(int controllerIndex)
{
    return GamerProfile_GetProfileSettings(controllerIndex)->clanPrefix;
}

