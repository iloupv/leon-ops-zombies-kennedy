#include "cscr_tempmemory.h"

thread_local HunkUser *g_user;

void __cdecl TempMemoryReset(HunkUser *user)
{
    g_user = user;
}

char *__cdecl TempMallocAlignStrict(int len)
{
    return (char *)Hunk_UserAlloc(g_user, len, 1, 0);
}

void __cdecl TempMemorySetPos(char *pos)
{
    Hunk_UserSetPos(g_user, (unsigned char*)pos);
}

