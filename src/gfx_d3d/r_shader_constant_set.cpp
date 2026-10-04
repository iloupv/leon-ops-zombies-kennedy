#include <universal/profile.h>
#include "r_shader_constant_set.h"
#include "r_material_load_obj.h"
#include "r_foliage.h"
#include "r_warn.h"
#include "r_dvars.h"
#include "r_debug.h"

void __cdecl R_InitShaderConstantSet(ShaderConstantSet *scs)
{
    scs->used = 0;
}

bool __cdecl R_ShaderConstantSetIsUsed(const ShaderConstantSet *scs)
{
    return scs->used != 0;
}

char __cdecl R_MapShaderConstantSet(ShaderConstantSet *scs, unsigned int index, const char *constantName)
{
    unsigned __int8 source; // [esp+3h] [ebp-1h] BYREF

    if ( index > 6 )
        return 0;
    if ( !R_FindScriptableConstantSource_ByName(constantName, &source) )
        return 0;
    scs->constantSource[index] = source;
    scs->used |= 1 << index;
    return 1;
}

char __cdecl R_UnmapShaderConstantSet(ShaderConstantSet *scs, unsigned int index)
{
    if ( index > 6 )
        return 0;
    scs->used &= ~(1 << index);
    return 1;
}

int __cdecl R_GetConstantIndex(const ShaderConstantSet *scs, const char *constantName)
{
    int i; // [esp+0h] [ebp-Ch]
    unsigned __int8 source; // [esp+7h] [ebp-5h] BYREF
    int retIndex; // [esp+8h] [ebp-4h]

    retIndex = -1;
    if ( R_FindScriptableConstantSource_ByName(constantName, &source) )
    {
        for ( i = 0; i < 7 && retIndex < 0; ++i )
        {
            if ( ((1 << i) & scs->used) != 0 && scs->constantSource[i] == source )
                retIndex = i;
        }
    }
    return retIndex;
}

int __cdecl R_GetFreeConstantIndex(const ShaderConstantSet *scs)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 7; ++i )
    {
        if ( ((1 << i) & scs->used) == 0 )
            return i;
    }
    return -1;
}

char __cdecl R_SetShaderConstantSetValue(ShaderConstantSet *scs, unsigned int index, float *v)
{
    float *v4; // [esp+0h] [ebp-4h]

    if ( index > 6 )
        return 0;
    if ( ((1 << index) & scs->used) == 0 )
        return 0;
    v4 = scs->value[index];
    *v4 = *v;
    v4[1] = v[1];
    v4[2] = v[2];
    v4[3] = v[3];
    return 1;
}

char __cdecl R_SetShaderConstantSetValue(
                ShaderConstantSet *scs,
                unsigned int index,
                float x,
                float y,
                float z,
                float w)
{
    float vec[4]; // [esp+0h] [ebp-10h] BYREF

    vec[0] = x;
    vec[1] = y;
    vec[2] = z;
    vec[3] = w;
    return R_SetShaderConstantSetValue(scs, index, vec);
}

int __cdecl RB_ApplyShaderConstantSet(GfxCmdBufSourceState *gfxSourceState, const ShaderConstantSet *scs)
{
    unsigned __int8 constantSource; // [esp+Bh] [ebp-9h]
    int i; // [esp+Ch] [ebp-8h]
    int updatedConstants; // [esp+10h] [ebp-4h]

    if ( !scs )
        return 0;
    if ( !scs->used )
        return 0;
    updatedConstants = 0;
    for ( i = 0; i < 7; ++i )
    {
        if ( ((1 << i) & scs->used) != 0 )
        {
            constantSource = scs->constantSource[i];
            if ( !R_FindScriptableConstantName_BySource(constantSource)
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp",
                            180,
                            0,
                            "%s",
                            "R_FindScriptableConstantName_BySource(constantSource)") )
            {
                __debugbreak();
            }
            updatedConstants |= R_UpdateCodeConstantFromVec4(gfxSourceState, (CodeConstant)constantSource, (float*)scs->value[i]);
        }
    }
    return updatedConstants;
}

const ShaderConstantSet *__cdecl RB_ShaderConstantSetFromDrawSurf(
                const GfxBackEndData *backEndData,
                const GfxDrawSurf *drawSurf)
{
    unsigned int index; // [esp+4h] [ebp-4h]

    index = XModelDrawSurf_DecodeIndex(drawSurf);
    if ( index )
        return &backEndData->shaderConstantSets[index];
    else
        return 0;
}

unsigned int __cdecl XModelDrawSurf_DecodeIndex(const GfxDrawSurf *drawSurf)
{
    int surfType = drawSurf->fields.surfType;
    if (surfType >= SF_BEGIN_XMODEL && surfType < SF_END_XMODEL)
        return (4 * ((drawSurf->packed >> 20) & 0x1C)) | HIWORD(*(unsigned int *)&drawSurf->fields) & 0xF;
    else
        return 0;
}

void __cdecl RB_ApplyShaderConstantSet(GfxCmdBufSourceState *gfxSourceState, const GfxDrawSurf *drawSurf)
{
    const ShaderConstantSet *cscEA; // [esp+0h] [ebp-8h]

    cscEA = RB_ShaderConstantSetFromDrawSurf(gfxSourceState->input.data, drawSurf);
    if ( cscEA )
        RB_ApplyShaderConstantSet(gfxSourceState, cscEA);
}

bool __cdecl RB_DrawSurf_Uses_ShaderConstantSet(const GfxDrawSurf *drawSurf)
{
    return XModelDrawSurf_DecodeIndex(drawSurf) != 0;
}

void __cdecl R_XModelDrawSurfEncodeShaderConstantSet(GfxDrawSurf *drawSurf, unsigned int constSetIndex)
{
    unsigned __int64 v2; // rax
    int v3; // ecx
    const ShaderConstantSet *scs; // [esp+4h] [ebp-4h]

    if ( constSetIndex >= 0x80
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp",
                    319,
                    0,
                    "constSetIndex doesn't index ARRAY_COUNT( frontEndDataOut->shaderConstantSets )\n\t%i not in [0, %i)",
                    constSetIndex,
                    128) )
    {
        __debugbreak();
    }
    if ( constSetIndex )
    {
        scs = R_ShaderConstantSet_FromFrontEndDataOut(constSetIndex);
        if ( !scs
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp", 324, 0, "%s", "scs") )
        {
            __debugbreak();
        }
        if ( R_ShaderConstantSet_Sets_Charred(scs) )
        {
            v2 = ((drawSurf->packed >> 20) & 0x1D | 2) << 20;
            v3 = HIDWORD(v2) | HIDWORD(drawSurf->packed);
            *(unsigned int *)&drawSurf->fields = v2 | *(unsigned int *)&drawSurf->fields & 0xFE0FFFFF;
            HIDWORD(drawSurf->packed) = v3;
        }
        XModelDrawSurf_EncodeIndex(drawSurf, constSetIndex);
    }
}

void __cdecl XModelDrawSurf_EncodeIndex(GfxDrawSurf *drawSurf, char scsIndex)
{
    unsigned __int64 v2; // rax
    unsigned __int64 v3; // rax
    int v4; // ecx
    unsigned __int64 v5; // rax
    int v6; // ecx
    unsigned __int8 customIndex; // [esp+4h] [ebp-Ch]

    v2 = drawSurf->packed >> 20;
    customIndex = v2 & 0x1F;
    if ( (v2 & 0x1C) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp",
                    220,
                    0,
                    "%s",
                    "(customIndex & (~0x3)) == 0") )
    {
        __debugbreak();
    }
    v3 = (unsigned __int64)((((unsigned __int8)(scsIndex & 0x70) >> 2) | customIndex) & 0x1F) << 20;
    v4 = HIDWORD(v3) | HIDWORD(drawSurf->packed);
    *(unsigned int *)&drawSurf->fields = v3 | *(unsigned int *)&drawSurf->fields & 0xFE0FFFFF;
    HIDWORD(drawSurf->packed) = v4;
    v5 = (unsigned __int64)(scsIndex & 0xF) << 16;
    v6 = HIDWORD(v5) | HIDWORD(drawSurf->packed);
    *(unsigned int *)&drawSurf->fields = v5 | *(unsigned int *)&drawSurf->fields & 0xFFF0FFFF;
    HIDWORD(drawSurf->packed) = v6;
}

bool __cdecl R_ShaderConstantSet_Sets_Charred(const ShaderConstantSet *scs)
{
    int i; // [esp+4h] [ebp-4h]

    if ( !scs )
        return 0;
    for ( i = 0; i < 7; ++i )
    {
        if ( ((1 << i) & scs->used) != 0 && scs->constantSource[i] == 118 )
            return scs->value[i][0] > 0.0;
    }
    return 0;
}

int __cdecl R_ShaderConstantSet_CopyToFrontEndDataOut(const ShaderConstantSet *scs)
{
    if ( !scs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp", 355, 0, "%s", "scs") )
    {
        __debugbreak();
    }
    if ( !R_ShaderConstantSetIsUsed(scs)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp",
                    356,
                    0,
                    "%s",
                    "R_ShaderConstantSetIsUsed(scs)") )
    {
        __debugbreak();
    }
    return CopyShaderConstantSetToBackendData(scs);
}

// mod (L43): r_mod_scsShare 1. The frame's set index is 7 bits of the 64-bit draw-surf sort key (XModelDrawSurf_EncodeIndex:
// bits 16-19 and 22-24; XModelDrawSurf_DecodeIndex), so the 128 slots can't grow without re-packing the key. Instead a
// set byte-identical to one already copied this frame reuses that slot: same constants, same draw. Off = retail.
volatile long g_scsFrameStamp = 1; // bumped where shaderConstantSetCount is reset (R_ToggleSmpFrame, r_rendercmds.cpp)
static volatile long s_scsReadyStamp[128]; // g_scsFrameStamp once slot i is fully copied

int __cdecl CopyShaderConstantSetToBackendData(const ShaderConstantSet *scs)
{
    int scsIndex; // [esp+Ch] [ebp-4h]

    if ( !scs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp", 299, 0, "%s", "scs") )
    {
        __debugbreak();
    }
    if ( r_mod_scsShare && r_mod_scsShare->current.enabled && scs ) // mod (L43)
    {
        const long stamp = g_scsFrameStamp;
        unsigned int count = frontEndDataOut->shaderConstantSetCount;
        if ( count > 128 )
            count = 128;
        for ( unsigned int i = 1; i < count; ++i )
        {
            if ( s_scsReadyStamp[i] == stamp && !memcmp(&frontEndDataOut->shaderConstantSets[i], scs, sizeof(*scs)) )
                return i;
        }
    }
    scsIndex = _InterlockedExchangeAdd(&frontEndDataOut->shaderConstantSetCount, 1u);
    if ( scsIndex >= 128 )
    {
        frontEndDataOut->shaderConstantSetCount = 128;
        R_WarnOncePerFrame(R_WARN_MAX_SHADER_CONSTANT_SETS, 128);
        _InterlockedIncrement(&g_modRendWarn[0]); // mod (L43): overflow count for BO1_SVPROF
        return 0;
    }
    else
    {
        memcpy(&frontEndDataOut->shaderConstantSets[scsIndex], scs, sizeof(frontEndDataOut->shaderConstantSets[scsIndex]));
        _InterlockedExchange(&s_scsReadyStamp[scsIndex], g_scsFrameStamp); // mod (L43): r_mod_scsShare (full barrier after the copy)
    }
    return scsIndex;
}

const ShaderConstantSet *__cdecl R_ShaderConstantSet_FromFrontEndDataOut(unsigned int constSetindex)
{
    if ( constSetindex >= 0x80
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_shader_constant_set.cpp",
                    363,
                    0,
                    "constSetindex doesn't index ARRAY_COUNT( frontEndDataOut->shaderConstantSets )\n\t%i not in [0, %i)",
                    constSetindex,
                    128) )
    {
        __debugbreak();
    }
    if ( constSetindex )
        return &frontEndDataOut->shaderConstantSets[constSetindex];
    else
        return 0;
}

void __cdecl RB_SaveCurrentShaderConstantSetValues(
                ShaderConstantSet *destSet,
                GfxCmdBufSourceState *gfxSourceState,
                const ShaderConstantSet *srcSet)
{
    int v3; // edx
    float *v4; // [esp+0h] [ebp-Ch]
    int i; // [esp+8h] [ebp-4h]

    R_InitShaderConstantSet(destSet);
    if ( srcSet && srcSet->used )
    {
        *(unsigned int *)destSet->constantSource = *(unsigned int *)srcSet->constantSource;
        *(_WORD *)&destSet->constantSource[4] = *(_WORD *)&srcSet->constantSource[4];
        destSet->constantSource[6] = srcSet->constantSource[6];
        destSet->used = srcSet->used;
        for ( i = 0; i < 7; ++i )
        {
            if ( ((1 << i) & destSet->used) != 0 )
            {
                v4 = destSet->value[i];
                v3 = destSet->constantSource[i];
                *v4 = gfxSourceState->input.consts[v3][0];
                v4[1] = gfxSourceState->input.consts[v3][1];
                v4[2] = gfxSourceState->input.consts[v3][2];
                v4[3] = gfxSourceState->input.consts[v3][3];
            }
        }
    }
}

ScopedShaderConstantSetUndo::ScopedShaderConstantSetUndo(GfxCmdBufSourceState *sourceState, const ShaderConstantSet *cscEA)
{
    this->m_sourceState = sourceState;
    if ( cscEA )
        RB_SaveCurrentShaderConstantSetValues(&this->m_scs, sourceState, cscEA);
    else
        R_InitShaderConstantSet(&this->m_scs);
    //return this;
}

ScopedShaderConstantSetUndo::~ScopedShaderConstantSetUndo()
{
    RB_ApplyShaderConstantSet(this->m_sourceState, &this->m_scs);
}


float my_scale = 0.22f;
float my_vert_spacing = 4.0f;
float my_height_offset = 5.0f;

void __cdecl R_ShaderConstantShowDebug(
                const float *eyePos,
                const float *objOrigin,
                float objRad,
                const ShaderConstantSet *scs)
{
    const char *constName; // [esp+34h] [ebp-258h]
    int i; // [esp+38h] [ebp-254h]
    float obj2eye; // [esp+3Ch] [ebp-250h]
    const float *colors[8]; // [esp+40h] [ebp-24Ch]
    float up[3]; // [esp+60h] [ebp-22Ch]
    unsigned int colorIndex; // [esp+6Ch] [ebp-220h]
    const float *color; // [esp+70h] [ebp-21Ch]
    char text[516]; // [esp+74h] [ebp-218h] BYREF
    float debugRangeSq; // [esp+27Ch] [ebp-10h]
    float textOrigin[3]; // [esp+280h] [ebp-Ch] BYREF

    if ( scs )
    {
        if ( r_shader_constant_set_debug_range->current.value > 0.0 )
        {
            debugRangeSq = r_shader_constant_set_debug_range->current.value * r_shader_constant_set_debug_range->current.value;
            obj2eye = Vec3DistanceSq(eyePos, objOrigin);
            if ( obj2eye <= debugRangeSq )
            {
                up[0] = 0.0f;
                up[1] = 0.0f;
                up[2] = 1.0f;
                textOrigin[0] = (float)((float)(objRad + my_height_offset) * 0.0) + *objOrigin;
                textOrigin[1] = (float)((float)(objRad + my_height_offset) * 0.0) + objOrigin[1];
                textOrigin[2] = (float)((float)(objRad + my_height_offset) * 1.0) + objOrigin[2];
                colors[0] = colorBlue;
                colors[1] = colorYellow;
                colors[2] = colorMagenta;
                colors[3] = colorCyan;
                colors[4] = colorWhite;
                colors[5] = colorOrange;
                colors[6] = colorRed;
                colors[7] = colorGreen;
                colorIndex = (unsigned __int8)scs >> 5;
                color = colors[colorIndex];
                R_AddDebugLine(&frontEndDataOut->debugGlobals, objOrigin, textOrigin, color, 0);
                for ( i = 0; i < 7; ++i )
                {
                    if ( ((1 << i) & scs->used) != 0 )
                    {
                        constName = R_FindScriptableConstantName_BySource(scs->constantSource[i]);
                        if ( constName )
                        {
                            _snprintf(
                                text,
                                0x200u,
                                "%d:%s %.2f %.2f %.2f %.2f",
                                i,
                                constName,
                                scs->value[i][0],
                                scs->value[i][1],
                                scs->value[i][2],
                                scs->value[i][3]);
                            R_AddDebugString(&frontEndDataOut->debugGlobals, textOrigin, color, my_scale, text);
                            textOrigin[0] = (float)((-(my_vert_spacing)) * up[0]) + textOrigin[0];
                            textOrigin[1] = (float)((-(my_vert_spacing)) * up[1]) + textOrigin[1];
                            textOrigin[2] = (float)((-(my_vert_spacing)) * up[2]) + textOrigin[2];
                        }
                    }
                }
            }
        }
    }
}

