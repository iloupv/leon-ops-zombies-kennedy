#include "cl_parse.h"
#include <client_mp/cl_main_mp.h>
#include <qcommon/msg.h>

void __cdecl CL_ParsePacketMatchState(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *oldframe,
                clSnapshot_t *newframe)
{
    MatchState *newMatchState; // [esp+0h] [ebp-8h]
    MatchState *oldMatchState; // [esp+4h] [ebp-4h]
    int savedregs; // [esp+8h] [ebp+0h] BYREF

    oldMatchState = 0;
    if ( oldframe )
        oldMatchState = &cl->parseMatchStates[oldframe->parseMatchStateNum & 0x1F];
    newMatchState = &cl->parseMatchStates[cl->parseMatchStateNum & 0x1F];
    newframe->parseMatchStateNum = cl->parseMatchStateNum;
    MSG_ReadDeltaMatchState(msg, time, oldMatchState, newMatchState);
    ++cl->parseMatchStateNum;
}

