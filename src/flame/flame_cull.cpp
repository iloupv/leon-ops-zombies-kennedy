#include "flame_cull.h"
#include "flame_class_chunk.h"

void __cdecl Flame_Cull_Stream_Chunk(bool is_server, flameChunk_s *cull, flameChunk_s *keep)
{
    if ( cull->spawnSmokeInterval && !keep->spawnSmokeInterval )
        keep->spawnSmokeInterval = cull->spawnSmokeInterval;
    if ( cull->spawnDripsInterval && !keep->spawnDripsInterval )
        keep->spawnDripsInterval = cull->spawnDripsInterval;
    Flame_Class_Chunk_Free(is_server, cull);
}

void __cdecl Flame_Cull_Stream_Chunks(bool is_server, flameStream_s *stream)
{
    double v2; // st7
    double v3; // st7
    float v4; // [esp+1Ch] [ebp-34h]
    float v5; // [esp+24h] [ebp-2Ch]
    float thisEndOrg[3]; // [esp+28h] [ebp-28h] BYREF
    float nextEndOrg[3]; // [esp+34h] [ebp-1Ch] BYREF
    flameChunk_s *thisChunk; // [esp+40h] [ebp-10h]
    int lifeFrac; // [esp+44h] [ebp-Ch]
    flameChunk_s *nextChunk; // [esp+48h] [ebp-8h]
    float cullDist; // [esp+4Ch] [ebp-4h]

    thisChunk = stream->chunkList;
    while ( thisChunk )
    {
        nextChunk = (flameChunk_s *)thisChunk->gen.listLocal.next;
        lifeFrac = thisChunk->gen.age.lastUpdateTime - thisChunk->gen.age.startTime;
        if ( !nextChunk )
            goto LABEL_12;
        if ( (float)(stream->flameVars->flameVar_streamChunkCullMinLife * 1000.0) <= (float)lifeFrac
            && (float)lifeFrac <= (float)(stream->flameVars->flameVar_streamChunkCullMaxLife * 1000.0) )
        {
            cullDist = thisChunk->gen.size.current * stream->flameVars->flameVar_streamChunkCullDistSizeFrac;
            v5 = (float)(thisChunk->gen.age.endTime - thisChunk->gen.age.lastUpdateTime);
            thisEndOrg[0] = (float)(v5 * thisChunk->gen.phys.velocity[0]) + thisChunk->gen.phys.origin[0];
            thisEndOrg[1] = (float)(v5 * thisChunk->gen.phys.velocity[1]) + thisChunk->gen.phys.origin[1];
            thisEndOrg[2] = (float)(v5 * thisChunk->gen.phys.velocity[2]) + thisChunk->gen.phys.origin[2];
            v4 = (float)(nextChunk->gen.age.endTime - nextChunk->gen.age.lastUpdateTime);
            nextEndOrg[0] = (float)(v4 * nextChunk->gen.phys.velocity[0]) + nextChunk->gen.phys.origin[0];
            nextEndOrg[1] = (float)(v4 * nextChunk->gen.phys.velocity[1]) + nextChunk->gen.phys.origin[1];
            nextEndOrg[2] = (float)(v4 * nextChunk->gen.phys.velocity[2]) + nextChunk->gen.phys.origin[2];
            v2 = Vec3DistanceSq(nextEndOrg, thisEndOrg);
            if ( cullDist * cullDist > v2 )
            {
                v3 = Vec3DistanceSq(nextChunk->gen.phys.origin, thisChunk->gen.phys.origin);
                if ( cullDist * cullDist > v3 )
                {
                    if ( nextChunk->gen.size.current <= thisChunk->gen.size.current )
                    {
                        Flame_Cull_Stream_Chunk(is_server, nextChunk, thisChunk);
                        nextChunk = (flameChunk_s *)thisChunk->gen.listLocal.next;
                    }
                    else
                    {
                        Flame_Cull_Stream_Chunk(is_server, thisChunk, nextChunk);
                    }
                }
            }
LABEL_12:
            thisChunk = nextChunk;
        }
        else
        {
            thisChunk = nextChunk;
        }
    }
}

