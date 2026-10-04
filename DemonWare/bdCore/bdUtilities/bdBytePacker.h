#pragma once



struct bdBytePacker
{
    static bool appendBuffer(
        unsigned __int8 *dst,          // destination buffer
        unsigned int     dstCapacity,  // total size of dst (in bytes)
        unsigned int     writeOffset,  // current write position in dst
        unsigned int *newOffset,       // resulting write position after append
        unsigned __int8 *src,          // source buffer to append from
        unsigned int     byteCount    // number of bytes to append
        );

    static bool appendEncodedUInt16(
        unsigned __int8 *dst,
        unsigned int dstCapacity,
        unsigned int writeOffset,
        unsigned int *newOffset,
        unsigned __int16 val);

    static bool removeBuffer(
        unsigned __int8 *src,
        unsigned int srcCapacity,
        unsigned int offset,
        unsigned int *newOffset,
        unsigned __int8 *dst,
        unsigned int count);

    static bool removeEncodedUInt16(
        unsigned __int8 *src,
        unsigned int srcCapacity,
        unsigned int offset,
        unsigned int *newOffset,
        unsigned __int16 *val);

    static bool rewindBytes(
        const void *buffer,
        unsigned int bufferCapacity,
        unsigned int offset,
        unsigned int *newOffset,
        unsigned int bytes);

    static bool skipBytes(
        const void *buffer,
        unsigned int bufferCapacity,
        unsigned int offset,
        unsigned int *newOffset,
        unsigned int bytes);
};