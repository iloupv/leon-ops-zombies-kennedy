#pragma once

enum XModelLodRampType : __int32
{                                       // XREF: ?DObjGetSurfaceData@@YAXPBUDObj@@QBMM_NQAD@Z/r
                                        // ?R_GetAdjustedLodDist@@YAMMW4XModelLodRampType@@@Z/r
    XMODEL_LOD_RAMP_RIGID   = 0x0,
    XMODEL_LOD_RAMP_SKINNED = 0x1,
    XMODEL_LOD_RAMP_COUNT   = 0x2,
};

double __cdecl R_GetBaseLodDist(const float *origin);
double __cdecl R_GetAdjustedLodDist(float dist, XModelLodRampType lodRampType);
