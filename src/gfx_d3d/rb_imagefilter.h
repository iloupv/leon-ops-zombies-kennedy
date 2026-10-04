#pragma once
#include "r_material.h"
#include "rb_state.h"

struct __declspec(align(4)) GfxImageFilterPass // sizeof=0x9C
{                                       // XREF: GfxImageFilter/r
    const Material *material;
    float srcWidth;
    float srcHeight;
    int dstWidth;
    int dstHeight;
    int tapHalfCount;
    float tapOffsetsAndWeights[8][4];
    unsigned __int8 src;
    unsigned __int8 dst;
    // padding byte
    // padding byte
};

struct __declspec(align(4)) GfxImageFilter // sizeof=0x138C
{                                       // XREF: ?RB_GaussianFilterImage@@YAXMEEH@Z/r
    int passCount;                      // XREF: RB_GaussianFilterImage(float,uchar,uchar,int)+E0/w
                                        // RB_GaussianFilterImage(float,uchar,uchar,int)+E6/r
    GfxImageFilterPass passes[32];      // XREF: RB_GaussianFilterImage(float,uchar,uchar,int)+9F/o
    GfxImage *sourceImage;              // XREF: RB_GaussianFilterImage(float,uchar,uchar,int)+96/w
    GfxRenderTargetId finalTarget;        // XREF: RB_GaussianFilterImage(float,uchar,uchar,int)+9C/w
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl RB_GaussianFilterImage(float radius, GfxRenderTargetId srcRenderTargetId, GfxRenderTargetId dstRenderTargetId);
void __cdecl RB_VirtualToSceneRadius(float radius, float *radiusX, float *radiusY);
int __cdecl RB_GenerateGaussianFilterChain(
                float radiusX,
                float radiusY,
                int srcWidth,
                int srcHeight,
                int dstWidth,
                int dstHeight,
                int passLimit,
                GfxImageFilterPass *filterPass);
void __cdecl RB_GenerateGaussianFilter1D(float radius, int *res, int axis, GfxImageFilterPass *filterPass);
int __cdecl RB_PickSymmetricFilterMaterial(int halfTapCount, const Material **material);
int __cdecl RB_GaussianFilterPoints1D(
                float pixels,
                int srcRes,
                int dstRes,
                int tapLimit,
                float *tapOffsets,
                float *tapWeights);
void __cdecl RB_GenerateGaussianFilter2D(
                float radius,
                int srcWidth,
                int srcHeight,
                int dstWidth,
                int dstHeight,
                GfxImageFilterPass *filterPass);
void __cdecl RB_FilterImage(GfxImageFilter *filter);
void __cdecl RB_SetupFilterPass(const GfxImageFilterPass *filterPass);
void __cdecl RB_FilterPingPong(const GfxImageFilter *filter, int passIndex);
