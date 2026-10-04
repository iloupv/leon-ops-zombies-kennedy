#pragma once

#include "bdCypher.h"

#define LTC_DES 1
#define LTC_CBC_MODE 1
#include <libs/libtomcrypt-1.17/src/headers/tomcrypt.h>
//#include <libs/libtomcrypt-1.17/src/headers/tomcrypt_cipher.h>

struct bdCypher3Des : bdCypher // sizeof=0x38C
{                                       // XREF: bdLobbyConnection/r
    symmetric_CBC m_cbc;

    bdCypher3Des();
    virtual ~bdCypher3Des() = default;

    bool init(const unsigned char *buf, unsigned int size);
    virtual bool encrypt(const unsigned __int8 *, const unsigned __int8 *, unsigned __int8 *, const unsigned int) override;
    virtual bool decrypt(const unsigned __int8 *, const unsigned __int8 *, unsigned __int8 *, const unsigned int) override;
};