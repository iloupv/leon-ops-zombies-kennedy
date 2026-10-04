#include "com_clients.h"

#include <cstring>
#include <universal/com_memory.h>
#include <client/cl_main.h>
#include <client_mp/cl_main_mp.h>
#include <cgame_mp/cg_local_mp.h>

ClientGameState clientGameStates[1];

void __cdecl Com_InitClientGameStates()
{
    int client; // [esp+0h] [ebp-4h]

    clientGameStates[0].flags = 0;
    clientGameStates[0].localClientNum = 0;
    clientGameStates[0].controllerIndex = 0;
    clientGameStates[0].uiContextIndex = 0;
    clientGameStates[0].networkID = NS_CLIENT1;
    for (client = 0; client < 1; ++client)
    {
        clientGameStates[client].localClientNum = client;
        clientGameStates[client].networkID = (netsrc_t)client;
    }
}

void __cdecl Com_LocalClients_CompressClients()
{
    int testIndex; // [esp+60h] [ebp-18h]
    int swapIndex; // [esp+64h] [ebp-14h]
    int localClientNum; // [esp+68h] [ebp-10h]
    int activeIndex; // [esp+6Ch] [ebp-Ch]
    int cgsIndex; // [esp+70h] [ebp-8h]
    int lastUsedIndex; // [esp+74h] [ebp-4h]

    lastUsedIndex = 0;
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        if (Com_LocalClient_IsBeingUsed(cgsIndex))
        {
            lastUsedIndex = cgsIndex;
        }
        else
        {
            for (activeIndex = cgsIndex + 1; activeIndex < 1; ++activeIndex)
            {
                if (Com_LocalClient_IsBeingUsed(activeIndex))
                {
                    SwapClients(cgsIndex, activeIndex);
                    lastUsedIndex = cgsIndex;
                    break;
                }
            }
        }
    }
    for (localClientNum = 0; localClientNum <= lastUsedIndex; ++localClientNum)
    {
        if (!Com_LocalClient_IsBeingUsed(localClientNum)
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
                108,
                0,
                "%s",
                "Com_LocalClient_IsBeingUsed(cgsIndex)"))
        {
            __debugbreak();
        }
        swapIndex = localClientNum;
        for (testIndex = localClientNum + 1; testIndex <= lastUsedIndex; ++testIndex)
        {
            if (clientGameStates[localClientNum].controllerIndex > clientGameStates[testIndex].controllerIndex)
                swapIndex = testIndex;
        }
        if (swapIndex != localClientNum)
            SwapClients(localClientNum, swapIndex);
    }
}

void __cdecl SwapClients(int clientA, int clientB)
{
    clientUIActive_t *v2; // eax
    clientUIActive_t *v3; // eax
    clientUIActive_t *v4; // ecx
    ClientGameState *v5; // ecx
    ClientGameState *v6; // edx
    int clientGamestateTemp; // [esp+0h] [ebp-30h]
    int clientGamestateTemp_4; // [esp+4h] [ebp-2Ch]
    int clientGamestateTemp_8; // [esp+8h] [ebp-28h]
    int clientGamestateTemp_12; // [esp+Ch] [ebp-24h]
    netsrc_t clientGamestateTemp_16; // [esp+10h] [ebp-20h]
    int uiActiveTemp; // [esp+14h] [ebp-1Ch]
    int uiActiveTemp_4; // [esp+18h] [ebp-18h]
    connstate_t uiActiveTemp_8; // [esp+1Ch] [ebp-14h]
    int uiActiveTemp_12; // [esp+20h] [ebp-10h]
    unsigned __int8 *clientConnectionTemp; // [esp+24h] [ebp-Ch]
    LargeLocal clientConnectionTemp_large_local(720712); // [esp+28h] [ebp-8h] BYREF

    //LargeLocal::LargeLocal(&clientConnectionTemp_large_local, 720712);
    //clientConnectionTemp = LargeLocal::GetBuf(&clientConnectionTemp_large_local);
    clientConnectionTemp = clientConnectionTemp_large_local.GetBuf();
    v2 = &clientUIActives[clientA];
    uiActiveTemp = v2->flags;
    uiActiveTemp_4 = v2->keyCatchers;
    uiActiveTemp_8 = v2->connectionState;
    uiActiveTemp_12 = v2->nextScrollTime;
    v3 = &clientUIActives[clientB];
    v4 = &clientUIActives[clientA];
    v4->flags = v3->flags;
    v4->keyCatchers = v3->keyCatchers;
    v4->connectionState = v3->connectionState;
    v4->nextScrollTime = v3->nextScrollTime;
    v3->flags = uiActiveTemp;
    v3->keyCatchers = uiActiveTemp_4;
    v3->connectionState = uiActiveTemp_8;
    v3->nextScrollTime = uiActiveTemp_12;
    v5 = &clientGameStates[clientA];
    clientGamestateTemp = v5->flags;
    clientGamestateTemp_4 = v5->localClientNum;
    clientGamestateTemp_8 = v5->controllerIndex;
    clientGamestateTemp_12 = v5->uiContextIndex;
    clientGamestateTemp_16 = v5->networkID;
    v6 = &clientGameStates[clientB];
    v5->flags = v6->flags;
    v5->localClientNum = v6->localClientNum;
    v5->controllerIndex = v6->controllerIndex;
    v5->uiContextIndex = v6->uiContextIndex;
    v5->networkID = v6->networkID;
    v6->flags = clientGamestateTemp;
    v6->localClientNum = clientGamestateTemp_4;
    v6->controllerIndex = clientGamestateTemp_8;
    v6->uiContextIndex = clientGamestateTemp_12;
    v6->networkID = clientGamestateTemp_16;
    clientGameStates[clientA].localClientNum = clientA;
    clientGameStates[clientB].localClientNum = clientB;
    if (clientConnections)
    {
        memcpy(clientConnectionTemp, (unsigned __int8 *)&clientConnections[clientA], 0xAFF48u);
        memcpy(
            (unsigned __int8 *)&clientConnections[clientA],
            (unsigned __int8 *)&clientConnections[clientB],
            sizeof(clientConnection_t));
        memcpy((unsigned __int8 *)&clientConnections[clientB], clientConnectionTemp, sizeof(clientConnection_t));
    }
    //LargeLocal::~LargeLocal(&clientConnectionTemp_large_local);
}

void __cdecl Com_LocalClients_AssignUIContextsForInGame()
{
    int cgsIndex; // [esp+0h] [ebp-8h]
    int contextIndex; // [esp+4h] [ebp-4h]

    contextIndex = 0;
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
        clientGameStates[cgsIndex].uiContextIndex = contextIndex++;
}

void __cdecl Com_LocalClients_AssignUIContextsForFrontEnd()
{
    int cgsIndex; // [esp+0h] [ebp-8h]
    int contextIndex; // [esp+4h] [ebp-4h]

    contextIndex = 0;
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        clientGameStates[cgsIndex].uiContextIndex = 0;
        ++contextIndex;
    }
}

int __cdecl Com_LocalClient_GetUIContextIndex(int localClientNum)
{
    int cgsIndex; // [esp+0h] [ebp-4h]

    if (localClientNum
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
            162,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1))
    {
        __debugbreak();
    }
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        if (clientGameStates[cgsIndex].localClientNum == localClientNum)
            return clientGameStates[cgsIndex].uiContextIndex;
    }
    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp", 171, 0, "%s", "0"))
        __debugbreak();
    return -1;
}

int __cdecl Com_LocalClient_GetControllerIndex(int localClientNum)
{
    int cgsIndex; // [esp+0h] [ebp-4h]

    if (localClientNum
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
            177,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1))
    {
        __debugbreak();
    }
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        if (clientGameStates[cgsIndex].localClientNum == localClientNum)
            return clientGameStates[cgsIndex].controllerIndex;
    }
    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp", 186, 0, "%s", "0"))
        __debugbreak();
    return -1;
}

netsrc_t __cdecl Com_LocalClient_GetNetworkID(int localClientNum)
{
    int cgsIndex; // [esp+0h] [ebp-4h]

    if (localClientNum
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
            192,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1))
    {
        __debugbreak();
    }
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        if (clientGameStates[cgsIndex].localClientNum == localClientNum)
            return clientGameStates[cgsIndex].networkID;
    }
    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp", 201, 0, "%s", "0"))
        __debugbreak();
    return NS_PACKET;
}

void __cdecl Com_LocalClient_SetControllerIndex(int localClientNum, int controllerIndex)
{
    if (localClientNum
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
            208,
            0,
            "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
            localClientNum,
            1))
    {
        __debugbreak();
    }
    clientGameStates[localClientNum].controllerIndex = controllerIndex;
}

void __cdecl Com_LocalClient_SetPrimary(int localClientNum, bool primary)
{
    int v2; // eax

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
                    239,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( primary )
    {
        Com_LocalClient_ClearAllPrimary();
        v2 = clientGameStates[localClientNum].flags | 2;
    }
    else
    {
        v2 = clientGameStates[localClientNum].flags & 0xFFFFFFFD;
    }
    clientGameStates[localClientNum].flags = v2;
}

int Com_LocalClient_ClearAllPrimary()
{
    int result; // eax
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 1; ++i )
    {
        clientGameStates[i].flags &= ~2u;
        result = i + 1;
    }
    return result;
}

int __cdecl Com_LocalClients_GetPrimary()
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < MAX_LOCAL_CLIENTS; ++i )
    {
        if ( Com_LocalClient_IsPrimary(i) )
            return i;
    }

    if ( !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
                    261,
                    0,
                    "%s\n\tCom_LocalClient_IsPrimarys",
                    "0",
                    "Someone asked for the primary client and it was not set\n") )
        __debugbreak();

    return -1;
}

bool __cdecl Com_LocalClient_IsPrimary(int localClientNum)
{
    bcassert(localClientNum, MAX_LOCAL_CLIENTS);
    return (clientGameStates[localClientNum].flags & 2) != 0;
}

bool __cdecl Com_LocalClient_IsBeingUsed(int localClientNum)
{
    bcassert(localClientNum, MAX_LOCAL_CLIENTS);
    return (clientGameStates[localClientNum].flags & 1) != 0;
}

void __cdecl Com_LocalClient_SetBeingUsed(int localClientNum, bool beingUsed)
{
    int v2; // edx

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
                    279,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( beingUsed )
        v2 = clientGameStates[localClientNum].flags | 1;
    else
        v2 = clientGameStates[localClientNum].flags & 0xFFFFFFFE;
    clientGameStates[localClientNum].flags = v2;
}

int __cdecl Com_LocalClients_GetUsedControllerCount()
{
    int i; // [esp+0h] [ebp-8h]
    int count; // [esp+4h] [ebp-4h]

    i = 0;
    count = 0;
    while ( i < 1 )
    {
        if ( Com_LocalClient_IsBeingUsed(i) )
            ++count;
        ++i;
    }
    return count;
}

int __cdecl Com_ControllerIndex_GetLocalClientNum(int controllerIndex)
{
    int cgsIndex; // [esp+0h] [ebp-4h]

    if (controllerIndex
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
            358,
            0,
            "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
            controllerIndex,
            1))
    {
        __debugbreak();
    }
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        if (clientGameStates[cgsIndex].controllerIndex == controllerIndex)
            return clientGameStates[cgsIndex].localClientNum;
    }
    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp", 369, 0, "%s", "0"))
        __debugbreak();
    return -1;
}

int __cdecl Com_ControllerIndex_GetUIContextIndex(int controllerIndex)
{
    int cgsIndex; // [esp+0h] [ebp-4h]

    if (controllerIndex
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp",
            395,
            0,
            "controllerIndex doesn't index MAX_GPAD_COUNT\n\t%i not in [0, %i)",
            controllerIndex,
            1))
    {
        __debugbreak();
    }
    for (cgsIndex = 0; cgsIndex < 1; ++cgsIndex)
    {
        if (clientGameStates[cgsIndex].controllerIndex == controllerIndex)
            return clientGameStates[cgsIndex].uiContextIndex;
    }
    if (!Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\com_clients.cpp", 405, 0, "%s", "0"))
        __debugbreak();
    return -1;
}

int __cdecl Com_ControllerIndexes_GetPrimary()
{
    // There is some extra logic here for IsDedicatedServer() but I believe it will always return 0 unless we enable splitscreen (BO1TODO)
    return 0;
}

