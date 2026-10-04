#include "com_encode.h"
#include "assertive.h"

int __cdecl Com_EncodeYEnc(unsigned __int8 *indata, int inlen, char *outdata, int outlen)
{
    char c; // [esp+3h] [ebp-9h]
    int len; // [esp+4h] [ebp-8h]
    int in; // [esp+8h] [ebp-4h]

    len = 0;
    for ( in = 0; in < inlen; ++in )
    {
        c = indata[in] + 42;
        if ( YEncIsCritical(c) )
        {
            c += 64;
            if ( len + 1 == outlen )
            {
                *outdata = 0;
                return 0;
            }
            outdata[len++] = 61;
        }
        if ( len + 1 == outlen )
        {
            *outdata = 0;
            return 0;
        }
        if ( YEncIsCritical(c)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_encode.cpp",
                        49,
                        0,
                        "%s",
                        "!YEncIsCritical( c )") )
        {
            __debugbreak();
        }
        outdata[len++] = c;
    }
    if ( len >= outlen
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_encode.cpp", 54, 0, "%s", "len < outlen") )
    {
        __debugbreak();
    }
    outdata[len] = 0;
    return len;
}

bool __cdecl YEncIsCritical(unsigned __int8 c)
{
    return !c || c == 61 || c == 10 || c == 13 || c == 127 || c == 92 || c == 37 || c == 146 || c == 34 || c == 59;
}

int __cdecl Com_DecodeYEnc(const char *indata, unsigned __int8 *outdata, int outlen)
{
    unsigned __int8 c; // [esp+3h] [ebp-9h]
    int len; // [esp+4h] [ebp-8h]
    bool escape; // [esp+Bh] [ebp-1h]

    escape = 0;
    len = 0;
    while ( *indata )
    {
        c = *indata;
        if ( escape || c != 61 )
        {
            if ( len == outlen )
                return 0;
            if ( escape )
            {
                c -= 64;
                escape = 0;
            }
            outdata[len++] = c - 42;
        }
        else
        {
            escape = 1;
        }
        ++indata;
    }
    return len;
}

