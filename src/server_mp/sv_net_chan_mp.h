#pragma once
#include <server/server.h>

bool __cdecl SV_Netchan_TransmitNextFragment(client_t *client, netchan_t *chan);
void __cdecl SV_Netchan_OutgoingSequenceIncremented(client_t *client, netchan_t *chan);
bool __cdecl SV_Netchan_Transmit(client_t *client, unsigned __int8 *data, int length, bool reliable);
void __cdecl SV_Netchan_AddOOBProfilePacket(int iLength);
