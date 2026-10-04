#pragma once
#include <qcommon/net_chan_mp.h>

struct reliableMsg // sizeof=0x34
{                                       // XREF: reliableClient/r
    bool active;
    unsigned __int8 sequenceNum;
    // padding byte
    // padding byte
    msg_t msg;
};

struct reliableClient // sizeof=0x81C
{                                       // XREF: .data:s_reliableClientMessages/r
    netadr_t addr;
    int lastSendTime;
    int firstSendTime;
    unsigned __int8 nextSequence;
    // padding byte
    // padding byte
    // padding byte
    int bufferUsed;                     // XREF: RMsg_FindSlotForAddr+98/r
    int bytesAckd;
    unsigned __int8 reliableBuf[1000];
    reliableMsg msgs[20];
};

void __cdecl RMsg_DropAllPacketsForClient(unsigned int clientSlot);
int __cdecl RMsg_FindSlotForAddr(netadr_t *to);
void __cdecl RMsg_Init();
void __cdecl RMsg_SendMessages();
void __cdecl RMsg_AddRMsgsToPacket(unsigned int clientSlot, msg_t *outMsg);
int __cdecl RMsg_FindMessageSlot(unsigned int clientSlot, unsigned __int8 sequenceNum);
int __cdecl RMsg_FindOldestSeq(unsigned int clientSlot);
void __cdecl RMsg_AckSequence(netadr_t *from, unsigned __int8 sequenceNum);
void __cdecl RMsg_MarkPacketReceived(netadr_t *from, unsigned __int8 sequenceNum);
void __cdecl RMsg_AckMsgSlot(reliableClient *client, unsigned int clientSlot, unsigned int msgSlot);
int __cdecl RMsg_CountOutstandingMessages(unsigned int clientSlot);
