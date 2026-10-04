#include "cg_animtree_mp.h"
#include <clientscript/cscr_main.h>
#include <clientscript/cscr_memorytree.h>
#include <client_mp/cl_cgame_mp.h>
#include <clientscript/cscr_animtree.h>
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include "cg_local_mp.h"
#include <qcommon/dobj_management.h>

void __cdecl CGScr_LoadAnimTrees()
{
    signed int i; // [esp+14h] [ebp-8h]
    char *string; // [esp+18h] [ebp-4h]

    Scr_BeginLoadAnimTrees(SCRIPTINSTANCE_SERVER, 0);
    for ( i = 3244; i < MAX_CONFIGSTRINGS; ++i )
    {
        string = CL_GetConfigString(i);
        if ( *string )
        {
            if ( strcmp("multiplayer", string) )
            {
                Scr_ClientUsingTree(SCRIPTINSTANCE_SERVER, string);
                Scr_PrecacheAnimTrees(SCRIPTINSTANCE_SERVER, (void *(__cdecl *)(int))Hunk_AllocXAnimCreate, 0, 1);
            }
        }
    }
}

unsigned __int8 *__cdecl Hunk_AllocXAnimCreate(unsigned int size)
{
    return Hunk_AllocLow(size, "XAnimCreateAnims", 13);
}

void __cdecl CG_FreeClientDObjInfo(int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < com_maxclients->current.integer; ++i )
        CG_SafeDObjFree(localClientNum, i);
}

static int cg_entityLastTypeHi[MAX_CL_ENTNUM - 1537]; // mod (L43): cg_s::iEntityLastType for 1537+
static XModel *cg_entityLastXModelHi[MAX_CL_ENTNUM - 1537];

void __cdecl CG_SetDObjInfo(int localClientNum, int iEntNum, int iEntType, XModel *pXModel)
{
    cg_s *cgameGlob; // eax

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( iEntNum > 1536 ) // mod (L43): networked numbers 1537+ (bo1_mod_netents); cg_s keeps its retail [1536]
    {
        cg_entityLastTypeHi[iEntNum - 1537] = iEntType;
        cg_entityLastXModelHi[iEntNum - 1537] = pXModel;
        return;
    }
    cgameGlob->iEntityLastType[iEntNum] = iEntType;
    cgameGlob->pEntityLastXModel[iEntNum] = pXModel;
}

bool __cdecl CG_CheckDObjInfoMatches(int localClientNum, int iEntNum, int iEntType, XModel *pXModel)
{
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( iEntNum > 1536 ) // mod (L43)
        return cg_entityLastTypeHi[iEntNum - 1537] == iEntType && cg_entityLastXModelHi[iEntNum - 1537] == pXModel;
    return cgameGlob->iEntityLastType[iEntNum] == iEntType && cgameGlob->pEntityLastXModel[iEntNum] == pXModel;
}

void __cdecl CG_SafeDObjFree(int localClientNum, int entIndex)
{
    centity_s *cent; // [esp+8h] [ebp-4h]

    Com_SafeClientDObjFree(entIndex, localClientNum);
    CG_SetDObjInfo(localClientNum, entIndex, 0, 0);
    cent = CG_GetEntity(localClientNum, entIndex);
    if ( cent->tree )
    {
        if ( entIndex >= 32 )
        {
            XAnimFreeTree(cent->tree, (void (__cdecl *)(void *, int, scriptInstance_t))MT_Free, SCRIPTINSTANCE_SERVER);
            cent->tree = 0;
        }
    }
}

void __cdecl CG_FreeEntityDObjInfo(int localClientNum)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 32; i < (1 << g_entNumBits); i = EntNum_NextNet(i) ) // mod (L43): networked numbers (retail < 1024)
        CG_SafeDObjFree(localClientNum, i);
    memset(cg_entityLastTypeHi, 0, sizeof(cg_entityLastTypeHi)); // mod (L43)
    memset(cg_entityLastXModelHi, 0, sizeof(cg_entityLastXModelHi));
}

