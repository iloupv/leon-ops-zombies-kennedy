#pragma once

enum EMemTrack : __int32
{                                       // XREF: PhysicalMemoryPrim/r
                                        // ?PMem_BeginAlloc@@YAXPBDIW4EMemTrack@@@Z/r ...
    TRACK_DEBUG                 = 0x0,
    TRACK_HUNK                  = 0x1,
    TRACK_BINARIES              = 0x2,
    TRACK_MISC_SWAP             = 0x3,
    TRACK_DELIMITER1            = 0x4,
    TRACK_AI                    = 0x5,
    TRACK_AI_NODES              = 0x6,
    TRACK_SCRIPT                = 0x7,
    TRACK_FX                    = 0x8,
    TRACK_GLASS                 = 0x9,
    TRACK_NETWORK_ENTITY        = 0xA,
    TRACK_MISC                  = 0xB,
    TRACK_FASTFILE              = 0xC,
    TRACK_ANIMATION             = 0xD,
    TRACK_WORLD_GLOBALS         = 0xE,
    TRACK_SOUND_GLOBALS         = 0xF,
    TRACK_CLIENT_ANIMSCRIPT     = 0x10,
    TRACK_SOUND                 = 0x11,
    TRACK_DELIMITER2            = 0x12,
    TRACK_RENDERER_GLOBALS      = 0x13,
    TRACK_RENDERER_IMAGES       = 0x14,
    TRACK_RENDERER_WORLD        = 0x15,
    TRACK_RENDERER_MODELS       = 0x16,
    TRACK_RENDERER_MISC         = 0x17,
    TRACK_CINEMATICS            = 0x18,
    TRACK_DELIMITER3            = 0x19,
    TRACK_COLLISION_MISC        = 0x1A,
    TRACK_COLLISION_BRUSH       = 0x1B,
    TRACK_COLLISION_MODEL_TRI   = 0x1C,
    TRACK_COLLISION_TERRAIN     = 0x1D,
    TRACK_PHYSICS               = 0x1E,
    TRACK_MAP_ENTS              = 0x1F,
    TRACK_TEMP                  = 0x20,
    TRACK_DELIMITER4            = 0x21,
    TRACK_LOCALIZATION          = 0x22,
    TRACK_FLAME                 = 0x23,
    TRACK_UI                    = 0x24,
    TRACK_TL                    = 0x25,
    TRACK_ZMEM                  = 0x26,
    TRACK_FIREMANAGER           = 0x27,
    TRACK_PROFILE               = 0x28,
    TRACK_WATERSIM              = 0x29,
    TRACK_CLIENT                = 0x2A,
    TRACK_RECORDER              = 0x2B,
    TRACK_RSTREAM               = 0x2C,
    TRACK_RENDERER_STREAMBUFFER = 0x2D,
    TRACK_RENDERER_STREAMBUFFER_EXTRA = 0x2E,
    TRACK_GEOSTREAM             = 0x2F,
    TRACK_DDL                   = 0x30,
    TRACK_ONLINE                = 0x31,
    TRACK_EMBLEM                = 0x32,
    TRACK_MINSPEC_IMAGES        = 0x33,
    TRACK_DELIMITER5            = 0x34,
    TRACK_NONE                  = 0x35,
    TRACK_COUNT                 = 0x36,
};

struct PhysicalMemoryAllocation // sizeof=0x8
{                                       // XREF: PhysicalMemoryPrim/r
    const char *name;
    unsigned int pos;
};

struct PhysicalMemoryPrim // sizeof=0x110
{                                       // XREF: PhysicalMemory/r
    const char *allocName;              // XREF: _PMem_Alloc(uint,uint,uint,uint,EMemTrack,char const *,int)+24/r
    unsigned int allocListCount;
    unsigned int pos;
    PhysicalMemoryAllocation allocList[32];
    EMemTrack memTrack;
};

struct PhysicalMemory // sizeof=0x22C
{                                       // XREF: .data:g_mem/r
    const char *name;
    unsigned __int8 *buf;
    PhysicalMemoryPrim prim[2];         // XREF: PMem_BeginAlloc(char const *,uint,EMemTrack)+59/o
                                        // PMem_EndAlloc(char const *,uint)+3D/o ...
    unsigned int size;
};

void __cdecl PMem_Init();
void __cdecl PMem_InitPhysicalMemory(
                PhysicalMemory *pmem,
                const char *name,
                unsigned __int8 *memory,
                unsigned int memorySize);
void __cdecl PMem_BeginAlloc(const char *name, unsigned int allocType, EMemTrack memTrack);
void __cdecl PMem_BeginAllocInPrim(PhysicalMemoryPrim *prim, const char *name, EMemTrack memTrack);
void __cdecl PMem_EndAlloc(const char *name, unsigned int allocType);
void __cdecl PMem_EndAllocInPrim(PhysicalMemoryPrim *prim, const char *name);
void __cdecl PMem_Free(const char *name);
void __cdecl PMem_FreeInPrim(PhysicalMemoryPrim *prim, const char *name, int location);
void __cdecl PMem_FreeIndex(PhysicalMemoryPrim *prim, unsigned int allocIndex, int location);
int __cdecl PMem_GetOverAllocatedSize();
unsigned __int8 *__cdecl _PMem_AllocNamed(
                unsigned int size,
                unsigned int alignment,
                signed int type,
                unsigned int allocType,
                const char *name,
                EMemTrack memTrack);
unsigned __int8 *__cdecl _PMem_Alloc(
                unsigned int size,
                unsigned int alignment,
                unsigned int type,
                unsigned int allocType,
                EMemTrack memTrack,
                const char *file,
                int lineNum);


extern PhysicalMemory g_mem;