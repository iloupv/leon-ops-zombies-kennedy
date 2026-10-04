#pragma once
#include <qcommon/net_chan_mp.h>

void __cdecl CL_Netchan_TransmitNextFragment(netchan_t *chan);
void __cdecl CL_Netchan_Transmit(netchan_t *chan, unsigned __int8 *data, int length);
void __cdecl CL_Netchan_AddOOBProfilePacket(int localClientNum, int iLength);
void __cdecl CL_Netchan_UpdateProfileStats(int localClientNum);
void __cdecl CL_Netchan_PrintProfileStats(int localClientNum, int bPrintToConsole);
void __cdecl CL_ProfDraw(int y, char *pszString);
