#include "com_pack.h"
#include "com_math.h"

#include <cfloat>
#include "q_shared.h"

PackedUnitVec __cdecl Vec3PackUnitVec(const float *unitVec)
{
    float v2; // [esp+0h] [ebp-8Ch]
    float v3; // [esp+4h] [ebp-88h]
    float v4; // [esp+40h] [ebp-4Ch]
    float v5; // [esp+44h] [ebp-48h]
    PackedUnitVec out; // [esp+58h] [ebp-34h]
    unsigned __int8 testEncoding[4]; // [esp+5Ch] [ebp-30h]
    float decodeScale; // [esp+60h] [ebp-2Ch]
    float encodeScale; // [esp+64h] [ebp-28h]
    float normalized[3]; // [esp+68h] [ebp-24h] BYREF
    float bestLenError; // [esp+74h] [ebp-18h]
    float bestDirError; // [esp+78h] [ebp-14h]
    float lenError; // [esp+7Ch] [ebp-10h]
    float decoded[3]; // [esp+80h] [ebp-Ch] BYREF

    Vec3NormalizeTo(unitVec, normalized);
    out.packed = 0;
    bestDirError = FLT_MAX;
    bestLenError = FLT_MAX;
    testEncoding[3] = 0;
    do
    {
        encodeScale = 32385.0 / ((double)testEncoding[3] - -192.0);
        testEncoding[0] = (int)(normalized[0] * encodeScale + 127.5);
        testEncoding[1] = (int)(normalized[1] * encodeScale + 127.5);
        testEncoding[2] = (int)(normalized[2] * encodeScale + 127.5);
        decodeScale = ((double)testEncoding[3] - -192.0) / 32385.0;
        decoded[0] = ((double)testEncoding[0] - 127.0) * decodeScale;
        decoded[1] = ((double)testEncoding[1] - 127.0) * decodeScale;
        decoded[2] = ((double)testEncoding[2] - 127.0) * decodeScale;
        v5 = Vec3Normalize(decoded) - 1.0;
        v3 = fabs(v5);
        lenError = v3;
        if (v3 < 0.001f)
        {
            v4 = Vec3Dot(decoded, normalized) - 1.0f;
            v2 = fabs(v4);
            if (v2 < (double)bestDirError || v2 == bestDirError && lenError < (double)bestLenError)
            {
                bestDirError = v2;
                bestLenError = lenError;
                out.packed = *(unsigned int *)testEncoding;
                if (lenError + v2 == 0.0)
                    return *(PackedUnitVec *)testEncoding;
            }
        }
        ++testEncoding[3];
    } while (testEncoding[3]);

    iassert(out.packed != 0);

    return out;
}

void __cdecl Vec3UnpackUnitVec(PackedUnitVec in, float *out)
{
    float decodeScale; // [esp+10h] [ebp-4h]

    decodeScale = (in.array[3] - -192.0f) / 32385.0f;

    out[0] = (in.array[0] - 127.0f) * decodeScale;
    out[1] = (in.array[1] - 127.0f) * decodeScale;
    out[2] = (in.array[2] - 127.0f) * decodeScale;
}

int __cdecl Vec4PackQuat(const float *in)
{
    float v2; // [esp+8h] [ebp-58h]
    unsigned int maxIndex; // [esp+2Ch] [ebp-34h]
    float rotatedValue; // [esp+30h] [ebp-30h]
    float rotatedValue_4; // [esp+34h] [ebp-2Ch]
    float rotatedValue_8; // [esp+38h] [ebp-28h]
    float rotatedValue_12; // [esp+3Ch] [ebp-24h]
    unsigned int i; // [esp+40h] [ebp-20h]
    unsigned int x; // [esp+44h] [ebp-1Ch]
    unsigned int y; // [esp+48h] [ebp-18h]
    float value[4]; // [esp+4Ch] [ebp-14h] BYREF
    float maxValue; // [esp+5Ch] [ebp-4h]

    value[0] = *in;
    value[1] = in[1];
    value[2] = in[2];
    value[3] = in[3];
    Vec4Normalize(value);
    maxValue = -1.0f;
    maxIndex = 0;
    for ( i = 0; i < 4; ++i )
    {
        if ( maxValue < fabs(value[i]) )
        {
            maxValue = fabs(value[i]);
            maxIndex = i;
        }
    }
    rotatedValue_12 = value[maxIndex];
    rotatedValue = value[((_BYTE)maxIndex + 1) & 3];
    rotatedValue_4 = value[((_BYTE)maxIndex + 2) & 3];
    rotatedValue_8 = value[((_BYTE)maxIndex + 3) & 3];
    if ( rotatedValue_12 == 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_pack.cpp", 237, 0, "%s", "rotatedValue[3]") )
    {
        __debugbreak();
    }
    if ( rotatedValue_12 < 0.0 )
        v2 = -1.0f;
    else
        v2 = 1.0f;
    x = FloatPack9S((float)(1.0 / rotatedValue_12) * rotatedValue);
    y = FloatPack10S((float)(1.0 / rotatedValue_12) * rotatedValue_4);
    return ((v2 < 0.0) << 31)
             | (((3 - (_BYTE)maxIndex) & 3) << 29)
             | (FloatPack10S((float)(1.0 / rotatedValue_12) * rotatedValue_8) << 19)
             | x
             | (y << 9);
}

int __cdecl FloatPack9S(float unitFloat)
{
    float v1; // xmm0_4

    v1 = (unitFloat - -49344.75294117647) / 16448.25098039216;
    return LOWORD(v1) & 0x1FF;
}

int __cdecl FloatPack10S(float unitFloat)
{
    float v1; // xmm0_4

    v1 = (unitFloat - -24624.0939334638) / 8208.031311154598;
    return LOWORD(v1) & 0x3FF;
}

PackedTexCoords __cdecl Vec2PackTexCoords(const float *in)
{
    __int16 v2; // [esp+0h] [ebp-3Ch]
    __int16 v3; // [esp+4h] [ebp-38h]
    int v4; // [esp+Ch] [ebp-30h]
    int v5; // [esp+18h] [ebp-24h]
    int v6; // [esp+20h] [ebp-1Ch]

    if ( (int)((2 * *(unsigned int *)in) ^ 0x80000000) >> 14 < 0x3FFF )
        v6 = (int)((2 * *(unsigned int *)in) ^ 0x80000000) >> 14;
    else
        v6 = 0x3FFF;
    if ( v6 > -16384 )
        v3 = v6;
    else
        v3 = -16384;
    v5 = *((unsigned int *)in + 1);
    if ( (int)((2 * v5) ^ 0x80000000) >> 14 < 0x3FFF )
        v4 = (int)((2 * v5) ^ 0x80000000) >> 14;
    else
        v4 = 0x3FFF;
    if ( v4 > -16384 )
        v2 = v4;
    else
        v2 = -16384;
    return (PackedTexCoords)((v2 & 0x3FFF | (v5 >> 16) & 0xC000) + ((v3 & 0x3FFF | (*(int *)in >> 16) & 0xC000) << 16));
}

void __cdecl Vec2UnpackTexCoords(PackedTexCoords in, float *out)
{
    unsigned int v2; // [esp+0h] [ebp-20h]
    unsigned int v3; // [esp+10h] [ebp-10h]

    if ( HIWORD(in.packed) )
        v3 = (HIWORD(in.packed) << 16) & 0x80000000
             | (((((HIWORD(in.packed) << 14) & 0xFFFC000) - (~(HIWORD(in.packed) << 14) & 0x10000000)) ^ 0x80000000) >> 1);
    else
        v3 = 0;
    *(unsigned int *)out = v3;
    if ( LOWORD(in.packed) )
        v2 = (LOWORD(in.packed) << 16) & 0x80000000
             | (((((LOWORD(in.packed) << 14) & 0xFFFC000) - (~(LOWORD(in.packed) << 14) & 0x10000000)) ^ 0x80000000) >> 1);
    else
        v2 = 0;
    *((unsigned int *)out + 1) = v2;
}

void __cdecl Byte4UnpackRgba(const unsigned __int8 *from, float *to)
{
    to[0] = (float)((double)from[0] * 0.003921568859368563);
    to[1] = (float)((double)from[1] * 0.003921568859368563);
    to[2] = (float)((double)from[2] * 0.003921568859368563);
    to[3] = (float)((double)from[3] * 0.003921568859368563);
}