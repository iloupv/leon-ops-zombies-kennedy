#include "fx_update_util.h"

#include <gfx_d3d/fxprimitives.h>
#include "fx_random.h"
#include "fx_system.h"
#include <win32/win_common.h>
#include <qcommon/common.h>

void __cdecl FX_OffsetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int randomSeed,
    float *spawnOrigin)
{
    float v4; // [esp+Ch] [ebp-70h]
    float v5; // [esp+14h] [ebp-68h]
    float dir[3]; // [esp+38h] [ebp-44h] BYREF
    float height; // [esp+44h] [ebp-38h]
    float yaw; // [esp+48h] [ebp-34h]
    float radius; // [esp+4Ch] [ebp-30h]
    float sinYaw; // [esp+50h] [ebp-2Ch]
    float cosYaw; // [esp+54h] [ebp-28h]
    float axis[3][3]; // [esp+58h] [ebp-24h] BYREF

    if ((elemDef->flags & 0x30) != 0)
    {
        if ((elemDef->flags & 0x30) == 0x10)
        {
            FX_RandomDir(randomSeed, dir);
            radius = (float)(fx_randomTable[randomSeed + 11] * elemDef->spawnOffsetRadius.amplitude)
                + elemDef->spawnOffsetRadius.base;
            *spawnOrigin = (float)(radius * dir[0]) + *spawnOrigin;
            spawnOrigin[1] = (float)(radius * dir[1]) + spawnOrigin[1];
            spawnOrigin[2] = (float)(radius * dir[2]) + spawnOrigin[2];
        }
        else
        {
            if ((elemDef->flags & 0x30) != 0x20
                && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    117,
                    0,
                    "%s\n\t(elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) = %i",
                    "((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_CYLINDER)",
                    elemDef->flags & 0x30))
            {
                __debugbreak();
            }
            radius = (float)(fx_randomTable[randomSeed + 11] * elemDef->spawnOffsetRadius.amplitude)
                + elemDef->spawnOffsetRadius.base;
            yaw = fx_randomTable[randomSeed + 9] * 6.2831855;
            UnitQuatToAxis(effectFrame->quat, axis);
            cosYaw = cos(yaw);
            sinYaw = sin(yaw);
            v4 = radius * cosYaw;
            v5 = radius * sinYaw;
            *spawnOrigin = (float)((float)((float)(radius * cosYaw) * axis[1][0]) + *spawnOrigin)
                + (float)((float)(radius * sinYaw) * axis[2][0]);
            spawnOrigin[1] = (float)((float)(v4 * axis[1][1]) + spawnOrigin[1]) + (float)(v5 * axis[2][1]);
            spawnOrigin[2] = (float)((float)(v4 * axis[1][2]) + spawnOrigin[2]) + (float)(v5 * axis[2][2]);
            height = (float)(fx_randomTable[randomSeed + 10] * elemDef->spawnOffsetHeight.amplitude)
                + elemDef->spawnOffsetHeight.base;
            *spawnOrigin = (float)(height * axis[0][0]) + *spawnOrigin;
            spawnOrigin[1] = (float)(height * axis[0][1]) + spawnOrigin[1];
            spawnOrigin[2] = (float)(height * axis[0][2]) + spawnOrigin[2];
        }
    }
}

void __cdecl FX_GetOriginForTrailElem(
                FxEffect *effect,
                const FxElemDef *elemDef,
                const FxSpatialFrame *effectFrameWhenPlayed,
                int randomSeed,
                float *outOrigin,
                float *outRight,
                float *outUp)
{
    float effectFrameAxis[3][3]; // [esp+8h] [ebp-24h] BYREF

    if ( (!outRight || !outUp)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    137,
                    0,
                    "%s",
                    "(outRight != NULL) && (outUp != NULL)") )
    {
        __debugbreak();
    }
    if ( (elemDef->flags & 0xC0) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    138,
                    0,
                    "%s\n\t((elemDef->flags & FX_ELEM_RUN_MASK)) = %i",
                    "((elemDef->flags & FX_ELEM_RUN_MASK) == FX_ELEM_RUN_RELATIVE_TO_WORLD)",
                    elemDef->flags & 0xC0) )
    {
        __debugbreak();
    }
    UnitQuatToAxis(effectFrameWhenPlayed->quat, effectFrameAxis);
    *outRight = effectFrameAxis[1][0];
    outRight[1] = effectFrameAxis[1][1];
    outRight[2] = effectFrameAxis[1][2];
    *outUp = effectFrameAxis[2][0];
    outUp[1] = effectFrameAxis[2][1];
    outUp[2] = effectFrameAxis[2][2];
    FX_GetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin);
    FX_OffsetSpawnOrigin(effectFrameWhenPlayed, elemDef, randomSeed, outOrigin);
}

void __cdecl FX_GetSpawnOrigin(
    const FxSpatialFrame *effectFrame,
    const FxElemDef *elemDef,
    int randomSeed,
    float *spawnOrigin)
{
    float offset[3]; // [esp+1Ch] [ebp-Ch] BYREF

    offset[0] = (float)(fx_randomTable[randomSeed + 6] * elemDef->spawnOrigin[0].amplitude) + elemDef->spawnOrigin[0].base;
    offset[1] = (float)(fx_randomTable[randomSeed + 7] * elemDef->spawnOrigin[1].amplitude) + elemDef->spawnOrigin[1].base;
    offset[2] = (float)(fx_randomTable[randomSeed + 8] * elemDef->spawnOrigin[2].amplitude) + elemDef->spawnOrigin[2].base;
    if ((elemDef->flags & 2) != 0)
    {
        FX_TransformPosFromLocalToWorld(effectFrame, offset, spawnOrigin);
    }
    else
    {
        *spawnOrigin = offset[0] + effectFrame->origin[0];
        spawnOrigin[1] = offset[1] + effectFrame->origin[1];
        spawnOrigin[2] = offset[2] + effectFrame->origin[2];
    }
}


void __cdecl FX_TransformPosFromLocalToWorld(const FxSpatialFrame *frame, float *posLocal, float *posWorld)
{
    float v3; // [esp+4h] [ebp-38h]
    float v4; // [esp+Ch] [ebp-30h]
    float v5; // [esp+10h] [ebp-2Ch]
    float axis[3][3]; // [esp+18h] [ebp-24h] BYREF

    UnitQuatToAxis(frame->quat, axis);
    v5 = *posLocal;
    *posWorld = (float)(*posLocal * axis[0][0]) + frame->origin[0];
    posWorld[1] = (float)(v5 * axis[0][1]) + frame->origin[1];
    posWorld[2] = (float)(v5 * axis[0][2]) + frame->origin[2];
    v4 = posLocal[1];
    *posWorld = (float)(v4 * axis[1][0]) + *posWorld;
    posWorld[1] = (float)(v4 * axis[1][1]) + posWorld[1];
    posWorld[2] = (float)(v4 * axis[1][2]) + posWorld[2];
    v3 = posLocal[2];
    *posWorld = (float)(v3 * axis[2][0]) + *posWorld;
    posWorld[1] = (float)(v3 * axis[2][1]) + posWorld[1];
    posWorld[2] = (float)(v3 * axis[2][2]) + posWorld[2];
}

void __cdecl FX_SpatialFrameToOrientation(const FxSpatialFrame *frame, orientation_t *orient)
{
    orient->origin[0] = frame->origin[0];
    orient->origin[1] = frame->origin[1];
    orient->origin[2] = frame->origin[2];
    UnitQuatToAxis(frame->quat, orient->axis);
}

void __cdecl FX_OrientationDirToWorldDir(const orientation_t *orient, const float *dir, float *out)
{
    if ( dir == out
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    174,
                    0,
                    "%s",
                    "dir != out") )
    {
        __debugbreak();
    }
    *out = (float)((float)(*dir * orient->axis[0][0]) + (float)(dir[1] * orient->axis[1][0]))
             + (float)(dir[2] * orient->axis[2][0]);
    out[1] = (float)((float)(*dir * orient->axis[0][1]) + (float)(dir[1] * orient->axis[1][1]))
                 + (float)(dir[2] * orient->axis[2][1]);
    out[2] = (float)((float)(*dir * orient->axis[0][2]) + (float)(dir[1] * orient->axis[1][2]))
                 + (float)(dir[2] * orient->axis[2][2]);
}

void __cdecl FX_GetOrientation(
                const FxElemDef *elemDef,
                const FxSpatialFrame *frameAtSpawn,
                const FxSpatialFrame *frameNow,
                int randomSeed,
                orientation_t *orient)
{
    const char *v5; // eax
    const char *v6; // eax
    unsigned int v7; // [esp+20h] [ebp-50h]
    float v8; // [esp+30h] [ebp-40h]
    float up[3]; // [esp+60h] [ebp-10h] BYREF
    int runFlags; // [esp+6Ch] [ebp-4h]

    if ( !Vec4IsNormalized(frameAtSpawn->quat) )
    {
        v5 = va("%g %g %g %g", frameAtSpawn->quat[0], frameAtSpawn->quat[1], frameAtSpawn->quat[2], frameAtSpawn->quat[3]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                        185,
                        0,
                        "%s\n\t%s",
                        "Vec4IsNormalized( frameAtSpawn->quat )",
                        v5) )
            __debugbreak();
    }
    if ( !Vec4IsNormalized(frameNow->quat) )
    {
        v6 = va("%g %g %g %g", frameNow->quat[0], frameNow->quat[1], frameNow->quat[2], frameNow->quat[3]);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                        186,
                        0,
                        "%s\n\t%s",
                        "Vec4IsNormalized( frameNow->quat )",
                        v6) )
            __debugbreak();
    }
    runFlags = elemDef->flags & 0xC0;
    if ( runFlags )
    {
        if ( runFlags == 64 )
        {
            FX_SpatialFrameToOrientation(frameAtSpawn, orient);
        }
        else if ( runFlags == 128 )
        {
            FX_SpatialFrameToOrientation(frameNow, orient);
        }
        else
        {
            if ( runFlags != 192
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                            210,
                            0,
                            "%s\n\t(runFlags) = %i",
                            "(runFlags == FX_ELEM_RUN_RELATIVE_TO_OFFSET)",
                            runFlags) )
            {
                __debugbreak();
            }
            if ( (elemDef->flags & 0x30) == 0
                && !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                            211,
                            0,
                            "%s\n\t(elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) = %i",
                            "((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) != FX_ELEM_SPAWN_OFFSET_NONE)",
                            elemDef->flags & 0x30) )
            {
                __debugbreak();
            }
            FX_GetSpawnOrigin(frameAtSpawn, elemDef, randomSeed, orient->origin);
            orient->axis[0][0] = 0.0f;
            orient->axis[0][1] = 0.0f;
            orient->axis[0][2] = 0.0f;
            FX_OffsetSpawnOrigin(frameAtSpawn, elemDef, randomSeed, orient->axis[0]);
            orient->origin[0] = orient->axis[0][0] + orient->origin[0];
            orient->origin[1] = orient->axis[0][1] + orient->origin[1];
            orient->origin[2] = orient->axis[0][2] + orient->origin[2];
            if ( Vec3Normalize(orient->axis[0]) == 0.0 )
            {
                orient->axis[0][0] = 1.0f;
                orient->axis[0][1] = 0.0f;
                orient->axis[0][2] = 0.0f;
            }
            if ( (elemDef->flags & 0x30) == 0x10 )
            {
                Vec3Basis_RightHanded(orient->axis[0], orient->axis[1], orient->axis[2]);
            }
            else
            {
                if ( (elemDef->flags & 0x30) != 0x20
                    && !Assert_MyHandler(
                                "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                                228,
                                0,
                                "%s\n\t(elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) = %i",
                                "((elemDef->flags & FX_ELEM_SPAWN_OFFSET_MASK) == FX_ELEM_SPAWN_OFFSET_CYLINDER)",
                                elemDef->flags & 0x30) )
                {
                    __debugbreak();
                }
                if ( (elemDef->flags & 2) != 0 )
                {
                    up[0] = 0.0f;
                    up[1] = 0.0f;
                    up[2] = 1.0f;
                    v7 = elemDef->flags & 0x1C000;
                    if ( v7 > 0xC000 )
                    {
                        if ( v7 == 0x10000 )
                        {
                            up[0] = -1.0f;
                            up[1] = 0.0f;
                            up[2] = 0.0f;
                        }
                        else if ( v7 == 81920 )
                        {
                            up[0] = 0.0f;
                            up[1] = 0.0f;
                            up[2] = -1.0f;
                        }
                    }
                    else
                    {
                        switch ( v7 )
                        {
                            case 0xC000u:
                                up[0] = 0.0f;
                                up[1] = -1.0f;
                                up[2] = 0.0f;
                                break;
                            case 0x4000u:
                                up[0] = 1.0f;
                                up[1] = 0.0f;
                                up[2] = 0.0f;
                                break;
                            case 0x8000u:
                                up[0] = 0.0f;
                                up[1] = 1.0f;
                                up[2] = 0.0f;
                                break;
                        }
                    }
                }
                else
                {
                    v8 = orient->axis[0][2];
                    up[0] = 0.0f;
                    if ( fabs(v8) < 0.99900001 )
                    {
                        up[1] = 0.0f;
                        up[2] = 1.0f;
                    }
                    else
                    {
                        up[1] = 1.0f;
                        up[2] = 0.0f;
                    }
                }
                Vec3Cross(up, orient->axis[0], orient->axis[1]);
                Vec3Normalize(orient->axis[1]);
                Vec3Cross(orient->axis[0], orient->axis[1], orient->axis[2]);
            }
        }
    }
    else
    {
        orient->origin[0] = 0.0f;
        orient->origin[1] = 0.0f;
        orient->origin[2] = 0.0f;
        orient->axis[0][0] = 1.0f;
        orient->axis[0][1] = 0.0f;
        orient->axis[0][2] = 0.0f;
        orient->axis[1][0] = 0.0f;
        orient->axis[1][1] = 1.0f;
        orient->axis[1][2] = 0.0f;
        orient->axis[2][0] = 0.0f;
        orient->axis[2][1] = 0.0f;
        orient->axis[2][2] = 1.0f;
    }
}

void FX_GetVelocityAtTime(
    const FxElemDef *elemDef,
    int randomSeed,
    float msecLifeSpan,
    float msecElapsed,
    const orientation_t *orient,
    const float *baseVel,
    float *velocity)
{
    char *v8; // eax
    int v9; // eax
    char *v10; // eax
    double v11; // [esp+18h] [ebp-58h]
    float v12; // [esp+20h] [ebp-50h]
    float v13; // [esp+24h] [ebp-4Ch]
    float velocityWorld[3]; // [esp+28h] [ebp-48h] BYREF
    float sampleTime; // [esp+34h] [ebp-3Ch]
    float samplePoint; // [esp+38h] [ebp-38h]
    float velocityScale; // [esp+3Ch] [ebp-34h]
    int sampleIndex; // [esp+40h] [ebp-30h]
    const FxElemVelStateSample *samples; // [esp+44h] [ebp-2Ch]
    int intervalCount; // [esp+48h] [ebp-28h]
    float velocityLocal[3]; // [esp+4Ch] [ebp-24h] BYREF
    float weight[2]; // [esp+58h] [ebp-18h] BYREF
    float rangeLerp[3]; // [esp+60h] [ebp-10h] BYREF
    float sampleLerp; // [esp+6Ch] [ebp-4h]

    if (!elemDef
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp", 310, 0, "%s", "elemDef"))
    {
        __debugbreak();
    }
    if (!elemDef->velSamples
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
            311,
            0,
            "%s",
            "elemDef->velSamples"))
    {
        __debugbreak();
    }
    if (!elemDef->velIntervalCount
        && !Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
            312,
            0,
            "%s\n\t(elemDef->velIntervalCount) = %i",
            "(elemDef->velIntervalCount >= 1)",
            elemDef->velIntervalCount))
    {
        __debugbreak();
    }
    sampleTime = msecElapsed / msecLifeSpan;
    if ((float)(msecElapsed / msecLifeSpan) < 0.0 || sampleTime >= 1.0)
    {
        v8 = va("%g: %g, %g", sampleTime, msecElapsed, msecLifeSpan);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
            315,
            0,
            "%s\n\t%s",
            "0.0f <= sampleTime && sampleTime < 1.0f",
            v8))
            __debugbreak();
    }
    v13 = fx_randomTable[randomSeed];
    rangeLerp[0] = v13;
    v12 = fx_randomTable[randomSeed + 1];
    rangeLerp[1] = v12;
    *((float *)&v11 + 1) = fx_randomTable[randomSeed + 2];
    rangeLerp[2] = *((float *)&v11 + 1);
    intervalCount = elemDef->velIntervalCount;
    samplePoint = (float)intervalCount * sampleTime;
    *(float *)&v11 = floor(samplePoint);
    v9 = (int)*(float *)&v11;
    sampleIndex = v9;
    sampleLerp = samplePoint - (float)v9;
    if (v9 < 0 || sampleIndex >= intervalCount)
    {
        v10 = va("%i for %g on %i intervals", sampleIndex, sampleTime, intervalCount);
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
            325,
            1,
            "%s\n\t(va( \"%i for %g on %i intervals\", sampleIndex, sampleTime, intervalCount )) = %i",
            "(sampleIndex >= 0 && sampleIndex < intervalCount)",
            v10,
            v11,
            v12,
            v13))
            __debugbreak();
    }
    weight[1] = (float)intervalCount * sampleLerp;
    weight[0] = (float)intervalCount - weight[1];
    samples = &elemDef->velSamples[sampleIndex];
    *velocity = *baseVel;
    velocity[1] = baseVel[1];
    velocity[2] = baseVel[2];
    velocityScale = 1000.0f;
    if ((elemDef->flags & 0x2000000) != 0)
    {
        FX_GetVelocityAtTimeInFrame(&samples->world, &samples[1].world, rangeLerp, weight, velocityWorld);
        *velocity = (float)(velocityScale * velocityWorld[0]) + *velocity;
        velocity[1] = (float)(velocityScale * velocityWorld[1]) + velocity[1];
        velocity[2] = (float)(velocityScale * velocityWorld[2]) + velocity[2];
    }
    if ((elemDef->flags & 0x1000000) != 0)
    {
        FX_GetVelocityAtTimeInFrame(&samples->local, &samples[1].local, rangeLerp, weight, velocityLocal);
        FX_OrientationDirToWorldDir(orient, velocityLocal, velocityWorld);
        *velocity = (float)(velocityScale * velocityWorld[0]) + *velocity;
        velocity[1] = (float)(velocityScale * velocityWorld[1]) + velocity[1];
        velocity[2] = (float)(velocityScale * velocityWorld[2]) + velocity[2];
    }
}


void __cdecl FX_GetVelocityAtTimeInFrame(
                const FxElemVelStateInFrame *statePrev,
                const FxElemVelStateInFrame *stateNext,
                const float *rangeLerp,
                const float *weight,
                float *velocity)
{
    *velocity = (float)((float)(*rangeLerp * statePrev->velocity.amplitude[0]) + statePrev->velocity.base[0]) * *weight;
    *velocity = (float)((float)((float)(*rangeLerp * stateNext->velocity.amplitude[0]) + stateNext->velocity.base[0])
                                        * weight[1])
                        + *velocity;
    velocity[1] = (float)((float)(rangeLerp[1] * statePrev->velocity.amplitude[1]) + statePrev->velocity.base[1])
                            * *weight;
    velocity[1] = (float)((float)((float)(rangeLerp[1] * stateNext->velocity.amplitude[1]) + stateNext->velocity.base[1])
                                            * weight[1])
                            + velocity[1];
    velocity[2] = (float)((float)(rangeLerp[2] * statePrev->velocity.amplitude[2]) + statePrev->velocity.base[2])
                            * *weight;
    velocity[2] = (float)((float)((float)(rangeLerp[2] * stateNext->velocity.amplitude[2]) + stateNext->velocity.base[2])
                                            * weight[1])
                            + velocity[2];
}

void __cdecl FX_OrientationPosToWorldPos(const orientation_t *orient, const float *pos, float *out)
{
    if ( pos == out
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    352,
                    0,
                    "%s",
                    "pos != out") )
    {
        __debugbreak();
    }
    *out = (float)((float)((float)(*pos * orient->axis[0][0]) + orient->origin[0]) + (float)(pos[1] * orient->axis[1][0]))
             + (float)(pos[2] * orient->axis[2][0]);
    out[1] = (float)((float)((float)(*pos * orient->axis[0][1]) + orient->origin[1]) + (float)(pos[1] * orient->axis[1][1]))
                 + (float)(pos[2] * orient->axis[2][1]);
    out[2] = (float)((float)((float)(*pos * orient->axis[0][2]) + orient->origin[2]) + (float)(pos[1] * orient->axis[1][2]))
                 + (float)(pos[2] * orient->axis[2][2]);
}

void __cdecl FX_OrientationPosFromWorldPos(const orientation_t *orient, const float *pos, float *out)
{
    float dir; // [esp+0h] [ebp-Ch]
    float dir_4; // [esp+4h] [ebp-8h]
    float dir_8; // [esp+8h] [ebp-4h]

    if ( pos == out
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    363,
                    0,
                    "%s",
                    "pos != out") )
    {
        __debugbreak();
    }
    dir = *pos - orient->origin[0];
    dir_4 = pos[1] - orient->origin[1];
    dir_8 = pos[2] - orient->origin[2];
    *out = (float)((float)(dir * orient->axis[0][0]) + (float)(dir_4 * orient->axis[0][1]))
             + (float)(dir_8 * orient->axis[0][2]);
    out[1] = (float)((float)(dir * orient->axis[1][0]) + (float)(dir_4 * orient->axis[1][1]))
                 + (float)(dir_8 * orient->axis[1][2]);
    out[2] = (float)((float)(dir * orient->axis[2][0]) + (float)(dir_4 * orient->axis[2][1]))
                 + (float)(dir_8 * orient->axis[2][2]);
}

int warnCount_1;
void __cdecl FX_AddVisBlocker(FxSystem *system, const float *posWorld, float radius, float opacity)
{
    FxVisState *visState; // [esp+4h] [ebp-Ch]
    int blockerIndex; // [esp+8h] [ebp-8h]
    FxVisBlocker *localVisBlocker; // [esp+Ch] [ebp-4h]

    visState = system->visStateBufferWrite;
    blockerIndex = visState->blockerCount + 1;
    if ( blockerIndex < 256 )
    {
        localVisBlocker = &visState->blocker[blockerIndex];
        localVisBlocker->origin[0] = *posWorld;
        localVisBlocker->origin[1] = posWorld[1];
        localVisBlocker->origin[2] = posWorld[2];
        if ( radius >= 4096.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                        412,
                        0,
                        "%s",
                        "radius < 65536.0f * FX_VIS_BLOCKER_RADIUS_INV_SCALE") )
        {
            __debugbreak();
        }
        if ( (opacity < 0.0 || opacity >= 1.0)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                        413,
                        0,
                        "%s",
                        "opacity >= 0.0f && opacity < 65536.0f * FX_VIS_BLOCKER_VISIBILITY_INV_SCALE") )
        {
            __debugbreak();
        }
        localVisBlocker->radius = (int)(float)(radius * 16.0);
        localVisBlocker->visibility = (int)(float)((float)(1.0 - opacity) * 65536.0);
        _InterlockedExchangeAdd(&visState->blockerCount, 1u);
    }
    else if ( warnCount_1 != system->frameCount )
    {
        warnCount_1 = system->frameCount;
        Com_PrintWarning(21, "More than %i visibility blocking particles exist concurrently\n", 256);
    }
}

void __cdecl FX_ToggleVisBlockerFrame(FxSystem *system)
{
    FxVisState *visStateSwapCache; // [esp+0h] [ebp-4h]

    if ( !system
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp", 429, 0, "%s", "system") )
    {
        __debugbreak();
    }
    Sys_EnterCriticalSection(CRITSECT_FX_VIS);
    visStateSwapCache = (FxVisState *)system->visStateBufferRead;
    system->visStateBufferRead = system->visStateBufferWrite;
    system->visStateBufferWrite = visStateSwapCache;
    system->visStateBufferWrite->blockerCount = 0;
    fx_serverVisClient = system->localClientNum;
    Sys_LeaveCriticalSection(CRITSECT_FX_VIS);
}

char __cdecl FX_CullSphere(const FxCamera *camera, unsigned int frustumPlaneCount, const float *posWorld, float radius)
{
    const char *v4; // eax
    double v5; // st7
    const char *v6; // eax
    unsigned int planeIndex; // [esp+30h] [ebp-4h]

    if ( !camera->isValid
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                    468,
                    0,
                    "%s",
                    "camera->isValid") )
    {
        __debugbreak();
    }
    if ( frustumPlaneCount != camera->frustumPlaneCount && frustumPlaneCount != 5 )
    {
        v4 = va("%i, %i", frustumPlaneCount, camera->frustumPlaneCount);
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                        469,
                        0,
                        "%s\n\t%s",
                        "frustumPlaneCount == camera->frustumPlaneCount || frustumPlaneCount == 5",
                        v4) )
            __debugbreak();
    }
    for ( planeIndex = 0; planeIndex < frustumPlaneCount; ++planeIndex )
    {
        if ( !Vec3IsNormalized(camera->frustum[planeIndex]) )
        {
            v5 = Vec3Length(camera->frustum[planeIndex]);
            v6 = va(
                         "(%g %g %g) len %g",
                         camera->frustum[planeIndex][0],
                         camera->frustum[planeIndex][1],
                         camera->frustum[planeIndex][2],
                         v5);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\EffectsCore\\fx_update_util.cpp",
                            473,
                            0,
                            "%s\n\t%s",
                            "Vec3IsNormalized( camera->frustum[planeIndex] )",
                            v6) )
                __debugbreak();
        }
        if ((-(radius)) >= 
            (float)((float)((float)((float)(camera->frustum[planeIndex][0] * *posWorld) 
            + (float)(camera->frustum[planeIndex][1] * posWorld[1])) 
            + (float)(camera->frustum[planeIndex][2] * posWorld[2]))
            - camera->frustum[planeIndex][3]))
        {
            return 1;
        }
    }
    return 0;
}

void __cdecl FX_AnglesToOrientedAxis(float *anglesInRad, const orientation_t *orient, float (*axisOut)[3])
{
    float v3; // [esp+8h] [ebp-40h]
    float v4; // [esp+14h] [ebp-34h]
    float v5; // [esp+20h] [ebp-28h]
    float cy; // [esp+24h] [ebp-24h]
    float sr; // [esp+28h] [ebp-20h]
    float localDir[3]; // [esp+2Ch] [ebp-1Ch] BYREF
    float v9; // [esp+38h] [ebp-10h]
    float cr; // [esp+3Ch] [ebp-Ch]
    float cp; // [esp+40h] [ebp-8h]
    float sy; // [esp+44h] [ebp-4h]

    v5 = anglesInRad[1];
    cy = cos(v5);
    sy = sin(v5);
    v4 = *anglesInRad;
    cp = cos(*anglesInRad);
    v9 = sin(v4);
    v3 = anglesInRad[2];
    cr = cos(v3);
    sr = sin(v3);
    localDir[0] = cp * cy;
    localDir[1] = cp * sy;
    localDir[2] = -v9;
    FX_OrientationDirToWorldDir(orient, localDir, (float *)axisOut);
    localDir[0] = (float)((float)(sr * v9) * cy) - (float)(cr * sy);
    localDir[1] = (float)((float)(sr * v9) * sy) + (float)(cr * cy);
    localDir[2] = sr * cp;
    FX_OrientationDirToWorldDir(orient, localDir, &(*axisOut)[3]);
    localDir[0] = (float)((float)(cr * v9) * cy) + (float)(sr * sy);
    localDir[1] = (float)((float)(cr * v9) * sy) - (float)(sr * cy);
    localDir[2] = cr * cp;
    FX_OrientationDirToWorldDir(orient, localDir, &(*axisOut)[6]);
}

void __cdecl FX_GetElemAxis(
    const FxElemDef *elemDef,
    int randomSeed,
    const orientation_t *orient,
    float msecElapsed,
    float (*axis)[3])
{
    float angles[3]; // [esp+30h] [ebp-Ch] BYREF

    angles[0] = (float)(fx_randomTable[randomSeed + 12] * elemDef->spawnAngles[0].amplitude)
        + elemDef->spawnAngles[0].base;
    angles[1] = (float)(fx_randomTable[randomSeed + 13] * elemDef->spawnAngles[1].amplitude)
        + elemDef->spawnAngles[1].base;
    angles[2] = (float)(fx_randomTable[randomSeed + 14] * elemDef->spawnAngles[2].amplitude)
        + elemDef->spawnAngles[2].base;
    angles[0] = (float)((float)((float)(fx_randomTable[randomSeed + 3] * elemDef->angularVelocity[0].amplitude)
        + elemDef->angularVelocity[0].base)
        * msecElapsed)
        + angles[0];
    angles[1] = (float)((float)((float)(fx_randomTable[randomSeed + 4] * elemDef->angularVelocity[1].amplitude)
        + elemDef->angularVelocity[1].base)
        * msecElapsed)
        + angles[1];
    angles[2] = (float)((float)((float)(fx_randomTable[randomSeed + 5] * elemDef->angularVelocity[2].amplitude)
        + elemDef->angularVelocity[2].base)
        * msecElapsed)
        + angles[2];
    FX_AnglesToOrientedAxis(angles, orient, axis);
}

int __cdecl FX_GetElemLifeSpanMsec(int elemRandomSeed, float elemWindInterpolate, const FxElemDef *elemDef)
{
    int lifespan; // [esp+Ch] [ebp-4h]

    lifespan = elemDef->lifeSpanMsec.base
                     + (((elemDef->lifeSpanMsec.amplitude + 1) * LOWORD(fx_randomTable[elemRandomSeed + 17])) >> 16);
    if ( elemDef->maxWindStrength )
        return (int)(float)((float)((float)(1.0 - elemWindInterpolate) * (float)lifespan)
                                            + (float)(elemWindInterpolate * (float)elemDef->lifespanAtMaxWind));
    return lifespan;
}

