#pragma once

struct PhysPreset;
struct PhysConstraints;
struct XModelPartsLoad;
struct XModel;
struct Material;
struct XSurface;
struct XModelSurfs;
struct GfxImage;
struct XModelPieces;

struct XModelConfigEntry // sizeof=0x404
{                                       // XREF: XModelConfig/r
    char filename[1024];                // XREF: XModelLoadFile+24B/r
                                        // XModelLoadFile+794/r ...
    float dist;                         // XREF: XModelLoadFile+3A5/r
};

struct XModelConfig // sizeof=0x1C34
{                                       // XREF: XModelLoadFile/r
    XModelConfigEntry entries[4];       // XREF: XModelLoadFile+24B/r
                                        // XModelLoadFile+3A5/r ...
    float mins[3];
    float maxs[3];                      // XREF: XModelLoadFile+F85/r
                                        // XModelLoadFile+FC2/r
    int collLod;                        // XREF: XModelLoadFile+FD8/r
    unsigned int flags;                 // XREF: XModelLoadFile+1AF/r
    char physicsPresetFilename[1024];   // XREF: XModelLoadFile:loc_8043AA/r
                                        // XModelLoadFile+1039/o
    char physicsConstraintsFilename[1024];
                                        // XREF: XModelLoadFile:loc_8043D4/r
                                        // XModelLoadFile+1063/o
    char collMapFilename[1024];         // XREF: XModelLoadFile:loc_8043FE/r
                                        // XModelLoadFile+1098/o
    float bakeLightmapScale;
};

PhysPreset *__cdecl XModel_PhysPresetPrecache(const char *name, void *(__cdecl *Alloc)(int));
PhysConstraints *__cdecl XModel_PhysConstraintsPrecache(const char *name, void *(__cdecl *Alloc)(int));
__int64 __cdecl FS_GetModTime();
void __cdecl XModelCopyXModelParts(const XModelPartsLoad *modelParts, XModel *model);
void __cdecl R_GetXModelBounds(XModel *model, const float (*axes)[3], float *mins, float *maxs);
XModel *__cdecl XModelLoad(char *name, void *(__cdecl *Alloc)(int), void *(__cdecl *AllocColl)(int));
XModel *__cdecl XModelLoadFile(
                char *name,
                void *(__cdecl *Alloc)(int),
                void *(__cdecl *AllocColl)(int),
                __int64 *xmodelModifiedTime);
char __cdecl XModelLoadConfigFile(const char *name, float **pos, XModelConfig *config);
void __cdecl XModelLoadCollData(
                const unsigned __int8 **pos,
                XModel *model,
                void *(__cdecl *AllocColl)(int),
                const char *name);
XModelPartsLoad *__cdecl XModelPartsPrecache(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int64 *xmodelModifiedTime);
XModelPartsLoad *__cdecl XModelPartsLoadFile(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int64 *xmodelModifiedTime);
void __cdecl ConsumeQuatNoSwap(const unsigned __int8 **pos, __int16 *out);
void __cdecl XModelCalcBasePose(XModelPartsLoad *modelParts);
int __cdecl XModelAllowLoadMesh();
void __cdecl XModelGenerateHighMipVolume(XModel *model);
double __cdecl BoxMaxDimension(const float *mins, const float *maxs);
char __cdecl Material_FirstIncludesSecond(Material *mat1, Material *mat2);
char __cdecl Material_IncludesImage(Material *mat, GfxImage *image);
void __cdecl XModelGetSurfaceVertexPos(const XSurface *surface, int vertIndex, float *out);
void __cdecl XModelGetSurfaceTexCoords(const XSurface *surface, int vertIndex, float *out);
int __cdecl XModelSurfsPrecache(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int16 modelNumsurfs,
                const char *modelName,
                XModelSurfs *outModelSurfs,
                __int64 *xmodelModifiedTime);
XModelSurfs *__cdecl R_XModelSurfsLoadFile(
                XModel *model,
                const char *name,
                void *(__cdecl *Alloc)(int),
                __int16 modelNumsurfs,
                const char *modelName,
                __int64 *xmodelModifiedTime);
void __cdecl XModelReadSurfaces(
                XModel *model,
                const char *name,
                XModelSurfs *modelSurfs,
                int *modelPartBits,
                int surfCount,
                unsigned __int8 **pos,
                void *(__cdecl *AllocMesh)(int));
int __cdecl XModelGetFlags(const XModel *model);
XModelPieces *__cdecl XModelPiecesLoadFile(const char *name, void *(__cdecl *Alloc)(int));
XModelPieces *__cdecl XModelPiecesPrecache(const char *name, void *(__cdecl *Alloc)(int));
int __cdecl XModelGetStaticBounds(const XModel *model, float (*axis)[3], float *mins, float *maxs);
int __cdecl XModelGetStaticBoundsExact(const XModel *model, float (*axis)[3], float *mins, float *maxs);
int __cdecl XModelGetStaticBounds(const XModel *model, float *mins, float *maxs);
