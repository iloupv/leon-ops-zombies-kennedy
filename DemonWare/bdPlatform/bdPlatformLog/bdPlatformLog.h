#pragma once

enum bdLogMessageType : __int32
{                                       // XREF: ?publish@bdLogSubscriber@@UBEXW4bdLogMessageType@@QBD11I1@Z/r
                                        // ?logMessage@bdLogSubscriber@@QBEXW4bdLogMessageType@@QBD11I1@Z/r ...
    BD_LOG_INFO    = 0x0,
    BD_LOG_WARNING = 0x1,
    BD_LOG_ERROR   = 0x2,
};

void bdLogMessage(
    enum bdLogMessageType a1,
    const char *a2,
    const char *a3,
    const char *a4,
    const char *a5,
    unsigned int a6,
    const char *a7,
    ...);