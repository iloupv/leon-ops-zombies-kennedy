#pragma once

int __cdecl Live_CountGuestsInUse();
void __cdecl Live_ShowConnectingDialog();
void __cdecl Live_DemonwareDisconnectCleanup(int localControllerIndex);
bool __cdecl Live_IsPublicOnlineMatch();
bool __cdecl Live_IsWagerMatch();
bool __cdecl Live_IsCombatTrainingMatch();
void __cdecl Live_Init();
