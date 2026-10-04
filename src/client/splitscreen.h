#pragma once

int __cdecl CL_LocalClient_GetActiveCount();
char __cdecl CL_LocalClient_IsActive(int localClientNum);
char __cdecl CL_LocalClient_IsFirstActive(int localClientNum);
char __cdecl CL_AllLocalClientsDisconnected();
char __cdecl CL_AnyLocalClientStateActive();
char __cdecl CL_AnyLocalClientChallenging();
void __cdecl CL_LocalClients_SetAllUsedActive();
int __cdecl CL_LocalActiveIndexFromClientNum(int localClientNum);
const char *__cdecl CL_ControllerIndex_GetUsername();
