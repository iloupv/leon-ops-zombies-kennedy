#include "cl_ui_mp.h"
#include "cl_main_mp.h"
#include <cgame_mp/cg_main_mp.h>
#include <client/client.h>
#include <client/cl_keys.h>
#include <gfx_d3d/r_rendercmds.h>

void __cdecl CL_GetClientState(int localClientNum, uiClientState_s *state)
{
    clientConnection_t *clc; // [esp+4h] [ebp-4h]

    CL_GetLocalClientGlobals(localClientNum);
    clc = CL_GetLocalClientConnection(localClientNum);
    state->connectPacketCount = clc->connectPacketCount;
    state->connState = CL_GetLocalClientConnectionState(localClientNum);
    I_strncpyz(state->servername, cls.servername, 1024);
    I_strncpyz(state->messageString, clc->serverMessage, 1024);
}

bool __cdecl CL_AllowPopup(int localClientNum)
{
    connstate_t connstate; // [esp+0h] [ebp-8h]

    connstate = CL_GetLocalClientConnectionState(localClientNum);
    if ( CL_GetLocalClientConnection(localClientNum)->demoplaying )
        return 0;
    if ( Demo_IsPlaying() )
        return 0;
    return connstate == CA_ACTIVE;
}

void __cdecl Key_KeynumToStringBuf(int keynum, char *buf, int buflen)
{
    const char *v3; // eax

    v3 = Key_KeynumToString(keynum, 1);
    I_strncpyz(buf, v3, buflen);
}

int __cdecl CL_GetClientName(int localClientNum, int index, char *buf, int size, bool addClanName)
{
    const char *v6; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    const clientState_s *pState; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    *buf = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( !LocalClientGlobals->snap.valid )
        return 0;
    for ( i = 0; i < LocalClientGlobals->snap.numClients; ++i )
    {
        pState = &LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                                                                                         & 0x7FF];
        if ( pState->clientIndex == index )
        {
            v6 = CS_DisplayName(pState, (addClanName ? 2 : 0) | 1);
            I_strncpyz(buf, v6, size);
            I_CleanStr(buf);
            return 1;
        }
    }
    return 0;
}

int __cdecl CL_GetClientClanName(int localClientNum, int index, char *buf, int size)
{
    const char *v5; // eax
    clientActive_t *LocalClientGlobals; // [esp+0h] [ebp-Ch]
    const clientState_s *pState; // [esp+4h] [ebp-8h]
    int i; // [esp+8h] [ebp-4h]

    *buf = 0;
    LocalClientGlobals = CL_GetLocalClientGlobals(localClientNum);
    if ( !LocalClientGlobals->snap.valid )
        return 0;
    for ( i = 0; i < LocalClientGlobals->snap.numClients; ++i )
    {
        pState = &LocalClientGlobals->parseClients[((_WORD)i + (unsigned __int16)LocalClientGlobals->snap.parseClientsNum)
                                                                                         & 0x7FF];
        if ( pState->clientIndex == index )
        {
            v5 = CS_DisplayName(pState, 2);
            I_strncpyz(buf, v5, size);
            I_CleanStr(buf);
            return 1;
        }
    }
    return 0;
}

int __cdecl CL_ShutdownUI()
{
    int localClientNum; // [esp+0h] [ebp-4h]

    if ( !cls.uiStarted )
        return 0;
    Key_RemoveCatcher(0, -17);
    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        UI_Shutdown(localClientNum);
    cls.uiStarted = 0;
    return 1;
}

void __cdecl CL_InitUI()
{
    int localClientNum; // [esp+0h] [ebp-8h]
    int remoteScreenUpdateNesting; // [esp+4h] [ebp-4h]

    UI_InitOnceForAllClients();
    for ( localClientNum = 0; localClientNum < 1; ++localClientNum )
        UI_Init(localClientNum);
    cls.uiStarted = 1;
    remoteScreenUpdateNesting = R_PopRemoteScreenUpdate();
    R_PushRemoteScreenUpdate(remoteScreenUpdateNesting);
}

