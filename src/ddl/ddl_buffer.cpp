#include "ddl_buffer.h"

#include <cstring>
#include "ddl_api.h"

void __cdecl DDL_Buffer_WriteHeader(char *buffer, ddlBufferHeader_t *header)
{
    memcpy(buffer, header, 0x28u);
}

void __cdecl DDL_Buffer_ReadHeader(char *buffer, ddlBufferHeader_t *header)
{
    memcpy(header, buffer, sizeof(ddlBufferHeader_t));
}

int __cdecl DDL_Buffer_ReadBits(char *buffer, int offset, int size, unsigned int *value)
{
    int v5; // ecx
    int v6; // [esp+0h] [ebp-28h]
    int bit; // [esp+14h] [ebp-14h]
    int fragSize; // [esp+1Ch] [ebp-Ch]
    int byteOffset; // [esp+20h] [ebp-8h]
    int offseta; // [esp+34h] [ebp+Ch]
    int sizea; // [esp+38h] [ebp+10h]

    offseta = offset + 320;
    if ( offseta >= 0 )
    {
        byteOffset = offseta / 8;
        bit = 0;
        *value = 0;
        v5 = offseta % 8;
        if ( offseta % 8 )
        {
            if ( size < 8 - v5 )
                v6 = size;
            else
                v6 = 8 - v5;
            *value |= ((1 << v6) - 1) & ((int)(unsigned __int8)buffer[byteOffset] >> (offseta % 8));
            size -= v6;
            bit = v6;
            ++byteOffset;
        }
        fragSize = size % 8;
        sizea = size - size % 8;
        while ( bit < sizea )
        {
            *value |= (unsigned __int8)buffer[byteOffset++] << bit;
            bit += 8;
        }
        if ( fragSize )
            *value |= (((1 << fragSize) - 1) & (unsigned __int8)buffer[byteOffset]) << bit;
        return 1;
    }
    else
    {
        DDL_PrintError("DDL: Negative offset.\n");
        return 0;
    }
}

int __cdecl DDL_Buffer_WriteBits(char *buffer, int offset, int size, unsigned int value)
{
    int bit; // [esp+0h] [ebp-10h]
    int byteOffset; // [esp+8h] [ebp-8h]
    int bitOffset; // [esp+Ch] [ebp-4h]
    int offseta; // [esp+1Ch] [ebp+Ch]

    offseta = offset + 320;
    if ( offseta >= 0 )
    {
        byteOffset = offseta / 8;
        bitOffset = offseta % 8;
        for ( bit = 0; bit < size; ++bit )
        {
            if ( (value & (1 << bit)) != 0 )
                buffer[byteOffset] |= 1 << bitOffset;
            else
                buffer[byteOffset] &= ~(1 << bitOffset);
            if ( ++bitOffset == 8 )
            {
                bitOffset = 0;
                ++byteOffset;
            }
        }
        return 1;
    }
    else
    {
        DDL_PrintError("DDL: Negative offset.\n");
        return 0;
    }
}

int __cdecl DDL_Buffer_ClearBits(char *buffer, int offset, int size)
{
    return DDL_Buffer_WriteBits(buffer, offset, size, 0);
}

