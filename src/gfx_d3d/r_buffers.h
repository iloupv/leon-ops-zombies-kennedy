#pragma once

#include <d3d9.h>
#include <universal/com_pack.h>

// mod (L25): bytes in each skinned vertex cache pool (r_mod_skinCacheMB at buffer creation; retail 0x800000) and the
// retail "almost full" (0x6AACC0) / "full" (0x7D8000 = 32 * (256 - 5) K verts) marks scaled to it.
extern unsigned int g_skinnedCacheBytes;
inline unsigned int R_SkinCacheAlmostFullBytes() { return (unsigned int)((unsigned long long)g_skinnedCacheBytes * 0x6AACC0 / 0x800000); }
inline unsigned int R_SkinCacheFullBytes() { return g_skinnedCacheBytes - 32 * 5 * 1024; }

struct GfxDynamicIndices // sizeof=0xC
{                                       // XREF: GfxBuffers/r
    volatile int used;
    int total;
    unsigned __int16 *indices;          // XREF: R_CacheStaticModelIndices+117/r
                                        // R_PreTessStaticModelCachedList+EF/r ...
};

struct GfxPackedVertexNormal // sizeof=0x8
{                                       // XREF: GfxBuffers/r
    PackedUnitVec normal;               // XREF: .rdata:00D146E5/o
                                        // .rdata:00D146E9/o ...
    PackedUnitVec tangent;              // XREF: .rdata:00DC09F4/o
};

struct GfxIndexBufferState // sizeof=0x10
{                                       // XREF: GfxBuffers/r
                                        // GfxBuffers/r
    volatile int used;
    int total;
    IDirect3DIndexBuffer9 *buffer;      // XREF: R_DestroyDynamicBuffers(void)+28/r
    // R_DestroyDynamicBuffers(void)+5B/r ...
    unsigned __int16 *indices;          // XREF: R_CreateDynamicBuffers(void)+14E/w
    // R_DestroyDynamicBuffers(void)+A0/r ...
};

struct GfxVertexBufferState // sizeof=0x10
{                                       // XREF: GfxMeshData/r
                                        // GfxBuffers/r ...
    volatile unsigned int used;
    int total;                          // XREF: R_CreateDynamicBuffers(void)+91/r
    IDirect3DVertexBuffer9 *buffer;     // XREF: R_DestroyDynamicBuffers(void)+17D/r
    // R_DestroyDynamicBuffers(void)+1B0/r ...
    unsigned __int8 *verts;             // XREF: R_InitRenderBuffers(void)+5D/w
    // R_InitGlassRenderBuffers(int,int,int)+64/w ...
};

struct __declspec(align(4)) GfxBuffers // sizeof=0x2400A8
{                                       // XREF: .data:GfxBuffers gfxBuf/r
    GfxDynamicIndices smodelCache;      // XREF: R_CacheStaticModelIndices+117/r
                                        // R_PreTessStaticModelCachedList+EF/r ...
    IDirect3DVertexBuffer9 *smodelCacheVb;
                                        // XREF: R_ReleaseForShutdownOrReset(void):loc_A4EB02/r
                                        // RB_PatchStaticModelCache:loc_AB5D19/r ...
    GfxIndexBufferState preTessIndexBufferPool[2];
                                        // XREF: R_ToggleSmpFrame+1C0/o
                                        // R_CreateDynamicBuffers(void)+125/o ...
    GfxIndexBufferState *preTessIndexBuffer;
                                        // XREF: R_ToggleSmpFrame+1C6/w
                                        // R_ToggleSmpFrame+1D1/r ...
    int preTessBufferFrame;             // XREF: R_ToggleSmpFrame+65/r
                                        // R_ToggleSmpFrame:loc_A7AB7B/w ...
    GfxIndexBufferState dynamicIndexBufferPool[1];
                                        // XREF: R_CreateDynamicBuffers(void)+E4/o
                                        // R_CreateDynamicBuffers(void):loc_AC5D04/o ...
    GfxIndexBufferState *dynamicIndexBuffer; // XREF: RB_SwapBuffers+39A/r
                                        // R_CreateDynamicBuffers(void):loc_AC5D04/w ...
    GfxVertexBufferState skinnedCacheVbPool[2];
                                        // XREF: R_ToggleSmpFrame+1A6/o
                                        // R_CreateDynamicBuffers(void)+76/o ...
    unsigned __int8 *skinnedCacheLockAddr;
                                        // XREF: R_LockSkinnedCache(void)+6/r
                                        // R_LockSkinnedCache(void)+C9/w ...
    GfxVertexBufferState dynamicVertexBufferPool[1];
                                        // XREF: R_CreateDynamicBuffers(void)+34/o
                                        // R_CreateDynamicBuffers(void):loc_AC5C55/o ...
    GfxVertexBufferState *dynamicVertexBuffer;
                                        // XREF: R_CreateDynamicBuffers(void):loc_AC5C55/w
                                        // R_DrawTessTechnique+1C0/r ...
    IDirect3DVertexBuffer9 *particleCloudVertexBuffer;
                                        // XREF: R_CreateParticleCloudBuffer(void)+7E/o
                                        // R_CreateParticleCloudBuffer(void)+642/r ...
    IDirect3DIndexBuffer9 *particleCloudIndexBuffer;
                                        // XREF: R_CreateParticleCloudBuffer(void)+AC/o
                                        // R_CreateParticleCloudBuffer(void):loc_AC6763/r ...
    int dynamicBufferFrame;             // XREF: R_ToggleSmpFrame+4C/r
                                        // R_ToggleSmpFrame:loc_A7AB60/w ...
    volatile unsigned int dynamicBufferCurrentFrame[2];
                                        // XREF: R_InitRenderBuffers(void)+EA/w
                                        // R_ShutdownRenderBuffers(void)+99/w
    GfxPackedVertexNormal skinnedCacheNormals[2][147456];
                                        // XREF: R_LockSkinnedCache(void)+DA/o
                                        // R_LockSkinnedCache(void)+FF/o ...
    GfxPackedVertexNormal *skinnedCacheNormalsAddr;
    GfxPackedVertexNormal *oldSkinnedCacheNormalsAddr;
    unsigned int skinnedCacheNormalsFrameCount;
    bool fastSkin;
    bool skinCache;
    // padding byte
    // padding byte
};

struct GfxParticleCloudVertex // sizeof=0x8
{
    __int16 xyz[3];
    __int16 corner;
};

void *__cdecl R_LockVertexBuffer(
                IDirect3DVertexBuffer9 *handle,
                unsigned int offset,
                unsigned int bytes,
                unsigned int lockFlags);
void __cdecl R_UnlockVertexBuffer(IDirect3DVertexBuffer9 *handle);
void *__cdecl R_LockIndexBuffer(
                IDirect3DIndexBuffer9 *handle,
                unsigned int offset,
                unsigned int bytes,
                unsigned int lockFlags);
void __cdecl R_UnlockIndexBuffer(IDirect3DIndexBuffer9 *handle);
void *__cdecl R_AllocDynamicVertexBuffer(IDirect3DVertexBuffer9 **vb, int sizeInBytes);
void *__cdecl R_AllocStaticVertexBuffer(IDirect3DVertexBuffer9 **vb, int sizeInBytes);
void __cdecl R_FinishStaticVertexBuffer(IDirect3DVertexBuffer9 *vb);
void __cdecl R_FreeStaticVertexBuffer(IDirect3DVertexBuffer9 *vb);
void *__cdecl R_AllocDynamicIndexBuffer(IDirect3DIndexBuffer9 **ib, unsigned int sizeInBytes);
void *__cdecl R_AllocStaticIndexBuffer(IDirect3DIndexBuffer9 **ib, int sizeInBytes);
void __cdecl R_FinishStaticIndexBuffer(IDirect3DIndexBuffer9 *ib);
void __cdecl R_FreeStaticIndexBuffer(IDirect3DIndexBuffer9 *ib);
void __cdecl Load_VertexBuffer(IDirect3DVertexBuffer9 **vb, unsigned __int8 *bufferData, int sizeInBytes);
void __cdecl R_InitDynamicVertexBufferState(GfxVertexBufferState *vb, int bytes);
void __cdecl R_InitDynamicIndices(GfxDynamicIndices *ib, int indexCount);
void __cdecl R_CreateDynamicBuffers();
void __cdecl R_InitDynamicIndexBufferState(GfxIndexBufferState *ib, int indexCount);
void __cdecl R_DestroyDynamicBuffers();
void __cdecl R_ShutdownDynamicIndices(GfxDynamicIndices *ib);
void __cdecl R_CreateParticleCloudBuffer();
void __cdecl R_DestroyParticleCloudBuffer();


extern GfxBuffers gfxBuf;