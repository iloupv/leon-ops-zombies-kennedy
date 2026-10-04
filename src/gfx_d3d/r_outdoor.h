#pragma once

struct GfxWorld;
struct GfxImage;

struct OutdoorGlob // sizeof=0x40
{                                       // XREF: .data:outdoorGlob/r
    float bbox[2][3];                   // XREF: Outdoor_ApplyBoundingBox+D/w
    float scale[3];                     // XREF: Outdoor_UpdateTransforms+50/w
    float invScale[3];                  // XREF: Outdoor_UpdateTransforms+70/w
    float add[3];                       // XREF: Outdoor_UpdateTransforms+9B/w
    unsigned __int8 *pic;               // XREF: R_GenerateOutdoorImage(GfxImage *)+43/w
};

void __cdecl R_RegisterOutdoorImage(GfxWorld *world, const float *outdoorMin, const float *outdoorMax);
void __cdecl Outdoor_ApplyBoundingBox(const float *outdoorMin, const float *outdoorMax);
int Outdoor_UpdateTransforms();
void __cdecl Outdoor_SetRendererOutdoorLookupMatrix(GfxWorld *world);
void __cdecl R_GenerateOutdoorImage(GfxImage *outdoorImage);
void Outdoor_TempHunkFreePic();
void Outdoor_ComputeTexels();
double __cdecl Outdoor_TraceHeightInWorld(float worldX, float worldY);
void __cdecl Outdoor_WriteToPic(float zWorld, unsigned __int8 *outByte);
int __cdecl Outdoor_TransformToTextureClamped(int dimension, float inWorld);
