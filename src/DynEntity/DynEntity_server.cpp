#include "DynEntity_server.h"
#include "DynEntity_load_obj.h"
#include "DynEntity_coll.h"
#include "DynEntity_client.h"
#include <qcommon/cm_load.h>

#include <algorithm>
#include <ragdoll/ragdoll_update.h>

const dvar_t *dynEnt_damageRadiusScale;

void __cdecl DynEntSv_RegisterDvars()
{
    dynEnt_damageRadiusScale = _Dvar_RegisterFloat(
                                                             "dynEnt_damageRadiusScale",
                                                             1.0,
                                                             0.0,
                                                             1.0,
                                                             0x80u,
                                                             "Scale applied to radius for radius damage");
}

void __cdecl DynEntSv_InitEntities()
{
    DynEntityDrawType drawType; // [esp+0h] [ebp-1Ch]
    const DynEntityDef *dynEntDef; // [esp+8h] [ebp-14h]
    DynEntityServer *dynEntServer; // [esp+Ch] [ebp-10h]
    unsigned __int16 dynEntCount; // [esp+10h] [ebp-Ch]
    unsigned __int16 dynEntId; // [esp+14h] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+18h] [ebp-4h]

    for ( drawType = DYNENT_DRAW_MODEL; (unsigned int)drawType < DYNENT_DRAW_COUNT; ++drawType )
    {
        DynEnt_ClearCollWorld((DynEntityCollType)(drawType + 2));
        dynEntCount = DynEnt_GetEntityCount((DynEntityCollType)(drawType + 2));
        for ( dynEntId = 0; dynEntId < (int)dynEntCount; ++dynEntId )
        {
            dynEntDef = DynEnt_GetEntityDef(dynEntId, drawType);
            dynEntServer = DynEnt_GetServerEntity(dynEntId, drawType);
            dynEntColl = DynEnt_GetEntityColl((DynEntityCollType)(drawType + 2), dynEntId);
            dynEntServer->flags = 1;
            dynEntServer->health = dynEntDef->health;
            dynEntColl->nextEntInSector = 0;
            dynEntColl->sector = 0;
            if ( drawType )
                DynEntSv_LinkBrush(dynEntId);
            else
                DynEntSv_LinkModel(dynEntId);
        }
    }
}

void __cdecl DynEntSv_LinkModel(unsigned __int16 dynEntId)
{
    int v1; // [esp+4h] [ebp-200h]
    int v2; // [esp+8h] [ebp-1FCh]
    int v3; // [esp+Ch] [ebp-1F8h]
    int v4; // [esp+14h] [ebp-1F0h]
    int v5; // [esp+18h] [ebp-1ECh]
    int v6; // [esp+1Ch] [ebp-1E8h]
    int v7; // [esp+24h] [ebp-1E0h]
    int v8; // [esp+28h] [ebp-1DCh]
    int v9; // [esp+2Ch] [ebp-1D8h]
    float v10; // [esp+134h] [ebp-D0h]
    float v11; // [esp+138h] [ebp-CCh]
    float v12; // [esp+13Ch] [ebp-C8h]
    float modelBoundsVec3[2][3]; // [esp+160h] [ebp-A4h] BYREF
    float4 worldBoundsFloat4[2]; // [esp+178h] [ebp-8Ch]
    XModel *model; // [esp+198h] [ebp-6Ch]
    float radius; // [esp+19Ch] [ebp-68h]
    const DynEntityDef *dynEntDef; // [esp+1A0h] [ebp-64h]
    float modelAxis[3][3]; // [esp+1A4h] [ebp-60h] BYREF
    float worldBoundsVec3[2][3]; // [esp+1C8h] [ebp-3Ch] BYREF
    float4 modelBoundsFloat4[2]; // [esp+1E0h] [ebp-24h]

    dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_MODEL);
    model = dynEntDef->xModel;
    if ( !model
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 67, 0, "%s", "model") )
    {
        __debugbreak();
    }
    radius = XModelGetRadius(model);
    XModelGetBounds(model, modelBoundsVec3[0], modelBoundsVec3[1]);
    *(_QWORD *)modelBoundsFloat4[0].v = *(_QWORD *)&modelBoundsVec3[0][0];
    *(_QWORD *)&modelBoundsFloat4[0].unitVec[2].packed = __PAIR64__(
                                                                                                                 0,
                                                                                                                 LODWORD(modelBoundsVec3[0][2]));
    modelBoundsFloat4[1].v[0] = modelBoundsVec3[1][0];
    modelBoundsFloat4[1].v[1] = modelBoundsVec3[1][1];
    modelBoundsFloat4[1].v[2] = modelBoundsVec3[1][2];
    modelBoundsFloat4[1].u[3] = 0;
    UnitQuatToAxis(dynEntDef->pose.quat, modelAxis);
    v10 = dynEntDef->pose.origin[0];
    v11 = dynEntDef->pose.origin[1];
    v12 = dynEntDef->pose.origin[2];
    if ( modelAxis[0][0] >= 0.0 )
        v9 = 0;
    else
        v9 = -1;
    if ( modelAxis[0][1] >= 0.0 )
        v8 = 0;
    else
        v8 = -1;
    if ( modelAxis[0][2] >= 0.0 )
        v7 = 0;
    else
        v7 = -1;
    if ( modelAxis[1][0] >= 0.0 )
        v6 = 0;
    else
        v6 = -1;
    if ( modelAxis[1][1] >= 0.0 )
        v5 = 0;
    else
        v5 = -1;
    if ( modelAxis[1][2] >= 0.0 )
        v4 = 0;
    else
        v4 = -1;
    if ( modelAxis[2][0] >= 0.0 )
        v3 = 0;
    else
        v3 = -1;
    if ( modelAxis[2][1] >= 0.0 )
        v2 = 0;
    else
        v2 = -1;
    if ( modelAxis[2][2] >= 0.0 )
        v1 = 0;
    else
        v1 = -1;
    worldBoundsFloat4[0].v[0] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v9 | modelBoundsFloat4[0].u[0] & ~v9)
                                                                        * modelAxis[0][0])
                                                        + v10;
    worldBoundsFloat4[0].v[1] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v8 | modelBoundsFloat4[0].u[0] & ~v8)
                                                                        * modelAxis[0][1])
                                                        + v11;
    worldBoundsFloat4[0].v[2] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[0] & v7 | modelBoundsFloat4[0].u[0] & ~v7)
                                                                        * modelAxis[0][2])
                                                        + v12;
    worldBoundsFloat4[0].v[3] = (float)(modelBoundsFloat4[0].v[0] * 0.0) + 0.0;
    worldBoundsFloat4[0].v[0] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v6 | modelBoundsFloat4[0].u[1] & ~v6)
                                                                        * modelAxis[1][0])
                                                        + worldBoundsFloat4[0].v[0];
    worldBoundsFloat4[0].v[1] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v5 | modelBoundsFloat4[0].u[1] & ~v5)
                                                                        * modelAxis[1][1])
                                                        + worldBoundsFloat4[0].v[1];
    worldBoundsFloat4[0].v[2] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[1] & v4 | modelBoundsFloat4[0].u[1] & ~v4)
                                                                        * modelAxis[1][2])
                                                        + worldBoundsFloat4[0].v[2];
    worldBoundsFloat4[0].v[3] = (float)(modelBoundsFloat4[0].v[1] * 0.0) + worldBoundsFloat4[0].v[3];
    worldBoundsFloat4[0].v[0] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v3 | modelBoundsFloat4[0].u[2] & ~v3)
                                                                        * modelAxis[2][0])
                                                        + worldBoundsFloat4[0].v[0];
    worldBoundsFloat4[0].v[1] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v2 | modelBoundsFloat4[0].u[2] & ~v2)
                                                                        * modelAxis[2][1])
                                                        + worldBoundsFloat4[0].v[1];
    worldBoundsFloat4[0].v[2] = (float)(COERCE_FLOAT(modelBoundsFloat4[1].u[2] & v1 | modelBoundsFloat4[0].u[2] & ~v1)
                                                                        * modelAxis[2][2])
                                                        + worldBoundsFloat4[0].v[2];
    worldBoundsFloat4[0].v[3] = (float)(modelBoundsFloat4[0].v[2] * 0.0) + worldBoundsFloat4[0].v[3];
    worldBoundsFloat4[1].v[0] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v9 | modelBoundsFloat4[1].u[0] & ~v9)
                                                                        * modelAxis[0][0])
                                                        + v10;
    worldBoundsFloat4[1].v[1] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v8 | modelBoundsFloat4[1].u[0] & ~v8)
                                                                        * modelAxis[0][1])
                                                        + v11;
    worldBoundsFloat4[1].v[2] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[0] & v7 | modelBoundsFloat4[1].u[0] & ~v7)
                                                                        * modelAxis[0][2])
                                                        + v12;
    worldBoundsFloat4[1].v[3] = (float)(modelBoundsFloat4[1].v[0] * 0.0) + 0.0;
    worldBoundsFloat4[1].v[0] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v6 | modelBoundsFloat4[1].u[1] & ~v6)
                                                                        * modelAxis[1][0])
                                                        + worldBoundsFloat4[1].v[0];
    worldBoundsFloat4[1].v[1] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v5 | modelBoundsFloat4[1].u[1] & ~v5)
                                                                        * modelAxis[1][1])
                                                        + worldBoundsFloat4[1].v[1];
    worldBoundsFloat4[1].v[2] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[1] & v4 | modelBoundsFloat4[1].u[1] & ~v4)
                                                                        * modelAxis[1][2])
                                                        + worldBoundsFloat4[1].v[2];
    worldBoundsFloat4[1].v[3] = (float)(modelBoundsFloat4[1].v[1] * 0.0) + worldBoundsFloat4[1].v[3];
    worldBoundsFloat4[1].v[0] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v3 | modelBoundsFloat4[1].u[2] & ~v3)
                                                                        * modelAxis[2][0])
                                                        + worldBoundsFloat4[1].v[0];
    worldBoundsFloat4[1].v[1] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v2 | modelBoundsFloat4[1].u[2] & ~v2)
                                                                        * modelAxis[2][1])
                                                        + worldBoundsFloat4[1].v[1];
    worldBoundsFloat4[1].v[2] = (float)(COERCE_FLOAT(modelBoundsFloat4[0].u[2] & v1 | modelBoundsFloat4[1].u[2] & ~v1)
                                                                        * modelAxis[2][2])
                                                        + worldBoundsFloat4[1].v[2];
    worldBoundsFloat4[1].v[3] = (float)(modelBoundsFloat4[1].v[2] * 0.0) + worldBoundsFloat4[1].v[3];
    *(_QWORD *)&worldBoundsVec3[0][0] = *(_QWORD *)worldBoundsFloat4[0].v;
    LODWORD(worldBoundsVec3[0][2]) = worldBoundsFloat4[0].u[2];
    worldBoundsVec3[1][0] = worldBoundsFloat4[1].v[0];
    worldBoundsVec3[1][1] = worldBoundsFloat4[1].v[1];
    worldBoundsVec3[1][2] = worldBoundsFloat4[1].v[2];
    DynEnt_LinkEntity(DYNENT_COLL_SERVER_FIRST, dynEntId, worldBoundsVec3[0], worldBoundsVec3[1]);
}

void __cdecl DynEntSv_LinkBrush(unsigned __int16 dynEntId)
{
    int v1; // [esp+4h] [ebp-1D4h]
    int v2; // [esp+8h] [ebp-1D0h]
    int v3; // [esp+Ch] [ebp-1CCh]
    int v4; // [esp+14h] [ebp-1C4h]
    int v5; // [esp+18h] [ebp-1C0h]
    int v6; // [esp+1Ch] [ebp-1BCh]
    int v7; // [esp+24h] [ebp-1B4h]
    int v8; // [esp+28h] [ebp-1B0h]
    int v9; // [esp+2Ch] [ebp-1ACh]
    float v10; // [esp+130h] [ebp-A8h]
    float v11; // [esp+134h] [ebp-A4h]
    float v12; // [esp+138h] [ebp-A0h]
    __int64 rotatedBounds; // [esp+154h] [ebp-84h]
    __int64 rotatedBounds_16; // [esp+164h] [ebp-74h]
    float absMaxs[3]; // [esp+174h] [ebp-64h] BYREF
    const DynEntityDef *dynEntDef; // [esp+180h] [ebp-58h]
    float4 bounds[2]; // [esp+184h] [ebp-54h]
    float axis[3][3]; // [esp+1A8h] [ebp-30h] BYREF
    float absMins[3]; // [esp+1CCh] [ebp-Ch] BYREF
    unsigned int vars0; // [esp+1D8h] [ebp+0h]

    dynEntDef = DynEnt_GetEntityDef(dynEntId, DYNENT_DRAW_BRUSH);
    if ( dynEntDef->xModel
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    96,
                    0,
                    "%s",
                    "!dynEntDef->xModel") )
    {
        __debugbreak();
    }
    CM_ModelBounds(dynEntDef->brushModel, absMins, absMaxs);

    //*(_QWORD *)bounds[0].v = *(_QWORD *)absMins;
    //*(_QWORD *)&bounds[0].unitVec[2].packed = __PAIR64__(vars0, LODWORD(absMins[2]));

    bounds[0].v[0] = absMins[0];
    bounds[0].v[1] = absMins[1];
    bounds[0].v[2] = absMins[2];

    bounds[1].v[0] = absMaxs[0];
    bounds[1].v[1] = absMaxs[1];
    bounds[1].v[2] = absMaxs[2];

    //*(_QWORD *)bounds[1].v = *(_QWORD *)absMaxs;
    //*(_QWORD *)&bounds[1].unitVec[2].packed = __PAIR64__((unsigned int)dynEntDef, LODWORD(absMaxs[2]));

    UnitQuatToAxis(dynEntDef->pose.quat, axis);

    v10 = dynEntDef->pose.origin[0];
    v11 = dynEntDef->pose.origin[1];
    v12 = dynEntDef->pose.origin[2];

    if ( axis[0][0] >= 0.0 )
        v9 = 0;
    else
        v9 = -1;
    if ( axis[0][1] >= 0.0 )
        v8 = 0;
    else
        v8 = -1;
    if ( axis[0][2] >= 0.0 )
        v7 = 0;
    else
        v7 = -1;
    if ( axis[1][0] >= 0.0 )
        v6 = 0;
    else
        v6 = -1;
    if ( axis[1][1] >= 0.0 )
        v5 = 0;
    else
        v5 = -1;
    if ( axis[1][2] >= 0.0 )
        v4 = 0;
    else
        v4 = -1;
    if ( axis[2][0] >= 0.0 )
        v3 = 0;
    else
        v3 = -1;
    if ( axis[2][1] >= 0.0 )
        v2 = 0;
    else
        v2 = -1;
    if ( axis[2][2] >= 0.0 )
        v1 = 0;
    else
        v1 = -1;

    *(float *)&rotatedBounds = (float)(COERCE_FLOAT(bounds[1].u[2] & v3 | bounds[0].u[2] & ~v3) * axis[2][0])
                                                     + (float)((float)(COERCE_FLOAT(bounds[1].u[1] & v6 | bounds[0].u[1] & ~v6) * axis[1][0])
                                                                     + (float)((float)(COERCE_FLOAT(bounds[1].u[0] & v9 | bounds[0].u[0] & ~v9)
                                                                                                     * axis[0][0])
                                                                                     + v10));
    *((float *)&rotatedBounds + 1) = (float)(COERCE_FLOAT(bounds[1].u[2] & v2 | bounds[0].u[2] & ~v2) * axis[2][1])
                                                                 + (float)((float)(COERCE_FLOAT(bounds[1].u[1] & v5 | bounds[0].u[1] & ~v5) * axis[1][1])
                                                                                 + (float)((float)(COERCE_FLOAT(bounds[1].u[0] & v8 | bounds[0].u[0] & ~v8)
                                                                                                                 * axis[0][1])
                                                                                                 + v11));
    *(float *)&rotatedBounds_16 = (float)(COERCE_FLOAT(bounds[0].u[2] & v3 | bounds[1].u[2] & ~v3) * axis[2][0])
                                                            + (float)((float)(COERCE_FLOAT(bounds[0].u[1] & v6 | bounds[1].u[1] & ~v6) * axis[1][0])
                                                                            + (float)((float)(COERCE_FLOAT(bounds[0].u[0] & v9 | bounds[1].u[0] & ~v9)
                                                                                                            * axis[0][0])
                                                                                            + v10));
    *((float *)&rotatedBounds_16 + 1) = (float)(COERCE_FLOAT(bounds[0].u[2] & v2 | bounds[1].u[2] & ~v2) * axis[2][1])
                                                                        + (float)((float)(COERCE_FLOAT(bounds[0].u[1] & v5 | bounds[1].u[1] & ~v5)
                                                                                                        * axis[1][1])
                                                                                        + (float)((float)(COERCE_FLOAT(bounds[0].u[0] & v8 | bounds[1].u[0] & ~v8)
                                                                                                                        * axis[0][1])
                                                                                                        + v11));
    *(_QWORD *)absMins = rotatedBounds;
    absMins[2] = (float)(COERCE_FLOAT(bounds[1].u[2] & v1 | bounds[0].u[2] & ~v1) * axis[2][2])
                         + (float)((float)(COERCE_FLOAT(bounds[1].u[1] & v4 | bounds[0].u[1] & ~v4) * axis[1][2])
                                         + (float)((float)(COERCE_FLOAT(bounds[1].u[0] & v7 | bounds[0].u[0] & ~v7) * axis[0][2]) + v12));
    *(_QWORD *)absMaxs = rotatedBounds_16;
    absMaxs[2] = (float)(COERCE_FLOAT(bounds[0].u[2] & v1 | bounds[1].u[2] & ~v1) * axis[2][2])
                         + (float)((float)(COERCE_FLOAT(bounds[0].u[1] & v4 | bounds[1].u[1] & ~v4) * axis[1][2])
                                         + (float)((float)(COERCE_FLOAT(bounds[0].u[0] & v7 | bounds[1].u[0] & ~v7) * axis[0][2]) + v12));
    DynEnt_LinkEntity(DYNENT_COLL_SERVER_BRUSH, dynEntId, absMins, absMaxs);
}

void __cdecl DynEntSv_PointTrace(const pointtrace_t *clip, trace_t *results)
{
    _QWORD start[2]; // [esp+Ch] [ebp-20h] BYREF
    _QWORD end[2]; // [esp+1Ch] [ebp-10h] BYREF

    if ( !clip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 270, 0, "%s", "clip") )
    {
        __debugbreak();
    }
    if ( !results
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 271, 0, "%s", "results") )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    272,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if ( results->fraction > 0.0 )
    {
        if ( results->fraction > 1.0 )
            results->fraction = 1.0f;
        start[0] = *(_QWORD *)clip->extents.start.vec.v;
        LODWORD(start[1]) = clip->extents.start.vec.u[2];
        end[0] = *(_QWORD *)clip->extents.end.vec.v;
        LODWORD(end[1]) = clip->extents.end.vec.u[2];
        HIDWORD(start[1]) = 0;
        HIDWORD(end[1]) = LODWORD(results->fraction);
        DynEntSv_PointTrace_r(DYNENT_DRAW_BRUSH, clip, 1u, (float *)start, (float *)end, results);
        if ( results->fraction != 0.0 )
            DynEntSv_PointTrace_r(DYNENT_DRAW_MODEL, clip, 1u, (float *)start, (float *)end, results);
    }
}

void __cdecl DynEntSv_PointTrace_r(
                DynEntityDrawType drawType,
                const pointtrace_t *clip,
                unsigned int sectorIndex,
                float *p1,
                float *p2,
                trace_t *results)
{
    float v6; // [esp+10h] [ebp-50h]
    DynEntityCollSector *sector; // [esp+18h] [ebp-48h]
    unsigned __int16 listIndex; // [esp+1Ch] [ebp-44h]
    float t1; // [esp+20h] [ebp-40h]
    float frac; // [esp+24h] [ebp-3Ch]
    DynEntityCollType collType; // [esp+28h] [ebp-38h]
    const DynEntityDef *dynEntDef; // [esp+2Ch] [ebp-34h]
    float t2; // [esp+34h] [ebp-2Ch]
    int contentmask; // [esp+38h] [ebp-28h]
    DynEntityColl *dynEntColl; // [esp+3Ch] [ebp-24h]
    float p[4]; // [esp+40h] [ebp-20h] BYREF
    float mid[4]; // [esp+50h] [ebp-10h] BYREF

    if ( !clip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 186, 0, "%s", "clip") )
    {
        __debugbreak();
    }
    if ( !p1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 187, 0, "%s", "p1") )
    {
        __debugbreak();
    }
    if ( !p2
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 188, 0, "%s", "p2") )
    {
        __debugbreak();
    }
    if ( !results
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 189, 0, "%s", "results") )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    190,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    collType = (DynEntityCollType)(drawType + 2);
    contentmask = clip->contentmask;
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while ( sectorIndex )
    {
        sector = DynEnt_GetCollSector(collType, sectorIndex);
        if ( (clip->contentmask & sector->contents) == 0 )
            break;
        for ( listIndex = sector->entListHead; listIndex; listIndex = dynEntColl->nextEntInSector )
        {
            dynEntColl = DynEnt_GetEntityColl(collType, listIndex - 1);
            DynEnt_GetServerEntity(listIndex - 1, drawType);
            dynEntDef = DynEnt_GetEntityDef(listIndex - 1, drawType);
            if ( (contentmask & DynEnt_GetContents(dynEntDef)) != 0 )
            {
                if ( drawType )
                {
                    if ( drawType != DYNENT_DRAW_BRUSH
                        && !Assert_MyHandler(
                                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                                    223,
                                    0,
                                    "%s\n\t(drawType) = %i",
                                    "(drawType == DYNENT_DRAW_BRUSH)",
                                    drawType) )
                    {
                        __debugbreak();
                    }
                    DynEnt_PointTraceToBrush(dynEntDef, &dynEntDef->pose, clip, results);
                }
                else
                {
                    DynEnt_PointTraceToModel(dynEntDef, &dynEntDef->pose, clip, results);
                }
                if ( results->fraction == 0.0 )
                    return;
            }
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        if ( (float)(t1 * t2) < 0.0 )
        {
            if ( p[3] >= results->fraction )
                return;
            frac = t1 / (float)(t1 - t2);
            if ( frac < 0.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                            244,
                            0,
                            "%s",
                            "frac >= 0.0f") )
            {
                __debugbreak();
            }
            if ( frac > 1.0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                            245,
                            0,
                            "%s",
                            "frac <= 1.0f") )
            {
                __debugbreak();
            }
            mid[0] = (float)((float)(*p2 - p[0]) * frac) + p[0];
            mid[1] = (float)((float)(p2[1] - p[1]) * frac) + p[1];
            mid[2] = (float)((float)(p2[2] - p[2]) * frac) + p[2];
            mid[3] = (float)((float)(p2[3] - p[3]) * frac) + p[3];
            DynEntSv_PointTrace_r(drawType, clip, sector->tree.child[t2 >= 0.0], p, mid, results);
            if ( results->fraction == 0.0 )
                return;
            sectorIndex = sector->tree.child[t2 < 0.0];
            p[0] = mid[0];
            p[1] = mid[1];
            p[2] = mid[2];
            p[3] = mid[3];
        }
        else
        {
            if ( (float)(t2 - t1) < 0.0 )
                v6 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v6 = p[sector->tree.axis] - sector->tree.dist;
            sectorIndex = sector->tree.child[v6 < 0.0];
        }
    }
}

void __cdecl DynEntSv_PointTrace(
                unsigned __int16 id,
                DynEntityDrawType drawType,
                const pointtrace_t *clip,
                trace_t *results)
{
    const DynEntityDef *dynEntDef; // [esp+0h] [ebp-4h]

    dynEntDef = DynEnt_GetEntityDef(id, drawType);
    if ( drawType )
    {
        if ( drawType != DYNENT_DRAW_BRUSH
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                        306,
                        0,
                        "%s\n\t(drawType) = %i",
                        "(drawType == DYNENT_DRAW_BRUSH)",
                        drawType) )
        {
            __debugbreak();
        }
        DynEnt_PointTraceToBrush(dynEntDef, &dynEntDef->pose, clip, results);
    }
    else
    {
        DynEnt_PointTraceToModel(dynEntDef, &dynEntDef->pose, clip, results);
    }
}

void __cdecl DynEntSv_ClipMoveTrace(const moveclip_t *clip, trace_t *results)
{
    _QWORD start[2]; // [esp+14h] [ebp-20h] BYREF
    _QWORD end[2]; // [esp+24h] [ebp-10h] BYREF

    if ( !clip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 432, 0, "%s", "clip") )
    {
        __debugbreak();
    }
    if ( !results
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 433, 0, "%s", "results") )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    434,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    if (results->fraction != 0.0
        && (DynEnt_GetEntityProps(DYNENT_TYPE_CLUTTER)->clipMove
            || clip->contentmask == 0x280E833
            || clip->contentmask == 0x211))
    {
        start[0] = *(_QWORD *)clip->extents.start.vec.v;
        LODWORD(start[1]) = clip->extents.start.vec.u[2];
        end[0] = *(_QWORD *)clip->extents.end.vec.v;
        LODWORD(end[1]) = clip->extents.end.vec.u[2];
        HIDWORD(start[1]) = 0;
        HIDWORD(end[1]) = LODWORD(results->fraction);
        DynEntSv_ClipMoveTrace_r(clip, 1u, (float *)start, (float *)end, results);
    }
}

void __cdecl DynEntSv_ClipMoveTrace_r(
                const moveclip_t *clip,
                unsigned int sectorIndex,
                float *p1,
                float *p2,
                trace_t *results)
{
    float v5; // [esp+8h] [ebp-74h]
    float v6; // [esp+Ch] [ebp-70h]
    float v7; // [esp+14h] [ebp-68h]
    float v8; // [esp+18h] [ebp-64h]
    float v9; // [esp+1Ch] [ebp-60h]
    bool side; // [esp+20h] [ebp-5Ch]
    float diff; // [esp+24h] [ebp-58h]
    DynEntityCollSector *sector; // [esp+2Ch] [ebp-50h]
    unsigned __int16 listIndex; // [esp+30h] [ebp-4Ch]
    float t1; // [esp+34h] [ebp-48h]
    float frac; // [esp+38h] [ebp-44h]
    float offset; // [esp+44h] [ebp-38h]
    float t2; // [esp+48h] [ebp-34h]
    float frac2; // [esp+4Ch] [ebp-30h]
    float absDiff; // [esp+50h] [ebp-2Ch]
    DynEntityColl *dynEntColl; // [esp+58h] [ebp-24h]
    float p[4]; // [esp+5Ch] [ebp-20h] BYREF
    float mid[4]; // [esp+6Ch] [ebp-10h] BYREF

    if ( !clip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 331, 0, "%s", "clip") )
    {
        __debugbreak();
    }
    if ( !p1
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 332, 0, "%s", "p1") )
    {
        __debugbreak();
    }
    if ( !p2
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 333, 0, "%s", "p2") )
    {
        __debugbreak();
    }
    if ( !results
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 334, 0, "%s", "results") )
    {
        __debugbreak();
    }
    if ( results->fraction > 1.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    335,
                    0,
                    "%s\n\t(results->fraction) = %g",
                    "(results->fraction <= 1.0f)",
                    results->fraction) )
    {
        __debugbreak();
    }
    p[0] = *p1;
    p[1] = p1[1];
    p[2] = p1[2];
    p[3] = p1[3];
    while ( sectorIndex )
    {
        sector = DynEnt_GetCollSector(DYNENT_COLL_SERVER_BRUSH, sectorIndex);
        if ( (clip->contentmask & sector->contents) == 0 )
            break;
        for ( listIndex = sector->entListHead; listIndex; listIndex = dynEntColl->nextEntInSector )
        {
            dynEntColl = DynEnt_GetEntityColl(DYNENT_COLL_SERVER_BRUSH, listIndex - 1);
            if ( (DynEnt_GetServerEntity(listIndex - 1, DYNENT_DRAW_BRUSH)->flags & 1) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                            350,
                            0,
                            "%s",
                            "dynEntServer->flags & DYNENT_SV_ACTIVE") )
            {
                __debugbreak();
            }
            DynEnt_GetEntityDef(listIndex - 1, DYNENT_DRAW_BRUSH);
            results->fraction = 1.0f;
            if ( results->fraction == 0.0 )
                return;
        }
        t1 = p[sector->tree.axis] - sector->tree.dist;
        t2 = p2[sector->tree.axis] - sector->tree.dist;
        offset = clip->outerSize[sector->tree.axis];
        if ( (float)(t2 - t1) < 0.0 )
            v9 = p2[sector->tree.axis] - sector->tree.dist;
        else
            v9 = p[sector->tree.axis] - sector->tree.dist;
        if ( v9 < offset )
        {
            if ( (float)(t1 - t2) < 0.0 )
                v8 = p2[sector->tree.axis] - sector->tree.dist;
            else
                v8 = p[sector->tree.axis] - sector->tree.dist;
            //if ( COERCE_FLOAT(LODWORD(offset) ^ _mask__NegFloat_) < v8 )
            if ( -offset < v8 )
            {
                if ( p[3] >= results->fraction )
                    return;
                diff = t2 - t1;
                if ( (float)(t2 - t1) == 0.0 )
                {
                    frac = 1.0f;
                    frac2 = 0.0f;
                    side = 0;
                }
                else
                {
                    absDiff = fabs(diff);
                    if ( diff < 0.0 )
                        v7 = p[sector->tree.axis] - sector->tree.dist;
                    else
                        v7 = -t1;
                    frac = (float)(v7 + offset) * (float)(1.0 / absDiff);
                    frac2 = (float)(v7 - offset) * (float)(1.0 / absDiff);
                    side = diff >= 0.0;
                }
                if ( frac < 0.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                                399,
                                0,
                                "%s",
                                "frac >= 0.0f") )
                {
                    __debugbreak();
                }
                v6 = (float)(1.0 - frac) < 0.0 ? 1.0f : frac;
                mid[0] = (float)((float)(*p2 - p[0]) * v6) + p[0];
                mid[1] = (float)((float)(p2[1] - p[1]) * v6) + p[1];
                mid[2] = (float)((float)(p2[2] - p[2]) * v6) + p[2];
                mid[3] = (float)((float)(p2[3] - p[3]) * v6) + p[3];
                DynEntSv_ClipMoveTrace_r(clip, sector->tree.child[side], p, mid, results);
                if ( results->fraction == 0.0 )
                    return;
                if ( frac2 > 1.0
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                                411,
                                0,
                                "%s",
                                "frac2 <= 1.0f") )
                {
                    __debugbreak();
                }
                if ( (float)(frac2 - 0.0) < 0.0 )
                    v5 = 0.0f;
                else
                    v5 = frac2;
                p[0] = (float)((float)(*p2 - p[0]) * v5) + p[0];
                p[1] = (float)((float)(p2[1] - p[1]) * v5) + p[1];
                p[2] = (float)((float)(p2[2] - p[2]) * v5) + p[2];
                p[3] = (float)((float)(p2[3] - p[3]) * v5) + p[3];
                sectorIndex = sector->tree.child[1 - side];
            }
            else
            {
                sectorIndex = sector->tree.child[1];
            }
        }
        else
        {
            sectorIndex = sector->tree.child[0];
        }
    }
}

unsigned __int16 __cdecl DynEntSv_AreaEntities(
                DynEntityDrawType drawType,
                const float *mins,
                const float *maxs,
                int contentMask,
                unsigned __int16 dynEntMaxCount,
                unsigned __int16 *dynEntList)
{
    DynEntityAreaParms areaParms; // [esp+0h] [ebp-14h] BYREF

    if ( !mins
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 530, 0, "%s", "mins") )
    {
        __debugbreak();
    }
    if ( !maxs
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 531, 0, "%s", "maxs") )
    {
        __debugbreak();
    }
    if ( !dynEntMaxCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    532,
                    0,
                    "%s\n\t(dynEntMaxCount) = %i",
                    "(dynEntMaxCount > 0)",
                    0) )
    {
        __debugbreak();
    }
    if ( !dynEntList
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    533,
                    0,
                    "%s",
                    "dynEntList") )
    {
        __debugbreak();
    }
    areaParms.mins = mins;
    areaParms.maxs = maxs;
    areaParms.contentMask = contentMask;
    areaParms.list = dynEntList;
    areaParms.maxCount = dynEntMaxCount;
    areaParms.count = 0;
    DynEntSv_AreaEntities_r(drawType, 1u, &areaParms);
    return areaParms.count;
}

void __cdecl DynEntSv_AreaEntities_r(
                DynEntityDrawType drawType,
                unsigned int sectorIndex,
                DynEntityAreaParms *areaParms)
{
    DynEntityCollSector *sector; // [esp+0h] [ebp-1Ch]
    unsigned __int16 listIndex; // [esp+4h] [ebp-18h]
    DynEntityCollType collType; // [esp+8h] [ebp-14h]
    const DynEntityDef *dynEntDef; // [esp+Ch] [ebp-10h]
    DynEntityColl *dynEntColl; // [esp+14h] [ebp-8h]
    unsigned int nextSectorIndex; // [esp+18h] [ebp-4h]

    if ( !areaParms
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    469,
                    0,
                    "%s",
                    "areaParms") )
    {
        __debugbreak();
    }
    collType = (DynEntityCollType)(drawType + 2);
    while ( sectorIndex )
    {
        sector = DynEnt_GetCollSector(collType, sectorIndex);
        if ( (areaParms->contentMask & sector->contents) == 0 )
            break;
        for ( listIndex = sector->entListHead; listIndex; listIndex = dynEntColl->nextEntInSector )
        {
            dynEntColl = DynEnt_GetEntityColl(collType, listIndex - 1);
            if ( (DynEnt_GetServerEntity(listIndex - 1, drawType)->flags & 1) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                            484,
                            0,
                            "%s",
                            "dynEntServer->flags & DYNENT_SV_ACTIVE") )
            {
                __debugbreak();
            }
            dynEntDef = DynEnt_GetEntityDef(listIndex - 1, drawType);
            if ( DynEnt_EntityInArea(dynEntDef, dynEntColl, areaParms->mins, areaParms->maxs, areaParms->contentMask) )
            {
                if ( areaParms->count == areaParms->maxCount )
                    return;
                areaParms->list[areaParms->count++] = listIndex - 1;
            }
        }
        if ( areaParms->maxs[sector->tree.axis] <= sector->tree.dist )
        {
            if ( areaParms->mins[sector->tree.axis] >= sector->tree.dist )
                return;
            sectorIndex = sector->tree.child[1];
        }
        else if ( sector->tree.dist <= areaParms->mins[sector->tree.axis] )
        {
            sectorIndex = sector->tree.child[0];
        }
        else
        {
            nextSectorIndex = sector->tree.child[1];
            DynEntSv_AreaEntities_r(drawType, sector->tree.child[0], areaParms);
            sectorIndex = nextSectorIndex;
        }
    }
}

void __cdecl DynEntSv_Damage(
                unsigned __int16 dynEntId,
                DynEntityDrawType drawType,
                const float *hitPos,
                const float *hitDir,
                int damage,
                bool explosive)
{
    const DynEntityDef *dynEntDef; // [esp+4h] [ebp-8h]
    DynEntityServer *dynEntServer; // [esp+8h] [ebp-4h]

    if ( dynEntId == 0xFFFF
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    556,
                    0,
                    "%s",
                    "dynEntId != DYNENT_INVALID_ID") )
    {
        __debugbreak();
    }
    if ( !hitPos
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 557, 0, "%s", "hitPos") )
    {
        __debugbreak();
    }
    if ( !hitDir
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 558, 0, "%s", "hitDir") )
    {
        __debugbreak();
    }
    if ( damage < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    559,
                    0,
                    "%s\n\t(damage) = %i",
                    "((damage >= 0))",
                    damage) )
    {
        __debugbreak();
    }
    dynEntDef = DynEnt_GetEntityDef(dynEntId, drawType);
    dynEntServer = DynEnt_GetServerEntity(dynEntId, drawType);
    if ( (!explosive || (dynEntDef->flags & 8) == 0) && (dynEntDef->flags & 4) == 0 )
    {
        dynEntServer->health -= damage;
        if ( dynEntDef->type != DYNENT_TYPE_DESTRUCT || dynEntServer->health <= 0 )
        {
            if ( (dynEntServer->flags & 1) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                            575,
                            0,
                            "%s",
                            "dynEntServer->flags & DYNENT_SV_ACTIVE") )
            {
                __debugbreak();
            }
            dynEntServer->flags &= ~1u;
            DynEnt_UnlinkEntity((DynEntityCollType)(drawType + 2), dynEntId);
        }
    }
}

void __cdecl DynEntSv_RadiusDamage(
                float *origin,
                float radius,
                float coneAngleCos,
                float *coneDirection,
                float innerDamage,
                float outerDamage)
{
    float v6; // [esp+1Ch] [ebp-2050h]
    float RadiusDistSqr; // [esp+20h] [ebp-204Ch]
    float v8; // [esp+24h] [ebp-2048h]
    unsigned int ClosestEntities; // [esp+28h] [ebp-2044h]
    float radiusMaxs[3]; // [esp+2Ch] [ebp-2040h] BYREF
    float v[3]; // [esp+38h] [ebp-2034h] BYREF
    DynEntityDrawType drawType; // [esp+44h] [ebp-2028h]
    float v13; // [esp+48h] [ebp-2024h]
    const DynEntityDef *dynEntDef; // [esp+4Ch] [ebp-2020h]
    float v15; // [esp+50h] [ebp-201Ch]
    unsigned __int16 hitEnts[4096]; // [esp+54h] [ebp-2018h] BYREF
    unsigned int i; // [esp+2058h] [ebp-14h]
    float radiusMins[3]; // [esp+205Ch] [ebp-10h] BYREF
    DynEntityColl *dynEntColl; // [esp+2068h] [ebp-4h]
    float v20; // [esp+2078h] [ebp+Ch]

    if ( !origin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp", 653, 0, "%s", "origin") )
    {
        __debugbreak();
    }
    if ( radius < 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    654,
                    0,
                    "%s",
                    "radius >= 0.0f") )
    {
        __debugbreak();
    }
    v20 = radius * dynEnt_damageRadiusScale->current.value;
    if ( v20 != 0.0 )
    {
        v15 = v20 * v20;
        //LODWORD(v6) = COERCE_UNSIGNED_INT(1.4142135 * v20) ^ _mask__NegFloat_;
        v6 = -(1.4142135 * v20);
        radiusMins[0] = *origin + v6;
        radiusMins[1] = origin[1] + v6;
        radiusMins[2] = origin[2] + v6;
        radiusMaxs[0] = *origin + (float)(1.4142135 * v20);
        radiusMaxs[1] = origin[1] + (float)(1.4142135 * v20);
        radiusMaxs[2] = origin[2] + (float)(1.4142135 * v20);
        for ( drawType = DYNENT_DRAW_MODEL; (unsigned int)drawType < DYNENT_DRAW_COUNT; ++drawType )
        {
            ClosestEntities = DynEntSv_GetClosestEntities(drawType, radiusMins, radiusMaxs, origin, hitEnts);
            for ( i = 0; i < ClosestEntities; ++i )
            {
                dynEntDef = DynEnt_GetEntityDef(hitEnts[i], drawType);
                dynEntColl = DynEnt_GetEntityColl((DynEntityCollType)(drawType + 2), hitEnts[i]);
                RadiusDistSqr = DynEnt_GetRadiusDistSqr(dynEntColl, origin);
                if ( RadiusDistSqr < v15 && DynEnt_IsInCone(dynEntDef, &dynEntDef->pose, origin, coneAngleCos, coneDirection) )
                {
                    v13 = sqrtf(RadiusDistSqr);
                    v8 = (float)((float)(innerDamage - outerDamage) * (float)(1.0 - (float)(v13 / v20))) + outerDamage;
                    v[0] = dynEntDef->pose.origin[0] - *origin;
                    v[1] = dynEntDef->pose.origin[1] - origin[1];
                    v[2] = dynEntDef->pose.origin[2] - origin[2];
                    Vec3Normalize(v);
                    DynEntSv_Damage(hitEnts[i], drawType, dynEntDef->pose.origin, v, (int)v8, 1);
                }
            }
        }
    }
}

static bool __cdecl DynEntCl_CompareDynEntsForExplosion(const DynEntSortStruct &ent1, const DynEntSortStruct &ent2)
{
    return ent2.distSq > ent1.distSq;
}

unsigned int __cdecl DynEntSv_GetClosestEntities(
    DynEntityDrawType drawType,
    float *radiusMins,
    float *radiusMaxs,
    float *origin,
    unsigned __int16 *hitEnts)
{
    double RadiusDistSqr; // st7
    unsigned int unsignedInt_low; // [esp+110h] [ebp-8014h]
    DynEntSortStruct v8[4096]; // [esp+114h] [ebp-8010h] BYREF
    DynEntityServer *ServerEntity; // [esp+8118h] [ebp-Ch]
    unsigned int i; // [esp+811Ch] [ebp-8h]
    DynEntityColl *dynEntColl; // [esp+8120h] [ebp-4h]

    unsignedInt_low = DynEntSv_AreaEntities(drawType, radiusMins, radiusMaxs, 0x802011, 0x1000u, hitEnts);
    if (unsignedInt_low > dynEnt_explodeMaxEnts->current.integer)
    {
        for (i = 0; i < unsignedInt_low; ++i)
        {
            v8[i].id = hitEnts[i];
            ServerEntity = DynEnt_GetServerEntity(hitEnts[i], drawType);
            if ((ServerEntity->flags & 1) == 0
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                    619,
                    0,
                    "%s",
                    "dynEntServer->flags & DYNENT_SV_ACTIVE"))
            {
                __debugbreak();
            }
            dynEntColl = DynEnt_GetEntityColl((DynEntityCollType)(drawType + 2), hitEnts[i]);
            RadiusDistSqr = DynEnt_GetRadiusDistSqr(dynEntColl, origin);
            v8[i].distSq = RadiusDistSqr;
        }
        //std::_Sort<RagdollSortStruct *, int, bool(__cdecl *)(RagdollSortStruct const &, RagdollSortStruct const &)>(
        //    (MaterialMemory *)v8,
        //    (MaterialMemory *)&v8[unsignedInt_low],
        //    (int)(8 * unsignedInt_low) >> 3,
        //    (bool(__cdecl *)(const MaterialMemory *, const MaterialMemory *))DynEntCl_CompareDynEntsForExplosion);

        std::sort(&v8[0], &v8[unsignedInt_low], DynEntCl_CompareDynEntsForExplosion);
        unsignedInt_low = LOWORD(dynEnt_explodeMaxEnts->current.unsignedInt);
        if (unsignedInt_low != dynEnt_explodeMaxEnts->current.integer
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\DynEntity\\DynEntity_server.cpp",
                628,
                0,
                "%s",
                "hitCount == (uint)dynEnt_explodeMaxEnts->current.integer"))
        {
            __debugbreak();
        }
        for (i = 0; i < unsignedInt_low; ++i)
            hitEnts[i] = v8[i].id;
    }
    return unsignedInt_low;
}
