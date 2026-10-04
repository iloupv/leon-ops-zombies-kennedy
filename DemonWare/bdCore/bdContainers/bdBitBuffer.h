#pragma once

#include <DemonWare/bdCore/bdReference/bdReferencable.h>
#include "bdFastArray.h"

enum bdBitBufferDataType : __int32
{                                       // XREF: ?writeDataType@bdByteBuffer@@IAE_NW4bdBitBufferDataType@@@Z/r
                                        // ?readDataType@bdByteBuffer@@IAE_NW4bdBitBufferDataType@@@Z/r ...
    BD_BB_NO_TYPE                    = 0x0,
    BD_BB_BOOL_TYPE                  = 0x1,
    BD_BB_SIGNED_CHAR8_TYPE          = 0x2,
    BD_BB_UNSIGNED_CHAR8_TYPE        = 0x3,
    BD_BB_WCHAR16_TYPE               = 0x4,
    BD_BB_SIGNED_INTEGER16_TYPE      = 0x5,
    BD_BB_UNSIGNED_INTEGER16_TYPE    = 0x6,
    BD_BB_SIGNED_INTEGER32_TYPE      = 0x7,
    BD_BB_UNSIGNED_INTEGER32_TYPE    = 0x8,
    BD_BB_SIGNED_INTEGER64_TYPE      = 0x9,
    BD_BB_UNSIGNED_INTEGER64_TYPE    = 0xA,
    BD_BB_RANGED_SIGNED_INTEGER32_TYPE = 0xB,
    BD_BB_RANGED_UNSIGNED_INTEGER32_TYPE = 0xC,
    BD_BB_FLOAT32_TYPE               = 0xD,
    BD_BB_FLOAT64_TYPE               = 0xE,
    BD_BB_RANGED_FLOAT32_TYPE        = 0xF,
    BD_BB_SIGNED_CHAR8_STRING_TYPE   = 0x10,
    BD_BB_UNSIGNED_CHAR8_STRING_TYPE = 0x11,
    BD_BB_MBSTRING_TYPE              = 0x12,
    BD_BB_BLOB_TYPE                  = 0x13,
    BD_BB_NAN_TYPE                   = 0x14,
    BD_BB_FULL_TYPE                  = 0x15,
    BD_BB_MAX_TYPE                   = 0x20,
};

struct __declspec(align(4)) bdBitBuffer : bdReferencable // sizeof=0x24
{                                       // XREF: ?update@bdLANDiscoveryServer@@QAEXXZ/r
                                        // ?update@bdLANDiscoveryServer@@QAEXXZ/r
    bdFastArray<unsigned char> m_data;  // XREF: bdLANDiscoveryServer::update(void)+249/r
                                        // bdLANDiscoveryServer::update(void)+255/r
    unsigned int m_writePosition;
    unsigned int m_maxWritePosition;
    unsigned int m_readPosition;
    bool m_failedRead;
    bool m_typeChecked;
    // padding byte
    // padding byte


    bdBitBuffer(unsigned int size, bool typeChecked);

    //~bdBitBuffer(); // should be implicit

    bool getTypeCheck();
    bool readBits(char *buf, unsigned int size);
    bool readDataType(enum bdBitBufferDataType type);
    bool readInt32(int *arg0);
    bool readRangedUInt32(
        unsigned int *buf,
        unsigned int start,
        unsigned int end,
        bool typeChecked);

    bool readUByte8(char *buf);
    bool readUInt32(unsigned int *arg0);
    void setTypeCheck(bool val);
    static void typeToString(enum bdBitBufferDataType a1, char *const dst, unsigned int size);
    void writeBits(unsigned char *buf, unsigned int size);
    bool writeBool(bool val);
    void writeDataType(enum bdBitBufferDataType a2);
    void writeRangedUInt32(
        unsigned int buf,
        unsigned int start,
        unsigned int end,
        bool typeChecked);
};