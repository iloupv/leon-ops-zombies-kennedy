#include "r_draw_material.h"
#include "r_draw_method.h"
#include "r_char_tech_util.h"
#include "r_draw_shadowablelight.h"
#include "rb_pixelcost.h"
#include "r_shader_constant_set.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include "r_foliage.h"
#include "r_warn.h"
#include "rb_draw3d.h"
#include <universal/profile.h>
#include <qcommon/cm_mapkit.h>

unsigned __int8 gNvFloatZTech_Intz[70] =
{
  80u,
  81u,
  82u,
  83u,
  84u,
  85u,
  86u,
  87u,
  88u,
  89u,
  90u,
  91u,
  92u,
  93u,
  94u,
  95u,
  96u,
  97u,
  98u,
  99u,
  100u,
  101u,
  102u,
  103u,
  104u,
  105u,
  106u,
  107u,
  38u,
  39u,
  40u,
  41u,
  42u,
  43u,
  44u,
  45u,
  46u,
  47u,
  48u,
  49u,
  50u,
  51u,
  52u,
  53u,
  54u,
  55u,
  56u,
  57u,
  58u,
  59u,
  59u,
  61u,
  61u,
  63u,
  63u,
  65u,
  65u,
  67u,
  67u,
  69u,
  69u,
  71u,
  71u,
  108u,
  109u,
  110u,
  111u,
  112u,
  113u,
  114u
};

unsigned __int8 __cdecl RemoveSunShadowTech(unsigned __int8 srcTech)
{
    unsigned __int8 remappedTech; // [esp+7h] [ebp-1h]

    switch ( srcTech )
    {
        case 0xCu:
            remappedTech = 11;
            break;
        case 0x28u:
            remappedTech = 39;
            break;
        case 0x2Fu:
            remappedTech = 46;
            break;
        case 0x36u:
            remappedTech = 53;
            break;
        case 0x3Fu:
            remappedTech = 61;
            break;
        case 0x40u:
            remappedTech = 62;
            break;
        case 0x4Bu:
            remappedTech = 74;
            break;
        default:
            remappedTech = srcTech;
            break;
    }
    return remappedTech;
}

unsigned int __cdecl R_GfxDrawSurf_GetFade(const GfxDrawSurf *drawSurf)
{
    if (drawSurf->fields.surfType == 2)
    {
        return drawSurf->fields.fade;
    }

    return 0;
}

unsigned __int8 __cdecl R_Get_NvFloatZLitTech(const Material *material, unsigned __int8 originalTechType)
{
    unsigned __int8 newTechType; // [esp+Bh] [ebp-9h]
    unsigned int index; // [esp+Ch] [ebp-8h]

    if ( !material )
        return originalTechType;
    if ( (Material_GetTechniqueSet(material)->techsetFlags & 1) == 0 )
        return originalTechType;
    index = originalTechType - 10;
    if ( index >= 0x46 )
        return originalTechType;
    newTechType = gNvFloatZTech_Intz[index];
    if ( Material_GetTechnique(material, newTechType) )
        return newTechType;
    else
        return originalTechType;
}

unsigned __int8 __cdecl R_GetTechType(
                GfxCmdBufContext context,
                const GfxDrawSurfListInfo *info,
                unsigned __int8 baseTechType,
                GfxDrawSurf drawSurf)
{
    unsigned __int8 result; // al
    GfxFadeType fadeType; // [esp+34h] [ebp-18h]
    const GfxBackEndData *data; // [esp+40h] [ebp-Ch]
    char needsCharredTech; // [esp+46h] [ebp-6h]
    unsigned __int8 actualTechType; // [esp+47h] [ebp-5h]
    unsigned int surfType; // [esp+48h] [ebp-4h]

    surfType = drawSurf.fields.surfType;

    switch ( baseTechType )
    {
        case 5u:
        case 6u:
            if ( !dx.supportsIntZ || context.state->renderTargetId == 22 )
                goto LABEL_38;
            if ( baseTechType == 5 )
                return 7;
            if ( baseTechType == 6 )
                result = 8;
            else
LABEL_38:
                result = baseTechType;
            break;
        case 0xAu:
            // mod: mapkit-fix-4g - empty-base brush models take the retail lit technique again (8e50396's unlit
            // technique drew the kit boards at 4x retail brightness); R_TessBModel binds them a neutral lightmap.
            needsCharredTech = TechLit_NeedsCharredTech(&drawSurf);
            fadeType = (GfxFadeType)(R_GfxDrawSurf_GetFade(&drawSurf) != 0);
            data = context.source->input.data;

            bcassert(drawSurf.fields.primaryLightIndex, data->shadowableLightCount);
            
            actualTechType = data->primaryLightTechType[surfType][fadeType][needsCharredTech != 0][drawSurf.fields.primaryLightIndex];
            if ( info->isMissileCamera || (drawSurf.packed & 0x200000000000000LL) != 0 )
                actualTechType = RemoveSunShadowTech(actualTechType);
            if ( actualTechType >= 0xAu && actualTechType <= 0x10u )
            {
                if ( ((drawSurf.packed >> 29) & 1) != 0 && ((drawSurf.packed >> 30) & 1) != 0 )
                {
                    actualTechType += 21;
                }
                else if ( ((drawSurf.packed >> 29) & 1) != 0 )
                {
                    actualTechType += 14;
                }
                else if ( ((drawSurf.packed >> 30) & 1) != 0 )
                {
                    actualTechType += 7;
                }
            }
            if ( dx.supportsIntZ && context.state->renderTargetId != 22 )
                actualTechType = R_Get_NvFloatZLitTech(rgp.sortedMaterials[drawSurf.fields.materialSortedIndex], actualTechType);
            result = actualTechType;
            break;
        case 0x73u:
        case 0x74u:
        case 0x75u:
            if ( TechLit_NeedsCharredTech(&drawSurf) )
            {
                if ( baseTechType == 115 )
                {
                    result = 118;
                }
                else if ( baseTechType == 116 )
                {
                    result = 119;
                }
                else
                {
                    result = 120;
                }
            }
            else
            {
                result = baseTechType;
            }
            break;
        case 0x7Fu:
            if ( surfType == 4 || surfType == 3 )
                result = 0x80;
            else
                result = baseTechType;
            break;
        default:
            result = baseTechType;
            break;
    }
    return result;
}

int __cdecl R_GetPossibleTechniqueTypes(unsigned __int8 techType, GfxDrawSurf drawSurf, unsigned __int8 *techTypes)
{
    unsigned __int8 *tech; // [esp+20h] [ebp-8h]

    *techTypes = techType;
    tech = techTypes + 1;
    if ( techType >= 0xAu && techType < 0x73u && techType > 0x10u && techType <= 0x25u )
    {
        if ( ((drawSurf.packed >> 29) & 1) != 0 && ((drawSurf.packed >> 30) & 1) != 0 )
        {
            *tech = techType - 21;
            tech = techTypes + 2;
        }
        else if ( ((drawSurf.packed >> 30) & 1) != 0 )
        {
            *tech = techType - 7;
            tech = techTypes + 2;
        }
        else if ( ((drawSurf.packed >> 29) & 1) != 0 )
        {
            *tech = techType - 14;
            tech = techTypes + 2;
        }
    }
    *tech = -1;
    if ( (unsigned int)(tech + 1 - techTypes) > 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_material.cpp",
                    738,
                    0,
                    "%s",
                    "count <= (uint)MAX_POSSIBLE_TECHNIQUES") )
    {
        __debugbreak();
    }
    return tech + 1 - techTypes;
}

int __cdecl R_SetTechnique(
                GfxCmdBufContext context,
                GfxCmdBufContext *prepassContext,
                const GfxDrawSurfListInfo *info,
                GfxDrawSurf drawSurf)
{
    PROF_SCOPED("R_SetTechnique");

    unsigned __int64 v5; // rax
    GfxDrawSurf overrideDrawSurf; // [esp+8h] [ebp-10h]
    unsigned __int8 techType; // [esp+13h] [ebp-5h]
    unsigned __int8 techTypes[4]; // [esp+14h] [ebp-4h] BYREF

    techType = R_GetTechType(context, info, info->baseTechType, drawSurf);
    R_GetPossibleTechniqueTypes(techType, drawSurf, techTypes);
    if ( techType > 1u )
    {
        if ( info->baseTechType == 10 )
            R_SetShadowableLight(context.source, drawSurf.fields.primaryLightIndex);
        if ( !R_TrySetMaterialWithFunc(
                        R_SetMaterial,
                        context,
                        drawSurf,
                        techTypes) )
            return 0;
    }
    else if ( !R_TrySetMaterialWithFunc(
                             R_SetPrepassMaterial,
                             context,
                             drawSurf,
                             techTypes) )
    {
        return 0;
    }
    if ( prepassContext->state && !R_SetPrepassMaterial(*prepassContext, drawSurf, 0) )
        prepassContext->state = 0;
    context.state->origMaterial = context.state->material;
    context.state->origTechType = context.state->techType;
    if ( pixelCostMode > GFX_PIXEL_COST_MODE_MEASURE_MSEC )
    {
        //v5 = ((R_PixelCost_GetAccumulationMaterial(context.state->material)->info.drawSurf.packed >> 31) & 0xFFF) << 31;
        //*(unsigned int *)&overrideDrawSurf.fields = v5 | *(unsigned int *)&drawSurf.fields & 0x7FFFFFFF;
        //HIDWORD(overrideDrawSurf.packed) = HIDWORD(v5) | HIDWORD(drawSurf.packed) & 0xFFFFF800;
        overrideDrawSurf = drawSurf;
        overrideDrawSurf.fields.materialSortedIndex = R_PixelCost_GetAccumulationMaterial(context.state->material)->info.drawSurf.fields.materialSortedIndex;

        R_SetMaterial(context, overrideDrawSurf, 4u);
    }
    return 1;
}

int __cdecl R_SetPrepassMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, unsigned __int8 techType)
{
    const MaterialTechnique *technique; // [esp+8h] [ebp-Ch]
    Material *material; // [esp+Ch] [ebp-8h]
    unsigned int prepass; // [esp+10h] [ebp-4h]

    prepass = (drawSurf.packed >> 55) & 3;
    if ( prepass == 2 )
        return 0;
    if ( R_GfxDrawSurf_GetFade(&drawSurf) )
        return 0;
    material = rgp.sortedMaterials[drawSurf.fields.materialSortedIndex];
    if ( !prepass && (material->stateFlags & 1) != 0 )
        material = rgp.depthPrepassMaterial;
    context.state->material = material;
    technique = Material_GetTechnique(material, techType);
    context.state->technique = technique;
    if ( !technique )
        return 0;
    if ( (technique->flags & 2) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_material.cpp",
                    214,
                    0,
                    "%s",
                    "!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_SCENE)") )
    {
        __debugbreak();
    }
    if ( (technique->flags & 1) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_material.cpp",
                    215,
                    0,
                    "%s",
                    "!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_POST_SUN)") )
    {
        __debugbreak();
    }
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_material.cpp", 217, 0, "%s", "material") )
    {
        __debugbreak();
    }
    RB_ApplyShaderConstantSet(context.source, &drawSurf);
    context.state->techType = techType;
    return 1;
}

int __cdecl R_SetMaterial(GfxCmdBufContext context, GfxDrawSurf drawSurf, unsigned __int8 techType)
{
    const char *v4; // eax
    const char *v5; // eax
    float v6; // [esp+10h] [ebp-34h]
    float v8; // [esp+24h] [ebp-20h]
    float fadeVal; // [esp+30h] [ebp-14h]
    const MaterialTechnique *technique; // [esp+38h] [ebp-Ch]
    int penetrationMode; // [esp+3Ch] [ebp-8h]
    const Material *material; // [esp+40h] [ebp-4h]

    while ( 1 )
    {
        material = rgp.sortedMaterials[drawSurf.fields.materialSortedIndex];
        technique = Material_GetTechnique(material, techType);
        context.state->material = material;
        context.state->technique = technique;
        if ( !technique )
            return 0;
        penetrationMode = r_showPenetration->current.integer;
        if ( !penetrationMode )
            break;
        if ( !techType || techType == 1 || techType == 2 || techType == 3 )
            break;
        if ( techType == 4 )
            break;
        if ( ((material->info.gameFlags & 0x80) == 0) != (penetrationMode == 1) )
            break;
        v8 = context.source->materialTime * 5.0;
        v6 = floor(v8);
        if ( v8 - v6 >= 0.5 )
            break;
        techType = 4;
    }
    if ( (technique->flags & 2) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_material.cpp",
                    360,
                    0,
                    "%s",
                    "!(technique->flags & MTL_TECHFLAG_NEEDS_RESOLVED_SCENE)") )
    {
        __debugbreak();
    }
    if ( (techType == 5 || techType == 4) && (technique->flags & 0x10) != 0 && !context.source->constVersions[28] )
        return 0;
    if ( !material
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_draw_material.cpp", 370, 0, "%s", "material") )
    {
        __debugbreak();
    }
    if ( r_logFile->current.integer )
    {
        v4 = RB_LogTechniqueType(techType);
        v5 = va("R_SetMaterial( %s, %s, %s )\n", material->info.name, technique->name, v4);
        RB_LogPrint(v5);
    }
    if ( R_GfxDrawSurf_GetFade(&drawSurf) )
    {
        fadeVal = (float)((drawSurf.packed >> 16) & 0xF) * 0.0625;
        context.source->input.consts[33][0] = fadeVal;
        context.source->input.consts[33][1] = fadeVal;
        context.source->input.consts[33][2] = fadeVal;
        context.source->input.consts[33][3] = fadeVal;
        R_DirtyCodeConstant(context.source, CONST_SRC_CODE_ALPHA_FADE);
    }
    RB_ApplyShaderConstantSet(context.source, &drawSurf);
    if ( drawSurf.fields.surfType == 0 && (material->info.gameFlags & 8) != 0 )
        return 0;
    context.state->techType = techType;
    return 1;
}

int __cdecl R_TrySetMaterialWithFunc(
                int (__cdecl *func)(GfxCmdBufContext, GfxDrawSurf, const unsigned __int8),
                GfxCmdBufContext context,
                GfxDrawSurf drawSurf,
                unsigned __int8 *techType)
{
    const char *v5; // [esp+0h] [ebp-18h]
    unsigned int i; // [esp+4h] [ebp-14h]
    int ret; // [esp+8h] [ebp-10h]
    unsigned __int8 techAssigned; // [esp+Fh] [ebp-9h]
    const char *techName; // [esp+10h] [ebp-8h]

    ret = 0;
    techAssigned = -1;
    for ( i = 0; techType[i] != 255; ++i )
    {
        if (func(context, drawSurf, techType[i] ))
        {
            if ( !i )
                return 1;
            techAssigned = techType[i];
            ret = 1;
            break;
        }
    }
    techName = RB_LogTechniqueType(*techType);
    if ( techAssigned == 255 )
        v5 = "TECHNIQUE_NONE";
    else
        v5 = RB_LogTechniqueType(techAssigned);
    R_WarnOncePerFrame(
        R_WARN_MISSING_TECHNIQUE,
        rgp.sortedMaterials[drawSurf.fields.materialSortedIndex]->info.name,
        techName,
        *techType,
        v5,
        techAssigned);
    return ret;
}

void __cdecl R_SetGameTime(GfxCmdBufSourceState *source, float gameTime)
{
    float gameTimeVec[4]; // [esp+1Ch] [ebp-10h] BYREF

    R_CalcGameTimeVec(gameTime, gameTimeVec);

    R_SetCodeConstant(source,
        CONST_SRC_CODE_GAMETIME,
        gameTimeVec[0],
        gameTimeVec[1],
        gameTimeVec[2],
        gameTimeVec[3]
        );
}

int __cdecl R_UpdateMaterialTime(
                GfxCmdBufSourceState *source,
                float materialTime,
                float burn,
                float fade,
                float wetness)
{
    source->destructibleBurnAmount = burn;
    source->destructibleFadeAmount = fade;
    source->wetness = wetness;
    if (materialTime == source->materialTime)
        return 0;
    source->materialTime = materialTime;
    R_SetGameTime(source, source->sceneDef.floatTime - materialTime);
    return 1;
}

