#include "r_mapkit_render.h"
#include "r_rope_render.h"
#include "fxprimitives.h"
#include "r_drawsurf.h"
#include "r_model_lighting.h"
#include "r_primarylights.h"
#include "r_material.h"
#include "r_init.h"
#include "r_state.h"
#include "r_dvars.h"
#include "r_image_load_obj.h"
#include "r_image.h"
#include <universal/com_workercmds.h>
#include <qcommon/cm_mapkit.h>
#include <qcommon/common.h>

#include <string.h>

// mod: mapkit - each box face is one quad in the code mesh, lit like a rope (R_Rope_GenerateVerts_Internal):
// model lighting (light grid) sampled at the box's lightOrigin (inside the room for room boxes) and the primary
// light that touches the box.

namespace
{
    const int MK_DRAW_MAX = 512;

    Material *s_mkMaterial[MK_DRAW_MAX];
    unsigned __int16 s_mkLighting[MK_DRAW_MAX];
    int s_mkCachedCount = -1;
    const MapkitBox *s_mkCachedFirst;
    GfxLightingInfo s_mkLightingInfo[MK_DRAW_MAX];

    PackedUnitVec Mapkit_PackUnitVec(const float *dir)
    {
        PackedUnitVec result;
        result.array[0] = (int)(float)(dir[0] * 127.0f + 127.5f);
        result.array[1] = (int)(float)(dir[1] * 127.0f + 127.5f);
        result.array[2] = (int)(float)(dir[2] * 127.0f + 127.5f);
        result.array[3] = 63;
        return result;
    }

    void Mapkit_EmitVert(GfxPackedVertex *vert, const float *pos, const float *normal, const float *tangent, float u, float v)
    {
        vert->xyz[0] = pos[0];
        vert->xyz[1] = pos[1];
        vert->xyz[2] = pos[2];
        vert->binormalSign = 1.0f;
        vert->color.packed = -1;
        vert->normal = Mapkit_PackUnitVec(normal);
        vert->tangent = Mapkit_PackUnitVec(tangent);
        float texcoord[2] = { u, v };
        vert->texCoord = Vec2PackTexCoords(texcoord);
    }

    void Mapkit_RefreshMaterials(int count)
    {
        const MapkitBox *first = CM_Mapkit_GetBox(0);
        if (count == s_mkCachedCount && first == s_mkCachedFirst)
            return;
        for (int i = 0; i < count && i < MK_DRAW_MAX; ++i)
        {
            const MapkitBox *box = CM_Mapkit_GetBox(i);
            s_mkMaterial[i] = Material_RegisterHandle(box->material, 0);
            s_mkLighting[i] = 0;
            if (s_mkMaterial[i] && s_mkMaterial[i] == rgp.defaultMaterial)
                Com_PrintWarning(8, "mapkit: material '%s' is not loaded (box %d draws as default)\n", box->material, i);
        }
        s_mkCachedCount = count;
        s_mkCachedFirst = first;
    }
}

void __cdecl R_Mapkit_GenerateVerts(const float *vieworg)
{
    int count = CM_Mapkit_BoxCount();
    if (!count || !rgp.world)
        return;
    if (count > MK_DRAW_MAX)
        count = MK_DRAW_MAX;
    Mapkit_RefreshMaterials(count);

    R_BeginCodeMeshVerts();
    for (int b = 0; b < count; ++b)
    {
        const MapkitBox *box = CM_Mapkit_GetBox(b);
        const Material *material = s_mkMaterial[b];
        if (!material || box->hidden)
            continue;

        // the primary light is picked at the lighting sample point, not over the whole box: R_GetLightingAtPoint
        // (rb_light.cpp) keeps only grid corners baked for the light it is given, so the light must be the one at
        // lightOrigin. (The near-black east wall of runs mk9/mk17 was not this: it was the material,
        // mc/pent_art_wall_wood07_dark; the same wall with a concrete material drew lit, run mk19.)
        float lightOrigin[3] = { box->lightOrigin[0], box->lightOrigin[1], box->lightOrigin[2] };
        const float pointHalf[3] = { 1.0f, 1.0f, 1.0f };
        unsigned int primaryLightIndex = CM_Mapkit_Empty() ? 0 : R_GetNonSunPrimaryLightForBox(0, lightOrigin, pointHalf);
        if (!primaryLightIndex)
            primaryLightIndex = rgp.world->sunPrimaryLightIndex;
        unsigned int lightHandle = R_AllocModelLighting_PrimaryLight(lightOrigin, primaryLightIndex, 0, &s_mkLighting[b], &s_mkLightingInfo[b]);
        if (!lightHandle)
            continue;

        r_double_index_t *baseIndices;
        unsigned __int16 baseVertex;
        unsigned int argOffset;
        if (!R_ReserveCodeMeshIndices(72, &baseIndices) || !R_ReserveCodeMeshVerts(24, &baseVertex) || !R_ReserveCodeMeshArgs(1, &argOffset))
            break;

        GfxPackedVertex *verts = R_GetCodeMeshVerts(baseVertex);
        r_double_index_t *indices = baseIndices;
        const float inv = 1.0f / box->texScale;
        for (int face = 0; face < 6; ++face)
        {
            const int axis = face >> 1;          // the face's normal axis
            const bool positive = (face & 1) != 0;
            const int ua = axis == 0 ? 1 : 0;    // texture u runs along ua, v along va
            const int va = axis == 2 ? 1 : 2;
            float normal[3] = { 0.0f, 0.0f, 0.0f };
            float tangent[3] = { 0.0f, 0.0f, 0.0f };
            normal[axis] = positive ? 1.0f : -1.0f;
            tangent[ua] = 1.0f;
            // drawn 1 u out from the collision face: a face coplanar with retail geometry (the room floor on Five's
            // floor at z 16) no longer z-fights with it (runs mk5/mk9: striped floor)
            const float d = positive ? box->maxs[axis] + 1.0f : box->mins[axis] - 1.0f;
            for (int corner = 0; corner < 4; ++corner)
            {
                float pos[3];
                pos[axis] = d;
                pos[ua] = (corner & 1) ? box->maxs[ua] + 1.0f : box->mins[ua] - 1.0f; // the whole drawn box is 1 u larger
                pos[va] = (corner & 2) ? box->maxs[va] + 1.0f : box->mins[va] - 1.0f;
                Mapkit_EmitVert(verts++, pos, normal, tangent, pos[ua] * inv, -pos[va] * inv);
            }
            // both windings (4 triangles), so the face shows from inside and outside whatever the material's cull mode
            const unsigned __int16 v0 = baseVertex + face * 4;
            static const unsigned __int16 order[12] = { 0, 1, 2, 1, 3, 2, 0, 2, 1, 1, 2, 3 };
            for (int k = 0; k < 12; k += 2)
            {
                r_double_index_t pair;
                pair.value[0] = v0 + order[k];
                pair.value[1] = v0 + order[k + 1];
                *indices++ = pair;
            }
        }

        float4 *meshArgs = (float4 *)R_GetCodeMeshArgs(argOffset);
        meshArgs->u[0] = lightHandle;
        meshArgs->u[1] = lightHandle;
        meshArgs->u[2] = lightHandle;
        meshArgs->u[3] = lightHandle;
        R_AddRopeCodeMeshDrawSurf(material, baseIndices, 72, argOffset, 1u, primaryLightIndex, 9u);
    }
    R_EndCodeMeshVerts();
    Sys_AssistAndWaitWorkerCmdInternal(&r_model_lightingWorkerCmd);
}

// mod: mapkit-fix-4g - the empty base's brush models (every kit window's pf82 panels, re-placed) are drawn by retail's
// own lit technique for brush models (R_TessBModel), with a neutral lightmap bound in place of the bake that moved with
// them (pf82's is dark: its room side is behind a hallway wall). The retail texels (retail research notes/visuals.md 7.2 and
// the world lit shader, 14.3): primary = the primary-light visibility; secondary = R5G6B5 (chroma_r, direction,
// chroma_b), the ambient map stacked over the direct one; secondaryB = G16R16, the two HDR luminances / 31.875
// (R ambient, G direct). Primary = white is retail's own R_SetLightmap path when the sun direction has changed, so the
// sun primary light lights the panels unshadowed, as it does the kit boxes (R_GetModelLighting's empty-base branch).
// The ambient is that branch's flat grey, tinted by the mood light containing the placement origin (CM_Mapkit_LightAt),
// packed with Load_LightGridColors' rule (lum = 0.25*(r+b) + 0.5*g, chroma = c / (4*lum), neutral 8/31).
// MEASURED: the grey's luminance (dvar r_mapkitBModelLight) is calibrated on photos, not taken from the exe.
namespace
{
    struct MkBModelLightmap
    {
        unsigned int key;
        GfxImage secondary;
        GfxImage secondaryB;
    };
    enum { MK_BMODEL_LIGHTMAPS = 32 };
    MkBModelLightmap s_mkBmLightmaps[MK_BMODEL_LIGHTMAPS];
    int s_mkBmLightmapCount;
    IDirect3DDevice9 *s_mkBmDevice; // images belong to one device (a vid_restart makes a new one)
    unsigned int s_mkBmCursor;

    const MkBModelLightmap *Mapkit_BModelLightmap(unsigned short secPixel, unsigned short ambient16)
    {
        if (s_mkBmDevice != dx.device)
        {
            s_mkBmDevice = dx.device;
            s_mkBmLightmapCount = 0;
        }
        const unsigned int key = ((unsigned int)secPixel << 16) | ambient16;
        for (int i = 0; i < s_mkBmLightmapCount; ++i)
        {
            if (s_mkBmLightmaps[i].key == key)
                return &s_mkBmLightmaps[i];
        }
        MkBModelLightmap *lm;
        if (s_mkBmLightmapCount < MK_BMODEL_LIGHTMAPS)
            lm = &s_mkBmLightmaps[s_mkBmLightmapCount++];
        else
        {
            lm = &s_mkBmLightmaps[s_mkBmCursor++ % MK_BMODEL_LIGHTMAPS];
            Image_Release(&lm->secondary);
            Image_Release(&lm->secondaryB);
        }
        memset(lm, 0, sizeof(*lm));
        lm->key = key;
        lm->secondary.name = "$mapkit_bmodel_secondary";
        lm->secondary.semantic = 1;
        lm->secondaryB.name = "$mapkit_bmodel_secondaryb";
        lm->secondaryB.semantic = 1;
        unsigned short sec = secPixel;
        Image_Generate2D(&lm->secondary, (unsigned __int8 *)&sec, 1, 1, D3DFMT_R5G6B5);
        unsigned int secB = ambient16; // G16R16: R (low 16 bits) = ambient, G = direct (none)
        Image_Generate2D(&lm->secondaryB, (unsigned __int8 *)&secB, 1, 1, D3DFMT_G16R16);
        return lm;
    }
}

void __cdecl R_Mapkit_SetBModelLightmap(GfxCmdBufContext context, const float *origin, const float *quat)
{
    if (r_lightMap->current.integer != 1)
        return; // the retail r_lightMap override (R_SetLightmap) stays
    // the mood light is sampled 16 u into the room: a window piece's origin sits in the wall, outside the room's light
    // volume (mood_example win1: y 2294 < the volume's 2300). pf82's room side is its model +y (retail yaw 90), turned
    // by the placement quat (x y z w): v' = v + w t + q x t, t = 2 q x v, v = (0, 16, 0).
    const float t[3] = { -2.0f * quat[2] * 16.0f, 0.0f, 2.0f * quat[0] * 16.0f };
    const float p[3] = {
        origin[0] + quat[3] * t[0] + (quat[1] * t[2] - quat[2] * t[1]),
        origin[1] + 16.0f + quat[3] * t[1] + (quat[2] * t[0] - quat[0] * t[2]),
        origin[2] + quat[3] * t[2] + (quat[0] * t[1] - quat[1] * t[0]) };
    float tint[3] = { 1.0f, 1.0f, 1.0f };
    CM_Mapkit_LightAt(p, tint);

    const float lum = 0.25f * (tint[0] + tint[2]) + 0.5f * tint[1];
    int cr = 8, cb = 8;
    float ambient = 0.0f;
    if (lum > 0.0f)
    {
        cr = (int)(tint[0] / (4.0f * lum) * 31.0f + 0.5f);
        cb = (int)(tint[2] / (4.0f * lum) * 31.0f + 0.5f);
        cr = cr > 31 ? 31 : cr;
        cb = cb > 31 ? 31 : cb;
        ambient = r_mapkitBModelLight->current.value * lum / 31.875f;
    }
    const int a16 = (int)(ambient * 65535.0f + 0.5f);
    // the direction channel at its middle (no direction), as for a flat ambient
    const unsigned short secPixel = (unsigned short)((cr << 11) | (32 << 5) | cb);
    const MkBModelLightmap *lm = Mapkit_BModelLightmap(secPixel, (unsigned short)(a16 > 65535 ? 65535 : a16));
    const MaterialPass *pass = context.state->pass;
    if ((pass->customSamplerFlags & 2) != 0)
        R_SetSampler(context, 0xCu, 0x62u, rgp.whiteImage);
    if ((pass->customSamplerFlags & 4) != 0)
        R_SetSampler(context, 0xDu, 0x62u, &lm->secondary);
    if ((pass->customSamplerFlags & 8) != 0)
        R_SetSampler(context, 0xEu, 0x62u, &lm->secondaryB);
}
