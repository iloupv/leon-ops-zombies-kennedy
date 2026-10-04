#include "r_model_lod.h"
#include <universal/com_math.h>
#include "r_init.h"

double __cdecl R_GetBaseLodDist(const float *origin)
{
    float distToExtraCamOrigin; // [esp+24h] [ebp-8h]
    float distToLodOrigin; // [esp+28h] [ebp-4h]

    distToLodOrigin = Vec3Distance(origin, rg.lodParms.origin);
    if ( !rg.lodParms.extraCamOriginValid )
        return distToLodOrigin;
    distToExtraCamOrigin = Vec3Distance(origin, rg.lodParms.extraCamOrigin);
    if ( distToExtraCamOrigin <= distToLodOrigin )
        return distToExtraCamOrigin;
    else
        return distToLodOrigin;
}

double __cdecl R_GetAdjustedLodDist(float dist, XModelLodRampType lodRampType)
{
    if ( !rg.lodParms.valid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_model_lod.cpp",
                    44,
                    0,
                    "%s",
                    "model_lod_glob->valid") )
    {
        __debugbreak();
    }
    return dist * rg.lodParms.ramp[lodRampType].scale + rg.lodParms.ramp[lodRampType].bias;
}

