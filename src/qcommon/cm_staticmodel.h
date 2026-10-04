#pragma once

struct XModel;
struct trace_t;

struct cStaticModelWritable // sizeof=0x2
{                                       // XREF: cStaticModel_s/r
    unsigned __int16 nextModelInWorldSector;
};

struct cStaticModel_s // sizeof=0x50
{                                       // XREF: cStaticModel_t/r
    cStaticModelWritable writable;
    // padding byte
    // padding byte
    XModel *xmodel;
    float origin[3];
    float invScaledAxis[3][3];
    float absmin[3];
    float absmax[3];
};

unsigned __int8 *__cdecl CM_Hunk_AllocXModel(unsigned int size);
XModel *__cdecl CM_XModelPrecache(char *name);
unsigned __int8 *__cdecl CM_Hunk_AllocXModelColl(unsigned int size);
void __cdecl CM_TraceStaticModel(
                cStaticModel_s *sm,
                trace_t *results,
                const float *start,
                const float *end,
                int contentmask);
bool __cdecl CM_TraceStaticModelComplete(cStaticModel_s *sm, const float *start, const float *end, int contentmask);
