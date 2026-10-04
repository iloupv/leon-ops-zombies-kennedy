#pragma once
#include "msg_mp.h"

struct ClientGameState // sizeof=0x14
{                                       // XREF: .data:ClientGameState * clientGameStates/r
    int flags;                          // XREF: Com_InitClientGameStates(void)+6/w
    int localClientNum;                 // XREF: Com_InitClientGameStates(void)+B/w
    int controllerIndex;                // XREF: Com_InitClientGameStates(void)+10/w
    int uiContextIndex;                 // XREF: Com_InitClientGameStates(void)+15/w
    netsrc_t networkID;                 // XREF: Com_InitClientGameStates(void)+1A/w
};

void __cdecl Com_InitClientGameStates();
void __cdecl Com_LocalClients_CompressClients();
void __cdecl SwapClients(int clientA, int clientB);
void __cdecl Com_LocalClients_AssignUIContextsForInGame();
void __cdecl Com_LocalClients_AssignUIContextsForFrontEnd();
int __cdecl Com_LocalClient_GetUIContextIndex(int localClientNum);
int __cdecl Com_LocalClient_GetControllerIndex(int localClientNum);
netsrc_t __cdecl Com_LocalClient_GetNetworkID(int localClientNum);
void __cdecl Com_LocalClient_SetControllerIndex(int localClientNum, int controllerIndex);
void __cdecl Com_LocalClient_SetPrimary(int localClientNum, bool primary);
int Com_LocalClient_ClearAllPrimary();
int __cdecl Com_LocalClients_GetPrimary();
bool __cdecl Com_LocalClient_IsPrimary(int localClientNum);
bool __cdecl Com_LocalClient_IsBeingUsed(int localClientNum);
void __cdecl Com_LocalClient_SetBeingUsed(int localClientNum, bool beingUsed);
int __cdecl Com_LocalClients_GetUsedControllerCount();
int __cdecl Com_ControllerIndex_GetLocalClientNum(int controllerIndex);
int __cdecl Com_ControllerIndex_GetUIContextIndex(int controllerIndex);
int __cdecl Com_ControllerIndexes_GetPrimary();
