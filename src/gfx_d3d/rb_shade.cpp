#include "rb_shade.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "rb_pixelcost.h"
#include "r_state.h"
#include "r_shade.h"

void __cdecl RB_ClearPixelShader()
{
    if ( gfxCmdBufState.pixelShader )
        R_HW_SetPixelShader(gfxCmdBufState.prim.device, 0);
    gfxCmdBufState.pixelShader = 0;
}

void __cdecl R_HW_SetPixelShader(IDirect3DDevice9 *device, const MaterialPixelShader *mtlShader)
{
    HRESULT v2; // eax
    const char *v3; // eax
    int hr; // [esp+8h] [ebp-4h]

    if ( !device
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h", 662, 0, "%s", "device") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetPixelShader( mtlShader ? mtlShader->prog.ps : 0 )\n");
    if ( mtlShader )
        v2 = device->SetPixelShader(mtlShader->prog.ps);
    else
        v2 = device->SetPixelShader(0);
    hr = v2;
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetPixelShader( mtlShader ? mtlShader->p"
            "rog.ps : 0 ) failed: %s\n",
            663,
            v3);
    }
}

void __cdecl RB_ClearVertexShader()
{
    if ( gfxCmdBufState.vertexShader )
        R_HW_SetVertexShader(gfxCmdBufState.prim.device, 0);
    gfxCmdBufState.vertexShader = 0;
}

void __cdecl R_HW_SetVertexShader(IDirect3DDevice9 *device, const MaterialVertexShader *mtlShader)
{
    HRESULT v2; // eax
    const char *v3; // eax
    int hr; // [esp+8h] [ebp-4h]

    if ( !device
        && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h", 669, 0, "%s", "device") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("device->SetVertexShader( mtlShader ? mtlShader->prog.vs : 0 )\n");
    if ( mtlShader )
        v2 = device->SetVertexShader(mtlShader->prog.vs);
    else
        v2 = device->SetVertexShader(0);
    hr = v2;
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v3 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_setstate_d3d.h (%i) device->SetVertexShader( mtlShader ? mtlShader->"
            "prog.vs : 0 ) failed: %s\n",
            670,
            v3);
    }
}

void __cdecl RB_ClearVertexDecl()
{
    if ( gfxCmdBufState.prim.vertexDecl )
        R_SetVertexDecl(&gfxCmdBufState.prim, 0);
    if ( gfxCmdBufState.prim.vertexDecl )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                        58,
                        1,
                        "%s",
                        "gfxCmdBufState.prim.vertexDecl == NULL") )
            __debugbreak();
    }
}

void __cdecl R_SetVertexDecl(GfxCmdBufPrimState *primState, const MaterialVertexDeclaration *vertexDecl)
{
    const char *v2; // eax
    IDirect3DVertexDeclaration9 *v3; // [esp+0h] [ebp-14h]
    int hr; // [esp+8h] [ebp-Ch]
    IDirect3DDevice9 *device; // [esp+10h] [ebp-4h]

    if ( vertexDecl )
        v3 = vertexDecl->routing.decl[primState->vertDeclType];
    else
        v3 = 0;
    if ( primState->vertexDecl != v3 )
    {
        device = primState->device;
        if ( !primState->device
            && !Assert_MyHandler("c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h", 1949, 0, "%s", "device") )
        {
            __debugbreak();
        }
        if ( v3 )
        {
            if ( r_logFile && r_logFile->current.integer )
                RB_LogPrint("device->SetVertexDeclaration( decl )\n");
            //hr = ((int (__thiscall *)(IDirect3DDevice9 *, IDirect3DDevice9 *, IDirect3DVertexDeclaration9 *))device->SetVertexDeclaration)(
            //             device,
            //             device,
            //             v3);
            hr = device->SetVertexDeclaration(v3);
            if ( hr < 0 )
            {
                ++g_disableRendering;
                v2 = R_ErrorDescription(hr);
                Com_Error(
                    ERR_FATAL,
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_state.h (%i) device->SetVertexDeclaration( decl ) failed: %s\n",
                    1953,
                    v2);
            }
        }
        primState->vertexDecl = v3;
    }
}

void __cdecl R_BeginPixMaterial(GfxCmdBufState *state)
{
    const MaterialTechnique *technique; // [esp+0h] [ebp-14h]
    const Material *material; // [esp+4h] [ebp-10h]

    if ( r_pix_material->current.enabled )
    {
        material = state->material;
        technique = state->technique;
        if ( state->pixMaterial != material )
        {
            if ( state->pixTechnique )
            {
                //PIXEndNamedEvent();
                state->pixTechnique = 0;
            }
            //if ( state->pixMaterial )
                //PIXEndNamedEvent();
            //PIXBeginNamedEvent(-1, material->info.name); // BO1TODO: Tracy runtime start/stop zones
        }
        if ( state->pixTechnique != technique )
        {
            //if ( state->pixTechnique )
            //    PIXEndNamedEvent();
            //PIXBeginNamedEvent(-1, technique->name); // BO1TODO: Tracy runtime start/stop zones
        }
        state->pixMaterial = material;
        state->pixTechnique = technique;
    }
    else
    {
        state->pixMaterial = 0;
        state->pixTechnique = 0;
    }
}

void __cdecl R_EndPixMaterial(GfxCmdBufState *state)
{
    if ( !state->pixCombine )
        R_EndPixMaterialInternal(state);
}

void __cdecl R_EndPixMaterialInternal(GfxCmdBufState *state)
{
    //if ( state->pixTechnique )
    //    PIXEndNamedEvent();
    //if ( state->pixMaterial )
    //    PIXEndNamedEvent();
    state->pixTechnique = 0;
    state->pixMaterial = 0;
}

void __cdecl R_SetPixPrimarySortKey(GfxCmdBufState *state, unsigned int primarySortKey)
{
    const char *v2; // eax

    if ( state->pixPrimarySortKey != primarySortKey )
    {
        R_EndPixMaterialInternal(state);
        state->pixPrimarySortKey = primarySortKey;
        v2 = va("key: %d", state->pixPrimarySortKey);
        //PIXSetMarker(-1, v2);
    }
}

void __cdecl R_BeginPixMaterials(GfxCmdBufState *state)
{
    if ( state->pixCombine
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 187, 0, "%s", "!state->pixCombine") )
    {
        __debugbreak();
    }
    if ( state->pixMaterial
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    188,
                    0,
                    "%s",
                    "!state->pixMaterial") )
    {
        __debugbreak();
    }
    if ( state->pixTechnique
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    189,
                    0,
                    "%s",
                    "!state->pixTechnique") )
    {
        __debugbreak();
    }
    state->pixCombine = 1;
    state->pixPrimarySortKey = -1;
}

void __cdecl R_EndPixMaterials(GfxCmdBufState *state)
{
    if ( !state->pixCombine
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 197, 0, "%s", "state->pixCombine") )
    {
        __debugbreak();
    }
    //if ( state->pixTechnique )
    //    PIXEndNamedEvent();
    //if ( state->pixMaterial )
    //    PIXEndNamedEvent();
    state->pixTechnique = 0;
    state->pixMaterial = 0;
    state->pixCombine = 0;
}

void __cdecl RB_SetTessTechnique(const Material *material, unsigned __int8 techType)
{
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 319, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( gfxCmdBufState.origMaterial != material || gfxCmdBufState.origTechType != techType )
    {
        if ( tess.indexCount )
            RB_EndTessSurface();
        RB_BeginSurface(material, techType);
    }
}

void __cdecl RB_BeginSurface(const Material *material, unsigned __int8 techType)
{
    const char *v2; // eax
    const char *v3; // eax

    if ( tess.indexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    336,
                    0,
                    "%s\n\t(tess.indexCount) = %i",
                    "(tess.indexCount == 0)",
                    tess.indexCount) )
    {
        __debugbreak();
    }
    if ( tess.vertexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    337,
                    0,
                    "%s\n\t(tess.vertexCount) = %i",
                    "(tess.vertexCount == 0)",
                    tess.vertexCount) )
    {
        __debugbreak();
    }
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 339, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( gfxCmdBufState.prim.primStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    341,
                    0,
                    "%s",
                    "!gfxCmdBufState.prim.primStats") )
    {
        __debugbreak();
    }
    if ( r_logFile->current.integer )
    {
        v2 = RB_LogTechniqueType(techType);
        v3 = va("---------- RB_BeginSurface( %s, %s )\n", material->info.name, v2);
        RB_LogPrint(v3);
    }
    tess.firstVertex = 0;
    tess.lastVertex = 0;
    gfxCmdBufState.material = material;
    gfxCmdBufState.techType = techType;
    gfxCmdBufState.prim.vertDeclType = VERTDECL_GENERIC;
    gfxCmdBufState.origMaterial = material;
    gfxCmdBufState.origTechType = techType;
    if ( pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC )
    {
        gfxCmdBufState.material = R_PixelCost_GetAccumulationMaterial(material);
        gfxCmdBufState.techType = 4;
    }
    gfxCmdBufState.technique = Material_GetTechnique(gfxCmdBufState.material, gfxCmdBufState.techType);
    if ( !gfxCmdBufState.technique
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    370,
                    0,
                    "%s",
                    "gfxCmdBufState.technique") )
    {
        __debugbreak();
    }
}

void __cdecl RB_EndTessSurface()
{
    RB_EndSurfacePrologue();
    RB_DrawTessSurface();
    RB_EndSurfaceEpilogue();
}

GfxPrimStats *RB_EndSurfacePrologue()
{
    GfxPrimStats *result; // eax

    if ( !gfxCmdBufState.material
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    375,
                    0,
                    "%s",
                    "gfxCmdBufState.material") )
    {
        __debugbreak();
    }
    tess.finishedFilling = 1;
    if ( !gfxCmdBufState.prim.primStats
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    382,
                    0,
                    "%s",
                    "gfxCmdBufState.prim.primStats") )
    {
        __debugbreak();
    }
    gfxCmdBufState.prim.primStats->dynamicIndexCount += tess.indexCount;
    result = gfxCmdBufState.prim.primStats;
    gfxCmdBufState.prim.primStats->dynamicVertexCount += tess.vertexCount;
    return result;
}

void RB_EndSurfaceEpilogue()
{
    if ( tess.vertexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    390,
                    0,
                    "%s",
                    "tess.vertexCount == 0") )
    {
        __debugbreak();
    }
    if ( tess.indexCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    391,
                    0,
                    "%s",
                    "tess.indexCount == 0") )
    {
        __debugbreak();
    }
    gfxCmdBufState.prim.primStats = 0;
    tess.finishedFilling = 0;
}

void RB_DrawTessSurface()
{
  GfxViewport scissor; // [esp+0h] [ebp-2Ch] BYREF
  GfxViewport viewport; // [esp+10h] [ebp-1Ch] BYREF
  GfxDrawPrimArgs args; // [esp+20h] [ebp-Ch] BYREF

  if ( !tess.indexCount
    && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 475, 0, "%s", "tess.indexCount") )
  {
    __debugbreak();
  }
  if ( gfxCmdBufSourceState.viewportIsDirty )
  {
    R_GetViewport(&gfxCmdBufSourceState, &viewport);
    R_SetViewport(&gfxCmdBufState, &viewport);
    R_UpdateViewport(&gfxCmdBufSourceState, &viewport);
    if ( R_GetScissor(&gfxCmdBufSourceState, &scissor) )
      R_SetScissor(&gfxCmdBufState, &scissor);
    else
      R_ClearScissor(&gfxCmdBufState);
  }
  args.vertexCount = tess.vertexCount;
  args.triCount = tess.indexCount / 3;
  if ( gfxCmdBufState.prim.vertDeclType
    && !Assert_MyHandler(
          "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
          499,
          1,
          "%s\n\t(gfxCmdBufState.prim.vertDeclType) = %i",
          "(gfxCmdBufState.prim.vertDeclType == VERTDECL_GENERIC)",
          gfxCmdBufState.prim.vertDeclType) )
  {
    __debugbreak();
  }
  args.baseIndex = R_SetIndexData(&gfxCmdBufState.prim, (unsigned char*)tess.indices, tess.indexCount / 3);
  R_DrawTessTechnique(gfxCmdBufContext, &args);
  tess.indexCount = 0;
  tess.vertexCount = 0;
}

void __cdecl R_DrawTessTechnique(GfxCmdBufContext context, const GfxDrawPrimArgs *args)
{
    const char *v2; // eax
    const MaterialTechnique *technique; // [esp+0h] [ebp-14h]
    IDirect3DVertexBuffer9 *vb; // [esp+4h] [ebp-10h]
    unsigned int vertexOffset; // [esp+8h] [ebp-Ch]
    bool isPixelCostEnabled; // [esp+Fh] [ebp-5h]
    unsigned int passIndex; // [esp+10h] [ebp-4h]

    if ( (!dx.d3d9 || !dx.device)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    226,
                    0,
                    "%s",
                    "dx.d3d9 && dx.device") )
    {
        __debugbreak();
    }
    if ( !context.state->material
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    227,
                    0,
                    "%s",
                    "context.state->material") )
    {
        __debugbreak();
    }
    technique = context.state->technique;
    if ( !technique
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 247, 0, "%s", "technique") )
    {
        __debugbreak();
    }
    R_BeginPixMaterial(context.state);
    if ( r_logFile->current.integer )
    {
        v2 = va("\n---------- R_DrawTechnique( %s ) ----------\n", technique->name);
        RB_LogPrint(v2);
    }
    isPixelCostEnabled = pixelCostMode != GFX_PIXEL_COST_MODE_OFF;
    if ( pixelCostMode )
        R_PixelCost_BeginSurface(context);
    if ( context.state->prim.vertDeclType
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp",
                    264,
                    1,
                    "%s\n\t(context.state->prim.vertDeclType) = %i",
                    "(context.state->prim.vertDeclType == VERTDECL_GENERIC)",
                    context.state->prim.vertDeclType) )
    {
        __debugbreak();
    }
    R_CheckVertexDataOverflow(32 * tess.vertexCount);
    vertexOffset = R_SetVertexData(context.state, &tess, tess.vertexCount, 32);
    for ( passIndex = 0; passIndex < technique->passCount; ++passIndex )
    {
        R_SetupPass(context, passIndex);
        R_UpdateVertexDecl(context.state);
        R_SetupPassCriticalPixelShaderArgs(context);
        vb = gfxBuf.dynamicVertexBuffer->buffer;
        if ( !vb && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\rb_shade.cpp", 279, 0, "%s", "vb") )
            __debugbreak();
        R_SetStreamSource(&context.state->prim, vb, vertexOffset, 0x20u);
        R_SetupPassPerObjectArgs(context);
        R_SetupPassPerPrimArgs(context);
        R_DrawIndexedPrimitive(&context.state->prim, args);
    }
    if ( isPixelCostEnabled )
        R_PixelCost_EndSurface(context);
    if ( r_logFile->current.integer )
        RB_LogPrint("\n");
    R_EndPixMaterial(context.state);
}

void __cdecl R_CheckVertexDataOverflow(int dataSize)
{
    if ( !gfxBuf.dynamicVertexBuffer
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.h",
                    29,
                    0,
                    "%s",
                    "gfxBuf.dynamicVertexBuffer") )
    {
        __debugbreak();
    }
    if ( (dataSize <= 0 || dataSize > gfxBuf.dynamicVertexBuffer->total)
        && !Assert_MyHandler(
                    "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shade.h",
                    30,
                    0,
                    "%s\n\t(dataSize) = %i",
                    "(dataSize > 0 && dataSize <= gfxBuf.dynamicVertexBuffer->total)",
                    dataSize) )
    {
        __debugbreak();
    }
    if ( dataSize + gfxBuf.dynamicVertexBuffer->used > gfxBuf.dynamicVertexBuffer->total )
        gfxBuf.dynamicVertexBuffer->used = 0;
}

void __cdecl R_SetStreamSource(
                GfxCmdBufPrimState *primState,
                IDirect3DVertexBuffer9 *vb,
                unsigned int vertexOffset,
                unsigned int vertexStride)
{
    if ( primState->streams[0].vb != vb
        || primState->streams[0].offset != vertexOffset
        || primState->streams[0].stride != vertexStride )
    {
        R_ChangeStreamSource(primState, 0, vb, vertexOffset, vertexStride);
    }
    if ( primState->streams[1].vb || primState->streams[1].offset || primState->streams[1].stride )
        R_ChangeStreamSource(primState, 1u, 0, 0, 0);
    if ( primState->streams[2].vb || primState->streams[2].offset || primState->streams[2].stride )
        R_ChangeStreamSource(primState, 2u, 0, 0, 0);
}

void __cdecl RB_TessOverflow()
{
    GfxPrimStats *primStats; // [esp+0h] [ebp-4h]

    primStats = gfxCmdBufState.prim.primStats;
    RB_EndTessSurface();
    RB_BeginSurface(gfxCmdBufState.origMaterial, gfxCmdBufState.origTechType);
    gfxCmdBufState.prim.primStats = primStats;
}

