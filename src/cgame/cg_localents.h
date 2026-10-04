#pragma once
#include <gfx_d3d/r_rendercmds.h>

enum leType_t : __int32
{                                       // XREF: localEntity_s/r
    LE_MOVING_TRACER_RED   = 0x0,
    LE_MOVING_TRACER_GREEN = 0x1,
    LE_MOVING_BULLET       = 0x2,
};

struct localEntity_s // sizeof=0x68
{                                       // XREF: .data:cg_localEntities/r
                                        // .data:localEntity_s * cg_activeLocalEntities/r ...
    localEntity_s *prev;                // XREF: CG_InitLocalEntities(int)+4B/w
                                        // CG_AllocLocalEntity(int)+4E/r
    localEntity_s *next;                // XREF: CG_InitLocalEntities(int)+34/w
                                        // CG_InitLocalEntities(int)+AA/w ...
    leType_t leType;
    float tracerWidth;
    float tracerLength;
    int endTime;
    trajectory_t pos;
    float color[4];
    float tracerClipDist;
    GfxEntity refEntity;
};

void __cdecl CG_InitLocalEntities(int localClientNum);
localEntity_s *__cdecl CG_AllocLocalEntity(int localClientNum);
void __cdecl CG_FreeLocalEntity(int localClientNum, localEntity_s *le);
void __cdecl CG_AddLocalEntityTracerBeams(int localClientNum);
void __cdecl CG_AddMovingTracer(const cg_s *cgameGlob, localEntity_s *le, const refdef_s *refdef);
