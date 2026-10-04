#pragma once

#include <universal/q_shared.h>

// I am altering the configuration/layout of this crappy library. Pray I dont alter it further.
namespace bdBitOperations
{
    inline int highBitNumber(unsigned int a1)
    {
        int v2; // [esp+0h] [ebp-4h]
        unsigned int v3; // [esp+Ch] [ebp+8h]

        v2 = (a1 & 0xFFFF0000) != 0 ? 0x10 : 0;
        v3 = a1 >> ((a1 & 0xFFFF0000) != 0 ? 0x10 : 0);
        if ((v3 & 0xFF00) != 0)
        {
            v2 |= 8u;
            v3 >>= 8;
        }
        if ((v3 & 0xF0) != 0)
        {
            v2 |= 4u;
            v3 >>= 4;
        }
        if ((v3 & 0xC) != 0)
        {
            v2 |= 2u;
            v3 >>= 2;
        }
        return v2 | (v3 >> 1);
    }

    inline unsigned int nextPowerOf2(unsigned int a1)
    {
        unsigned int v2; // [esp+0h] [ebp-4h]
        unsigned int v3; // [esp+0h] [ebp-4h]
        unsigned int v4; // [esp+0h] [ebp-4h]

        v2 = a1 | (a1 >> 1) | ((a1 | (a1 >> 1)) >> 2);
        v3 = v2 | (v2 >> 4) | ((v2 | (v2 >> 4)) >> 8);
        v4 = (v3 | HIWORD(v3)) & ~((v3 | HIWORD(v3)) >> 1);
        if (v4 != a1)
            v4 *= 2;
        return v4;
    }
}
