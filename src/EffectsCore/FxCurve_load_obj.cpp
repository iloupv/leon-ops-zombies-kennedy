#include "FxCurve_load_obj.h"
#include <universal/com_memory.h>

const FxCurve *__cdecl FxCurve_AllocAndCreateWithKeys(float *keyArray, int dimensionCount, int keyCount)
{
    bool v4; // [esp+Ch] [ebp-24h]
    bool v5; // [esp+10h] [ebp-20h]
    int createdKeyCount; // [esp+14h] [ebp-1Ch]
    int keySize; // [esp+18h] [ebp-18h]
    int keyIndex; // [esp+20h] [ebp-10h]
    int keyIndexa; // [esp+20h] [ebp-10h]
    unsigned __int8 *newCurve; // [esp+28h] [ebp-8h]
    int elementIndex; // [esp+2Ch] [ebp-4h]
    int elementIndexa; // [esp+2Ch] [ebp-4h]

    if ( !keyArray
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    19,
                    0,
                    "%s",
                    "keyArray") )
    {
        __debugbreak();
    }
    if ( keyCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    20,
                    0,
                    "%s\n\t(keyCount) = %i",
                    "(keyCount > 0)",
                    keyCount) )
    {
        __debugbreak();
    }
    if ( dimensionCount <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    21,
                    0,
                    "%s\n\t(dimensionCount) = %i",
                    "(dimensionCount > 0)",
                    dimensionCount) )
    {
        __debugbreak();
    }
    keySize = dimensionCount + 1;
    v5 = *keyArray != 0.0;
    v4 = keyArray[(dimensionCount + 1) * (keyCount - 1)] != 1.0;
    createdKeyCount = v4 + keyCount + v5;
    if ( createdKeyCount < 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    29,
                    1,
                    "%s\n\t(createdKeyCount) = %i",
                    "(createdKeyCount >= 2)",
                    createdKeyCount) )
    {
        __debugbreak();
    }
    newCurve = Hunk_AllocAlign(4 * createdKeyCount * keySize + 8, 4, "FxCurve_AllocAndCreateWithKeys", 8);
    if ( !newCurve
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    38,
                    0,
                    "%s",
                    "newCurve") )
    {
        __debugbreak();
    }
    *(unsigned int *)newCurve = dimensionCount;
    keyIndex = 0;
    if ( v5 )
    {
        *((unsigned int *)newCurve + 2) = 0;
        for ( elementIndex = 0; elementIndex != dimensionCount; ++elementIndex )
            *(float *)&newCurve[4 * elementIndex + 12] = keyArray[elementIndex + 1];
        keyIndex = 1;
    }
    memcpy(&newCurve[4 * keySize * keyIndex + 8], (unsigned __int8 *)keyArray, 4 * keySize * keyCount);
    keyIndexa = keyCount + keyIndex;
    if ( v4 )
    {
        *(float *)&newCurve[4 * keySize * keyIndexa + 8] = 1.0f;
        for ( elementIndexa = 0; elementIndexa != dimensionCount; ++elementIndexa )
            *(float *)&newCurve[4 * keySize * keyIndexa + 12 + 4 * elementIndexa] = keyArray[elementIndexa + 1];
        ++keyIndexa;
    }
    if ( keyIndexa != createdKeyCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    61,
                    1,
                    "keyIndex == createdKeyCount\n\t%i, %i",
                    keyIndexa,
                    createdKeyCount) )
    {
        __debugbreak();
    }
    if ( *((float *)newCurve + 2) != 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    62,
                    1,
                    "%s\n\t(newCurve->keys[0 * keySize + 0]) = %g",
                    "(newCurve->keys[0 * keySize + 0] == 0.0f)",
                    *((float *)newCurve + 2)) )
    {
        __debugbreak();
    }
    if ( *(float *)&newCurve[4 * keySize * (createdKeyCount - 1) + 8] != 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\FxCurve_load_obj.cpp",
                    63,
                    1,
                    "%s\n\t(newCurve->keys[(createdKeyCount - 1) * keySize + 0]) = %g",
                    "(newCurve->keys[(createdKeyCount - 1) * keySize + 0] == 1.0f)",
                    *(float *)&newCurve[4 * keySize * (createdKeyCount - 1) + 8]) )
    {
        __debugbreak();
    }
    *((unsigned int *)newCurve + 1) = keyCount;
    return (const FxCurve *)newCurve;
}

