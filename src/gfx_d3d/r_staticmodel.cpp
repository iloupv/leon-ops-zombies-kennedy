#include "r_staticmodel.h"
#include "r_bsp.h"
#include "r_init.h"

int __cdecl R_StaticModelGetMemoryUsage(XModel *model, int *modelCount)
{
    int cellIndex; // [esp+0h] [ebp-14h]
    GfxAabbTree *tree; // [esp+8h] [ebp-Ch]
    int smodelIndexIter; // [esp+Ch] [ebp-8h]
    int usage; // [esp+10h] [ebp-4h]

    usage = 0;
    *modelCount = 0;
    if ( !rgp.world )
        return 0;
    for ( cellIndex = 0; cellIndex < rgp.world->dpvsPlanes.cellCount; ++cellIndex )
    {
        tree = rgp.world->cells[cellIndex].aabbTree;
        for ( smodelIndexIter = 0; smodelIndexIter < tree->smodelIndexCount; ++smodelIndexIter )
        {
            if ( rgp.world->dpvs.smodelDrawInsts[tree->smodelIndexes[smodelIndexIter]].model == model )
            {
                usage += R_StaticModelGetMemoryUsageInst();
                ++*modelCount;
            }
        }
    }
    return usage;
}

int __cdecl R_StaticModelGetMemoryUsageInst()
{
    return 76;
}

