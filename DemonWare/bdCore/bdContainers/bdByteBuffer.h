#pragma once

#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include "bdBitBuffer.h"

struct __declspec(align(2)) bdByteBuffer : bdReferencable // sizeof=0x1C
{                                       // XREF: bdTaskByteBuffer/r
    unsigned int m_size;
    unsigned __int8 *m_data;
    unsigned __int8 *m_readPtr;
    unsigned __int8 *m_writePtr;
    bool m_typeChecked;
    bool m_typeCheckedCopy;
    bool m_allocatedData;
    // padding byte


    bdByteBuffer(unsigned int size, bool typeChecked);
    bdByteBuffer(unsigned __int8 *src, unsigned int count, bool typeChecked);

    ~bdByteBuffer();

    void allocateBuffer();
    int getDataSize();
    bool read(unsigned __int8 *dst, unsigned int count);
    bool readAndAllocateBlob(unsigned __int8 **src, unsigned int *count);
    void readArrayEnd();
    bool readArrayStart(char type, unsigned int *buf);
    bool readBlob(unsigned __int8 *const dst, unsigned int *pCount);
    bool readBool(bool *buf);
    bool readDataType(enum bdBitBufferDataType type);
    bool readInt32(int *buf);
    bool readInt64(__int64 *a2);
    bool readString(char *const buf, unsigned int count);
    bool readUByte8(unsigned __int8 *buf);
    bool readUInt16(unsigned __int16 *buf);
    bool readUInt32(unsigned int *buf);
    bool readUInt64(unsigned __int64 *buf);
    bool write(unsigned __int8 *src, unsigned int count);
    bool writeArrayStart(char val, unsigned int size, unsigned int stride);
    bool writeBlob(unsigned __int8 *src, unsigned int count);
    bool writeBool(unsigned __int8 val);
    bool writeDataType(enum bdBitBufferDataType a2);
    bool writeInt32(int val);
    bool writeInt64(__int64 val);
    bool writeNAN();
    bool writeNoType();
    bool writeString(char *buf, unsigned int cnt);
    bool writeUByte8(unsigned __int8 val);
    bool writeUInt16(unsigned __int16 val);
    bool writeUInt32(unsigned int val);
    bool writeUInt64(unsigned __int64 val);
};