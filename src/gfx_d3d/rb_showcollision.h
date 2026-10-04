#pragma once
#include "r_rendercmds.h"

void __cdecl RB_ShowCollision(const GfxViewParms *viewParms);
void __cdecl BuildFrustumPlanes(const GfxViewParms *viewParms, cplane_s *frustumPlanes);
void __cdecl RB_DrawCollisionPoly(int numPoints, float (*points)[3], const float *colorFloat);
