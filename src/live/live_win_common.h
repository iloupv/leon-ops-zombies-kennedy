#pragma once


char __cdecl Live_Frame_MP(int localControllerIndex);
void __cdecl Live_UpdateUiPopup(const char *popupname, bool reset);
void __cdecl Live_ShowPlayerProfile(int localClientNum, unsigned __int64 playerXUID, char *playerName);
void __cdecl SocketRouter_EmergencyFrame();
