#pragma once
#include "r_material.h"

struct BuiltinImageConstructorTable // sizeof=0x8
{                                       // XREF: .rdata:constructorTable/r
    const char *name;                   // XREF: Image_LoadBuiltin+2B/r
    void (__cdecl *LoadCallback)(GfxImage *);
};

char __cdecl Image_ValidateHeader(GfxImageFileHeader *imageFile, const char *filepath);
void __cdecl Image_LoadFromData(GfxImage *image, GfxImageFileHeader *fileHeader, unsigned __int8 *srcData);
void __cdecl Image_LoadWavelet(
                GfxImage *image,
                const GfxImageFileHeader *fileHeader,
                const unsigned __int8 *data,
                _D3DFORMAT format,
                int bytesPerPixel);
void __cdecl Image_SetupFromFile(GfxImage *image, const GfxImageFileHeader *fileHeader, _D3DFORMAT imageFormat);
unsigned int __cdecl Image_CountMipmapsForFile(const GfxImageFileHeader *fileHeader);
unsigned __int8 *__cdecl Image_AllocTempMemory(int bytes);
void __cdecl Image_FreeTempMemory(unsigned __int8 *mem, int bytes);
void __cdecl Image_LoadBitmap(
                GfxImage *image,
                const GfxImageFileHeader *fileHeader,
                unsigned __int8 *data,
                _D3DFORMAT format,
                int bytesPerPixel);
void __cdecl Image_ExpandBgr(const unsigned __int8 *src, unsigned int count, unsigned __int8 *dst);
void __cdecl Image_LoadDxtc(
                GfxImage *image,
                const GfxImageFileHeader *fileHeader,
                unsigned __int8 *data,
                _D3DFORMAT format,
                int bytesPerBlock);
char __cdecl Image_LoadToBuffer(GfxImage *image, bool loadHighmip, unsigned __int8 **imageBuffer, int *bufferSize);
unsigned __int8 __cdecl Image_GetPcStreamedMips(GfxImageFileHeader *fileHeader);
void __cdecl Image_PrintTruncatedFileError(const char *filepath);
void __cdecl readCallback();
char __cdecl _Image_LoadFromFileWithReader(
                GfxImage *image,
                bool loadHighmip,
                int (__cdecl *OpenFileRead)(const char *, int *));
bool __cdecl Image_LoadFromFile(GfxImage *image, bool loadHighmip);
char __cdecl Image_LoadFromFileWithReader(
                GfxImage *image,
                bool loadHighmip,
                int (__cdecl *OpenFileRead)(const char *, int *));
void __cdecl Image_Generate2D(GfxImage *image, unsigned __int8 *pixels, int width, int height, _D3DFORMAT imageFormat);
void __cdecl Image_GenerateCube(
                GfxImage *image,
                const unsigned __int8 *(*pixels)[15],
                int edgeLen,
                _D3DFORMAT imageFormat,
                unsigned int mipCount);
void __cdecl Image_LoadWhite(GfxImage *image);
void __cdecl Image_LoadSolid(
                GfxImage *image,
                unsigned __int8 r,
                unsigned __int8 g,
                unsigned __int8 b,
                unsigned __int8 a);
void __cdecl Image_LoadWhiteR32F(GfxImage *image);
void __cdecl Image_LoadG16R16(GfxImage *image);
void __cdecl Image_LoadR5G6B5(GfxImage *image);
void __cdecl Image_LoadBlack(GfxImage *image);
void __cdecl Image_LoadBlackTransparent(GfxImage *image);
void __cdecl Image_LoadGray(GfxImage *image);
void __cdecl Image_LoadIdentityNormalMap(GfxImage *image);
void __cdecl Image_LoadWhiteTransparent(GfxImage *image);
void __cdecl Image_LoadPixelCostColorCode(GfxImage *image);
GfxImage *__cdecl Image_Load(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack);
GfxImage *__cdecl Image_LoadBuiltin(char *name, unsigned __int8 semantic, unsigned __int8 imageTrack);
void __cdecl Image_BuildWaterMap(GfxImage *image);
GfxImage *__cdecl R_CreateWaterMap(char *name, unsigned __int16 imageWidth, unsigned __int16 imageHeight);
