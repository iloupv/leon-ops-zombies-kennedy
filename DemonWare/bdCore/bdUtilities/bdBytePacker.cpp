#include "bdBytePacker.h"
#include <DemonWare/bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <cstring>

bool bdBytePacker::appendBuffer(
        unsigned __int8 *dst,          // destination buffer
        unsigned int     dstCapacity,  // total size of dst (in bytes)
        unsigned int     writeOffset,  // current write position in dst
        unsigned int *newOffset,       // resulting write position after append
        unsigned __int8 *src,          // source buffer to append from
        unsigned int     byteCount    // number of bytes to append
    )
{
    bool v7; // [esp+0h] [ebp-8h]
    bool v8; // [esp+5h] [ebp-3h]
    char v9; // [esp+7h] [ebp-1h]

    v9 = 1;
    *newOffset = byteCount + writeOffset;
    if (dst && src)
    {
        v8 = *newOffset <= dstCapacity;
        if (*newOffset > dstCapacity)
            bdLogMessage(
                BD_LOG_WARNING,
                "warn/",
                "byte packer",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdUtilities\\bdBytePacker.cpp",
                "bdBytePacker::appendBuffer",
                0x24u,
                "Not enough room left to write %u bytes.",
                byteCount);
        v7 = writeOffset <= dstCapacity && v8;
        v9 = v7;
        if (v7)
            memmove(&dst[writeOffset], src, byteCount);
    }
    return v9;
}

bool __cdecl bdBytePacker::appendEncodedUInt16(
    unsigned __int8 *dst,
    unsigned int dstCapacity,
    unsigned int writeOffset,
    unsigned int *newOffset,
    unsigned __int16 val)
{
    bool v6; // [esp+0h] [ebp-18h]
    unsigned __int8 v7; // [esp+Ah] [ebp-Eh] BYREF
    unsigned int v8; // [esp+Ch] [ebp-Ch]
    unsigned __int8 v9; // [esp+10h] [ebp-8h] BYREF
    unsigned __int8 src; // [esp+12h] [ebp-6h] BYREF
    char appended; // [esp+13h] [ebp-5h]
    char v13; // [esp+14h] [ebp-4h]
    bool v14; // [esp+15h] [ebp-3h]
    __int16 v15; // [esp+16h] [ebp-2h]

    if (val <= 0x7Fu)
    {
        v13 = val;
        v7 = val;
        return bdBytePacker::appendBuffer(dst, dstCapacity, writeOffset, newOffset, &v7, 1u);
    }
    else
    {
        v14 = 1;
        v15 = val | 0x8000;
        src = (unsigned __int16)(val | 0x8000) >> 8;
        appended = bdBytePacker::appendBuffer(dst, dstCapacity, writeOffset, newOffset, &src, 1u);
        v14 = appended != 0;
        v6 = 0;
        if (appended)
        {
            v8 = *newOffset;
            v9 = v15;
            if (bdBytePacker::appendBuffer(dst, dstCapacity, v8, newOffset, &v9, 1u))
                v6 = 1;
        }
        v14 = v6;
        return v6;
    }
}

bool __cdecl bdBytePacker::removeBuffer(
    unsigned __int8 *src,
    unsigned int srcCapacity,
    unsigned int offset,
    unsigned int *newOffset,
    unsigned __int8 *dst,
    unsigned int count)
{
    bool v7; // [esp+0h] [ebp-8h]
    bool v8; // [esp+5h] [ebp-3h]
    bool v9; // [esp+7h] [ebp-1h]

    v9 = 1;
    *newOffset = count + offset;
    if (dst && src)
    {
        v8 = *newOffset <= srcCapacity;
        if (*newOffset > srcCapacity)
            bdLogMessage(
                BD_LOG_WARNING,
                "warn/",
                "byte packer",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdUtilities\\bdBytePacker.cpp",
                "bdBytePacker::removeBuffer",
                0x6Au,
                "Not enough data left to read %u bytes.",
                count);
        v7 = v8 && offset <= srcCapacity;
        v9 = v7;
        if (v7)
            memmove(dst, &src[offset], count);
    }
    return v9;
}

bool __cdecl bdBytePacker::removeEncodedUInt16(
    unsigned __int8 *src,
    unsigned int srcCapacity,
    unsigned int offset,
    unsigned int *newOffset,
    unsigned __int16 *val)
{
    char v6; // [esp+0h] [ebp-14h]
    unsigned __int8 v7; // [esp+Dh] [ebp-7h] BYREF
    bool v8; // [esp+Eh] [ebp-6h]
    unsigned __int8 dst; // [esp+Fh] [ebp-5h] BYREF
    bool v10; // [esp+10h] [ebp-4h]
    unsigned __int8 v11; // [esp+11h] [ebp-3h]
    bool v12; // [esp+12h] [ebp-2h]
    unsigned __int8 v13; // [esp+13h] [ebp-1h]

    v12 = 1;
    v13 = 0;
    v10 = bdBytePacker::removeBuffer(src, srcCapacity, offset, newOffset, &dst, 1u);
    if (v10)
        v13 = dst;
    v12 = v10;
    if (v13 < 0x80u)
    {
        *val = v13;
        return v12;
    }
    else
    {
        v13 &= ~0x80u;
        v11 = 0;
        if (v12 && (v8 = bdBytePacker::removeBuffer(src, srcCapacity, *newOffset, newOffset, &v7, 1u)))
        {
            v11 = v7;
            v6 = 1;
        }
        else
        {
            v6 = 0;
        }
        v12 = v6;
        *val = v11 + (v13 << 8);
        return v12;
    }
}

bool __cdecl bdBytePacker::rewindBytes(
    const void *buffer,
    unsigned int bufferCapacity,
    unsigned int offset,
    unsigned int *newOffset,
    unsigned int bytes)
{
    char v6; // [esp+3h] [ebp-1h]

    v6 = 0;
    *newOffset = offset - bytes;
    if (buffer)
        return offset >= bytes;
    return v6;
}

bool __cdecl bdBytePacker::skipBytes(
    const void *buffer,
    unsigned int bufferCapacity,
    unsigned int offset,
    unsigned int *newOffset,
    unsigned int bytes)
{
    bool v6; // [esp+3h] [ebp-1h]

    v6 = 0;
    *newOffset = bytes + offset;
    if (buffer)
        return bufferCapacity >= *newOffset;
    return v6;
}