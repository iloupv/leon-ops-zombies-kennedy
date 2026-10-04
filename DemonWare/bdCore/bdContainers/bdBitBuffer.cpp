#include "bdBitBuffer.h"
#include <universal/q_shared.h>
#include <DemonWare/bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <DemonWare/bdCore/bdUtilities/bdBitOperations.h>

bdBitBuffer::bdBitBuffer(unsigned int size, bool typeChecked)
{
    unsigned char v5; // [esp+12h] [ebp-Eh] BYREF
    unsigned char v6; // [esp+13h] [ebp-Dh] BYREF
    int v7; // [esp+1Ch] [ebp-4h]

    //this->__vftable = (bdBitBuffer_vtbl *)&bdReferencable::`vftable';
    this->m_refCount = 0;
    v7 = 0;
    //this->__vftable = (bdBitBuffer_vtbl *)&bdBitBuffer::`vftable';
    this->m_data.m_data = 0;
    this->m_data.m_capacity = ((size & 7) != 0) + (size >> 3);
    this->m_data.m_size = 0;
    if (this->m_data.m_capacity)
        this->m_data.m_data = (unsigned __int8 *)bdMemory::allocate(this->m_data.m_capacity);
    LOBYTE(v7) = 1;
    this->m_writePosition = 0;
    this->m_maxWritePosition = 0;
    this->m_readPosition = 0;
    this->m_failedRead = 0;
    this->m_typeChecked = typeChecked;
    if (this->m_typeChecked)
    {
        v6 = -1;
        bdBitBuffer::writeBits(&v6, 1u);
    }
    else
    {
        v5 = 0;
        bdBitBuffer::writeBits(&v5, 1u);
    }
    this->m_readPosition = 1;
    //return this;
}

bool bdBitBuffer::getTypeCheck()
{
    return this->m_typeChecked;
}

bool bdBitBuffer::readBits(char *buf, unsigned int size)
{
    unsigned int v4; // [esp+0h] [ebp-28h]
    unsigned int v5; // [esp+14h] [ebp-14h]
    unsigned __int8 v6; // [esp+1Bh] [ebp-Dh]
    unsigned int v8; // [esp+24h] [ebp-4h]

    if (!size)
        return 1;
    if (size + this->m_readPosition <= this->m_maxWritePosition)
    {
        v8 = this->m_readPosition >> 3;
        while (size)
        {
            if (v8 >= this->m_data.m_size)
            {
                this->m_failedRead = 1;
                return 0;
            }
            if (size >= 8)
                v4 = 8;
            else
                v4 = size;
            v6 = this->m_data.m_data[v8++];
            v5 = this->m_readPosition & 7;
            if (v4 + v5 <= 8)
            {
                *buf++ = (255 >> (8 - v4)) & ((int)v6 >> v5);
            }
            else
            {
                if (v8 >= this->m_data.m_size)
                {
                    this->m_failedRead = 1;
                    return 0;
                }
                *buf++ = (255 >> (8 - v4)) & ((this->m_data.m_data[v8] << (8 - v5)) | ((int)v6 >> v5));
            }
            this->m_readPosition += v4;
            size -= v4;
        }
        return 1;
    }
    else
    {
        this->m_failedRead = 1;
        return 0;
    }
}

bool bdBitBuffer::readDataType(enum bdBitBufferDataType type)
{
    char dst[40]; // [esp+4h] [ebp-60h] BYREF
    char v4[40]; // [esp+2Ch] [ebp-38h] BYREF
    enum bdBitBufferDataType v5; // [esp+58h] [ebp-Ch]
    unsigned int v6; // [esp+5Ch] [ebp-8h] BYREF
    bool v7; // [esp+63h] [ebp-1h]

    v7 = 1;
    if (this->m_typeChecked)
    {
        v6 = 0;
        v7 = bdBitBuffer::readRangedUInt32(&v6, 0, 0x1Fu, 0);
        if (v7)
        {
            v5 = (bdBitBufferDataType)v6;
            v7 = v6 == type;
            if (v6 != type)
            {
                bdBitBuffer::typeToString(type, dst, 0x28u);
                bdBitBuffer::typeToString(v5, v4, 0x28u);
                bdLogMessage(
                    BD_LOG_ERROR,
                    "err/",
                    "bdCore/bitBuffer",
                    "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdBitBuffer.cpp",
                    "bdBitBuffer::readDataType",
                    0x20Bu,
                    "Expected: %s , read: %s ",
                    dst,
                    v4);
            }
        }
    }
    return v7;
}

bool bdBitBuffer::readInt32(int *arg0)
{
    bool v3; // [esp+0h] [ebp-10h]
    char a2[7]; // [esp+8h] [ebp-8h] BYREF
    bool DataType; // [esp+Fh] [ebp-1h]

    DataType = bdBitBuffer::readDataType(BD_BB_SIGNED_INTEGER32_TYPE);
    v3 = DataType && bdBitBuffer::readBits(a2, 0x20u);
    DataType = v3;
    if (v3)
        *arg0 = *(_DWORD *)a2;
    return DataType;
}

bool bdBitBuffer::readRangedUInt32(
    unsigned int *buf,
    unsigned int start,
    unsigned int end,
    bool typeChecked)
{
    unsigned int v6; // [esp+0h] [ebp-40h]
    unsigned int v7; // [esp+4h] [ebp-3Ch]
    bool v8; // [esp+8h] [ebp-38h]
    bool v9; // [esp+Ch] [ebp-34h]
    bool v10; // [esp+10h] [ebp-30h]
    char a2[4]; // [esp+28h] [ebp-18h] BYREF
    unsigned int v13; // [esp+2Ch] [ebp-14h]
    unsigned int a3; // [esp+30h] [ebp-10h]
    unsigned int v15; // [esp+34h] [ebp-Ch] BYREF
    unsigned int v16; // [esp+38h] [ebp-8h] BYREF
    bool DataType; // [esp+3Fh] [ebp-1h]

    DataType = 1;
    if (typeChecked)
    {
        DataType = bdBitBuffer::readDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);
        if (this->m_typeChecked)
        {
            v16 = 0;
            v15 = 0;
            v10 = DataType && bdBitBuffer::readUInt32(&v16);
            DataType = v10;
            v9 = v10 && bdBitBuffer::readUInt32(&v15);
            DataType = v9;
            if (v9 && (start != v16 || end != v15))
                bdLogMessage(
                    BD_LOG_ERROR,
                    "err/",
                    "bdCore/bitBuffer",
                    "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdBitBuffer.cpp",
                    "bdBitBuffer::readRangedUInt32",
                    0x11Du,
                    "Range error. Expected: (%u,%u), read: (%u,%u)",
                    start,
                    end,
                    v16,
                    v15);
        }
    }
    if (DataType)
    {
        v13 = end - start;
        a3 = 0;
        if (end != start)
            a3 = bdBitOperations::highBitNumber(v13) + 1;
        *(_DWORD *)a2 = 0;
        v8 = DataType && bdBitBuffer::readBits(a2, a3);
        DataType = v8;
        if (v8)
        {
            *buf = *(_DWORD *)a2;
            *buf += start;
            if (*buf <= end)
            {
                if (*buf >= start)
                    v6 = *buf;
                else
                    v6 = start;
                v7 = v6;
            }
            else
            {
                v7 = end;
            }
            *buf = v7;
        }
    }
    return DataType;
}

bool __thiscall bdBitBuffer::readUByte8(char *buf)
{
    return bdBitBuffer::readDataType(BD_BB_UNSIGNED_CHAR8_TYPE) && bdBitBuffer::readBits(buf, 8u);
}

bool bdBitBuffer::readUInt32(unsigned int *arg0)
{
    bool v3; // [esp+0h] [ebp-10h]
    char a2[7]; // [esp+8h] [ebp-8h] BYREF
    bool DataType; // [esp+Fh] [ebp-1h]

    DataType = bdBitBuffer::readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    v3 = DataType && bdBitBuffer::readBits(a2, 0x20u);
    DataType = v3;
    if (v3)
        *arg0 = *(_DWORD *)a2;
    return DataType;
}

void bdBitBuffer::setTypeCheck(bool val)
{
    this->m_typeChecked = val;
}

void __cdecl bdBitBuffer::typeToString(enum bdBitBufferDataType a1, char *const dst, unsigned int size)
{
    enum bdBitBufferDataType v3; // [esp+0h] [ebp-7Ch]
    const char *src[28]; // [esp+Ch] [ebp-70h]

    src[0] = "NoType";
    src[1] = "Bool";
    src[2] = "Char8";
    src[3] = "UChar8";
    src[4] = "WChar16";
    src[5] = "Int16";
    src[6] = "UInt16";
    src[7] = "Int32";
    src[8] = "UInt32";
    src[9] = "Int64";
    src[10] = "UInt64";
    src[11] = "RangedInt32";
    src[12] = "RangeUInt32";
    src[13] = "Float32";
    src[14] = "Float64";
    src[15] = "RangeFloat32";
    src[16] = "String";
    src[17] = "String";
    src[18] = "MultiByteString";
    src[19] = "Blob";
    src[20] = "NaN";
    src[21] = "FullType";
    src[22] = "Unknown Type";
    src[26] = (char *)23;
    src[25] = 0;
    src[24] = (char *)22;
    if (a1 <= (BD_BB_NAN_TYPE | BD_BB_SIGNED_CHAR8_TYPE))
        v3 = a1 < BD_BB_NO_TYPE ? BD_BB_NO_TYPE : a1;
    else
        v3 = (bdBitBufferDataType)((int)BD_BB_NAN_TYPE | (int)BD_BB_SIGNED_CHAR8_TYPE);
    src[27] = (char *)v3;
    //bdStrlcpy(dst, src[v3], size);
    strncpy(dst, src[v3], size);
}

void bdBitBuffer::writeBits(unsigned char *buf, unsigned int size)
{
    unsigned int m_writePosition; // [esp+0h] [ebp-6Ch]
    unsigned int v4; // [esp+4h] [ebp-68h]
    char v6; // [esp+2Bh] [ebp-41h] BYREF
    unsigned int v7; // [esp+2Ch] [ebp-40h]
    unsigned int v8; // [esp+30h] [ebp-3Ch]
    unsigned int v9; // [esp+34h] [ebp-38h]
    unsigned int v10; // [esp+38h] [ebp-34h]
    unsigned int v11; // [esp+3Ch] [ebp-30h]
    char v12; // [esp+42h] [ebp-2Ah]
    char v13; // [esp+43h] [ebp-29h]
    unsigned int v14; // [esp+44h] [ebp-28h]
    char v15; // [esp+4Ah] [ebp-22h]
    unsigned __int8 v16; // [esp+4Bh] [ebp-21h]
    int v17; // [esp+4Ch] [ebp-20h]
    char v18; // [esp+53h] [ebp-19h]
    int v19; // [esp+54h] [ebp-18h]
    unsigned __int8 v20; // [esp+5Bh] [ebp-11h]
    int v21; // [esp+5Ch] [ebp-10h]
    _BYTE *v22; // [esp+60h] [ebp-Ch]
    unsigned int v23; // [esp+64h] [ebp-8h]
    unsigned int v24; // [esp+68h] [ebp-4h]

    v23 = (this->m_writePosition + size - 1) >> 3;
    if (v23 >= this->m_data.m_size)
    {
        v6 = 0;
        //bdFastArray<unsigned char>::setGrow(&this->m_data.m_data, v23, &v6);
        this->m_data.setGrow(v23, v6);
    }
    v24 = size;
    v22 = buf;
    while (v24)
    {
        v7 = this->m_writePosition & 7;
        v10 = 8 - v7;
        if (v24 >= 8 - v7)
            v4 = v10;
        else
            v4 = v24;
        v8 = v4;
        v13 = (255 >> v10) | (255 << (v7 + v4));
        v14 = this->m_writePosition >> 3;
        v15 = v13 & this->m_data.m_data[v14];
        v21 = size - v24;
        v19 = ((_BYTE)size - (_BYTE)v24) & 7;
        v17 = 8 - v19;
        v9 = (size - v24) >> 3;
        v20 = v22[v9];
        v16 = 0;
        v11 = (size - 1) >> 3;
        if (v11 > v9)
            v16 = v22[v9 + 1];
        v20 = (v16 << v17) | ((int)v20 >> v19);
        v12 = ~v13;
        v18 = ~v13 & (v20 << v7);
        this->m_data.m_data[v14] = v18 | v15;
        this->m_writePosition += v8;
        v24 -= v8;
        if (this->m_maxWritePosition <= this->m_writePosition)
            m_writePosition = this->m_writePosition;
        else
            m_writePosition = this->m_maxWritePosition;
        this->m_maxWritePosition = m_writePosition;
    }
}

bool bdBitBuffer::writeBool(bool val)
{
    unsigned char v4; // [esp+6h] [ebp-2h] BYREF
    unsigned char v5; // [esp+7h] [ebp-1h] BYREF

    bdBitBuffer::writeDataType(BD_BB_BOOL_TYPE);
    if (val)
    {
        v5 = -1;
        bdBitBuffer::writeBits(&v5, 1u);
    }
    else
    {
        v4 = 0;
        bdBitBuffer::writeBits(&v4, 1u);
    }
    return val;
}

void bdBitBuffer::writeDataType(enum bdBitBufferDataType a2)
{
    if (this->m_typeChecked)
        bdBitBuffer::writeRangedUInt32(a2, 0, 0x1Fu, 0);
}

void bdBitBuffer::writeRangedUInt32(
    unsigned int buf,
    unsigned int start,
    unsigned int end,
    bool typeChecked)
{
    unsigned int v5; // [esp+0h] [ebp-30h]
    unsigned int v6; // [esp+4h] [ebp-2Ch]
    _DWORD v8[2]; // [esp+10h] [ebp-20h] BYREF
    unsigned int v9; // [esp+18h] [ebp-18h] BYREF
    int v10; // [esp+1Ch] [ebp-14h] BYREF
    unsigned int v11; // [esp+20h] [ebp-10h]
    unsigned int v12; // [esp+24h] [ebp-Ch]
    int v13; // [esp+28h] [ebp-8h]
    unsigned int v14; // [esp+2Ch] [ebp-4h]

    if (typeChecked)
    {
        bdBitBuffer::writeDataType(BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE);
        if (this->m_typeChecked)
        {
            v8[1] = start;
            bdBitBuffer::writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
            v9 = start;
            bdBitBuffer::writeBits((unsigned char*)&v9, 0x20u);
            bdBitBuffer::writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
            v8[0] = end;
            bdBitBuffer::writeBits((unsigned char *)v8, 0x20u);
        }
    }
    v11 = end - start;
    v12 = 0;
    if (end != start)
        v12 = bdBitOperations::highBitNumber(v11) + 1;
    if (buf <= end)
    {
        if (buf >= start)
            v5 = buf;
        else
            v5 = start;
        v6 = v5;
    }
    else
    {
        v6 = end;
    }
    v14 = v6;
    v13 = v6 - start;
    v10 = v6 - start;
    bdBitBuffer::writeBits((unsigned char *)&v10, v12);
}