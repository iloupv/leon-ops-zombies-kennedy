#include "bdPlatformLog.h"
#include <universal/assertive.h>


void bdLogMessage(
    enum bdLogMessageType a1,
    const char *a2,
    const char *a3,
    const char *a4,
    const char *a5,
    unsigned int a6,
    const char *a7,
    ...)
{
    iassert(0); // BADTODO
    //unsigned int v7; // [esp+4h] [ebp-224h]
    //const char *v8; // [esp+14h] [ebp-214h]
    //unsigned __int8 dst[256]; // [esp+18h] [ebp-210h] BYREF
    //int v10; // [esp+118h] [ebp-110h]
    //bdLinkable *i; // [esp+11Ch] [ebp-10Ch]
    //char buf[256]; // [esp+120h] [ebp-108h] BYREF
    //char *ap; // [esp+224h] [ebp-4h]
    //va_list va; // [esp+24Ch] [ebp+24h] BYREF
    //
    //va_start(va, a7);
    //if (g_logSubscriberList)
    //{
    //    memset(dst, 0, sizeof(dst));
    //    va_copy(ap, va);
    //    v7 = _vscprintf(a7, va);
    //    vsnprintf_s((char *)dst, 0x100u, 0xFFFFFFFF, a7, ap);
    //    if (v7 >= 0x100)
    //        bdLogMessage(
    //            BD_LOG_WARNING,
    //            "warn/",
    //            "bdPlatformLog",
    //            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdPlatform\\bdPlatformLog\\bdPlatformLog.cpp",
    //            "bdLogMessage",
    //            0xDAu,
    //            "Message truncated.",
    //            0);
    //    ap = 0;
    //    v8 = a2;
    //    v10 = 256;
    //    if (a2 && a3)
    //    {
    //        if (bdSnprintf(buf, 0x100u, "%s%s", a2, a3) >= 256)
    //            bdLogMessage(
    //                BD_LOG_WARNING,
    //                "warn/",
    //                "bdPlatformLog",
    //                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdPlatform\\bdPlatformLog\\bdPlatformLog.cpp",
    //                "bdLogMessage",
    //                0xE8u,
    //                "Channel name truncated: %s%s",
    //                a2,
    //                a3);
    //        v8 = buf;
    //    }
    //    for (i = g_logSubscriberList; i; i = bdLinkable::getNext(i))
    //        bdLogSubscriber::logMessage((bdLogSubscriber *)i, a1, v8, a4, a5, a6, (const char *const)dst);
    //}
}