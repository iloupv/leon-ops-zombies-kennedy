#include "splitscreen.h"
#include <universal/assertive.h>
#include <ui/ui_main.h>
#include <qcommon/com_clients.h>
#include "cl_main.h"

// This is used as a generic "Return 1" function
int __cdecl CL_LocalClient_GetActiveCount()
{
    return 1;
}

char __cdecl CL_LocalClient_IsActive(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\splitscreen.cpp",
                    53,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum >= 0 && localClientNum < 1)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return 1;
}

char __cdecl CL_LocalClient_IsFirstActive(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\splitscreen.cpp",
                    64,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum >= 0 && localClientNum < 1)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return 1;
}

char __cdecl CL_AllLocalClientsDisconnected()
{
    int client; // [esp+0h] [ebp-4h]

    if ( UI_IsFullscreen(0) )
        return 1;
    for ( client = 0; client < 1; ++client )
    {
        if (CL_LocalClient_IsActive(client) && clientUIActives[client].connectionState >= CA_CONNECTING)
            return 0;
    }
    return 1;
}

char __cdecl CL_AnyLocalClientStateActive()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if (CL_LocalClient_IsActive(localClientNum) && clientUIActives[localClientNum].connectionState == CA_ACTIVE)
            return 1;
    }
    return 0;
}

char __cdecl CL_AnyLocalClientChallenging()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        if (CL_LocalClient_IsActive(localClientNum) && clientUIActives[localClientNum].connectionState == CA_CHALLENGING)
            return 1;
    }
    return 0;
}

void __cdecl CL_LocalClients_SetAllUsedActive()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
    {
        Com_LocalClient_IsBeingUsed(localClientNum);
        //BLOPS_NULLSUB();
    }
}

int __cdecl CL_LocalActiveIndexFromClientNum(int localClientNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\splitscreen.cpp",
                    394,
                    0,
                    "%s\n\t(localClientNum) = %i",
                    "(localClientNum >= 0 && localClientNum < 1)",
                    localClientNum) )
    {
        __debugbreak();
    }
    return 0;
}

extern const dvar_t *name; // BO1TODO: remove this extern hack after live/ folder done
const char *__cdecl CL_ControllerIndex_GetUsername()
{
    return name->current.string;
}

