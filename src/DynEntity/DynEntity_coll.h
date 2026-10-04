#pragma once
#include <gfx_d3d/r_gfx.h>

enum DynEntityCollType : __int32
{                                       // XREF: Flame_Server_Trace/r
                                        // Flame_Client_Trace/r ...
    DYNENT_COLL_CLIENT_FIRST = 0x0,
    DYNENT_COLL_CLIENT_MODEL = 0x0,
    DYNENT_COLL_CLIENT_BRUSH = 0x1,
    DYNENT_COLL_SERVER_FIRST = 0x2,
    DYNENT_COLL_SERVER_MODEL = 0x2,
    DYNENT_COLL_SERVER_BRUSH = 0x3,
    DYNENT_COLL_COUNT        = 0x4,
};

struct DynEntityColl // sizeof=0x20
{
    unsigned __int16 sector;
    unsigned __int16 nextEntInSector;
    float linkMins[3];
    float linkMaxs[3];
    int contents;
};

struct DynEntityCollTree // sizeof=0xC
{                                       // XREF: DynEntityCollSector/r
    float dist;
    unsigned __int16 axis;
    //DynEntityCollTree::<unnamed_type_u> u;
    union //DynEntityCollTree::<unnamed_type_u> // sizeof=0x2
    {                                       // XREF: DynEntityCollTree/r
        unsigned __int16 parent;
        unsigned __int16 nextFree;
    } u;
    unsigned __int16 child[2];
};

struct __declspec(align(4)) DynEntityCollSector // sizeof=0x14
{                                       // XREF: DynEntityCollWorld/r
    DynEntityCollTree tree;
    int contents;
    unsigned __int16 entListHead;
    // padding byte
    // padding byte
};

struct DynEntityCollWorld // sizeof=0x501C
{                                       // XREF: .data:dynEntCollWorlds/r
    float mins[3];
    float maxs[3];
    unsigned __int16 freeHead;
    // padding byte
    // padding byte
    DynEntityCollSector sectors[1024];
};

struct DynEntityDef;
struct pointtrace_t;
struct trace_t;

DynEntityCollSector *__cdecl DynEnt_GetCollSector(DynEntityCollType collType, unsigned int sectorIndex);
DynEntityCollWorld *__cdecl DynEnt_GetCollWorld(DynEntityCollType collType);
void __cdecl DynEnt_ClearCollWorld(DynEntityCollType collType);
void __cdecl DynEnt_UnlinkEntity(DynEntityCollType collType, unsigned __int16 dynEntId);
void __cdecl DynEnt_LinkEntity(
                DynEntityCollType collType,
                unsigned __int16 dynEntId,
                const float *absMins,
                const float *absMaxs);
void __cdecl DynEnt_AddToCollSector(
                DynEntityCollType collType,
                unsigned __int16 dynEntId,
                unsigned __int16 sectorIndex);
void __cdecl DynEnt_SortCollSector(
                DynEntityCollType collType,
                unsigned __int16 sectorIndex,
                const float *mins,
                const float *maxs);
unsigned __int16 __cdecl DynEnt_AllocCollSector(DynEntityCollType collType, const float *mins, const float *maxs);
int __cdecl DynEnt_GetContents(const DynEntityDef *dynEntDef);
void __cdecl DynEnt_GetLocalBounds(const DynEntityDef *dynEntDef, float *mins, float *maxs);
double __cdecl DynEnt_GetRadiusDistSqr(const DynEntityColl *dynEntColl, const float *origin);
bool __cdecl DynEnt_IsInCone(
                const DynEntityDef *dynEntDef,
                const GfxPlacement *pose,
                const float *origin,
                float coneAngleCos,
                const float *coneDir);
double __cdecl DynEnt_GetCylindricalRadiusDistSqr(const DynEntityColl *dynEntColl, const float *origin);
bool __cdecl DynEnt_EntityInArea(
                const DynEntityDef *dynEntDef,
                const DynEntityColl *dynEntColl,
                const float *mins,
                const float *maxs,
                int contentMask);
void __cdecl DynEnt_PointTraceToModel(
                const DynEntityDef *dynEntDef,
                const GfxPlacement *pose,
                const pointtrace_t *clip,
                trace_t *results);
void __cdecl DynEnt_PointTraceToBrush(
                const DynEntityDef *dynEntDef,
                const GfxPlacement *pose,
                const pointtrace_t *clip,
                trace_t *results);
