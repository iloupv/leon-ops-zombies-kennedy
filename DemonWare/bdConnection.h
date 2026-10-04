#pragma once

#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include "bdCore/bdSocket/bdAddr.h"
#include <DemonWare/bdCore/bdContainers/bdFastArray.h>

struct bdConnectionListener; // dont even see a def for this in IDA

struct bdConnection : bdReferencable // sizeof=0x24
{                                       // XREF: ?receiveAll@bdConnectionStore@@QAE_NXZ/r
    enum Status : __int32
    {                                       // XREF: bdLobbyConnection::Status/r
                                            // bdConnection/r
        BD_NOT_CONNECTED = 0x0,
        BD_CONNECTING    = 0x1,
        BD_CONNECTED     = 0x2,
        BD_DISCONNECTING = 0x3,
        BD_DISCONNECTED  = 0x4,
    };
    bdReference<bdCommonAddr> m_addr;
    bdReference<bdAddrHandle> m_addrHandle;
    bdFastArray<bdConnectionListener *> m_listeners;
    unsigned int m_maxTransmissionRate;
    bdConnection::Status m_status;
};