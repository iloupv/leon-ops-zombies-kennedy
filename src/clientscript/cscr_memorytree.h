#pragma once
#include "cscr_main.h"
#include <universal/q_shared.h>

struct MemoryNode // sizeof=0x10
{                                       // XREF: scrMemTreeGlob_t/r
                                        // MT_RemoveHeadMemoryNode/r ...
    unsigned __int16 prev;              // XREF: MT_Init(scriptInstance_t)+6D/w
                                        // MT_AddMemoryNode+2FD/w ...
    unsigned __int16 next;              // XREF: MT_Init(scriptInstance_t)+7F/w
                                        // MT_AddMemoryNode+224/o ...
    unsigned int padding[3];            // XREF: MT_RemoveHeadMemoryNode+83/w
                                        // MT_RemoveHeadMemoryNode+89/w ...
};

struct __declspec(align(128)) scrMemTreeGlob_t // sizeof=0x100380
{                                       // XREF: .data:scrMemTreeGlob_t * gScrMemTreeGlob/r
    MemoryNode nodes[65536];            // XREF: MT_Init(scriptInstance_t)+6D/w
                                        // MT_Init(scriptInstance_t)+7F/w ...
    unsigned __int8 leftBits[256];      // XREF: MT_InitBits+9C/w
                                        // MT_GetScore+B1/r ...
    unsigned __int8 numBits[256];       // XREF: MT_InitBits+62/w
                                        // MT_GetScore+7F/r ...
    unsigned __int8 logBits[256];       // XREF: MT_InitBits+D8/w
                                        // MT_GetSize+65/r ...
    unsigned __int16 head[17];          // XREF: MT_DumpTree(scriptInstance_t)+19C/r
                                        // MT_Init(scriptInstance_t)+58/w ...
    // padding byte
    // padding byte
    int totalAlloc;                     // XREF: MT_DumpTree(scriptInstance_t)+EC/r
                                        // MT_DumpTree(scriptInstance_t)+259/r ...
    int totalAllocBuckets;              // XREF: MT_DumpTree(scriptInstance_t)+126/r
                                        // MT_DumpTree(scriptInstance_t)+16F/r ...
    int avgAlloc;                       // XREF: MT_DumpTree(scriptInstance_t)+2CC/r
                                        // MT_AllocIndex(scriptInstance_t,int,int)+133/r ...
    int avgAllocBytes;                  // XREF: MT_DumpTree(scriptInstance_t)+2BB/r
                                        // MT_AllocIndex(scriptInstance_t,int,int)+154/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct scrMemTreePub_t // sizeof=0x4
{
    char *mt_buffer;
};

struct scrMemTreeDebugGlob_t // sizeof=0x20000
{                                       // XREF: scrMemTreeDebugGlobArray_t/r
    unsigned __int8 mt_usage[65536];
    unsigned __int8 mt_usage_size[65536];
};

void __cdecl MT_DumpTree(scriptInstance_t inst);
int __cdecl MT_GetSubTreeSize(scriptInstance_t inst, int nodeNum);
const char *__cdecl MT_NodeInfoString(scriptInstance_t inst, unsigned int nodeNum);
int __cdecl Scr_GetStringUsage(scriptInstance_t inst);
void __cdecl MT_Init(scriptInstance_t inst);
void __cdecl MT_InitBits(scriptInstance_t inst);
void __cdecl MT_AddMemoryNode(scriptInstance_t inst, int newNode, unsigned int size);
int __cdecl MT_GetScore(scriptInstance_t inst, int num);
unsigned __int16 __cdecl MT_AllocIndex(scriptInstance_t inst, int numBytes, int type);
void __cdecl MT_RemoveHeadMemoryNode(scriptInstance_t inst, unsigned int size);
void __cdecl MT_Error(scriptInstance_t inst, const char *funcName, int numBytes);
int __cdecl MT_GetSize(scriptInstance_t inst, int numBytes);
void __cdecl MT_FreeIndex(scriptInstance_t inst, unsigned int nodeNum, int numBytes);
char __cdecl MT_RemoveMemoryNode(scriptInstance_t inst, int oldNode, unsigned int size);
char *__cdecl MT_Alloc(int numBytes, int type, scriptInstance_t inst);
void __cdecl MT_Free(_BYTE *p, int numBytes, scriptInstance_t inst);
bool __cdecl MT_Realloc(scriptInstance_t inst, int oldNumBytes, int newNumbytes);


extern scrMemTreeGlob_t gScrMemTreeGlob[2];
extern scrMemTreePub_t gScrMemTreePub[2];
extern scrMemTreeDebugGlob_t gScrMemTreeDebugGlob[2];