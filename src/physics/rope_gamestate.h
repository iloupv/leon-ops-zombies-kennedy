#pragma once
#include <qcommon/msg_mp.h>

int __cdecl GetEntityIndex(const unsigned __int16 *targetname);
void __cdecl Rope_WriteGameState(msg_t *msg);
void __cdecl Rope_ReadGameState(msg_t *msg, int localClientNum);
