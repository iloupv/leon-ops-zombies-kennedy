#pragma once
#include <client/cl_compositing.h>

struct __declspec(align(8)) PCacheComponent // sizeof=0x20
{                                       // XREF: PCachePublicProfile/r
                                        // PCachePlayerEmblem/r
    unsigned __int64 xuid;
    int controllerIndex;
    int type;
    unsigned int updateTime;
    unsigned int touchTime;
    unsigned int state;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

struct PCacheComponentPool // sizeof=0x14
{                                       // XREF: .data:s_componentPools/r
    int count;                          // XREF: PCache_Init(void)+4E/r
    int componentSize;                  // XREF: PCache_Init(void)+54/r
    void *array;                        // XREF: PCache_Init(void)+64/r
    unsigned int updateInterval;        // XREF: PCache_TouchComponent(PCacheComponent *)+C3/r
    void (__cdecl *release)(PCacheComponent *);
};

struct PCachePublicProfile // sizeof=0x120
{                                       // XREF: .data:s_publicProfiles/r
    PCacheComponent c;
    char ddl[256];
};

struct PCacheEntry // sizeof=0x20
{                                       // XREF: .data:s_entries/r
    unsigned __int64 xuid;              // XREF: PCache_Init(void)+96/w
    int controllerIndex;
    unsigned int neighborhood;
    unsigned int hash;
    unsigned int touchTime;
    PCacheComponent *components[2];
};

struct PCachePlayerEmblem // sizeof=0x1B0
{                                       // XREF: .data:s_playerEmblems/r
    PCacheComponent c;
    CompositeEmblemLayer layers[12];
    int jobID;
    GfxImage *image;
    __int16 backgroundID;
    // padding byte
    // padding byte
    Material *background;
};

__int64 __cdecl PCache_Time();
void __cdecl PCache_Lock();
void __cdecl PCache_Unlock();
bool __cdecl PCache_IsLocked();
unsigned __int64 __cdecl PCache_Hash(int controllerIndex, unsigned __int64 xuid);
void __cdecl PCache_Init();
void __cdecl PCache_GetRank_f();
void __cdecl PCache_Update(int controllerIndex);
PCacheComponent *__cdecl PCache_GetOrphanComponent(int controllerIndex, unsigned __int64 xuid, unsigned int type);
PCacheEntry *__cdecl PCache_GetEntry(int controllerIndex, unsigned __int64 xuid);
PCacheComponent *__cdecl PCache_GetEntryComponent(PCacheEntry *entry, unsigned int type);
PCacheComponent *__cdecl PCache_GetComponent(int controllerIndex, unsigned __int64 xuid, unsigned int type);
bool __cdecl PCache_TouchComponent(PCacheComponent *component);
void __cdecl PCache_InvalidateComponent(PCacheComponent *component);
void __cdecl PCache_ReleasePlayerEmblem(PCacheComponent *comp);
void __cdecl PCache_BatchUpdatePlayerEmblems(int controllerIndex, PCachePlayerEmblem *emblems, int count);
int __cdecl PCache_GetClientNumForXuid(int localClientNum, unsigned __int64 xuid);
void __cdecl PCache_InvalidatePlayerEmblem(int controllerIndex, unsigned __int64 xuid);
GfxImage *__cdecl PCache_GetPlayerEmblem(int controllerIndex, unsigned __int64 xuid);
__int16 __cdecl PCache_GetPlayerEmblemBackgroundID(int controllerIndex, unsigned __int64 xuid);
