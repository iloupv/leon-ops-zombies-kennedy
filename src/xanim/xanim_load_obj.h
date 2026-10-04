#pragma once
#include "xanim.h"

union XAnimPartQuatFrames // sizeof=0x4
{                                       // XREF: XAnimPartQuatDataFrames/r
    __int16 (*frames)[4];
    __int16 (*frames2)[2];
};

struct __declspec(align(4)) XAnimPartQuatDataFrames // sizeof=0x8
{                                       // XREF: XAnimPartQuatData/r
    XAnimPartQuatFrames u;
    XAnimDynamicIndices indices;
    // padding byte
    // padding byte
};

union XAnimPartQuatData // sizeof=0x8
{                                       // XREF: XAnimPartQuat/r
    XAnimPartQuatDataFrames frames;
    __int16 frame0[4];
    __int16 frame02[2];
};

struct XAnimPartQuat // sizeof=0xC
{
    unsigned __int16 size;
    // padding byte
    // padding byte
    XAnimPartQuatData u;
};

struct __declspec(align(4)) XAnimPartQuatPtr // sizeof=0x8
{                                       // XREF: ?XAnimLoadFile@@YAPAUXAnimParts@@PBDP6APAXH@Z_N@Z/r
    XAnimPartQuat *quat;                // XREF: XAnimLoadFile(char const *,void * (*)(int),bool)+BD3/r
    unsigned __int8 partIndex;          // XREF: XAnimLoadFile(char const *,void * (*)(int),bool)+799/w
                                        // XAnimLoadFile(char const *,void * (*)(int),bool)+BB3/w ...
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) XAnimPartTransPtr // sizeof=0x8
{                                       // XREF: ?XAnimLoadFile@@YAPAUXAnimParts@@PBDP6APAXH@Z_N@Z/r
    XAnimPartTrans *trans;              // XREF: XAnimLoadFile(char const *,void * (*)(int),bool)+E13/r
    unsigned __int8 partIndex;          // XREF: XAnimLoadFile(char const *,void * (*)(int),bool)+7AC/w
                                        // XAnimLoadFile(char const *,void * (*)(int),bool)+BC6/w ...
    // padding byte
    // padding byte
    // padding byte
};

unsigned __int8 *__cdecl GetDeltaQuaternions(
                XAnimDeltaPart *deltaPart,
                void *(__cdecl *Alloc)(int),
                unsigned __int8 *pos,
                unsigned __int16 numloopframes,
                bool useSmallIndices);
void __cdecl ConsumeQuat2(const unsigned __int8 **pos, __int16 *out);
unsigned __int8 *__cdecl GetDeltaTranslations(
                const char *name,
                XAnimDeltaPart *deltaPart,
                void *(__cdecl *Alloc)(int),
                const unsigned __int8 *pos,
                unsigned __int16 numloopframes,
                bool useSmallIndices);
unsigned __int8 *__cdecl LoadTrans(
                void *(__cdecl *Alloc)(int),
                unsigned __int8 *pos,
                unsigned __int16 numTransIndices,
                XAnimPartTrans *trans);
XAnimParts *__cdecl XAnimLoadFile(char *name, void *(__cdecl *Alloc)(int), bool streamedVersion);
void __cdecl ReadNoteTracks(const char *name, unsigned __int16 **pos, XAnimParts *parts, void *(__cdecl *Alloc)(int));
unsigned __int8 *__cdecl GetQuaternions(
                XAnimPartQuatPtr *part,
                unsigned __int8 *pos,
                bool bFlipQuat,
                bool bSimpleQuat,
                unsigned __int16 numloopframes,
                bool useSmallIndices);
void __cdecl ConsumeQuat(const unsigned __int8 **pos, __int16 *out);
void *__cdecl XAnimTempAlloc(int size);
unsigned __int8 *__cdecl GetTranslations(
                XAnimPartTransPtr *part,
                const unsigned __int8 *pos,
                unsigned __int16 numloopframes,
                bool useSmallIndices);
unsigned int __cdecl XAnimGetPartQuatType(unsigned int animPartIndex);
unsigned int __cdecl XAnimGetPartTransType(unsigned int animPartIndex);
int __cdecl XAnimCompareQuatParts(unsigned __int16 *p0, unsigned __int16 *p1);
int __cdecl XAnimCompareTransParts(unsigned __int16 *p0, unsigned __int16 *p1);
int __cdecl XAnimIsHighPrecisionPart(const char *name);
void __cdecl XAnimEmitFrameIndices(
                unsigned int tableSize,
                unsigned __int8 **dataByte,
                __int16 **dataShort,
                XAnimIndices *indices,
                XAnimDynamicIndices *frameIndices,
                bool useSmallIndices);
