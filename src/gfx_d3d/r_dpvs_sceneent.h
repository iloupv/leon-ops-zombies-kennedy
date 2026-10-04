#pragma once
#include "r_bsp.h"

struct DpvsEntityCmd // sizeof=0x10
{                                       // XREF: ?R_AddCellSceneEntSurfacesInFrustumCmd@@YAXPAX@Z/r
    struct GfxSceneEntity *sceneEnt;
    const DpvsPlane *planes;
    unsigned __int16 planeCount;
    unsigned __int16 cellIndex;
    unsigned __int8 *entVisData;
};

void    R_AddCellSceneEntSurfacesInFrustumCmd(GfxWorldDpvsPlanes *data);
