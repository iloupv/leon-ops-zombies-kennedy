#pragma once
#include "r_dpvs.h"

struct GfxSceneEntity;

void __cdecl R_AddEntitySurfacesInFrustumCmd(unsigned __int16 *data);
bool __cdecl R_BoundsInCell(mnode_t *node, int findCellIndex, const float *mins, const float *maxs);
bool __cdecl R_BoundsInCell_r(mnode_t *node, int findCellIndex, const float *mins, const float *maxs);
void __cdecl R_ForceAddEntitySurfacesInFrustumCmd(GfxSceneEntity *sceneEnt, unsigned __int8 *entVisData);
