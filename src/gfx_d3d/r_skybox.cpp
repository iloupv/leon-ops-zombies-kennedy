#include "r_skybox.h"
#include <xanim/xmodel.h>
#include "r_init.h"
#include "r_model.h"

XModel *g_skyboxModel;
unsigned __int16 myhandle;

void R_RegisterSkyboxModel()
{
    if ( rgp.world->skyBoxModel )
    {
        if ( *rgp.world->skyBoxModel )
            R_RegisterSkyboxModel((char*)rgp.world->skyBoxModel);
    }
}

void __cdecl R_ClearSkyboxModel()
{
    g_skyboxModel = 0;
}

void __cdecl R_AddSkyboxModel(const float *eyePos)
{
    GfxScaledPlacement placement; // [esp+4h] [ebp-20h] BYREF

    if ( g_skyboxModel )
    {
        placement.scale = 1.0f;
        placement.base.origin[0] = *eyePos;
        placement.base.origin[1] = eyePos[1];
        placement.base.origin[2] = eyePos[2];
        memset(&placement, 0, 12);
        placement.base.quat[3] = 1.0f;
        R_FilterXModelIntoScene(g_skyboxModel, &placement, 0, &myhandle, 0.0);
    }
}

XModel *__cdecl R_MapGetSkyboxModel()
{
    return g_skyboxModel;
}

