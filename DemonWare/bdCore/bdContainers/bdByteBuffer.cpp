#include "bdByteBuffer.h"
#include <DemonWare/bdCore/bdMemory/bdMemory.h>
#include <cstring>
#include <universal/q_shared.h>
#include <DemonWare/bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <DemonWare/bdCore/bdUtilities/bdBytePacker.h>

bdByteBuffer::bdByteBuffer(unsigned int size, bool typeChecked)
{
    //this->__vftable = (bdByteBuffer_vtbl *)&bdReferencable::`vftable';
    this->m_refCount = 0;
    //this->__vftable = (bdByteBuffer_vtbl *)&bdByteBuffer::`vftable';
    this->m_size = size;
    this->m_data = 0;
    this->m_typeChecked = typeChecked;
    this->m_typeCheckedCopy = this->m_typeChecked;
    this->m_allocatedData = 0;
    if (size)
    {
        this->m_data = (unsigned __int8 *)bdMemory::allocate(size);
        this->m_allocatedData = 1;
    }
    this->m_writePtr = this->m_data;
    this->m_readPtr = this->m_data;
}

bdByteBuffer::~bdByteBuffer()
{
    //this->__vftable = (bdByteBuffer_vtbl *)&bdByteBuffer::`vftable';
    if (this->m_data && this->m_allocatedData)
        bdMemory::deallocate(this->m_data);
    this->m_data = 0;
    this->m_readPtr = 0;
    this->m_writePtr = 0;
    //bdReferencable::~bdReferencable();
}

bdByteBuffer::bdByteBuffer(
    unsigned __int8 *src,
    unsigned int count,
    bool typeChecked)
{
    //this->__vftable = (bdByteBuffer_vtbl *)&bdReferencable::`vftable';
    this->m_refCount = 0;
    //this->__vftable = (bdByteBuffer_vtbl *)&bdByteBuffer::`vftable';
    this->m_size = count;
    this->m_data = 0;
    this->m_typeChecked = typeChecked;
    this->m_typeCheckedCopy = this->m_typeChecked;
    this->m_allocatedData = 0;
    if (count)
    {
        this->m_data = (unsigned __int8 *)bdMemory::allocate(count);
        this->m_allocatedData = 1;
        memcpy(this->m_data, src, count);
    }
    this->m_writePtr = this->m_data;
    this->m_readPtr = this->m_data;
}

void bdByteBuffer::allocateBuffer()
{
    if (this->m_data)
    {
        bdLogMessage(
            BD_LOG_WARNING,
            "warn/",
            "core/bytebuffer",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
            "bdByteBuffer::allocateBuffer",
            0x25u,
            "Buffer already allocated.");
    }
    else
    {
        this->m_data = (unsigned __int8 *)bdMemory::allocate(this->m_size);
        this->m_allocatedData = 1;
        this->m_writePtr = this->m_data;
        this->m_readPtr = this->m_data;
    }
}

int bdByteBuffer::getDataSize()
{
    return this->m_writePtr - this->m_data;
}

bool bdByteBuffer::read(unsigned __int8 *dst, unsigned int count)
{
    unsigned int v5; // [esp+4h] [ebp-Ch] BYREF
    unsigned int v6; // [esp+8h] [ebp-8h]
    bool v7; // [esp+Fh] [ebp-1h]

    v7 = 0;
    v6 = this->m_size - (this->m_readPtr - this->m_data);
    if (count > v6)
    {
        bdLogMessage(
            BD_LOG_WARNING,
            "warn/",
            "core/bytebuffer",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
            "bdByteBuffer::read",
            0x170u,
            "Could not read data from buffer. Insufficient data available.\n");
    }
    else
    {
        v7 = bdBytePacker::removeBuffer(this->m_readPtr, v6, 0, &v5, dst, count);
        this->m_readPtr += count;
    }
    return v7;
}

bool bdByteBuffer::readAndAllocateBlob(unsigned __int8 **src, unsigned int *count)
{
    unsigned __int8 *dst; // [esp+8h] [ebp-Ch]
    unsigned int v6; // [esp+Ch] [ebp-8h] BYREF
    bool DataType; // [esp+13h] [ebp-1h]

    dst = 0;
    v6 = 0;
    DataType = bdByteBuffer::readDataType(BD_BB_BLOB_TYPE);
    if (DataType)
    {
        DataType = bdByteBuffer::readUInt32(&v6);
        if (DataType)
            dst = (unsigned __int8 *)bdMemory::allocate(v6);
        if (DataType && dst)
            DataType = bdByteBuffer::read(dst, v6);
    }
    if (DataType)
    {
        *src = dst;
        *count = v6;
    }
    else
    {
        bdMemory::deallocate(dst);
    }
    return DataType;
}

void bdByteBuffer::readArrayEnd()
{
    this->m_typeChecked = this->m_typeCheckedCopy;
}

bool bdByteBuffer::readArrayStart(char type, unsigned int *buf)
{
    int v4; // [esp+0h] [ebp-1Ch]
    bool v6; // [esp+4h] [ebp-18h]
    bool v7; // [esp+8h] [ebp-14h]
    unsigned __int8 v9; // [esp+13h] [ebp-9h] BYREF
    unsigned int v10; // [esp+14h] [ebp-8h] BYREF
    unsigned __int8 typeRead; // [esp+1Ah] [ebp-2h] BYREF
    bool v12; // [esp+1Bh] [ebp-1h]

    this->m_typeCheckedCopy = this->m_typeChecked;
    this->m_typeChecked = 0;
    v12 = bdByteBuffer::readUByte8(&typeRead);
    if (v12)
    {
        if (typeRead - 100 != (unsigned __int8)type)
        {
            bdLogMessage(
                BD_LOG_ERROR,
                "err/",
                "err",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
                "bdByteBuffer::readArrayStart",
                0x72u,
                "readArrayStart: Expected type %d but read type %d\n",
                (unsigned __int8)type,
                typeRead);
            v12 = 0;
        }
        if (v12)
        {
            this->m_typeChecked = 0;
            v7 = v12 && bdByteBuffer::readUByte8(&v9);
            v12 = v7;
            v6 = v7 && bdByteBuffer::readUInt32(&v10);
            v12 = v6;
            return v6 && bdByteBuffer::readUInt32(buf);
        }
        return v12;
    }
    else
    {
        //bdLogMessage(
        //    BD_LOG_WARNING,
        //    "warn/",
        //    "warn",
        //    "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
        //    "bdByteBuffer::readArrayStart",
        //    0x6Bu,
        //    "readArrayStart: No array %d\n",
        //    v4);
        *buf = 0;
        return v12;
    }
}

bool bdByteBuffer::readBlob(unsigned __int8 *const dst, unsigned int *pCount)
{
    bool v3; // al
    unsigned int v6; // [esp+8h] [ebp-8h] BYREF
    bool DataType; // [esp+Fh] [ebp-1h]

    DataType = bdByteBuffer::readDataType(BD_BB_BLOB_TYPE);
    if (DataType)
    {
        v6 = 0;
        DataType = bdByteBuffer::readUInt32(&v6);
        if (DataType && dst)
        {
            if (*pCount >= v6)
                v3 = bdByteBuffer::read(dst, v6);
            else
                v3 = bdByteBuffer::read(dst, *pCount);
            DataType = v3;
            if (v6 > *pCount)
                bdLogMessage(
                    BD_LOG_WARNING,
                    "warn/",
                    "core/bytebuffer",
                    "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
                    "bdByteBuffer::readBlob",
                    0x238u,
                    "Reading BLOB (%u bytes) buffer too small (%u bytes).",
                    v6,
                    *pCount);
        }
        *pCount = v6;
    }
    return DataType;
}

bool bdByteBuffer::readBool(bool *buf)
{
    unsigned __int8 dst; // [esp+6h] [ebp-2h] BYREF
    bool DataType; // [esp+7h] [ebp-1h]

    DataType = bdByteBuffer::readDataType(BD_BB_BOOL_TYPE);
    if (DataType)
    {
        dst = 0;
        DataType = bdByteBuffer::read(&dst, 1u);
        if (DataType)
        {
            if (dst)
                *buf = 1;
            else
                *buf = 0;
        }
    }
    return DataType;
}

bool bdByteBuffer::readDataType(enum bdBitBufferDataType type)
{
    char dst[40]; // [esp+4h] [ebp-5Ch] BYREF
    char v4[40]; // [esp+2Ch] [ebp-34h] BYREF
    enum bdBitBufferDataType v5; // [esp+58h] [ebp-8h]
    unsigned __int8 v6; // [esp+5Eh] [ebp-2h] BYREF
    bool v7; // [esp+5Fh] [ebp-1h]

    v7 = 1;
    if (this->m_typeChecked)
    {
        v6 = 0;
        v7 = bdByteBuffer::read(&v6, 1u);
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
                    "core/bytebuffer",
                    "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
                    "bdByteBuffer::readDataType",
                    0x2A5u,
                    "Expected: %s , read: %s ",
                    dst,
                    v4);
            }
        }
    }
    return v7;
}

bool bdByteBuffer::readInt32(int *buf)
{
    bool v5; // [esp+Bh] [ebp-9h]
    unsigned __int8 dst[7]; // [esp+Ch] [ebp-8h] BYREF

    if (!bdByteBuffer::readDataType(BD_BB_SIGNED_INTEGER32_TYPE))
        return 0;
    v5 = bdByteBuffer::read(dst, 4u);
    if (v5)
        *buf = *(_DWORD *)dst;
    return v5;
}

bool bdByteBuffer::readInt64(__int64 *a2)
{
    bool v5; // [esp+Bh] [ebp-Dh]
    unsigned __int8 dst[11]; // [esp+Ch] [ebp-Ch] BYREF

    if (!bdByteBuffer::readDataType(BD_BB_SIGNED_INTEGER64_TYPE))
        return 0;
    v5 = bdByteBuffer::read(dst, 8u);
    if (v5)
        *a2 = *(_QWORD *)dst;
    return v5;
}

bool bdByteBuffer::readString(char *const buf, unsigned int count)
{
    unsigned __int8 dst; // [esp+7h] [ebp-9h] BYREF
    unsigned int v6; // [esp+8h] [ebp-8h]
    bool DataType; // [esp+Fh] [ebp-1h]

    DataType = bdByteBuffer::readDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
    if (DataType && buf)
    {
        v6 = 0;
        do
        {
            dst = 0;
            DataType = bdByteBuffer::read(&dst, 1u);
            if (v6 < count)
                buf[v6] = dst;
            if (DataType && dst)
                ++v6;
        } while (DataType && dst);
        if (count)
            buf[count - 1] = 0;
    }
    return DataType;
}

bool bdByteBuffer::readUByte8(unsigned __int8 *buf)
{
    bool v5; // [esp+9h] [ebp-3h]
    unsigned __int8 dst; // [esp+Ah] [ebp-2h] BYREF

    if (!bdByteBuffer::readDataType(BD_BB_UNSIGNED_CHAR8_TYPE))
        return 0;
    v5 = bdByteBuffer::read(&dst, 1u);
    if (v5)
        *buf = dst;
    return v5;
}

bool bdByteBuffer::readUInt16(unsigned __int16 *buf)
{
    bool v5; // [esp+Bh] [ebp-9h]
    unsigned __int8 dst[7]; // [esp+Ch] [ebp-8h] BYREF

    if (!bdByteBuffer::readDataType(BD_BB_UNSIGNED_INTEGER16_TYPE))
        return 0;
    v5 = bdByteBuffer::read(dst, 2u);
    if (v5)
        *buf = *(_WORD *)dst;
    return v5;
}

bool bdByteBuffer::readUInt32(unsigned int *buf)
{
    bool v5; // [esp+Bh] [ebp-9h]
    unsigned __int8 dst[7]; // [esp+Ch] [ebp-8h] BYREF

    if (!bdByteBuffer::readDataType(BD_BB_UNSIGNED_INTEGER32_TYPE))
        return 0;
    v5 = bdByteBuffer::read(dst, 4u);
    if (v5)
        *buf = *(_DWORD *)dst;
    return v5;
}

bool bdByteBuffer::readUInt64(unsigned __int64 *buf)
{
    bool v5; // [esp+Bh] [ebp-Dh]
    unsigned __int8 dst[11]; // [esp+Ch] [ebp-Ch] BYREF

    if (!bdByteBuffer::readDataType(BD_BB_UNSIGNED_INTEGER64_TYPE))
        return 0;
    v5 = bdByteBuffer::read(dst, 8u);
    if (v5)
        *buf = *(_QWORD *)dst;
    return v5;
}

bool bdByteBuffer::write(unsigned __int8 *src, unsigned int count)
{
    unsigned int v5; // [esp+4h] [ebp-Ch] BYREF
    unsigned int v6; // [esp+8h] [ebp-8h]
    bool appended; // [esp+Fh] [ebp-1h]

    appended = 0;
    if (this->m_data)
    {
        v6 = this->m_size - (this->m_writePtr - this->m_data);
        if (count <= v6)
        {
            appended = bdBytePacker::appendBuffer(this->m_writePtr, v6, 0, &v5, src, count);
            if (appended)
                this->m_writePtr += v5;
        }
        return appended;
    }
    else
    {
        this->m_size += count;
        return 1;
    }
}

bool bdByteBuffer::writeArrayStart(char val, unsigned int size, unsigned int stride)
{
    bool v6; // [esp+4h] [ebp-Ch]
    bool v8; // [esp+Fh] [ebp-1h]

    this->m_typeCheckedCopy = this->m_typeChecked;
    this->m_typeChecked = 0;
    v8 = bdByteBuffer::writeUByte8(val + 100);
    this->m_typeChecked = 1;
    v6 = v8 && bdByteBuffer::writeUInt32(stride * size);
    this->m_typeChecked = 0;
    return v6 && bdByteBuffer::writeUInt32(size);
}

bool bdByteBuffer::writeBlob(unsigned __int8 *src, unsigned int count)
{
    bool v5; // [esp+4h] [ebp-Ch]

    v5 = bdByteBuffer::writeDataType(BD_BB_BLOB_TYPE) && bdByteBuffer::writeUInt32(count);
    return v5 && bdByteBuffer::write(src, count);
}

bool bdByteBuffer::writeBool(unsigned __int8 val)
{
    char v3; // [esp+0h] [ebp-Ch]
    unsigned __int8 src; // [esp+Ah] [ebp-2h] BYREF
    bool v6; // [esp+Bh] [ebp-1h]

    v6 = bdByteBuffer::writeDataType(BD_BB_BOOL_TYPE);
    v3 = 0;
    if (v6)
    {
        src = val;
        if (bdByteBuffer::write(&src, 1u))
            return 1;
    }
    return v3;
}

bool bdByteBuffer::writeDataType(enum bdBitBufferDataType a2)
{
    char v3; // [esp+6h] [ebp-2h]
    unsigned __int8 src; // [esp+7h] [ebp-1h] BYREF

    v3 = 1;
    src = a2;
    if (this->m_typeChecked)
        return bdByteBuffer::write(&src, 1u);
    return v3;
}

bool bdByteBuffer::writeInt32(int val)
{
    bool v3; // [esp+0h] [ebp-10h]
    unsigned __int8 src[7]; // [esp+8h] [ebp-8h] BYREF
    char v6; // [esp+Fh] [ebp-1h]

    if (val == 0x7FFFFFFF)
        return bdByteBuffer::writeNAN();
    v6 = bdByteBuffer::writeDataType(BD_BB_SIGNED_INTEGER32_TYPE);
    v3 = 0;
    if (v6)
    {
        *(_DWORD *)src = val;
        if (bdByteBuffer::write(src, 4u))
            return 1;
    }
    return v3;
}

bool bdByteBuffer::writeInt64(__int64 val)
{
    bool v3; // [esp+0h] [ebp-14h]
    __int64 src; // [esp+8h] [ebp-Ch] BYREF
    char v6; // [esp+13h] [ebp-1h]

    if (val == 0x7FFFFFFFFFFFFFFFLL)
        return bdByteBuffer::writeNAN();
    v6 = bdByteBuffer::writeDataType(BD_BB_SIGNED_INTEGER64_TYPE);
    v3 = 0;
    if (v6)
    {
        src = val;
        if (bdByteBuffer::write((unsigned __int8 *)&src, 8u))
            return 1;
    }
    return v3;
}

bool bdByteBuffer::writeNAN()
{
    bool v2; // [esp+7h] [ebp-1h]

    v2 = 0;
    if (this->m_typeChecked)
        return bdByteBuffer::writeDataType(BD_BB_NAN_TYPE);
    bdLogMessage(
        BD_LOG_ERROR,
        "err/",
        "core/bytebuffer",
        "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
        "bdByteBuffer::writeNAN",
        0xDEu,
        "NaN types cannot be used on not type checked byte buffers");
    return v2;
}

bool bdByteBuffer::writeNoType()
{
    bool v2; // [esp+7h] [ebp-1h]

    v2 = 0;
    if (this->m_typeChecked)
        return bdByteBuffer::writeDataType(BD_BB_NO_TYPE);
    bdLogMessage(
        BD_LOG_ERROR,
        "err/",
        "core/bytebuffer",
        "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
        "bdByteBuffer::writeNoType",
        0xECu,
        "No type cannot be written to not type checked byte buffers");
    return v2;
}

bool bdByteBuffer::writeString(char *buf, unsigned int cnt)
{
    int v3; // eax
    bool v5; // [esp+0h] [ebp-2Ch]
    bool v6; // [esp+4h] [ebp-28h]
    unsigned int v7; // [esp+8h] [ebp-24h]
    unsigned __int8 src; // [esp+13h] [ebp-19h] BYREF
    int v10; // [esp+14h] [ebp-18h]
    unsigned int v11; // [esp+18h] [ebp-14h]
    unsigned __int8 v12; // [esp+1Eh] [ebp-Eh]
    char v13; // [esp+1Fh] [ebp-Dh]
    unsigned int v14; // [esp+20h] [ebp-Ch]
    unsigned int v15; // [esp+24h] [ebp-8h]
    char v16; // [esp+2Bh] [ebp-1h]

    v16 = 0;
    if (!buf)
        return bdByteBuffer::writeNAN();
    v14 = 0;
    v13 = 0;
    v16 = bdByteBuffer::writeDataType(BD_BB_SIGNED_CHAR8_STRING_TYPE);
    v3 = (int)memchr((unsigned __int8 *)buf, 0, cnt);
    v10 = v3;
    if (v3)
        v7 = v10 - (_DWORD)buf;
    else
        v7 = cnt;
    v11 = v7;
    v15 = v7;
    if (v7 >= cnt)
    {
        v14 = cnt - 1;
        v13 = 1;
        bdLogMessage(
            BD_LOG_WARNING,
            "warn/",
            "core/bytebuffer",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdContainers\\bdByteBuffer.cpp",
            "bdByteBuffer::writeString",
            0x14Cu,
            "String was not null terminated. Data will be truncated.");
    }
    else
    {
        v14 = v15 + 1;
        v13 = 0;
    }
    v6 = v16 && bdByteBuffer::write((unsigned __int8 *)buf, v14);
    v16 = v6;
    if (v13 == 1)
    {
        v12 = 0;
        v5 = 0;
        if (v16)
        {
            src = v12;
            if (bdByteBuffer::write(&src, 1u))
                return 1;
        }
        return v5;
    }
    return v16;
}

bool bdByteBuffer::writeUByte8(unsigned __int8 val)
{
    bool v3; // [esp+0h] [ebp-Ch]
    unsigned __int8 src; // [esp+Ah] [ebp-2h] BYREF
    char v6; // [esp+Bh] [ebp-1h]

    v6 = bdByteBuffer::writeDataType(BD_BB_UNSIGNED_CHAR8_TYPE);
    v3 = 0;
    if (v6)
    {
        src = val;
        if (bdByteBuffer::write(&src, 1u))
            return 1;
    }
    return v3;
}

bool bdByteBuffer::writeUInt16(unsigned __int16 val)
{
    bool v3; // [esp+0h] [ebp-10h]
    unsigned __int8 src[7]; // [esp+8h] [ebp-8h] BYREF
    char v6; // [esp+Fh] [ebp-1h]

    v6 = bdByteBuffer::writeDataType(BD_BB_UNSIGNED_INTEGER16_TYPE);
    v3 = 0;
    if (v6)
    {
        *(_WORD *)src = val;
        if (bdByteBuffer::write(src, 2u))
            return 1;
    }
    return v3;
}

bool bdByteBuffer::writeUInt32(unsigned int val)
{
    bool v3; // [esp+0h] [ebp-10h]
    unsigned __int8 src[7]; // [esp+8h] [ebp-8h] BYREF
    char v6; // [esp+Fh] [ebp-1h]

    if (val == -1)
        return bdByteBuffer::writeNAN();
    v6 = bdByteBuffer::writeDataType(BD_BB_UNSIGNED_INTEGER32_TYPE);
    v3 = 0;
    if (v6)
    {
        *(_DWORD *)src = val;
        if (bdByteBuffer::write(src, 4u))
            return 1;
    }
    return v3;
}

bool bdByteBuffer::writeUInt64(unsigned __int64 val)
{
    bool v3; // [esp+0h] [ebp-14h]
    unsigned __int64 src; // [esp+8h] [ebp-Ch] BYREF
    char v6; // [esp+13h] [ebp-1h]

    if ((HIDWORD(val) & (unsigned int)val) == 0xFFFFFFFF)
        return bdByteBuffer::writeNAN();
    v6 = bdByteBuffer::writeDataType(BD_BB_UNSIGNED_INTEGER64_TYPE);
    v3 = 0;
    if (v6)
    {
        src = val;
        if (bdByteBuffer::write((unsigned __int8 *)&src, 8u))
            return 1;
    }
    return v3;
}