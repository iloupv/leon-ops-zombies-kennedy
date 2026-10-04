#include "xmodel_utils.h"
#include "xmodel.h"
#include "xanim.h"

#include <bgame/bg_local.h>

struct TestLod // sizeof=0x8
{                                       // XREF: .data:g_testLods/r
    bool enabled;                       // XREF: XModelLodInfo_GetDist(XModelLodInfo const *,int)+7/r
                                        // XModelSetTestLods(int,float)+65/w
    // padding byte
    // padding byte
    // padding byte
    float dist;                         // XREF: XModelLodInfo_GetDist(XModelLodInfo const *,int)+24/r
                                        // XModelSetTestLods(int,float)+38/w
};

TestLod g_testLods[4] = 
{
    { false, -1.0f },
    { false, -1.0f },
    { false, -1.0f },
    { false, -1.0f },
};

int __cdecl XModelNumBones(const XModel *model)
{
    return model->numBones;
}

const char *__cdecl XModelGetName(const XModel *model)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 79, 0, "%s", "model") )
        __debugbreak();
    return model->name;
}

int __cdecl XModelGetSurfaces(const XModel *model, XSurface **surfaces, int submodel)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 97, 0, "%s", "model") )
        __debugbreak();
    if ( !surfaces
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 98, 0, "%s", "surfaces") )
    {
        __debugbreak();
    }
    if ( submodel < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 99, 0, "%s", "submodel >= 0") )
    {
        __debugbreak();
    }
    if ( (unsigned int)submodel >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    104,
                    0,
                    "submodel doesn't index MAX_LODS\n\t%i not in [0, %i)",
                    submodel,
                    4) )
    {
        __debugbreak();
    }
    if ( model->lodInfo[submodel].surfIndex >= (unsigned int)model->numsurfs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    108,
                    0,
                    "lodInfo->surfIndex doesn't index model->numsurfs\n\t%i not in [0, %i)",
                    model->lodInfo[submodel].surfIndex,
                    model->numsurfs) )
    {
        __debugbreak();
    }
    if ( model->lodInfo[submodel].numsurfs + model->lodInfo[submodel].surfIndex > model->numsurfs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    109,
                    0,
                    "%s",
                    "lodInfo->surfIndex + lodInfo->numsurfs <= model->numsurfs") )
    {
        __debugbreak();
    }
    *surfaces = &model->surfs[model->lodInfo[submodel].surfIndex];
    return model->lodInfo[submodel].numsurfs;
}

XSurface *__cdecl XModelGetSurface(const XModel *model, int lod, int surfIndex)
{
    unsigned int modelSurfIndex; // [esp+0h] [ebp-4h]

    if ( lod < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 165, 0, "%s", "lod >= 0") )
    {
        __debugbreak();
    }
    modelSurfIndex = surfIndex + model->lodInfo[lod].surfIndex;
    if ( modelSurfIndex >= model->numsurfs
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    168,
                    0,
                    "modelSurfIndex doesn't index model->numsurfs\n\t%i not in [0, %i)",
                    modelSurfIndex,
                    model->numsurfs) )
    {
        __debugbreak();
    }
    return &model->surfs[modelSurfIndex];
}

const XModelLodInfo *__cdecl XModelGetLodInfo(const XModel *model, int lod)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 184, 0, "%s", "model") )
        __debugbreak();
    if ( lod < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 185, 0, "%s", "lod >= 0") )
    {
        __debugbreak();
    }
    return &model->lodInfo[lod];
}

void __cdecl XModelSetSModelCacheForLod(
                XModel *model,
                unsigned int lod,
                unsigned int smcIndex,
                unsigned int smcAllocBits)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 203, 0, "%s", "model") )
        __debugbreak();
    if ( lod >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    204,
                    0,
                    "lod doesn't index MAX_LODS\n\t%i not in [0, %i)",
                    lod,
                    4) )
    {
        __debugbreak();
    }
    if ( model->lodInfo[lod].smcIndexPlusOne
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    205,
                    0,
                    "%s",
                    "model->lodInfo[lod].smcIndexPlusOne == 0") )
    {
        __debugbreak();
    }
    if ( model->lodInfo[lod].lod != lod
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    206,
                    0,
                    "%s",
                    "model->lodInfo[lod].lod == lod") )
    {
        __debugbreak();
    }
    model->lodInfo[lod].smcIndexPlusOne = truncate_cast<unsigned char>(smcIndex + 1);
    model->lodInfo[lod].smcAllocBits = truncate_cast<unsigned char>(smcAllocBits);
}

int __cdecl XModelGetStaticModelCacheVertCount(XModel *model, unsigned int lod)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 226, 0, "%s", "model") )
        __debugbreak();
    if ( lod >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    227,
                    0,
                    "lod doesn't index MAX_LODS\n\t%i not in [0, %i)",
                    lod,
                    4) )
    {
        __debugbreak();
    }
    if ( !model->lodInfo[lod].smcIndexPlusOne
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    228,
                    0,
                    "%s",
                    "model->lodInfo[lod].smcIndexPlusOne != 0") )
    {
        __debugbreak();
    }
    return 1 << model->lodInfo[lod].smcAllocBits;
}

void __cdecl XModelGetSurfaceStreamBounds(const XModel *model, int surfIndex, float *outMins, float *outMaxs)
{
    float v4; // [esp+0h] [ebp-18h]
    float v5; // [esp+8h] [ebp-10h]
    XModelHighMipBounds *v6; // [esp+10h] [ebp-8h]
    XModelHighMipBounds *v7; // [esp+14h] [ebp-4h]

    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 247, 0, "%s", "model") )
        __debugbreak();
    if ( (surfIndex < 0
         || surfIndex >= model->lodInfo[3].numsurfs
                                     + model->lodInfo[2].numsurfs
                                     + model->lodInfo[1].numsurfs
                                     + model->lodInfo[0].numsurfs)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    250,
                    0,
                    "%s\n\t(surfIndex) = %i",
                    "(surfIndex >= 0 && surfIndex < (model->lodInfo[0].numsurfs+model->lodInfo[1].numsurfs+model->lodInfo[2].numsur"
                    "fs+model->lodInfo[3].numsurfs))",
                    surfIndex) )
    {
        __debugbreak();
    }
    v7 = &model->streamInfo.highMipBounds[surfIndex];
    *outMins = v7->center[0];
    outMins[1] = v7->center[1];
    outMins[2] = v7->center[2];
    v6 = &model->streamInfo.highMipBounds[surfIndex];
    *outMaxs = v6->center[0];
    outMaxs[1] = v6->center[1];
    outMaxs[2] = v6->center[2];
    (v5) = -(sqrtf(model->streamInfo.highMipBounds[surfIndex].himipRadiusSq));
    *outMins = *outMins + v5;
    outMins[1] = outMins[1] + v5;
    outMins[2] = outMins[2] + v5;
    v4 = sqrtf(model->streamInfo.highMipBounds[surfIndex].himipRadiusSq);
    *outMaxs = *outMaxs + v4;
    outMaxs[1] = outMaxs[1] + v4;
    outMaxs[2] = outMaxs[2] + v4;
}

unsigned int __cdecl XModelGetSurfCount(const XModel *model, int lod)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 274, 0, "%s", "model") )
        __debugbreak();
    if ( lod < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 275, 0, "%s", "lod >= 0") )
    {
        __debugbreak();
    }
    return model->lodInfo[lod].numsurfs;
}

Material **__cdecl XModelGetSkins(const XModel *model, int lod)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 293, 0, "%s", "model") )
        __debugbreak();
    if ( lod < 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 294, 0, "%s", "lod >= 0") )
    {
        __debugbreak();
    }
    return &model->materialHandles[model->lodInfo[lod].surfIndex];
}

XModelLodRampType __cdecl XModelGetLodRampType(const XModel *model)
{
    return (XModelLodRampType)model->lodRampType;
}

int __cdecl XModelGetNumLods(const XModel *model)
{
    return model->numLods;
}

double __cdecl XModelGetLodOutDist(const XModel *model)
{
    int lodMax; // [esp+4h] [ebp-4h]

    for ( lodMax = XModelGetNumLods(model) - 1; lodMax > 0 && model->lodInfo[lodMax].dist <= 0.0; --lodMax )
        ;
    return model->lodInfo[lodMax].dist;
}

int __cdecl XModelGetBoneIndex(const XModel *model, unsigned int name, unsigned int offset, unsigned __int8 *index)
{
    unsigned int numBones; // [esp+0h] [ebp-Ch]
    unsigned int localBoneIndex; // [esp+4h] [ebp-8h]
    unsigned __int16 *boneNames; // [esp+8h] [ebp-4h]

    if ( !index && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 379, 0, "%s", "index") )
        __debugbreak();
    boneNames = model->localBoneNames;
    numBones = model->numBones;

    iassert(numBones < DOBJ_MAX_PARTS);

    for ( localBoneIndex = 0; ; ++localBoneIndex )
    {
        if ( localBoneIndex >= numBones )
            return 0;
        if ( name == boneNames[localBoneIndex] )
            break;
    }
    *index = localBoneIndex + offset;
    if ( *index != localBoneIndex + offset
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    395,
                    0,
                    "%s",
                    "*index == offset + localBoneIndex") )
    {
        __debugbreak();
    }
    return 1;
}

int __cdecl XModelGetParentBoneName(const XModel *model, unsigned int name)
{
    unsigned int numBones; // [esp+0h] [ebp-10h]
    unsigned int boneIndex; // [esp+4h] [ebp-Ch]
    unsigned __int16 *boneNames; // [esp+Ch] [ebp-4h]

    boneNames = model->localBoneNames;
    numBones = model->numBones;

    iassert(numBones < DOBJ_MAX_PARTS);

    if ( !model->localParentList )
        return 0;
    for ( boneIndex = 0; boneIndex < numBones; ++boneIndex )
    {
        if ( name == boneNames[boneIndex] )
            return boneNames[model->localParentList[boneIndex]];
    }
    return 0;
}

const DObjAnimMat *__cdecl XModelGetBasePose(const XModel *model)
{
    return model->baseMat;
}

double __cdecl XModelLodInfo_GetDist(const XModelLodInfo *lodInfo, int lodIndex)
{
    if ( g_testLods[lodIndex].enabled )
        return g_testLods[lodIndex].dist;
    else
        return lodInfo->dist;
}

int __cdecl XModelGetLodForDist(const XModel *model, float adjustedDist, float baseDist, bool noLodCullOut)
{
    int v6; // [esp+8h] [ebp-34h]
    int v7; // [esp+Ch] [ebp-30h]
    int result; // [esp+38h] [ebp-4h]

    result = XModelGetLodForDist_Internal(model, adjustedDist);
    if ( result == -1 )
    {
        if ( adjustedDist != baseDist )
            result = XModelGetLodForDist_Internal(model, baseDist);
        if ( noLodCullOut && result == -1 )
        {
            v6 = XModelGetNumLods(model) - 2;
            if ( v6 < 3 )
                v7 = v6;
            else
                v7 = 3;
            if ( v7 > 0 )
                return v7;
            else
                return 0;
        }
    }
    return result;
}

int __cdecl XModelGetLodForDist_Internal(const XModel *model, float dist)
{
    const char *v2; // eax
    int lodIndex; // [esp+Ch] [ebp-Ch]
    int lodCount; // [esp+10h] [ebp-8h]
    float lodDist; // [esp+14h] [ebp-4h]

    lodCount = XModelGetNumLods(model);
    if ( (LODWORD(dist) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 494, 0, "%s", "!IS_NAN(dist)") )
    {
        __debugbreak();
    }
    for ( lodIndex = 0; lodIndex < lodCount; ++lodIndex )
    {
        lodDist = XModelLodInfo_GetDist(&model->lodInfo[lodIndex], lodIndex);
        if ( (LODWORD(lodDist) & 0x7F800000) == 0x7F800000 )
        {
            if ( model->name )
                v2 = va("'%s' nan lod dist. %d/%d", model->name, lodIndex, lodCount);
            else
                v2 = va("'%s' nan lod dist. %d/%d", "", lodIndex, lodCount);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                            501,
                            0,
                            "%s\n\t%s",
                            "!IS_NAN(lodDist)",
                            v2) )
                __debugbreak();
        }
        if ( lodDist == 0.0 || lodDist > dist )
            return lodIndex;
    }
    return -1;
}

void __cdecl XModelSetTestLods(unsigned int lodLevel, float dist)
{
    if ( lodLevel >= 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    604,
                    0,
                    "%s",
                    "(unsigned)lodLevel < MAX_LODS") )
    {
        __debugbreak();
    }
    g_testLods[lodLevel].dist = dist;
    g_testLods[lodLevel].enabled = dist >= 0.0;
}

double __cdecl XModelGetLodDist(const XModel *model, unsigned int lod)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 614, 0, "%s", "model") )
        __debugbreak();
    if ( lod >= model->numLods
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    615,
                    0,
                    "lod doesn't index model->numLods\n\t%i not in [0, %i)",
                    lod,
                    model->numLods) )
    {
        __debugbreak();
    }
    return model->lodInfo[lod].dist;
}

int __cdecl XModelGetContents(const XModel *model)
{
    PhysGeomList *geomListEA; // [esp+4h] [ebp-10h]
    int i; // [esp+8h] [ebp-Ch]
    int contents; // [esp+Ch] [ebp-8h]

    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 630, 0, "%s", "model") )
        __debugbreak();
    contents = model->contents;
    for ( i = 0; i < model->numCollmaps; ++i )
    {
        geomListEA = model->collmaps[i].geomList;
        if ( geomListEA )
            contents |= geomListEA->contents;
    }
    return contents;
}

bool __cdecl XModelHasCollmap(const XModel *model)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 661, 0, "%s", "model") )
        __debugbreak();
    return model->numCollmaps != 0;
}

PhysGeomList *__cdecl XModelGetCollmap(const XModel *model, int collMapIndex)
{
    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 670, 0, "%s", "model") )
        __debugbreak();
    if ( collMapIndex < 0 || collMapIndex >= model->numCollmaps )
        return 0;
    else
        return model->collmaps[collMapIndex].geomList;
}

PhysGeomList *__cdecl XModelGetCollmapForBoneIndex(const XModel *model, int boneIndex)
{
    int collmapIndex; // [esp+0h] [ebp-4h]

    if ( !model && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp", 687, 0, "%s", "model") )
        __debugbreak();
    if ( (boneIndex < 0 || boneIndex >= model->numBones)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\xanim\\xmodel_utils.cpp",
                    688,
                    0,
                    "%s",
                    "boneIndex >= 0 && boneIndex < model->numBones") )
    {
        __debugbreak();
    }
    collmapIndex = model->localBoneInfo[boneIndex].collmap;
    if ( collmapIndex >= model->numCollmaps )
        return 0;
    else
        return model->collmaps[collmapIndex].geomList;
}
