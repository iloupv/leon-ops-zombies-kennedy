#include "com_convexhull.h"
#include <universal/q_shared.h>
#include "assertive.h"
#include "com_math.h"
#include <cstring>

unsigned int __cdecl Com_ConvexHull(float (*points)[2], unsigned int pointCount, float (*hull)[2])
{
    float *v4; // [esp+0h] [ebp-818h]
    float *v5; // [esp+4h] [ebp-814h]
    unsigned int hullOrder[256]; // [esp+8h] [ebp-810h] BYREF
    float offset[2]; // [esp+408h] [ebp-410h] BYREF
    unsigned int pointOrder[256]; // [esp+410h] [ebp-408h] BYREF
    unsigned int hullPointIter; // [esp+810h] [ebp-8h]
    unsigned int hullPointCount; // [esp+814h] [ebp-4h]

    iassert(pointCount >= 3 && pointCount < ARRAY_COUNT(pointOrder));
    iassert(hull != points);
    iassert(hull >= points + pointCount || points >= hull + pointCount);

    // this [64][2] cast is just for convenience. 
    float (*pPoints)[256][2] = (float(*)[256][2])points;
    float (*pHulls)[256][2] = (float(*)[256][2])hull;

    //LODWORD(offset[0]) = LODWORD((*points)[0][0]) ^ _mask__NegFloat_;
    //LODWORD(offset[1]) = LODWORD((*points)[0][1]) ^ _mask__NegFloat_;
    offset[0] = -(*pPoints)[0][0];
    offset[1] = -(*pPoints)[0][1];

    Com_TranslatePoints(pPoints, pointCount, offset);
    Com_InitialHull(pPoints, pointOrder, pointCount, hullOrder);
    hullPointCount = Com_GrowInitialHull(pPoints, pointOrder, pointCount - 2, hullOrder);

    for (unsigned int hullPointIter = 0; hullPointIter < hullPointCount; ++hullPointIter)
    {
        v4 = (*pHulls)[hullPointIter];
        v5 = (*pPoints)[hullOrder[hullPointIter]];

        v4[0] = v5[0] - offset[0];
        v4[1] = v5[1] - offset[1];
    }

    return hullPointCount;
}

void __cdecl Com_InitialHull(
                const float (*points)[256][2],
                unsigned int *pointOrder,
                unsigned int pointCount,
                unsigned int *hullOrder)
{
    unsigned int maxIndex; // [esp+4h] [ebp-Ch]
    unsigned int pointIndex; // [esp+8h] [ebp-8h]
    unsigned int minIndex; // [esp+Ch] [ebp-4h]

    minIndex = 0;
    maxIndex = 0;
    *pointOrder = 0;

    for ( pointIndex = 1; pointIndex < pointCount; ++pointIndex )
    {
        pointOrder[pointIndex] = pointIndex;
        if ( (*points)[pointIndex][1] < (*points)[maxIndex][1] )
        {
            if ( (*points)[minIndex][1] > (*points)[pointIndex][1] )
                minIndex = pointIndex;
        }
        else
        {
            maxIndex = pointIndex;
        }
    }
    
    iassert(minIndex != maxIndex);

    hullOrder[0] = minIndex;
    hullOrder[1] = maxIndex;

    if ( minIndex <= maxIndex )
    {
        Com_SwapHullPoints(pointOrder, maxIndex, pointCount - 1);
        Com_SwapHullPoints(pointOrder, minIndex, pointCount - 2);
    }
    else
    {
        Com_SwapHullPoints(pointOrder, minIndex, pointCount - 1);
        Com_SwapHullPoints(pointOrder, maxIndex, pointCount - 2);
    }
}

void __cdecl Com_SwapHullPoints(unsigned int *pointOrder, unsigned int pointIndex0, unsigned int pointIndex1)
{
    unsigned int swapCache; // [esp+4h] [ebp-4h]

    swapCache = pointOrder[pointIndex0];
    pointOrder[pointIndex0] = pointOrder[pointIndex1];
    pointOrder[pointIndex1] = swapCache;
}

unsigned int __cdecl Com_GrowInitialHull(
                const float (*points)[256][2],
                unsigned int *pointOrder,
                unsigned int pointCount,
                unsigned int *hullOrder)
{
    float *v4; // edx
    float *v5; // eax
    float edgeEq[3]; // [esp+1Ch] [ebp-2Ch] BYREF
    float dist; // [esp+28h] [ebp-20h]
    int botIndex; // [esp+2Ch] [ebp-1Ch]
    int topIndex; // [esp+30h] [ebp-18h]
    int frontIndex; // [esp+34h] [ebp-14h]
    float backDist; // [esp+38h] [ebp-10h]
    int backIndex; // [esp+3Ch] [ebp-Ch]
    float frontDist; // [esp+40h] [ebp-8h]
    unsigned int hullPointCount; // [esp+44h] [ebp-4h]

    if ( !pointCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                    168,
                    0,
                    "%s",
                    "pointCount >= 1") )
    {
        __debugbreak();
    }
    edgeEq[0] = (float)(*points)[hullOrder[1]][1] - (float)(*points)[*hullOrder][1];
    edgeEq[1] = (float)(*points)[*hullOrder][0] - (float)(*points)[hullOrder[1]][0];
    Vec2Normalize(edgeEq);
    edgeEq[2] = (float)(edgeEq[0] * (float)(*points)[*hullOrder][0])
                        + (float)(edgeEq[1] * (float)(*points)[*hullOrder][1]);
    botIndex = 0;
    topIndex = pointCount - 1;
    frontDist = 0.001f;
    frontIndex = -1;
    backDist = -0.001f;
    backIndex = -1;
    while ( botIndex <= topIndex )
    {
        while ( 1 )
        {
            v4 = (float *)(*points)[pointOrder[botIndex]];
            dist = (float)((float)(edgeEq[0] * *v4) + (float)(edgeEq[1] * v4[1])) - edgeEq[2];
            if ( dist < 0.0 )
                break;
            if ( dist > frontDist )
            {
                frontDist = dist;
                frontIndex = botIndex;
            }
            if ( ++botIndex > topIndex )
                goto done_splitting;
        }
        if ( backDist > dist )
        {
            backDist = dist;
            backIndex = botIndex;
        }
        if ( botIndex > topIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                        204,
                        1,
                        "%s",
                        "botIndex <= topIndex") )
        {
            __debugbreak();
        }
        while ( 1 )
        {
            v5 = (float *)(*points)[pointOrder[topIndex]];
            dist = (float)((float)(edgeEq[0] * *v5) + (float)(edgeEq[1] * v5[1])) - edgeEq[2];
            if ( dist > 0.0 )
                break;
            if ( backDist > dist )
            {
                backDist = dist;
                backIndex = topIndex;
            }
            if ( botIndex > --topIndex )
                goto done_splitting;
        }
        if ( dist > frontDist )
        {
            frontDist = dist;
            frontIndex = botIndex;
        }
        if ( botIndex >= topIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                        227,
                        1,
                        "%s",
                        "botIndex < topIndex") )
        {
            __debugbreak();
        }
        Com_SwapHullPoints(pointOrder, botIndex, topIndex);
        if ( backIndex == botIndex )
            backIndex = topIndex;
        ++botIndex;
        --topIndex;
    }
done_splitting:
    if ( topIndex != botIndex - 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                    237,
                    1,
                    "%s",
                    "topIndex == botIndex - 1") )
    {
        __debugbreak();
    }
    if ( frontIndex < 0 && backIndex < 0 )
        return 0;
    hullPointCount = 2;
    if ( frontIndex >= 0 )
    {
        if ( frontIndex > topIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                        245,
                        1,
                        "%s",
                        "frontIndex <= topIndex") )
        {
            __debugbreak();
        }
        Com_SwapHullPoints(pointOrder, frontIndex, topIndex);
        hullPointCount = Com_AddPointToHull(pointOrder[topIndex], 2u, hullOrder, hullPointCount);
        if ( topIndex > 0 )
        {
            hullPointCount = Com_RecursivelyGrowHull(
                                                 points,
                                                 pointOrder,
                                                 topIndex,
                                                 2u,
                                                 0,
                                                 hullOrder,
                                                 hullPointCount);
            hullPointCount = Com_RecursivelyGrowHull(
                                                 points,
                                                 pointOrder,
                                                 topIndex,
                                                 1u,
                                                 2u,
                                                 hullOrder,
                                                 hullPointCount);
        }
    }
    if ( backIndex >= 0 )
    {
        if ( backIndex < botIndex
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                        259,
                        1,
                        "%s",
                        "backIndex >= botIndex") )
        {
            __debugbreak();
        }
        Com_SwapHullPoints(pointOrder, backIndex, botIndex);
        hullPointCount = Com_AddPointToHull(pointOrder[botIndex], 1u, hullOrder, hullPointCount);
        if ( pointCount != botIndex + 1 )
        {
            hullPointCount = Com_RecursivelyGrowHull(
                                                 points,
                                                 &pointOrder[botIndex + 1],
                                                 pointCount - (botIndex + 1),
                                                 1u,
                                                 2u,
                                                 hullOrder,
                                                 hullPointCount);
            return Com_RecursivelyGrowHull(
                             points,
                             &pointOrder[botIndex + 1],
                             pointCount - (botIndex + 1),
                             0,
                             1u,
                             hullOrder,
                             hullPointCount);
        }
    }
    return hullPointCount;
}

unsigned int __cdecl Com_AddPointToHull(
                unsigned int pointIndex,
                unsigned int newIndex,
                unsigned int *hullOrder,
                unsigned int hullPointCount)
{
    if ( newIndex > hullPointCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\universal\\com_convexhull.cpp",
                    58,
                    0,
                    "newIndex not in [0, hullPointCount]\n\t%i not in [%i, %i]",
                    newIndex,
                    0,
                    hullPointCount) )
    {
        __debugbreak();
    }
    memmove(
        (unsigned __int8 *)&hullOrder[newIndex + 1],
        (unsigned __int8 *)&hullOrder[newIndex],
        4 * (hullPointCount - newIndex));
    hullOrder[newIndex] = pointIndex;
    return hullPointCount + 1;
}

unsigned int __cdecl Com_RecursivelyGrowHull(
                const float (*points)[256][2],
                unsigned int *pointOrder,
                unsigned int pointCount,
                unsigned int firstIndex,
                unsigned int secondIndex,
                unsigned int *hullOrder,
                unsigned int hullPointCount)
{
    // kcod4
    float *v7; // edx
    float *v8; // ecx
    float v11; // [esp+0h] [ebp-34h]
    float v12; // [esp+4h] [ebp-30h]
    float *v13; // [esp+10h] [ebp-24h]
    float edgeEq[3]; // [esp+14h] [ebp-20h] BYREF
    float dist; // [esp+20h] [ebp-14h]
    int topIndex; // [esp+24h] [ebp-10h]
    int botIndex; // [esp+28h] [ebp-Ch]
    int frontIndex; // [esp+2Ch] [ebp-8h]
    float frontDist; // [esp+30h] [ebp-4h]

    iassert(pointCount > 0);
    iassert(secondIndex == firstIndex + 1 || (firstIndex == hullPointCount - 1 && secondIndex == 0));

    edgeEq[0] = (float)(*points)[hullOrder[firstIndex]][1] - (float)(*points)[hullOrder[secondIndex]][1];
    edgeEq[1] = (float)(*points)[hullOrder[secondIndex]][0] - (float)(*points)[hullOrder[firstIndex]][0];
    Vec2Normalize(edgeEq);
    v13 = (float *)(*points)[hullOrder[firstIndex]];
    edgeEq[2] = v13[1] * edgeEq[1] + *v13 * edgeEq[0];
    botIndex = 0;
    topIndex = pointCount - 1;
    frontDist = 0.001;
    frontIndex = -1;
    while (botIndex <= topIndex)
    {
        while (1)
        {
            v7 = (float *)(*points)[pointOrder[botIndex]];
            v12 = v7[1] * edgeEq[1] + *v7 * edgeEq[0];
            dist = v12 - edgeEq[2];
            if (dist <= 0.0)
                break;
            if (dist > (double)frontDist)
            {
                frontDist = dist;
                frontIndex = botIndex;
            }
            if (++botIndex > topIndex)
                goto done_splitting_0;
        }
        iassert(botIndex <= topIndex);

        while (1)
        {
            v8 = (float *)(*points)[pointOrder[topIndex]];
            v11 = v8[1] * edgeEq[1] + *v8 * edgeEq[0];
            dist = v11 - edgeEq[2];
            if (dist > 0.0)
                break;
            if (botIndex > --topIndex)
                goto done_splitting_0;
        }
        if (dist > (double)frontDist)
        {
            frontDist = dist;
            frontIndex = botIndex;
        }
        iassert(botIndex < topIndex);
        Com_SwapHullPoints(pointOrder, botIndex++, topIndex--);
    }
done_splitting_0:
    iassert(topIndex == botIndex - 1);
    iassert(frontIndex <= topIndex);

    if (frontIndex < 0)
        return hullPointCount;

    Com_SwapHullPoints(pointOrder, frontIndex, topIndex);
    hullPointCount = Com_AddPointToHull(pointOrder[topIndex], firstIndex + 1, hullOrder, hullPointCount);

    if (!topIndex)
        return hullPointCount;

    if (secondIndex)
        secondIndex = firstIndex + 2;

    return Com_RecursivelyGrowHull(
        points,
        pointOrder,
        topIndex,
        firstIndex,
        firstIndex + 1,
        hullOrder,
        Com_RecursivelyGrowHull(points, pointOrder, topIndex, firstIndex + 1, secondIndex, hullOrder, hullPointCount)
    );
}

void __cdecl Com_TranslatePoints(float (*points)[256][2], unsigned int pointCount, float *offset)
{
    unsigned int pointIdx; // [esp+0h] [ebp-4h]

    for ( pointIdx = 0; pointIdx < pointCount; ++pointIdx )
    {
        (*points)[pointIdx][0] = (*points)[pointIdx][0] + *offset;
        (*points)[pointIdx][1] = (*points)[pointIdx][1] + offset[1];
    }
}

