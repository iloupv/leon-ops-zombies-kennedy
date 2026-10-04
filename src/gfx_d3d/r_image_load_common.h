#pragma once
#include "r_gfx.h"
#include "r_material.h"
#include "r_image.h"

void __cdecl Image_TrackFullscreenTexture(
                GfxImage *image,
                int fullscreenWidth,
                int fullscreenHeight,
                int picmip,
                _D3DFORMAT format);
void __cdecl Image_GetMipmapResolution(
                unsigned int baseWidth,
                unsigned int baseHeight,
                int mipmap,
                unsigned __int16 *mipWidth,
                unsigned __int16 *mipHeight);
int __cdecl Image_GetPlatformScreenWidth(int platform, int screenWidth);
int __cdecl Image_GetPlatformScreenHeight(int platform, int screenHeight);
void __cdecl Image_TrackTexture(GfxImage *image, char imageFlags, _D3DFORMAT format, int width, int height, int depth);
unsigned int __cdecl Image_GetCardMemoryAmountForMipLevel(
                _D3DFORMAT format,
                unsigned int mipWidth,
                unsigned int mipHeight,
                unsigned int mipDepth);
unsigned int __cdecl Image_GetCardMemoryAmount(
                char imageFlags,
                _D3DFORMAT format,
                unsigned int width,
                unsigned int height,
                unsigned int depth);
void __cdecl Image_UploadData(
                GfxImage *image,
                _D3DFORMAT format,
                _D3DCUBEMAP_FACES face,
                unsigned int mipLevel,
                unsigned __int8 *src);
void __cdecl Image_Upload2D_CopyData_PC(
                const GfxImage *image,
                _D3DFORMAT format,
                _D3DCUBEMAP_FACES face,
                unsigned int mipLevel,
                unsigned __int8 *src);
void __cdecl Image_Upload2D_CopyDataBlock_PC(
                int width,
                int height,
                unsigned __int8 *src,
                _D3DFORMAT format,
                int dstPitch,
                unsigned __int8 *dst);
void __cdecl Image_Upload3D_CopyData_PC(
                const GfxImage *image,
                _D3DFORMAT format,
                unsigned int mipLevel,
                unsigned __int8 *src);
int __cdecl Image_SourceBytesPerSlice_PC(_D3DFORMAT format, int width, int height);
unsigned int __cdecl Image_CountMipmaps(char imageFlags, unsigned int width, unsigned int height, unsigned int depth);
void __cdecl Image_GetPicmip(const GfxImage *image, Picmip *picmip);
void __cdecl Image_Setup(GfxImage *image, int width, int height, int depth, int imageFlags, _D3DFORMAT imageFormat);
void __cdecl Image_SetupAndLoad(
                GfxImage *image,
                int width,
                int height,
                int depth,
                int imageFlags,
                _D3DFORMAT imageFormat);
unsigned int __cdecl Image_CubemapFace(unsigned int faceIndex);
