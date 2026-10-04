#include "r_buffers.h"
#include <universal/assertive.h>
#include "r_init.h"
#include "r_dvars.h"
#include <universal/q_shared.h>
#include "rb_logfile.h"
#include <qcommon/common.h>
#include "r_utils.h"
#include <qcommon/threads.h>
#include "rb_resource.h"
#include <universal/com_memory.h>
#include "r_rendercmds.h"
#include "r_water_sim.h"

unsigned int g_skinnedCacheBytes = 0x800000; // mod (L25): r_mod_skinCacheMB (retail 0x800000)

GfxBuffers gfxBuf;

void *__cdecl R_LockVertexBuffer(
                IDirect3DVertexBuffer9 *handle,
                unsigned int offset,
                unsigned int bytes,
                unsigned int lockFlags)
{
    int hr; // [esp+0h] [ebp-Ch]
    void *bufferData; // [esp+8h] [ebp-4h] BYREF

    if ( !handle && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 134, 0, "%s", "handle") )
        __debugbreak();
    BO1_RenderCount(BO1_RC_BUF_LOCK, 1); // p1: bo1_frameperf (measurement only)
    BO1_RenderCount(BO1_RC_BUF_LOCK_KB, (long)(bytes >> 10));
    hr = handle->Lock(offset, bytes, &bufferData, lockFlags);
    if ( hr < 0 )
        R_FatalLockError(hr);
    return bufferData;
}

void __cdecl R_UnlockVertexBuffer(IDirect3DVertexBuffer9 *handle)
{

    if ( !handle && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 146, 0, "%s", "handle") )
        __debugbreak();
    handle->Unlock();
    //((void (__thiscall *)(IDirect3DVertexBuffer9 *, IDirect3DVertexBuffer9 *))handle->Unlock)(handle, handle);
}

void *__cdecl R_LockIndexBuffer(
                IDirect3DIndexBuffer9 *handle,
                unsigned int offset,
                unsigned int bytes,
                unsigned int lockFlags)
{
    int hr; // [esp+0h] [ebp-Ch]
    void *bufferData; // [esp+8h] [ebp-4h] BYREF

    if ( !handle && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 157, 0, "%s", "handle") )
        __debugbreak();
    BO1_RenderCount(BO1_RC_BUF_LOCK, 1); // p1: bo1_frameperf (measurement only)
    BO1_RenderCount(BO1_RC_BUF_LOCK_KB, (long)(bytes >> 10));
    hr = handle->Lock(offset, bytes, &bufferData, lockFlags);
    if ( hr < 0 )
        R_FatalLockError(hr);
    return bufferData;
}

void __cdecl R_UnlockIndexBuffer(IDirect3DIndexBuffer9 *handle)
{

    if ( !handle && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 169, 0, "%s", "handle") )
        __debugbreak();
    //((void (__thiscall *)(IDirect3DIndexBuffer9 *, IDirect3DIndexBuffer9 *))handle->Unlock)(handle, handle);
    handle->Unlock();
}

void *__cdecl R_AllocDynamicVertexBuffer(IDirect3DVertexBuffer9 **vb, int sizeInBytes)
{
    const char *v3; // eax
    const char *v4; // eax
    int hr; // [esp+0h] [ebp-8h]

    if ( !vb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 180, 0, "%s", "vb") )
        __debugbreak();
    if ( sizeInBytes <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    181,
                    0,
                    "%s\n\t(sizeInBytes) = %i",
                    "(sizeInBytes > 0)",
                    sizeInBytes) )
    {
        __debugbreak();
    }
    if ( !r_loadForRenderer->current.enabled )
        return 0;
    BO1_RenderCount(BO1_RC_BUF_CREATE, 1); // p1: bo1_frameperf (measurement only)
    hr = dx.device->CreateVertexBuffer(sizeInBytes, 520u, 0, D3DPOOL_DEFAULT, vb, 0);
    if ( hr < 0 )
    {
        v3 = R_ErrorDescription(hr);
        v4 = va("DirectX didn't create a %i-byte dynamic vertex buffer: %s\n", sizeInBytes, v3);
        R_FatalInitError(v4);
    }
    return 0;
}

void *__cdecl R_AllocStaticVertexBuffer(IDirect3DVertexBuffer9 **vb, int sizeInBytes)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const char *v5; // eax
    int hr; // [esp+0h] [ebp-Ch]
    int hra; // [esp+0h] [ebp-Ch]
    void *vertexBufferData; // [esp+4h] [ebp-8h] BYREF

    if ( !vb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 206, 0, "%s", "vb") )
        __debugbreak();
    if ( sizeInBytes <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    207,
                    0,
                    "%s\n\t(sizeInBytes) = %i",
                    "(sizeInBytes > 0)",
                    sizeInBytes) )
    {
        __debugbreak();
    }
    BO1_RenderCount(BO1_RC_BUF_CREATE, 1); // p1: bo1_frameperf (measurement only)
    hr = dx.device->CreateVertexBuffer(sizeInBytes, 8u, 0, D3DPOOL_DEFAULT, vb, 0);
    if ( hr < 0 )
    {
        v2 = R_ErrorDescription(hr);
        v3 = va("DirectX didn't create a %i-byte vertex buffer: %s\n", sizeInBytes, v2);
        R_FatalInitError(v3);
    }
    hra = (*vb)->Lock(0, 0, &vertexBufferData, 0);
    if ( hra < 0 )
    {
        v4 = R_ErrorDescription(hra);
        v5 = va("DirectX didn't lock a vertex buffer: %s\n", v4);
        R_FatalInitError(v5);
    }
    return vertexBufferData;
}

void __cdecl R_FinishStaticVertexBuffer(IDirect3DVertexBuffer9 *vb)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-8h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("vb->Unlock()\n");
    //hr = ((int (__thiscall *)(IDirect3DVertexBuffer9 *, IDirect3DVertexBuffer9 *))vb->Unlock)(vb, vb);
    hr = vb->Unlock();
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp (%i) vb->Unlock() failed: %s\n",
            229,
            v1);
    }
}

void __cdecl R_FreeStaticVertexBuffer(IDirect3DVertexBuffer9 *vb)
{
    IDirect3DVertexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    do
    {
        if ( r_logFile )
        {
            if ( r_logFile->current.integer )
                RB_LogPrint("vb->Release()\n");
        }
        varCopy = vb;
        vb = 0;
        R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
            (IDirect3DSurface9 *)varCopy,
            "vb",
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
            235);
    }
    while ( alwaysfails );
}

void *__cdecl R_AllocDynamicIndexBuffer(IDirect3DIndexBuffer9 **ib, unsigned int sizeInBytes)
{
    const char *v3; // eax
    const char *v4; // eax
    int hr; // [esp+0h] [ebp-8h]

    if ( !r_loadForRenderer->current.enabled )
        return 0;
    BO1_RenderCount(BO1_RC_BUF_CREATE, 1); // p1: bo1_frameperf (measurement only)
    hr = dx.device->CreateIndexBuffer(sizeInBytes, 520u, D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, 0);
    if ( hr < 0 )
    {
        v3 = R_ErrorDescription(hr);
        v4 = va("Couldn't create a %i-byte dynamic index buffer: %s", sizeInBytes, v3);
        R_FatalInitError(v4);
    }
    return 0;
}

void *__cdecl R_AllocStaticIndexBuffer(IDirect3DIndexBuffer9 **ib, int sizeInBytes)
{
    HRESULT hr; // [esp+0h] [ebp-Ch]
    void *indexBufferData; // [esp+4h] [ebp-8h] BYREF

    if ( !ib && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 266, 0, "%s", "ib") )
        __debugbreak();
    if ( sizeInBytes <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    267,
                    0,
                    "%s\n\t(sizeInBytes) = %i",
                    "(sizeInBytes > 0)",
                    sizeInBytes) )
    {
        __debugbreak();
    }
    BO1_RenderCount(BO1_RC_BUF_CREATE, 1); // p1: bo1_frameperf (measurement only)
    hr = dx.device->CreateIndexBuffer(sizeInBytes, 8u, D3DFMT_INDEX16, D3DPOOL_DEFAULT, ib, 0);
    if ( hr < 0 )
        return 0;
    if ( (*ib)->Lock(0, 0, &indexBufferData, 0) >= 0 )
        return indexBufferData;
    //(*ib)->Release(*ib);
    (*ib)->Release();
    return 0;
}

void __cdecl R_FinishStaticIndexBuffer(IDirect3DIndexBuffer9 *ib)
{
    const char *v1; // eax
    int hr; // [esp+4h] [ebp-8h]

    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("ib->Unlock()\n");
    //hr = ((int (__thiscall *)(IDirect3DIndexBuffer9 *, IDirect3DIndexBuffer9 *))ib->Unlock)(ib, ib);
    hr = ib->Unlock();
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v1 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp (%i) ib->Unlock() failed: %s\n",
            294,
            v1);
    }
}

void __cdecl R_FreeStaticIndexBuffer(IDirect3DIndexBuffer9 *ib)
{
    IDirect3DIndexBuffer9 *varCopy; // [esp+0h] [ebp-4h]

    do
    {
        if ( r_logFile )
        {
            if ( r_logFile->current.integer )
                RB_LogPrint("ib->Release()\n");
        }
        varCopy = ib;
        ib = 0;
        R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
            (IDirect3DSurface9 *)varCopy,
            "ib",
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
            300);
    }
    while ( alwaysfails );
}

void __cdecl Load_VertexBuffer(IDirect3DVertexBuffer9 **vb, unsigned __int8 *bufferData, int sizeInBytes)
{
    unsigned __int8 *v3; // eax

    if ( r_loadForRenderer->current.enabled && bufferData )
    {
        if ( Sys_IsRenderThread() )
        {
            v3 = (unsigned __int8 *)R_AllocStaticVertexBuffer(vb, sizeInBytes);
            memcpy(v3, bufferData, sizeInBytes);
            R_FinishStaticVertexBuffer(*vb);
        }
        else
        {
            RB_Resource_LoadVertexBuffer(vb, bufferData, sizeInBytes);
            RB_Resource_Flush();
        }
    }
    else
    {
        *vb = 0;
    }
}

void __cdecl R_InitDynamicVertexBufferState(GfxVertexBufferState *vb, int bytes)
{
    unsigned __int8 *verts; // [esp+0h] [ebp-4h]

    if ( !vb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 545, 0, "%s", "vb") )
        __debugbreak();
    vb->used = 0;
    vb->total = bytes;
    verts = (unsigned __int8 *)R_AllocDynamicVertexBuffer(&vb->buffer, bytes);
    if ( verts
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 556, 1, "%s", "verts == NULL") )
    {
        __debugbreak();
    }
    vb->verts = verts;
}

void __cdecl R_InitDynamicIndices(GfxDynamicIndices *ib, int indexCount)
{
    unsigned __int16 *indices; // [esp+0h] [ebp-8h]

    if ( !ib && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 592, 0, "%s", "ib") )
        __debugbreak();
    ib->used = 0;
    ib->total = indexCount;
    indices = (unsigned __int16 *)Z_VirtualAlloc(2 * indexCount, "Dynamic Index Buffer", 19);
    if ( !indices
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 610, 1, "%s", "indices != NULL") )
    {
        __debugbreak();
    }
    ib->indices = indices;
}

void __cdecl R_CreateDynamicBuffers()
{
    int bufferIter; // [esp+0h] [ebp-8h]
    int bufferItera; // [esp+0h] [ebp-8h]
    int bufferIterb; // [esp+0h] [ebp-8h]
    int bufferIterc; // [esp+0h] [ebp-8h]

    for (bufferIter = 0; bufferIter != 1; ++bufferIter)
        R_InitDynamicVertexBufferState(&gfxBuf.dynamicVertexBufferPool[bufferIter], 0x100000);
    gfxBuf.dynamicVertexBuffer = gfxBuf.dynamicVertexBufferPool;
    g_skinnedCacheBytes = r_mod_skinCacheMB ? r_mod_skinCacheMB->current.integer << 20 : 0x800000; // mod (L25): r_mod_skinCacheMB (retail 0x800000)
    for (bufferItera = 0; bufferItera != 2; ++bufferItera)
    {
        R_InitDynamicVertexBufferState(&gfxBuf.skinnedCacheVbPool[bufferItera], g_skinnedCacheBytes);
        gfxBuf.skinnedCacheVbPool[bufferItera].verts = (unsigned __int8 *)Z_VirtualAlloc(
            gfxBuf.skinnedCacheVbPool[bufferItera].total,
            "skinnedCacheVbPool",
            19);
    }
    R_InitTempSkinBuf();
    // mod (L53): both D3D allocations above are fatal on failure; expose their actual footprint for horde sizing.
    if (gfxBuf.skinnedCacheVbPool[0].buffer && gfxBuf.skinnedCacheVbPool[1].buffer)
        Com_Printf(8, "Skinned vertex cache: %u bytes per pool, %u D3D bytes + %u CPU staging bytes (2 pools)\n",
            g_skinnedCacheBytes, 2 * g_skinnedCacheBytes, 2 * g_skinnedCacheBytes);
    R_InitWaterSimulationVertexBuffers();
    for (bufferIterb = 0; bufferIterb != 1; ++bufferIterb)
        R_InitDynamicIndexBufferState(&gfxBuf.dynamicIndexBufferPool[bufferIterb], 0x100000);
    gfxBuf.dynamicIndexBuffer = gfxBuf.dynamicIndexBufferPool;
    for (bufferIterc = 0; bufferIterc != 2; ++bufferIterc)
    {
        R_InitDynamicIndexBufferState(&gfxBuf.preTessIndexBufferPool[bufferIterc], 0x100000);
        gfxBuf.preTessIndexBufferPool[bufferIterc].indices = (unsigned __int16 *)Z_VirtualAlloc(
            0x200000,
            "preTessIndexBuffer",
            19);
    }
    gfxBuf.preTessIndexBuffer = gfxBuf.preTessIndexBufferPool;
    gfxBuf.preTessBufferFrame = 0;
}

void __cdecl R_InitDynamicIndexBufferState(GfxIndexBufferState *ib, int indexCount)
{
    if ( !ib && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 569, 0, "%s", "ib") )
        __debugbreak();
    ib->used = 0;
    ib->total = indexCount;
    if ( R_AllocDynamicIndexBuffer(&ib->buffer, 2 * indexCount) )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 581, 1, "%s", "indices == NULL") )
            __debugbreak();
    }
}

void __cdecl R_DestroyDynamicBuffers()
{
    IDirect3DSurface9 *v0; // [esp+0h] [ebp-14h]
    IDirect3DSurface9 *buffer; // [esp+4h] [ebp-10h]
    IDirect3DSurface9 *var; // [esp+8h] [ebp-Ch]
    IDirect3DIndexBuffer9 *varCopy; // [esp+Ch] [ebp-8h]
    int bufferIter; // [esp+10h] [ebp-4h]
    int bufferItera; // [esp+10h] [ebp-4h]
    int bufferIterb; // [esp+10h] [ebp-4h]
    int bufferIterc; // [esp+10h] [ebp-4h]

    for ( bufferIter = 0; bufferIter != 2; ++bufferIter )
    {
        if ( gfxBuf.preTessIndexBufferPool[bufferIter].buffer )
        {
            do
            {
                if ( r_logFile )
                {
                    if ( r_logFile->current.integer )
                        RB_LogPrint("gfxBuf.preTessIndexBufferPool[bufferIter].buffer->Release()\n");
                }
                varCopy = gfxBuf.preTessIndexBufferPool[bufferIter].buffer;
                gfxBuf.preTessIndexBufferPool[bufferIter].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                    (IDirect3DSurface9 *)varCopy,
                    "gfxBuf.preTessIndexBufferPool[bufferIter].buffer",
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    693);
            }
            while ( alwaysfails );
        }
        Z_VirtualFree(gfxBuf.preTessIndexBufferPool[bufferIter].indices, 19);
        gfxBuf.preTessIndexBufferPool[bufferIter].indices = 0;
    }
    for ( bufferItera = 0; bufferItera != 1; ++bufferItera )
    {
        if ( gfxBuf.dynamicIndexBufferPool[bufferItera].buffer )
        {
            do
            {
                if ( r_logFile && r_logFile->current.integer )
                    RB_LogPrint("gfxBuf.dynamicIndexBufferPool[bufferIter].buffer->Release()\n");
                var = (IDirect3DSurface9 *)gfxBuf.dynamicIndexBufferPool[bufferItera].buffer;
                gfxBuf.dynamicIndexBufferPool[bufferItera].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                    var,
                    "gfxBuf.dynamicIndexBufferPool[bufferIter].buffer",
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    705);
            }
            while ( alwaysfails );
        }
    }
    R_FreeWaterSimulationVertexBuffers();
    for ( bufferIterb = 0; bufferIterb != 2; ++bufferIterb )
    {
        if ( gfxBuf.skinnedCacheVbPool[bufferIterb].buffer )
        {
            do
            {
                if ( r_logFile && r_logFile->current.integer )
                    RB_LogPrint("gfxBuf.skinnedCacheVbPool[bufferIter].buffer->Release()\n");
                buffer = (IDirect3DSurface9 *)gfxBuf.skinnedCacheVbPool[bufferIterb].buffer;
                gfxBuf.skinnedCacheVbPool[bufferIterb].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                    buffer,
                    "gfxBuf.skinnedCacheVbPool[bufferIter].buffer",
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    718);
            }
            while ( alwaysfails );
        }
        Z_VirtualFree(gfxBuf.skinnedCacheVbPool[bufferIterb].verts, 19);
        gfxBuf.skinnedCacheVbPool[bufferIterb].verts = 0;
    }
    for ( bufferIterc = 0; bufferIterc != 1; ++bufferIterc )
    {
        if ( gfxBuf.dynamicVertexBufferPool[bufferIterc].buffer )
        {
            do
            {
                if ( r_logFile && r_logFile->current.integer )
                    RB_LogPrint("gfxBuf.dynamicVertexBufferPool[bufferIter].buffer->Release()\n");
                v0 = (IDirect3DSurface9 *)gfxBuf.dynamicVertexBufferPool[bufferIterc].buffer;
                gfxBuf.dynamicVertexBufferPool[bufferIterc].buffer = 0;
                R_ReleaseAndSetNULL<IDirect3DIndexBuffer9>(
                    v0,
                    "gfxBuf.dynamicVertexBufferPool[bufferIter].buffer",
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                    729);
            }
            while ( alwaysfails );
        }
    }
    R_ShutdownTempSkinBuf();
}

void __cdecl R_ShutdownDynamicIndices(GfxDynamicIndices *ib)
{
    if ( !ib->indices
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp", 741, 0, "%s", "ib->indices") )
    {
        __debugbreak();
    }
    Z_VirtualFree(ib->indices, 19);
    ib->indices = 0;
}

void __cdecl R_CreateParticleCloudBuffer()
{
    float v0; // [esp+0h] [ebp-C4h]
    float v1; // [esp+4h] [ebp-C0h]
    float v2; // [esp+8h] [ebp-BCh]
    float v3; // [esp+10h] [ebp-B4h]
    float v4; // [esp+18h] [ebp-ACh]
    float v5; // [esp+20h] [ebp-A4h]
    unsigned __int16 particleIndex; // [esp+24h] [ebp-A0h]
    unsigned __int16 gaussParticleIndex; // [esp+28h] [ebp-9Ch]
    int i; // [esp+2Ch] [ebp-98h]
    int j; // [esp+30h] [ebp-94h]
    __int16 xyz[2][3]; // [esp+34h] [ebp-90h] BYREF
    unsigned __int16 *particleIndicesIter; // [esp+40h] [ebp-84h]
    float pos[2][3]; // [esp+44h] [ebp-80h] BYREF
    float rand3; // [esp+5Ch] [ebp-68h] BYREF
    int particleId; // [esp+60h] [ebp-64h]
    int cornerStep; // [esp+64h] [ebp-60h]
    int indexSizeInBytes; // [esp+68h] [ebp-5Ch]
    int xIter; // [esp+6Ch] [ebp-58h]
    int vertSizeInBytes; // [esp+70h] [ebp-54h]
    unsigned __int16 *particleIndices; // [esp+74h] [ebp-50h]
    int vertexIndex; // [esp+78h] [ebp-4Ch]
    float rand0; // [esp+7Ch] [ebp-48h] BYREF
    float rand1; // [esp+80h] [ebp-44h] BYREF
    int yIter; // [esp+84h] [ebp-40h]
    unsigned __int16 lfsr; // [esp+88h] [ebp-3Ch]
    int zIter; // [esp+8Ch] [ebp-38h]
    int cornerIter; // [esp+90h] [ebp-34h]
    float rand2; // [esp+94h] [ebp-30h] BYREF
    unsigned int location; // [esp+98h] [ebp-2Ch]
    GfxParticleCloudVertex *particleVerts; // [esp+9Ch] [ebp-28h]
    unsigned int mask; // [esp+A0h] [ebp-24h]
    GfxParticleCloudVertex *gaussParticleVertsIter; // [esp+A4h] [ebp-20h]
    unsigned __int16 quadIndices[6]; // [esp+A8h] [ebp-1Ch]
    unsigned __int16 *gaussParticleIndicesIter; // [esp+B8h] [ebp-Ch]
    GfxParticleCloudVertex *particleVertsIter; // [esp+BCh] [ebp-8h]
    int indIter; // [esp+C0h] [ebp-4h]

    quadIndices[0] = 0;
    quadIndices[1] = 1;
    quadIndices[2] = 2;
    quadIndices[3] = 2;
    quadIndices[4] = 1;
    quadIndices[5] = 3;
    location = 1;
    vertSizeInBytes = 0x10000;
    cornerStep = 0x2000;
    indexSizeInBytes = 12288;
    lfsr = 511;
    mask = 576;
    particleVerts = (GfxParticleCloudVertex *)R_AllocStaticVertexBuffer(&gfxBuf.particleCloudVertexBuffer, 0x10000);
    particleVertsIter = particleVerts;
    gaussParticleVertsIter = particleVerts + 4096;
    particleIndices = (unsigned __int16 *)R_AllocStaticIndexBuffer(&gfxBuf.particleCloudIndexBuffer, 24576);
    particleIndicesIter = particleIndices;
    gaussParticleIndicesIter = particleIndices + 6144;
    particleId = 0;
    vertexIndex = 0;
    for ( xIter = 0; xIter != 8; ++xIter )
    {
        for ( yIter = 0; yIter != 8; ++yIter )
        {
            zIter = 0;
            while ( zIter != 16 )
            {
                particleVertsIter = &particleVerts[4 * vertexIndex];
                lfsr = -(lfsr & 1) & 0x240 ^ ((int)lfsr >> 1);
                vertexIndex = lfsr & 0x3FF;
                pos[0][0] = (float)((float)((float)((float)rand() / 32767.0) + (float)xIter) * 0.25) + -1.0;
                pos[0][1] = (float)((float)((float)((float)rand() / 32767.0) + (float)yIter) * 0.25) + -1.0;
                pos[0][2] = (float)((float)((float)((float)rand() / 32767.0) + (float)zIter) * 0.125) + -1.0;
                GaussianRandom(&rand0, &rand1);
                GaussianRandom(&rand2, &rand3);
                if ( (float)((float)(rand0 * 0.25) - 1.0) < 0.0 )
                    v5 = rand0 * 0.25;
                else
                    v5 = 1.0f;
                if ( (float)(-1.0 - (float)(rand0 * 0.25)) < 0.0 )
                    v2 = v5;
                else
                    v2 = -1.0f;
                pos[1][0] = v2;
                if ( (float)((float)(rand1 * 0.25) - 1.0) < 0.0 )
                    v4 = rand1 * 0.25;
                else
                    v4 = 1.0f;
                if ( (float)(-1.0 - (float)(rand1 * 0.25)) < 0.0 )
                    v1 = v4;
                else
                    v1 = -1.0f;
                pos[1][1] = v1;
                if ( (float)((float)(rand2 * 0.25) - 1.0) < 0.0 )
                    v3 = rand2 * 0.25;
                else
                    v3 = 1.0f;
                if ( (float)(-1.0 - (float)(rand2 * 0.25)) < 0.0 )
                    v0 = v3;
                else
                    v0 = -1.0f;
                pos[1][2] = v0;
                for ( j = 0; j < 2; ++j )
                {
                    for ( i = 0; i < 3; ++i )
                        xyz[j][i] = (int)(float)(pos[j][i] * 32767.0);
                }
                for ( cornerIter = 0; cornerIter != 4; ++cornerIter )
                {
                    *(unsigned int *)particleVertsIter->xyz = *(unsigned int *)&xyz[0][0];
                    particleVertsIter->xyz[2] = xyz[0][2];
                    particleVertsIter->corner = (_WORD)cornerIter << 13;
                    ++particleVertsIter;
                    *(unsigned int *)gaussParticleVertsIter->xyz = *(unsigned int *)&xyz[1][0];
                    gaussParticleVertsIter->xyz[2] = xyz[1][2];
                    gaussParticleVertsIter->corner = (_WORD)cornerIter << 13;
                    ++gaussParticleVertsIter;
                }
                for ( indIter = 0; indIter != 6; ++indIter )
                {
                    particleIndex = 4 * particleId;
                    gaussParticleIndex = 4 * (particleId + 1024);
                    *particleIndicesIter = 4 * particleId + quadIndices[indIter];
                    if ( particleIndex + quadIndices[indIter] != *particleIndicesIter
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                                    907,
                                    0,
                                    "%s",
                                    "static_cast< int >( quadIndices[indIter] ) + particleIndex == static_cast< int >( *particleIndicesIter )") )
                    {
                        __debugbreak();
                    }
                    ++particleIndicesIter;
                    *gaussParticleIndicesIter = gaussParticleIndex + quadIndices[indIter];
                    if ( gaussParticleIndex + quadIndices[indIter] != *gaussParticleIndicesIter
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_buffers.cpp",
                                    911,
                                    0,
                                    "%s",
                                    "static_cast< int >( quadIndices[indIter] ) + gaussParticleIndex == static_cast< int >( *gaussParticleIndicesIter )") )
                    {
                        __debugbreak();
                    }
                    ++gaussParticleIndicesIter;
                }
                ++zIter;
                ++particleId;
            }
        }
    }
    R_FinishStaticIndexBuffer(gfxBuf.particleCloudIndexBuffer);
    R_FinishStaticVertexBuffer(gfxBuf.particleCloudVertexBuffer);
}

void __cdecl R_DestroyParticleCloudBuffer()
{
    if ( gfxBuf.particleCloudVertexBuffer )
    {
        R_FreeStaticVertexBuffer(gfxBuf.particleCloudVertexBuffer);
        gfxBuf.particleCloudVertexBuffer = 0;
    }
    if ( gfxBuf.particleCloudIndexBuffer )
    {
        R_FreeStaticIndexBuffer(gfxBuf.particleCloudIndexBuffer);
        gfxBuf.particleCloudIndexBuffer = 0;
    }
}

