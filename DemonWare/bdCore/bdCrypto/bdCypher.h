#pragma once

struct bdCypher // sizeof=0x4
{                                       // XREF: bdCypher3Des/r
    //bdCypher_vtbl *__vftable;
    virtual ~bdCypher() = default;
    virtual bool encrypt(const unsigned __int8 *, const unsigned __int8 *, unsigned __int8 *, const unsigned int) = 0;
    virtual bool decrypt(const unsigned __int8 *, const unsigned __int8 *, unsigned __int8 *, const unsigned int) = 0;
};