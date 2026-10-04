#pragma once

struct clientActive_t;
struct msg_t;
struct clSnapshot_t;


void __cdecl CL_ParsePacketMatchState(
                clientActive_t *cl,
                msg_t *msg,
                int time,
                clSnapshot_t *oldframe,
                clSnapshot_t *newframe);
