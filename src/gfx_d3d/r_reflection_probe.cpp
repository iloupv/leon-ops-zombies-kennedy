#include "r_reflection_probe.h"
#include "r_bsp.h"
#include "r_dvars.h"
#include <qcommon/com_bsp_load_obj.h>
#include <universal/com_memory.h>
#include <win32/win_main.h>
#include "r_workercmds_common.h"
#include "r_bsp_load_obj.h"
#include <EffectsCore/fx_update.h>
#include "r_image.h"
#include "r_image_load_obj.h"
#include <universal/com_files.h>
#include <universal/q_parse.h>

const char *fields_5[6] = { "name", "black_level", "white_level", "gamma", "saturation", NULL };

const dvar_t *r_reflectionProbeRegenerateAll;
const dvar_t *r_reflectionProbeGenerateExit;

int s_numColorCorrectionDataEntries;

ColorCorrectionData s_colorCorrectionDataEntries[1024];

void __cdecl R_GetReflectionProbePosition(unsigned int index, float *pos)
{
    GfxReflectionProbe *v2; // [esp+0h] [ebp-4h]

    if ( index >= s_world.draw.reflectionProbeCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                    32,
                    0,
                    "%s",
                    "index < s_world.draw.reflectionProbeCount") )
    {
        __debugbreak();
    }
    v2 = &s_world.draw.reflectionProbes[index];
    *pos = v2->origin[0];
    pos[1] = v2->origin[1];
    pos[2] = v2->origin[2];
}

void __cdecl R_ReflectionProbeRegisterDvars()
{
    r_reflectionProbeGenerate = _Dvar_RegisterBool(
                                                                "r_reflectionProbeGenerate",
                                                                0,
                                                                0,
                                                                "Generate cube maps for reflection probes.");
    r_reflectionProbeRegenerateAll = _Dvar_RegisterBool(
                                                                         "r_reflectionProbeRegenerateAll",
                                                                         0,
                                                                         0,
                                                                         "Regenerate cube maps for all reflection probes.");
    r_reflectionProbeGenerateExit = _Dvar_RegisterBool(
                                                                        "r_reflectionProbeGenerateExit",
                                                                        0,
                                                                        0,
                                                                        "Exit when done generating reflection cubes.");
    if ( r_reflectionProbeGenerate->current.enabled )
        Material_PreventOverrideTechniqueGeneration();
}

void __cdecl R_CalcCubeMapViewValues(refdef_s *refdef, CubemapShot cubemapShot, int cubemapSize)
{
    refdef->x = 0;
    refdef->y = 0;
    refdef->width = cubemapSize + 2;
    refdef->height = cubemapSize + 2;
    refdef->tanHalfFovX = (float)(cubemapSize + 2) / (float)cubemapSize;
    refdef->tanHalfFovY = refdef->tanHalfFovX;
    refdef->zNear = 0.0f;
    switch ( cubemapShot )
    {
        case CUBEMAPSHOT_RIGHT:
            refdef->viewaxis[0][0] = 1.0f;
            refdef->viewaxis[0][1] = 0.0f;
            refdef->viewaxis[0][2] = 0.0f;
            refdef->viewaxis[1][0] = 0.0f;
            refdef->viewaxis[1][1] = 1.0f;
            refdef->viewaxis[1][2] = 0.0f;
            refdef->viewaxis[2][0] = 0.0f;
            refdef->viewaxis[2][1] = 0.0f;
            refdef->viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_LEFT:
            refdef->viewaxis[0][0] = -1.0f;
            refdef->viewaxis[0][1] = 0.0f;
            refdef->viewaxis[0][2] = 0.0f;
            refdef->viewaxis[1][0] = 0.0f;
            refdef->viewaxis[1][1] = -1.0f;
            refdef->viewaxis[1][2] = 0.0f;
            refdef->viewaxis[2][0] = 0.0f;
            refdef->viewaxis[2][1] = 0.0f;
            refdef->viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_FRONT:
            refdef->viewaxis[0][0] = 0.0f;
            refdef->viewaxis[0][1] = -1.0f;
            refdef->viewaxis[0][2] = 0.0f;
            refdef->viewaxis[1][0] = 1.0f;
            refdef->viewaxis[1][1] = 0.0f;
            refdef->viewaxis[1][2] = 0.0f;
            refdef->viewaxis[2][0] = 0.0f;
            refdef->viewaxis[2][1] = 0.0f;
            refdef->viewaxis[2][2] = 1.0f;
            break;
        case CUBEMAPSHOT_UP:
            refdef->viewaxis[0][0] = 0.0f;
            refdef->viewaxis[0][1] = 0.0f;
            refdef->viewaxis[0][2] = 1.0f;
            refdef->viewaxis[1][0] = 0.0f;
            refdef->viewaxis[1][1] = 1.0f;
            refdef->viewaxis[1][2] = 0.0f;
            refdef->viewaxis[2][0] = -1.0f;
            refdef->viewaxis[2][1] = 0.0f;
            refdef->viewaxis[2][2] = 0.0f;
            break;
        case CUBEMAPSHOT_DOWN:
            refdef->viewaxis[0][0] = 0.0f;
            refdef->viewaxis[0][1] = 0.0f;
            refdef->viewaxis[0][2] = -1.0f;
            refdef->viewaxis[1][0] = 0.0f;
            refdef->viewaxis[1][1] = 1.0f;
            refdef->viewaxis[1][2] = 0.0f;
            refdef->viewaxis[2][0] = 1.0f;
            refdef->viewaxis[2][1] = 0.0f;
            refdef->viewaxis[2][2] = 0.0f;
            break;
        default:
            if ( cubemapShot != CUBEMAPSHOT_BACK
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                            90,
                            1,
                            "%s\n\t(cubemapShot) = %i",
                            "(cubemapShot == CUBEMAPSHOT_BACK)",
                            cubemapShot) )
            {
                __debugbreak();
            }
            refdef->viewaxis[0][0] = 0.0f;
            refdef->viewaxis[0][1] = 1.0f;
            refdef->viewaxis[0][2] = 0.0f;
            refdef->viewaxis[1][0] = -1.0f;
            refdef->viewaxis[1][1] = 0.0f;
            refdef->viewaxis[1][2] = 0.0f;
            refdef->viewaxis[2][0] = 0.0f;
            refdef->viewaxis[2][1] = 0.0f;
            refdef->viewaxis[2][2] = 1.0f;
            break;
    }
}

void __cdecl R_GenerateReflections(char *mapname, GfxReflectionProbe *probes, unsigned int probeCount)
{
    char v3; // al
    DiskGfxReflectionProbe *v4; // [esp+8h] [ebp-2Ch]
    GfxReflectionProbe *v5; // [esp+Ch] [ebp-28h]
    unsigned int lumpProbeCount; // [esp+10h] [ebp-24h] BYREF
    unsigned int version; // [esp+14h] [ebp-20h]
    const DiskGfxReflectionProbe *probeRawLumpData; // [esp+18h] [ebp-1Ch]
    DiskGfxReflectionProbe *probeRawGeneratedData; // [esp+1Ch] [ebp-18h]
    unsigned int probeIndex; // [esp+20h] [ebp-14h]
    bool generateProbe[8]; // [esp+24h] [ebp-10h] BYREF
    unsigned int lumpSize; // [esp+30h] [ebp-4h]

    if ( !r_reflectionProbeGenerate
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                    194,
                    0,
                    "%s",
                    "r_reflectionProbeGenerate") )
    {
        __debugbreak();
    }
    if ( probeCount >= 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                    195,
                    0,
                    "%s",
                    "probeCount < MAX_MAP_REFLECTION_PROBES") )
    {
        __debugbreak();
    }
    if ( r_reflectionProbeGenerate->current.enabled )
    {
        Com_LoadBsp(mapname);
        version = Com_GetBspVersion();
        if ( version != 45 )
            Com_Error(
                ERR_DROP,
                "You can only generate reflections for BSP version %i, but the BSP is version %i.    You need to recompile the map.",
                45,
                version);
        probeRawLumpData = (const DiskGfxReflectionProbe *)Com_GetBspLump(LUMP_REFLECTION_PROBES, 0x406E0u, &lumpProbeCount);
        if ( probeCount != lumpProbeCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                        207,
                        0,
                        "probeCount == lumpProbeCount\n\t%i, %i",
                        probeCount,
                        lumpProbeCount) )
        {
            __debugbreak();
        }
        if ( probeCount )
        {
            lumpSize = 263904 * probeCount;
            probeRawGeneratedData = (DiskGfxReflectionProbe *)Z_Malloc(263904 * probeCount, "R_GenerateReflections", 0);
            for ( probeIndex = 0; probeIndex < lumpProbeCount; ++probeIndex )
            {
                memcpy(
                    probeRawGeneratedData[probeIndex].colorCorrectionFilename,
                    probeRawLumpData[probeIndex].colorCorrectionFilename,
                    sizeof(probeRawGeneratedData[probeIndex].colorCorrectionFilename));
                v4 = &probeRawGeneratedData[probeIndex];
                v5 = &probes[probeIndex];
                v4->origin[0] = v5->origin[0];
                v4->origin[1] = v5->origin[1];
                v4->origin[2] = v5->origin[2];
                if ( r_reflectionProbeRegenerateAll->current.enabled )
                {
                    generateProbe[probeIndex] = 1;
                }
                else
                {
                    v3 = R_CopyReflectionsFromLumpData(&probeRawGeneratedData[probeIndex], probeRawLumpData, probeCount);
                    generateProbe[probeIndex] = v3 == 0;
                }
            }
            R_GenerateReflectionRawDataAll(probeRawGeneratedData, lumpProbeCount, generateProbe);
            Com_SaveLump(LUMP_REFLECTION_PROBES, probeRawGeneratedData, lumpSize, COM_SAVE_LUMP_AND_CLOSE);
            R_GenerateReflectionImages(probes, probeRawGeneratedData, probeCount, 0, 256);
            Z_Free((char *)probeRawGeneratedData, 0);
        }
        else
        {
            Com_SaveLump(LUMP_REFLECTION_PROBES, 0, 0, COM_SAVE_LUMP_AND_CLOSE);
        }
        if ( R_ReflectionProbeGenerateExitWhenDone() )
        {
            Sys_NormalExit();
            exit(0);
        }
        Dvar_SetBool((dvar_s *)r_reflectionProbeGenerate, 0);
    }
}

void __cdecl R_GenerateReflectionRawDataAll(DiskGfxReflectionProbe *probeRawData, int probeCount, bool *generateProbe)
{
    int probeIndex; // [esp+0h] [ebp-4h]
    int savedregs; // [esp+4h] [ebp+0h] BYREF

    for ( probeIndex = 0; probeIndex < probeCount; ++probeIndex )
    {
        if ( generateProbe[probeIndex] )
            R_GenerateReflectionRawData(&probeRawData[probeIndex]);
    }
}

void    R_GenerateReflectionRawData(DiskGfxReflectionProbe *probeRawData)
{
    void *v2; // esp
    float zfar; // [esp+0h] [ebp-185B0h]
    FxCmd v4; // [esp+10h] [ebp-185A0h] BYREF
    FxCameraUpdate v5; // [esp+44h] [ebp-1856Ch] BYREF
    CubemapShot i; // [esp+80h] [ebp-18530h]
    refdef_s dst; // [esp+18h] [ebp-40A0h] BYREF

    memset(&dst, 0, sizeof(dst));
    dst.vieworg[0] = probeRawData->origin[0];
    dst.vieworg[1] = probeRawData->origin[1];
    dst.vieworg[2] = probeRawData->origin[2];
    dst.localClientNum = 0;
    dst.time = 0;
    dst.blurRadius = 0.0;
    dst.useScissorViewport = 0;

    R_InitPrimaryLights(dst.primaryLights);

    for ( i = CUBEMAPSHOT_RIGHT; i < CUBEMAPSHOT_COUNT; ++i )
    {
        R_BeginCubemapShot(256, 0);
        R_BeginFrame();
        R_BeginSharedCmdList();
        R_ClearClientCmdList2D();
        R_ClearScene(0);
        FX_BeginUpdate(0);
        R_CalcCubeMapViewValues(&dst, i, 256);
        R_SetLodOrigin(&dst);
        zfar = R_GetFarPlaneDist();
        FX_GetCameraUpdateFromRefdefAndZFar(&v5, &dst, zfar);
        FX_SetNextUpdateCamera(0, &v5);
        FX_FillUpdateCmd(0, &v4);
        R_UpdateSpotLightEffect(&v4);
        R_UpdateNonDependentEffects(&v4);
        R_UpdateRemainingEffects(&v4);
        R_RenderScene(&dst, 0);
        R_EndFrame();
        R_IssueRenderCommands(3u);
        R_EndCubemapShot(i);
    }
    R_CreateReflectionRawDataFromCubemapShot(probeRawData, 256);
}

char __cdecl R_CopyReflectionsFromLumpData(
                DiskGfxReflectionProbe *probeRawData,
                const DiskGfxReflectionProbe *probeRawLumpData,
                int lumpProbeCount)
{
    const DiskGfxReflectionProbe *v5; // [esp+4h] [ebp-8h]
    int probeIndex; // [esp+8h] [ebp-4h]

    for ( probeIndex = 0; probeIndex < lumpProbeCount; ++probeIndex )
    {
        v5 = &probeRawLumpData[probeIndex];
        if ( probeRawData->origin[0] == v5->origin[0]
            && probeRawData->origin[1] == v5->origin[1]
            && probeRawData->origin[2] == v5->origin[2] )
        {
            memcpy(probeRawData->pixels, probeRawLumpData[probeIndex].pixels, sizeof(probeRawData->pixels));
            return 1;
        }
    }
    return 0;
}

bool __cdecl R_ReflectionProbeGenerateExitWhenDone()
{
    return r_reflectionProbeGenerate
            && r_reflectionProbeGenerate->current.enabled
            && r_reflectionProbeGenerateExit
            && r_reflectionProbeGenerateExit->current.enabled;
}

void __cdecl R_SetBlackProbe(unsigned __int8 *target, unsigned int bytestozero)
{
    memset(target, 0, bytestozero);
}

void __cdecl R_GenerateDummyReflectionImages(
                GfxReflectionProbe *probes,
                const DiskGfxDummyReflectionProbe *probeRawData,
                int probeCount,
                int probeBaseIndex,
                int probeHighMipSize)
{
    GfxReflectionProbe *v5; // [esp+8h] [ebp-10h]
    const DiskGfxDummyReflectionProbe *v6; // [esp+Ch] [ebp-Ch]
    unsigned __int8 *pixels; // [esp+10h] [ebp-8h]
    int probeIndex; // [esp+14h] [ebp-4h]

    if ( !s_numColorCorrectionDataEntries )
        R_LoadColorCorrectionData();
    if ( !s_numColorCorrectionDataEntries )
        R_CreateDefaultColorCorrectionEntry();
    pixels = (unsigned __int8 *)Hunk_AllocateTempMemory(144, "R_GenerateReflectionImages");
    for ( probeIndex = 0; probeIndex < probeCount; ++probeIndex )
    {
        v5 = &probes[probeIndex];
        v6 = &probeRawData[probeIndex];
        v5->origin[0] = v6->origin[0];
        v5->origin[1] = v6->origin[1];
        v5->origin[2] = v6->origin[2];
        v5->probeVolumeCount = 0;
        memcpy(pixels, v6->pixels, 0x90u);
        v5->reflectionImage = R_GenerateReflectionImageFromRawData(pixels, probeBaseIndex + probeIndex, probeHighMipSize);
    }
    Hunk_FreeTempMemory((char *)pixels);
}

GfxImage *__cdecl R_GenerateReflectionImageFromRawData(
                const unsigned __int8 *rawPixels,
                int probeIndex,
                int probeHighMipSize)
{
    char *v3; // eax
    int v5; // [esp+4h] [ebp-194h]
    int v6; // [esp+8h] [ebp-190h]
    int v7; // [esp+Ch] [ebp-18Ch]
    const unsigned __int8 *pixels[6][15]; // [esp+18h] [ebp-180h] BYREF
    int imgIndex; // [esp+180h] [ebp-18h]
    int mipmapLevelSize; // [esp+184h] [ebp-14h]
    GfxImage *reflectionImage; // [esp+188h] [ebp-10h]
    _D3DFORMAT imageFormat; // [esp+18Ch] [ebp-Ch]
    int scaledSize; // [esp+190h] [ebp-8h]
    int mipLevel; // [esp+194h] [ebp-4h]

    imageFormat = D3DFMT_DXT1;
    mipLevel = 0;
    mipmapLevelSize = probeHighMipSize * probeHighMipSize / 2;
    for ( imgIndex = 0; imgIndex < 6; ++imgIndex )
    {
        pixels[imgIndex][0] = rawPixels;
        rawPixels += mipmapLevelSize;
    }
    for ( imgIndex = 0; imgIndex < 6; ++imgIndex )
    {
        scaledSize = probeHighMipSize;
        mipLevel = 1;
        do
        {
            if ( scaledSize >> 1 > 1 )
                v7 = scaledSize >> 1;
            else
                v7 = 1;
            scaledSize = v7;
            if ( v7 <= 4 )
                v6 = 4;
            else
                v6 = scaledSize;
            if ( scaledSize <= 4 )
                v5 = 4;
            else
                v5 = scaledSize;
            mipmapLevelSize = v5 * v6 / 2;
            pixels[imgIndex][mipLevel] = rawPixels;
            rawPixels += mipmapLevelSize;
            ++mipLevel;
        }
        while ( scaledSize != 1 );
    }
    v3 = va("*reflection_probe%i", probeIndex);
    reflectionImage = Image_Alloc(v3, 1u, 1u, 0);
    if ( !reflectionImage
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                    456,
                    1,
                    "%s",
                    "reflectionImage") )
    {
        __debugbreak();
    }
    Image_GenerateCube(reflectionImage, pixels, probeHighMipSize, imageFormat, mipLevel);
    return reflectionImage;
}

void R_LoadColorCorrectionData()
{
    unsigned __int8 *filebuf; // [esp+4h] [ebp-Ch]
    int fileSize; // [esp+8h] [ebp-8h]
    int f; // [esp+Ch] [ebp-4h] BYREF

    fileSize = FS_FOpenFileByMode((char*)"reflections/reflections.csv", &f, FS_READ);
    if ( fileSize >= 0 )
    {
        Hunk_CheckTempMemoryHighClear();
        filebuf = (unsigned __int8 *)Hunk_AllocateTempMemoryHigh(fileSize + 1, "R_LoadColorCorrectionData");
        FS_Read(filebuf, fileSize, f);
        FS_FCloseFile(f);
        filebuf[fileSize] = 0;
        Com_BeginParseSession("reflections/reflections.csv");
        Com_SetCSV(1);
        R_ParseColorCorrectionData((const char *)filebuf, "reflections/reflections.csv");
        Com_EndParseSession();
        Hunk_ClearTempMemoryHigh();
    }
    else
    {
        Com_PrintError(1, (char *)"R_LoadColorCorrectionData: failed to open '%s'", "reflections/reflections.csv");
    }
}

void __cdecl R_ParseColorCorrectionData(const char *buf, const char *filename)
{
    ColorCorrectionData *ccd; // [esp+10h] [ebp-8h]
    parseInfo_t *token; // [esp+14h] [ebp-4h]

    if ( s_numColorCorrectionDataEntries
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                    492,
                    0,
                    "%s",
                    "s_numColorCorrectionDataEntries == 0") )
    {
        __debugbreak();
    }
    if ( R_VerifyFieldNames(&buf, filename) )
    {
        while ( 1 )
        {
            token = Com_Parse(&buf);
            if ( !buf )
                break;
            if ( s_numColorCorrectionDataEntries == 1024 )
            {
                Com_PrintError(1, (char *)"R_ParseColorCorrectionData: file '%s' max color correction entries [%d] exceeded. Ignoring the rest of the file '%s'", filename, 1024, filename);
                return;
            }
            ccd = &s_colorCorrectionDataEntries[s_numColorCorrectionDataEntries++];
            if ( !token
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                            511,
                            0,
                            "%s",
                            "token") )
            {
                __debugbreak();
            }
            if ( strlen(token->token) >= 0x40 )
                Com_PrintError(1, (char *)"R_ParseColorCorrectionData: file '%s' truncating name because '%s' is too longer than %d.", filename, token, 64); // too longer
            I_strncpyz(ccd->name, token->token, 64);
            ccd->black_level = Com_ParseFloatOnLine(&buf);
            ccd->white_level = Com_ParseFloatOnLine(&buf);
            ccd->gamma = Com_ParseFloatOnLine(&buf);
            ccd->saturation = Com_ParseFloatOnLine(&buf);
            ccd->range = ccd->white_level - ccd->black_level;
            Com_SkipRestOfLine(&buf);
        }
    }
}

char __cdecl R_VerifyFieldNames(const char **buf, const char *filename)
{
    int fieldIndex; // [esp+0h] [ebp-8h]
    parseInfo_t *token; // [esp+4h] [ebp-4h]

    for ( fieldIndex = 0; fields_5[fieldIndex]; ++fieldIndex )
    {
        token = Com_Parse(buf);
        if ( I_stricmp(fields_5[fieldIndex], token->token) )
        {
            Com_PrintError(1, (char *)"R_ParseColorCorrectionData: file '%s' column header %d was '%s' instead of '%s'", filename, fieldIndex, token, fields_5[fieldIndex]);
            return 0;
        }
    }
    Com_SkipRestOfLine(buf);
    return 1;
}

ColorCorrectionData *R_CreateDefaultColorCorrectionEntry()
{
    ColorCorrectionData *result; // eax
    ColorCorrectionData *ccd; // [esp+0h] [ebp-4h]

    if ( s_numColorCorrectionDataEntries
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_reflection_probe.cpp",
                    564,
                    0,
                    "%s",
                    "s_numColorCorrectionDataEntries == 0") )
    {
        __debugbreak();
    }
    ccd = &s_colorCorrectionDataEntries[s_numColorCorrectionDataEntries++];
    I_strncpyz(ccd->name, "default", 64);
    ccd->black_level = 0.0f;
    ccd->white_level = 1.0f;
    ccd->gamma = 1.0f;
    ccd->saturation = 1.0f;
    result = ccd;
    ccd->range = ccd->white_level - ccd->black_level;
    return result;
}

void __cdecl R_GenerateReflectionImages(
                GfxReflectionProbe *probes,
                const DiskGfxReflectionProbe *probeRawData,
                int probeCount,
                int probeBaseIndex,
                int probeHighMipSize)
{
    float *v5; // [esp+4h] [ebp-28h]
    float *v6; // [esp+8h] [ebp-24h]
    GfxReflectionProbe *v7; // [esp+Ch] [ebp-20h]
    const DiskGfxReflectionProbe *v8; // [esp+10h] [ebp-1Ch]
    int sideIndex; // [esp+14h] [ebp-18h]
    GfxReflectionProbeVolumeData *probeVolumeData; // [esp+1Ch] [ebp-10h]
    unsigned int probeVolumeCount; // [esp+20h] [ebp-Ch]
    unsigned __int8 *pixels; // [esp+24h] [ebp-8h]
    int probeIndex; // [esp+28h] [ebp-4h]

    if ( !s_numColorCorrectionDataEntries )
        R_LoadColorCorrectionData();
    if ( !s_numColorCorrectionDataEntries )
        R_CreateDefaultColorCorrectionEntry();
    pixels = (unsigned __int8 *)Hunk_AllocateTempMemory(262224, "R_GenerateReflectionImages");
    for ( probeIndex = 0; probeIndex < probeCount; ++probeIndex )
    {
        v7 = &probes[probeIndex];
        v8 = &probeRawData[probeIndex];
        v7->origin[0] = v8->origin[0];
        v7->origin[1] = v8->origin[1];
        v7->origin[2] = v8->origin[2];
        v7->probeVolumeCount = v8->probeVolumeCount;
        if ( v7->probeVolumeCount )
        {
            probes[probeIndex].probeVolumes = (GfxReflectionProbeVolumeData *)Hunk_Alloc(
                                                                                                                                                    96 * probeRawData[probeIndex].probeVolumeCount,
                                                                                                                                                    "R_GenerateReflectionImages",
                                                                                                                                                    21);
            for ( probeVolumeCount = 0; probeVolumeCount < probeRawData[probeIndex].probeVolumeCount; ++probeVolumeCount )
            {
                probeVolumeData = &probes[probeIndex].probeVolumes[probeVolumeCount];
                for ( sideIndex = 0; sideIndex < 6; ++sideIndex )
                {
                    v5 = probeVolumeData->volumePlanes[sideIndex];
                    v6 = (float*)probeRawData[probeIndex].probeVolumes[probeVolumeCount].volumePlanes[sideIndex];
                    *v5 = *v6;
                    v5[1] = v6[1];
                    v5[2] = v6[2];
                    v5[3] = v6[3];
                }
            }
        }
        memcpy(pixels, probeRawData[probeIndex].pixels, 0x40050u);
        probes[probeIndex].reflectionImage = R_GenerateReflectionImageFromRawData(
                                                                                     pixels,
                                                                                     probeBaseIndex + probeIndex,
                                                                                     probeHighMipSize);
    }
    Hunk_FreeTempMemory((char *)pixels);
}

