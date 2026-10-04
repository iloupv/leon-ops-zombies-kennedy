#include "cscr_instance.h"
#include "cscr_variable.h"
#include "cscr_stringlist.h"

scrVarDebugPub_t gScrVarDebugPubBuff[2];
scrStringDebugGlob_t gScrStringDebugGlobBuff[2];

void __cdecl Scr_ClearScrVarDebugPub(scriptInstance_t inst)
{
    gScrVarDebugPub[inst] = 0;
}

void __cdecl Scr_ResetScrVarDebugPub(scriptInstance_t inst)
{
    gScrVarDebugPub[inst] = &gScrVarDebugPubBuff[inst];
}

void __cdecl Scr_ClearScrStringDebugGlob(scriptInstance_t inst)
{
    gScrStringDebugGlob[inst] = 0;
}

void __cdecl Scr_ResetScrStringDebugGlob(scriptInstance_t inst)
{
    gScrStringDebugGlob[inst] = &gScrStringDebugGlobBuff[inst];
}

