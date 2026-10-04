#pragma once
#include <client/cl_main.h>

struct uiClientState_s // sizeof=0x808
{                                       // XREF: uiClientState_t/r
    connstate_t connState;              // XREF: UI_DrawConnectScreen(int)+1AE/r
    int connectPacketCount;             // XREF: UI_DrawConnectScreen(int)+201/r
    char servername[1024];              // XREF: UI_DrawConnectScreen(int)+174/o
    char messageString[1024];
};

void __cdecl CL_GetClientState(int localClientNum, uiClientState_s *state);
bool __cdecl CL_AllowPopup(int localClientNum);
void __cdecl Key_KeynumToStringBuf(int keynum, char *buf, int buflen);
int __cdecl CL_GetClientName(int localClientNum, int index, char *buf, int size, bool addClanName);
int __cdecl CL_GetClientClanName(int localClientNum, int index, char *buf, int size);
int __cdecl CL_ShutdownUI();
void __cdecl CL_InitUI();
