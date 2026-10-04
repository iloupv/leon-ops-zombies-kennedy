#pragma once

// mod: mapkit - draws the layout's boxes (qcommon/cm_mapkit.h) as lit code-mesh quads with retail materials.
void __cdecl R_Mapkit_GenerateVerts(const float *vieworg);

struct GfxCmdBufContext;
// mod: mapkit-fix-4g - binds a neutral (mood-tinted) lightmap for an empty-base brush model drawn at origin.
void __cdecl R_Mapkit_SetBModelLightmap(GfxCmdBufContext context, const float *origin, const float *quat);
