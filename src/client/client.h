#pragma once
#include "cl_main.h"
#include <client_mp/cl_main_mp.h>


inline connstate_t __cdecl CL_GetLocalClientConnectionState(int localClientNum)
{
    iassert(localClientNum == 0);
    return clientUIActives[localClientNum].connectionState;
}

extern clientActive_t *clients;
inline clientActive_t *__cdecl CL_GetLocalClientGlobals(int localClientNum)
{
    ///iassert(clients); // WOW! WEIRD
    iassert(localClientNum == 0);

    return clients;
    //return &clients[localClientNum];
}