#include "fx_sprite.h"
#include <gfx_d3d/r_drawsurf.h>
#include <xanim/xanim.h>
#include <gfx_d3d/r_scene.h>

FxSpriteInfo g_spriteInfo;
// zombies: SP keeps the sprite count in g_spriteInfo's first word and the sprites right after the 0x10-byte info
// (FX_SpriteAdd SP 0x00485D10: count 0x01A46E00, sprites 0x01A46E10, at most 0xEC). KB kept them apart and counted in
// indexCount, so FX_SpriteGenerateVerts looped over 0 sprites and waypoint / player sprites were never drawn (L2).
FxSprite g_sprites[0xEC];

void __cdecl FX_SpriteGenerateVerts(FxGenerateVertsCmd *cmd)
{
    int i; // [esp+0h] [ebp-8h]
    FxSpriteInfo *spriteInfo; // [esp+4h] [ebp-4h]

    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp", 243, 0, "%s", "cmd") )
        __debugbreak();
    spriteInfo = cmd->spriteInfo;
    if ( !spriteInfo
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp", 248, 0, "%s", "spriteInfo") )
    {
        __debugbreak();
    }
    // zombies: SP's &spriteInfo[2 * i + 1] is info + 0x10 + 0x20 * i, where SP keeps the sprites (g_sprites here)
    iassert(spriteInfo == &g_spriteInfo);
    for ( i = 0; i < (int)spriteInfo->indices; ++i )
        FX_GenerateSpriteCodeMeshVerts(&g_sprites[i], cmd);
}

void __cdecl FX_GenerateSpriteCodeMeshVerts(FxSprite *sprite, FxGenerateVertsCmd *cmd)
{
    if ( (sprite->flags & 2) != 0 )
        FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
            sprite->material,
            sprite->pos,
            sprite->radius,
            sprite->rgbaColor,
            sprite->flags,
            cmd);
    else
        FX_GenerateSpriteCodeMeshVertsFixedWorldSize(
            sprite->material,
            sprite->pos,
            sprite->radius,
            sprite->minScreenRadius,
            sprite->rgbaColor,
            sprite->flags,
            cmd);
}

void __cdecl FX_GenerateSpriteCodeMeshVertsFixedScreenSize(
                Material *material,
                const float *pos,
                float radius,
                const unsigned __int8 *rgbaColor,
                char spriteFlags,
                FxGenerateVertsCmd *cmd)
{
    float worldRadius; // [esp+Ch] [ebp-8h] BYREF

    worldRadius = 0.0f;
    if ( FX_HeightScreenToWorld(pos, radius, &worldRadius, cmd) )
        FX_BuildSpriteCodeMeshVerts(material, pos, worldRadius, rgbaColor, spriteFlags);
}

void __cdecl FX_BuildSpriteCodeMeshVerts(
                Material *material,
                const float *pos,
                float worldRadius,
                const unsigned __int8 *rgbaColor,
                char spriteFlags)
{
    int LocalClientNum; // eax
    cg_s *LocalClientGlobals; // eax
    int v7; // eax
    float *v8; // eax
    int v9; // eax
    cg_s *v10; // eax
    float left[3]; // [esp+18h] [ebp-24h] BYREF
    float worldOrigin[3]; // [esp+24h] [ebp-18h] BYREF
    float up[3]; // [esp+30h] [ebp-Ch] BYREF

    worldOrigin[0] = *pos;
    worldOrigin[1] = pos[1];
    worldOrigin[2] = pos[2];
    if ( (spriteFlags & 1) != 0 )
        worldOrigin[2] = worldOrigin[2] + worldRadius;
    LocalClientNum = R_GetLocalClientNum();
    LocalClientGlobals = CG_GetLocalClientGlobals(LocalClientNum);
    left[0] = worldRadius * LocalClientGlobals->refdef.viewaxis[1][0];
    left[1] = worldRadius * LocalClientGlobals->refdef.viewaxis[1][1];
    left[2] = worldRadius * LocalClientGlobals->refdef.viewaxis[1][2];
    v7 = R_GetLocalClientNum();
    // zombies: up is refdef.viewaxis[2] (SP 0x007B68F0 reads cg +0x8C14C, right after viewaxis[1] at +0x8C140); the
    // decompiled v8[68545] index landed 0x140 bytes past it in KB's cg_s (L2)
    v8 = CG_GetLocalClientGlobals(v7)->refdef.viewaxis[2];
    up[0] = worldRadius * v8[0];
    up[1] = worldRadius * v8[1];
    up[2] = worldRadius * v8[2];
    v9 = R_GetLocalClientNum();
    v10 = CG_GetLocalClientGlobals(v9);
    FX_BuildQuadStampCodeMeshVerts(
        material,
        v10->refdef.viewaxis[0],
        worldOrigin,
        left,
        up,
        rgbaColor,
        COERCE_INT(0.0f),
        COERCE_INT(0.0f),
        COERCE_INT(1.0f),
        COERCE_INT(1.0f));
}

void __cdecl FX_BuildQuadStampCodeMeshVerts(
                Material *material,
                const float *viewAxis,
                const float *origin,
                const float *left,
                const float *up,
                const unsigned __int8 *rgbaColor,
                int s0,
                int t0,
                int s1,
                int t1)
{
    __int16 v10; // [esp+10h] [ebp-1A4h]
    __int16 v11; // [esp+14h] [ebp-1A0h]
    __int16 v12; // [esp+18h] [ebp-19Ch]
    __int16 v13; // [esp+1Ch] [ebp-198h]
    __int16 v14; // [esp+20h] [ebp-194h]
    __int16 v15; // [esp+24h] [ebp-190h]
    __int16 v16; // [esp+28h] [ebp-18Ch]
    __int16 v17; // [esp+2Ch] [ebp-188h]
    int v18; // [esp+34h] [ebp-180h]
    int v19; // [esp+48h] [ebp-16Ch]
    GfxPackedVertex *v20; // [esp+64h] [ebp-150h]
    int v21; // [esp+6Ch] [ebp-148h]
    int v22; // [esp+80h] [ebp-134h]
    GfxPackedVertex *v23; // [esp+9Ch] [ebp-118h]
    int v24; // [esp+A4h] [ebp-110h]
    int v25; // [esp+B8h] [ebp-FCh]
    GfxPackedVertex *v26; // [esp+D4h] [ebp-E0h]
    int v27; // [esp+DCh] [ebp-D8h]
    int v28; // [esp+F0h] [ebp-C4h]
    PackedUnitVec v29; // [esp+10Ch] [ebp-A8h]
    PackedUnitVec v30; // [esp+12Ch] [ebp-88h]
    float testBinormal[3]; // [esp+178h] [ebp-3Ch] BYREF
    GfxPackedVertex *verts; // [esp+184h] [ebp-30h]
    r_double_index_t *indices; // [esp+188h] [ebp-2Ch] BYREF
    GfxColor nativeColor; // [esp+18Ch] [ebp-28h] BYREF
    r_double_index_t index; // [esp+190h] [ebp-24h]
    float leftDown[3]; // [esp+194h] [ebp-20h]
    unsigned __int16 baseVertex; // [esp+1A0h] [ebp-14h] BYREF
    float leftUp[3]; // [esp+1A4h] [ebp-10h]
    GfxPackedVertex *baseVerts; // [esp+1B0h] [ebp-4h]

    if ( R_ReserveCodeMeshVerts(4, &baseVertex) && R_ReserveCodeMeshIndices(6, &indices) )
    {
        Vec3Cross(viewAxis, left, testBinormal);
        if ( (float)((float)((float)(*up * testBinormal[0]) + (float)(up[1] * testBinormal[1]))
                             + (float)(up[2] * testBinormal[2])) <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp",
                        56,
                        0,
                        "Vec3Dot( up, testBinormal ) > 0.0f\n\t%g, %g",
                        (float)((float)((float)(*up * testBinormal[0]) + (float)(up[1] * testBinormal[1]))
                                    + (float)(up[2] * testBinormal[2])),
                        0.0) )
        {
            __debugbreak();
        }
        index.value[0] = baseVertex;
        index.value[1] = baseVertex + 1;
        *indices = index;
        index.value[0] = baseVertex + 3;
        index.value[1] = baseVertex + 3;
        indices[1] = index;
        index.value[0] = baseVertex + 1;
        index.value[1] = baseVertex + 2;
        indices[2] = index;
        R_AddCodeMeshDrawSurf(material, indices, 6u, 0, 0, "", 0xDu);
        leftUp[0] = *left + *up;
        leftUp[1] = left[1] + up[1];
        leftUp[2] = left[2] + up[2];
        leftDown[0] = *left - *up;
        leftDown[1] = left[1] - up[1];
        leftDown[2] = left[2] - up[2];
        Byte4CopyRgbaToVertexColor(rgbaColor, (unsigned __int8 *)&nativeColor);
        //v30.array[0] = (int)(float)((float)(COERCE_FLOAT(*(unsigned int *)viewAxis ^ _mask__NegFloat_) * 127.0) + 127.5);
        //v30.array[1] = (int)(float)((float)(COERCE_FLOAT(*((unsigned int *)viewAxis + 1) ^ _mask__NegFloat_) * 127.0) + 127.5);
        //v30.array[2] = (int)(float)((float)(COERCE_FLOAT(*((unsigned int *)viewAxis + 2) ^ _mask__NegFloat_) * 127.0) + 127.5);
        v30.array[0] = (int)(float)((float)(-(viewAxis[0]) * 127.0) + 127.5);
        v30.array[1] = (int)(float)((float)(-(viewAxis[1]) * 127.0) + 127.5);
        v30.array[2] = (int)(float)((float)(-(viewAxis[2]) * 127.0) + 127.5);
        v30.array[3] = 63;
        v29.array[0] = (int)(float)((float)(-(left[0]) * 127.0) + 127.5);
        v29.array[1] = (int)(float)((float)(-(left[1]) * 127.0) + 127.5);
        v29.array[2] = (int)(float)((float)(-(left[2]) * 127.0) + 127.5);
        v29.array[3] = 63;
        baseVerts = R_GetCodeMeshVerts(baseVertex);
        verts = baseVerts;
        baseVerts->xyz[0] = *origin + leftUp[0];
        verts->xyz[1] = origin[1] + leftUp[1];
        verts->xyz[2] = origin[2] + leftUp[2];
        verts->binormalSign = 1.0f;
        verts->normal = v30;
        verts->color = nativeColor;
        if ( (int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF )
            v28 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v28 = 0x3FFF;
        if ( v28 > -16384 )
            v17 = v28;
        else
            v17 = -16384;
        if ( (int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF )
            v27 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v27 = 0x3FFF;
        if ( v27 > -16384 )
            v16 = v27;
        else
            v16 = -16384;
        verts->texCoord.packed = (v16 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v17 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts->tangent = v29;
        v26 = verts + 1;
        verts[1].xyz[0] = *origin - leftDown[0];
        v26->xyz[1] = origin[1] - leftDown[1];
        v26->xyz[2] = origin[2] - leftDown[2];
        verts[1].binormalSign = 1.0f;
        verts[1].normal = v30;
        verts[1].color = nativeColor;
        if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
            v25 = (int)((2 * s1) ^ 0x80000000) >> 14;
        else
            v25 = 0x3FFF;
        if ( v25 > -16384 )
            v15 = v25;
        else
            v15 = -16384;
        if ( (int)((2 * t0) ^ 0x80000000) >> 14 < 0x3FFF )
            v24 = (int)((2 * t0) ^ 0x80000000) >> 14;
        else
            v24 = 0x3FFF;
        if ( v24 > -16384 )
            v14 = v24;
        else
            v14 = -16384;
        verts[1].texCoord.packed = (v14 & 0x3FFF | (t0 >> 16) & 0xC000) + ((v15 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
        verts[1].tangent = v29;
        v23 = verts + 2;
        verts[2].xyz[0] = *origin - leftUp[0];
        v23->xyz[1] = origin[1] - leftUp[1];
        v23->xyz[2] = origin[2] - leftUp[2];
        verts[2].binormalSign = 1.0f;
        verts[2].normal = v30;
        verts[2].color = nativeColor;
        if ( (int)((2 * s1) ^ 0x80000000) >> 14 < 0x3FFF )
            v22 = (int)((2 * s1) ^ 0x80000000) >> 14;
        else
            v22 = 0x3FFF;
        if ( v22 > -16384 )
            v13 = v22;
        else
            v13 = -16384;
        if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
            v21 = (int)((2 * t1) ^ 0x80000000) >> 14;
        else
            v21 = 0x3FFF;
        if ( v21 > -16384 )
            v12 = v21;
        else
            v12 = -16384;
        verts[2].texCoord.packed = (v12 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v13 & 0x3FFF | (s1 >> 16) & 0xC000) << 16);
        verts[2].tangent = v29;
        v20 = verts + 3;
        verts[3].xyz[0] = *origin + leftDown[0];
        v20->xyz[1] = origin[1] + leftDown[1];
        v20->xyz[2] = origin[2] + leftDown[2];
        verts[3].binormalSign = 1.0f;
        verts[3].normal = v30;
        verts[3].color = nativeColor;
        if ( (int)((2 * s0) ^ 0x80000000) >> 14 < 0x3FFF )
            v19 = (int)((2 * s0) ^ 0x80000000) >> 14;
        else
            v19 = 0x3FFF;
        if ( v19 > -16384 )
            v11 = v19;
        else
            v11 = -16384;
        if ( (int)((2 * t1) ^ 0x80000000) >> 14 < 0x3FFF )
            v18 = (int)((2 * t1) ^ 0x80000000) >> 14;
        else
            v18 = 0x3FFF;
        if ( v18 > -16384 )
            v10 = v18;
        else
            v10 = -16384;
        verts[3].texCoord.packed = (v10 & 0x3FFF | (t1 >> 16) & 0xC000) + ((v11 & 0x3FFF | (s0 >> 16) & 0xC000) << 16);
        verts[3].tangent = v29;
    }
}

void __cdecl Byte4CopyRgbaToVertexColor(const unsigned __int8 *rgbaFrom, unsigned __int8 *nativeTo)
{
    *(unsigned int *)nativeTo = (rgbaFrom[3] << 24) | rgbaFrom[2] | (rgbaFrom[1] << 8) | (*rgbaFrom << 16);
}

char __cdecl FX_HeightScreenToWorld(
                const float *worldOrigin,
                float screenHeight,
                float *worldHeight,
                FxGenerateVertsCmd *cmd)
{
    int LocalClientNum; // eax
    float clipSpaceW; // [esp+8h] [ebp-8h]

    if ( screenHeight <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp",
                    153,
                    0,
                    "%s\n\t(screenHeight) = %g",
                    "(screenHeight > 0)",
                    screenHeight) )
    {
        __debugbreak();
    }
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp", 154, 0, "%s", "cmd") )
        __debugbreak();
    clipSpaceW = FX_GetClipSpaceW(worldOrigin, cmd->vieworg, cmd->viewaxis);
    if ( clipSpaceW < 0.000001 )
        return 0;
    LocalClientNum = R_GetLocalClientNum();
    *worldHeight = (float)((float)(screenHeight * clipSpaceW) * 2.0)
                             * CG_GetLocalClientGlobals(LocalClientNum)->refdef.tanHalfFovY;
    if ( *worldHeight <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp",
                    163,
                    1,
                    "%s\n\t(*worldHeight) = %g",
                    "(*worldHeight > 0.0f)",
                    *worldHeight) )
    {
        __debugbreak();
    }
    return 1;
}

double __cdecl FX_GetClipSpaceW(const float *worldPoint, float *vieworg, float (*viewaxis)[3])
{
    return (float)(worldPoint[2] - vieworg[2]) * (*viewaxis)[2]
             + (float)(worldPoint[1] - vieworg[1]) * (*viewaxis)[1]
             + (float)(*worldPoint - *vieworg) * (*viewaxis)[0];
}

void __cdecl FX_GenerateSpriteCodeMeshVertsFixedWorldSize(
                Material *material,
                const float *pos,
                float radius,
                float minScreenRadius,
                const unsigned __int8 *rgbaColor,
                char spriteFlags,
                FxGenerateVertsCmd *cmd)
{
    float worldRadius; // [esp+Ch] [ebp-10h]
    float screenRadius; // [esp+14h] [ebp-8h] BYREF
    float screenScale; // [esp+18h] [ebp-4h]

    screenRadius = 0.0f;
    worldRadius = radius;
    if ( minScreenRadius > 0.0 )
    {
        if ( !FX_HeightWorldToScreen(pos, radius, &screenRadius, cmd) )
            return;
        if ( screenRadius <= 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp",
                        217,
                        0,
                        "%s\n\t(screenRadius) = %g",
                        "(screenRadius > 0)",
                        screenRadius) )
        {
            __debugbreak();
        }
        if ( minScreenRadius > screenRadius )
        {
            screenScale = minScreenRadius / screenRadius;
            worldRadius = radius * (float)(minScreenRadius / screenRadius);
        }
    }
    FX_BuildSpriteCodeMeshVerts(material, pos, worldRadius, rgbaColor, spriteFlags);
}

char __cdecl FX_HeightWorldToScreen(
                const float *worldOrigin,
                float worldHeight,
                float *screenHeight,
                FxGenerateVertsCmd *cmd)
{
    int LocalClientNum; // eax
    float clipSpaceW; // [esp+8h] [ebp-8h]

    if ( worldHeight <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp",
                    174,
                    0,
                    "%s\n\t(worldHeight) = %g",
                    "(worldHeight > 0)",
                    worldHeight) )
    {
        __debugbreak();
    }
    if ( !cmd && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp", 175, 0, "%s", "cmd") )
        __debugbreak();
    clipSpaceW = FX_GetClipSpaceW(worldOrigin, cmd->vieworg, cmd->viewaxis);
    if ( clipSpaceW < 0.000001 )
        return 0;
    LocalClientNum = R_GetLocalClientNum();
    *screenHeight = (float)(worldHeight / CG_GetLocalClientGlobals(LocalClientNum)->refdef.tanHalfFovY)
                                / (float)(clipSpaceW + clipSpaceW);
    if ( *screenHeight <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_sprite.cpp",
                    184,
                    1,
                    "%s\n\t(*screenHeight) = %g",
                    "(*screenHeight > 0.0f)",
                    *screenHeight) )
    {
        __debugbreak();
    }
    return 1;
}

void __cdecl FX_SpriteBegin()
{
    g_spriteInfo.indices = 0;
}

void __cdecl FX_SpriteAdd(FxSprite *sprite)
{
    // zombies: SP 0x00485D10 - the count is the first field (the one FX_SpriteBegin clears and FX_SpriteGenerateVerts reads)
    const int count = (int)g_spriteInfo.indices;
    if ( count != (int)arr_cnt(g_sprites) )
    {
        memcpy(&g_sprites[count], sprite, sizeof(FxSprite));
        g_spriteInfo.indices = (r_double_index_t *)(count + 1);
    }
}

FxSpriteInfo *__cdecl FX_SpriteGetInfo()
{
    return &g_spriteInfo;
}

