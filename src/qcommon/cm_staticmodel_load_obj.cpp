#include "cm_staticmodel_load_obj.h"
#include "com_bsp_load_obj.h"
#include "cm_load.h"
#include <universal/q_parse.h>
#include "com_profilemapload.h"
#include "cm_trace.h"
#include "cm_staticmodel.h"
#include <xanim/xmodel.h>
#include <universal/com_math_anglevectors.h>
#include <xanim/xmodel_load_obj.h>

void __cdecl CM_LoadStaticModels()
{
    char v0; // [esp+7h] [ebp-115h]
    char *v1; // [esp+Ch] [ebp-110h]
    char *v2; // [esp+10h] [ebp-10Ch]
    char v3; // [esp+17h] [ebp-105h]
    char *v4; // [esp+1Ch] [ebp-100h]
    char *v5; // [esp+20h] [ebp-FCh]
    unsigned int numStaticModels; // [esp+24h] [ebp-F8h]
    float origin[3]; // [esp+28h] [ebp-F4h] BYREF
    char modelName[64]; // [esp+34h] [ebp-E8h] BYREF
    float angles[3]; // [esp+74h] [ebp-A8h] BYREF
    int bMiscModel; // [esp+80h] [ebp-9Ch]
    char key[68]; // [esp+84h] [ebp-98h] BYREF
    const char *ptr; // [esp+C8h] [ebp-54h] BYREF
    const char *token; // [esp+CCh] [ebp-50h]
    float scale; // [esp+D0h] [ebp-4Ch]
    char value[68]; // [esp+D4h] [ebp-48h] BYREF

    ptr = Com_EntityString(0);
    if ( !ptr
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp", 106, 0, "%s", "ptr") )
    {
        __debugbreak();
    }
    cm.numStaticModels = 0;
    cm.staticModelList = 0;
    numStaticModels = 0;
    while ( 1 )
    {
        token = (const char *)Com_Parse(&ptr);
        if ( !ptr || *token != 123 )
            break;
        while ( 1 )
        {
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr )
                break;
            if ( *token == 125 )
                break;
            I_strncpyz(key, token, 64);
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr )
                break;
            I_strncpyz(value, token, 64);
            if ( !I_stricmp(key, "classname") && !I_stricmp(value, "misc_model") )
                ++numStaticModels;
        }
    }
    if ( numStaticModels )
    {
        cm.staticModelList = (cStaticModel_s *)CM_Hunk_Alloc(80 * numStaticModels, "CM_CreateStaticModel", 28);
        ptr = Com_EntityString(0);
        if ( !ptr
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp",
                        152,
                        0,
                        "%s",
                        "ptr") )
        {
            __debugbreak();
        }
        ProfLoad_Begin("Create static model collision");
        while ( 1 )
        {
            token = (const char *)Com_Parse(&ptr);
            if ( !ptr || *token != 123 )
                break;
            modelName[0] = 0;
            memset(origin, 0, sizeof(origin));
            memset(angles, 0, sizeof(angles));
            scale = 1.0f;
            bMiscModel = 0;
            while ( 1 )
            {
                token = (const char *)Com_Parse(&ptr);
                if ( !ptr )
                    break;
                if ( *token == 125 )
                    break;
                I_strncpyz(key, token, 64);
                token = (const char *)Com_Parse(&ptr);
                if ( !ptr )
                    break;
                I_strncpyz(value, token, 64);
                if ( I_stricmp(key, "classname") )
                {
                    if ( I_stricmp(key, "model") )
                    {
                        if ( I_stricmp(key, "origin") )
                        {
                            if ( I_stricmp(key, "angles") )
                            {
                                if ( !I_stricmp(key, "modelscale") )
                                    scale = atof(value);
                            }
                            else
                            {
                                sscanf(value, "%f %f %f", angles, &angles[1], &angles[2]);
                            }
                        }
                        else
                        {
                            sscanf(value, "%f %f %f", origin, &origin[1], &origin[2]);
                        }
                    }
                    else if ( Com_IsLegacyXModelName(value) )
                    {
                        v5 = &value[7];
                        v4 = modelName;
                        do
                        {
                            v3 = *v5;
                            *v4++ = *v5++;
                        }
                        while ( v3 );
                    }
                    else
                    {
                        v2 = value;
                        v1 = modelName;
                        do
                        {
                            v0 = *v2;
                            *v1++ = *v2++;
                        }
                        while ( v0 );
                    }
                }
                else if ( !I_stricmp(value, "misc_model") )
                {
                    bMiscModel = 1;
                }
            }
            if ( bMiscModel )
            {
                if ( cm.numStaticModels >= numStaticModels
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp",
                                213,
                                0,
                                "%s",
                                "cm.numStaticModels < numStaticModels") )
                {
                    __debugbreak();
                }
                if ( CM_CreateStaticModel(&cm.staticModelList[cm.numStaticModels], modelName, origin, angles, scale) )
                    ++cm.numStaticModels;
            }
        }
        ProfLoad_End();
    }
}

char __cdecl CM_CreateStaticModel(cStaticModel_s *staticModel, char *name, float *origin, float *angles, float scale)
{
    XModel *model; // [esp+34h] [ebp-4h]

    nanassertvec3(origin);
    nanassertvec3(angles);

    iassert(!IS_NAN(scale));
    
    if ( !name || !*name )
        Com_Error(ERR_DROP, "Missing model name at %.0f %.0f %.0f", *origin, origin[1], origin[2]);
    if ( scale == 0.0 )
        Com_Error(ERR_DROP, "Static model [%s] has scale of 0.0", name);
    model = CM_XModelPrecache(name);
    if ( !model )
        return 0;
    if ( !model->collSurfs )
        return 0;
    staticModel->xmodel = model;
    CM_InitStaticModel(staticModel, origin, angles, scale);
    return 1;
}

void __cdecl CM_InitStaticModel(cStaticModel_s *staticModel, float *origin, float *angles, float scale)
{
    float axis[3][3]; // [esp+64h] [ebp-24h] BYREF

    nanassertvec3(origin);
    nanassertvec3(angles);

    if ( (LODWORD(scale) & 0x7F800000) == 0x7F800000
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp",
                    22,
                    0,
                    "%s",
                    "!IS_NAN(scale)") )
    {
        __debugbreak();
    }
    staticModel->origin[0] = *origin;
    staticModel->origin[1] = origin[1];
    staticModel->origin[2] = origin[2];
    AnglesToAxis(angles, axis);
    MatrixTranspose(axis, staticModel->invScaledAxis);
    staticModel->invScaledAxis[0][0] = (float)(1.0 / scale) * staticModel->invScaledAxis[0][0];
    staticModel->invScaledAxis[0][1] = (float)(1.0 / scale) * staticModel->invScaledAxis[0][1];
    staticModel->invScaledAxis[0][2] = (float)(1.0 / scale) * staticModel->invScaledAxis[0][2];
    staticModel->invScaledAxis[1][0] = (float)(1.0 / scale) * staticModel->invScaledAxis[1][0];
    staticModel->invScaledAxis[1][1] = (float)(1.0 / scale) * staticModel->invScaledAxis[1][1];
    staticModel->invScaledAxis[1][2] = (float)(1.0 / scale) * staticModel->invScaledAxis[1][2];
    staticModel->invScaledAxis[2][0] = (float)(1.0 / scale) * staticModel->invScaledAxis[2][0];
    staticModel->invScaledAxis[2][1] = (float)(1.0 / scale) * staticModel->invScaledAxis[2][1];
    staticModel->invScaledAxis[2][2] = (float)(1.0 / scale) * staticModel->invScaledAxis[2][2];
    axis[0][0] = scale * axis[0][0];
    axis[0][1] = scale * axis[0][1];
    axis[0][2] = scale * axis[0][2];
    axis[1][0] = scale * axis[1][0];
    axis[1][1] = scale * axis[1][1];
    axis[1][2] = scale * axis[1][2];
    axis[2][0] = scale * axis[2][0];
    axis[2][1] = scale * axis[2][1];
    axis[2][2] = scale * axis[2][2];
    if ( XModelGetStaticBoundsExact(staticModel->xmodel, axis, staticModel->absmin, staticModel->absmax) )
    {
        staticModel->absmin[0] = staticModel->absmin[0] + *origin;
        staticModel->absmin[1] = staticModel->absmin[1] + origin[1];
        staticModel->absmin[2] = staticModel->absmin[2] + origin[2];
        staticModel->absmax[0] = staticModel->absmax[0] + *origin;
        staticModel->absmax[1] = staticModel->absmax[1] + origin[1];
        staticModel->absmax[2] = staticModel->absmax[2] + origin[2];
        if ( (float)(staticModel->absmax[0] - staticModel->absmin[0]) < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp",
                        48,
                        0,
                        "%s",
                        "staticModel->absmax[0] - staticModel->absmin[0] >= 0") )
        {
            __debugbreak();
        }
        if ( (float)(staticModel->absmax[1] - staticModel->absmin[1]) < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp",
                        49,
                        0,
                        "%s",
                        "staticModel->absmax[1] - staticModel->absmin[1] >= 0") )
        {
            __debugbreak();
        }
        if ( (float)(staticModel->absmax[2] - staticModel->absmin[2]) < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\qcommon\\cm_staticmodel_load_obj.cpp",
                        50,
                        0,
                        "%s",
                        "staticModel->absmax[2] - staticModel->absmin[2] >= 0") )
        {
            __debugbreak();
        }
    }
}

