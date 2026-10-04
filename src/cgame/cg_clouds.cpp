#include "cg_clouds.h"
#include <bgame/bg_wind.h>
#include <universal/com_math.h>
#include <universal/profile.h>

float cg_cloudWorldMin[3];
float cg_cloudWorldMax[3];

int cg_numClouds; // BO1TODO: no write xref's?
cgCloud_t cg_clouds[128];

void __cdecl CG_UpdateClouds(int msec)
{
    PROF_SCOPED("CG_UpdateClouds"); // ADD

    float v1; // [esp+0h] [ebp-44h]
    float radius; // [esp+10h] [ebp-34h]
    int n; // [esp+14h] [ebp-30h]
    float worldSize; // [esp+18h] [ebp-2Ch]
    float worldSize_4; // [esp+1Ch] [ebp-28h]
    cgCloud_t *cloud; // [esp+24h] [ebp-20h]
    float windDir[3]; // [esp+34h] [ebp-10h] BYREF
    float time; // [esp+40h] [ebp-4h]

    BG_GetGlobalWind(windDir);
    windDir[2] = 0.0f;
    Vec3Normalize(windDir);
    time = (float)msec / 1000.0f;
    worldSize = cg_cloudWorldMax[0] - cg_cloudWorldMin[0];
    worldSize_4 = cg_cloudWorldMax[1] - cg_cloudWorldMin[1];
    cloud = cg_clouds;
    for (n = 0; n < cg_numClouds; ++n)
    {
        v1 = cloud->speed * time;
        cloud->pos[0] = (float)(v1 * windDir[0]) + cloud->pos[0];
        cloud->pos[1] = (float)(v1 * windDir[1]) + cloud->pos[1];
        cloud->pos[2] = (float)(v1 * windDir[2]) + cloud->pos[2];
        radius = cloud->layer->radius;
        if ((float)(cg_cloudWorldMin[0] - radius) > cloud->pos[0])
            cloud->pos[0] = (float)((float)(radius * 2.0) + worldSize) + cloud->pos[0];
        if ((float)(cg_cloudWorldMin[1] - radius) > cloud->pos[1])
            cloud->pos[1] = (float)((float)(radius * 2.0) + worldSize_4) + cloud->pos[1];
        if (cloud->pos[0] > (float)(cg_cloudWorldMax[0] + radius))
            cloud->pos[0] = cloud->pos[0] - (float)((float)(radius * 2.0) + worldSize);
        if (cloud->pos[1] > (float)(cg_cloudWorldMax[1] + radius))
            cloud->pos[1] = cloud->pos[1] - (float)((float)(radius * 2.0) + worldSize_4);
        ++cloud;
    }
}