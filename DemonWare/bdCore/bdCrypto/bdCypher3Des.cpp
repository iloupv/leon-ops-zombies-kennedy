#include "bdCypher3Des.h"

#include <cstring>

#include <DemonWare/bdPlatform/bdPlatformLog/bdPlatformLog.h>
#include <libs/libtomcrypt-1.17/src/headers/tomcrypt_misc.h>

static void des3_done(symmetric_key *skey)
{
    //LTC_UNUSED_PARAM(skey);
    // 3DES has no dynamic key state to free
}

//ltc_cipher_descriptor des3_desc =
//{
//    (char *)"3des",            // name
//    14,                        // ID (LTC_CIPHER_DES3)
//    24,                        // min key length (3 × 8 bytes)
//    24,                        // max key length
//    8,                         // block length (DES block size)
//    0,                         // default rounds (0 = use default)
//    &des3_setup,
//    &des3_ecb_encrypt,
//    &des3_ecb_decrypt,
//    &des3_test,
//    &des3_done,
//    &des3_keysize
//};
//
//
//

bdCypher3Des::bdCypher3Des()
{
    //this->__vftable = (bdCypher3Des_vtbl *)&bdCypher::`vftable';
    //this->__vftable = (bdCypher3Des_vtbl *)&bdCypher3Des::`vftable';
#if 0
    if (register_cipher(&des3_desc) == -1)
            bdLogMessage(
                BD_LOG_ERROR,
                "err/",
                "cypher3DES",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
                "bdCypher3Des::bdCypher3Des",
                0x17u,
                "Error registering cipher.");
#endif
}

bool bdCypher3Des::init(const unsigned char *buf, unsigned int size)
{
    int cipher; // eax
    unsigned __int8 dst[8]; // [esp+4h] [ebp-10h] BYREF

    memset(dst, 0, sizeof(dst));
    cipher = find_cipher("3des");
    if (!cbc_start(cipher, dst, buf, size, 0, &this->m_cbc))
        return 1;
    bdLogMessage(
        BD_LOG_ERROR,
        "err/",
        "cypher3DES",
        "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
        "bdCypher3Des::init",
        0x2Du,
        "Error starting cipher.");
    return 0;
}

bool bdCypher3Des::encrypt(
    const unsigned __int8 *src,
    const unsigned __int8 *a3,
    unsigned __int8 *a4,
    unsigned int a5)
{
    const char *v5; // eax
    const char *v7; // eax
    int v9; // [esp+4h] [ebp-8h]
    int v10; // [esp+4h] [ebp-8h]

    v9 = cbc_setiv(src, 8u, &this->m_cbc);
    if (v9)
    {
        bdLogMessage(
            BD_LOG_ERROR,
            "err/",
            "cypher3DES",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
            "bdCypher3Des::encrypt",
            0x3Cu,
            "Failed to set IV seed");
        v5 = (const char *)error_to_string(v9);
        bdLogMessage(
            BD_LOG_ERROR,
            "err/",
            "cypher3DES",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
            "bdCypher3Des::encrypt",
            0x3Du,
            "> %s",
            v5);
        return 0;
    }
    else
    {
        v10 = cbc_encrypt(a3, a4, a5, &this->m_cbc);
        if (v10)
        {
            bdLogMessage(
                BD_LOG_ERROR,
                "err/",
                "cypher3DES",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
                "bdCypher3Des::encrypt",
                0x4Bu,
                "Error encrypting ");
            v7 = (const char *)error_to_string(v10);
            bdLogMessage(
                BD_LOG_ERROR,
                "err/",
                "cypher3DES",
                "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
                "bdCypher3Des::encrypt",
                0x4Cu,
                "> %s",
                v7);
            return 0;
        }
        else
        {
            return 1;
        }
    }
}

bool bdCypher3Des::decrypt(
    const unsigned __int8 *src,
    const unsigned __int8 *a3,
    unsigned __int8 *a4,
    unsigned int a5)
{
    const char *v5; // eax
    int v8; // [esp+4h] [ebp-8h]

    v8 = cbc_setiv(src, 8u, &this->m_cbc);
    if (v8)
    {
        bdLogMessage(
            BD_LOG_ERROR,
            "err/",
            "cypher3DES",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
            "bdCypher3Des::decrypt",
            0x5Du,
            "Failed to set IV seed");
        v5 = (const char *)error_to_string(v8);
        bdLogMessage(
            BD_LOG_ERROR,
            "err/",
            "cypher3DES",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
            "bdCypher3Des::decrypt",
            0x5Eu,
            "> %s",
            v5);
        return 0;
    }
    else if (cbc_decrypt(a3, a4, a5, &this->m_cbc))
    {
        bdLogMessage(
            BD_LOG_ERROR,
            "err/",
            "cypher3DES",
            "C:\\projects_pc\\cod\\codsrc\\DemonWare\\bdCore\\bdCrypto\\bdCypher3Des.cpp",
            "bdCypher3Des::decrypt",
            0x6Au,
            "Error decrypting.");
        return 0;
    }
    else
    {
        return 1;
    }
}