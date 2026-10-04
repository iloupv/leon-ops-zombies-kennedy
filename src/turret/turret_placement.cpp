#include "turret_placement.h"
#include <qcommon/cm_trace.h>
#include <universal/com_math_anglevectors.h>
#include <game/g_debug.h>
#include <cgame/cg_drawtools.h>

#include <bgame/bg_public.h>
#include <bgame/bg_local.h>
#include <cgame_mp/cg_local_mp.h>
#include <bgame/bg_misc.h>
#include <qcommon/common.h>
#include <game_mp/g_main_mp.h>

const dvar_t *turret_placement_debug;
const dvar_t *turret_placement_feet_offset;
const dvar_t *turret_placement_feet_trace_dist_z;
const dvar_t *turret_placement_trace_maxs;
const dvar_t *turret_placement_trace_mins;
const dvar_t *turret_placement_trace_radius_canon_safety;
const dvar_t *turret_placement_trace_dist;
const dvar_t *turret_placement_trace_pitch;
const dvar_t *turret_placement_trace_min_normal;

void(__cdecl *turretTraceHandlers[2])(trace_t *, const float *, const float *, const float *, const float *, int, int, col_context_t *)  { &CG_TraceCapsule, &G_TraceCapsule };


void __cdecl Turret_PlaceTurret_RegisterDvars()
{
    turret_placement_debug = _Dvar_RegisterBool("turret_placement_debug", 0, 0, "Enables turret placement debug lines");
    turret_placement_feet_offset = _Dvar_RegisterVec3(
                                                                     "turret_placement_feet_offset",
                                                                     (17.0),
                                                                     (20.0),
                                                                     (10.0),
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0,
                                                                     "Position of the feet from the center axis.");
    turret_placement_feet_trace_dist_z = _Dvar_RegisterFloat(
                                                                                 "turret_placement_feet_trace_dist_z",
                                                                                 20.0,
                                                                                 0.0,
                                                                                 3.4028235e38,
                                                                                 0,
                                                                                 "Max distance for a foot to be considered touching the ground");
    turret_placement_trace_maxs = _Dvar_RegisterFloat(
                                                                    "turret_placement_trace_maxs",
                                                                    30.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0,
                                                                    "maxs of the bound used for the placement trace");
    turret_placement_trace_mins = _Dvar_RegisterFloat(
                                                                    "turret_placement_trace_mins",
                                                                    -30.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0,
                                                                    "mins of the bound used for the placement trace");
    turret_placement_trace_radius_canon_safety = _Dvar_RegisterFloat(
                                                                                                 "turret_placement_trace_radius_canon_safety",
                                                                                                 5.0,
                                                                                                 0.0,
                                                                                                 3.4028235e38,
                                                                                                 0,
                                                                                                 "Extra radius used in the forward direction to compensate for the canon length");
    turret_placement_trace_dist = _Dvar_RegisterFloat(
                                                                    "turret_placement_trace_dist",
                                                                    42.0,
                                                                    0.0,
                                                                    3.4028235e38,
                                                                    0,
                                                                    "Distance along the trace axis where the turret will attempt to position itself");
    turret_placement_trace_pitch = _Dvar_RegisterFloat(
                                                                     "turret_placement_trace_pitch",
                                                                     20.0,
                                                                     0.0,
                                                                     3.4028235e38,
                                                                     0,
                                                                     "Pitch used for the trace axis");
    turret_placement_trace_min_normal = _Dvar_RegisterFloat(
                                                                                "turret_placement_trace_min_normal",
                                                                                0.69999999,
                                                                                0.0,
                                                                                3.4028235e38,
                                                                                0,
                                                                                "Minimum normal to accept a turret position");
}

int __cdecl Turret_PlaceTurret_Internal(
                turret_handler handler,
                int clientNum,
                unsigned int moverEntNum,
                const float *moverOrigin,
                const float *viewOrigin,
                const float *viewAngles,
                float *outOrigin,
                float *outAngles)
{
    float traceDir[3]; // [esp+2Ch] [ebp-D8h] BYREF
    col_context_t context; // [esp+38h] [ebp-CCh] BYREF
    float mins[3]; // [esp+60h] [ebp-A4h] BYREF
    float traceAngles[3]; // [esp+6Ch] [ebp-98h] BYREF
    trace_t trace; // [esp+78h] [ebp-8Ch] BYREF
    float maxs[3]; // [esp+B4h] [ebp-50h] BYREF
    float traceDist; // [esp+C0h] [ebp-44h]
    float axis[3][3]; // [esp+C4h] [ebp-40h] BYREF
    unsigned __int16 hitEntId; // [esp+E8h] [ebp-1Ch]
    float traceStart[3]; // [esp+ECh] [ebp-18h] BYREF
    float traceEnd[3]; // [esp+F8h] [ebp-Ch] BYREF

    if ( moverEntNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp",
                    231,
                    0,
                    "moverEntNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    moverEntNum,
                    32) )
    {
        __debugbreak();
    }
    if ( !outOrigin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 232, 0, "%s", "outOrigin") )
    {
        __debugbreak();
    }
    if ( !outAngles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 233, 0, "%s", "outAngles") )
    {
        __debugbreak();
    }
    *outAngles = 0.0f;
    outAngles[1] = 0.0f;
    outAngles[2] = 0.0f;
    outAngles[1] = viewAngles[1];
    traceDist = turret_placement_trace_dist->current.value;
    traceDist = traceDist + turret_placement_trace_radius_canon_safety->current.value;
    traceAngles[0] = turret_placement_trace_pitch->current.value;
    traceAngles[1] = viewAngles[1];
    traceAngles[2] = 0.0f;
    AngleVectors(traceAngles, traceDir, 0, 0);
    traceStart[0] = *viewOrigin;
    traceStart[1] = viewOrigin[1];
    traceStart[2] = viewOrigin[2];
    traceStart[0] = (float)(traceDist * traceDir[0]) + traceStart[0];
    traceStart[1] = (float)(traceDist * traceDir[1]) + traceStart[1];
    traceStart[2] = (float)(traceDist * traceDir[2]) + traceStart[2];
    traceEnd[0] = traceStart[0];
    traceEnd[1] = traceStart[1];
    traceEnd[2] = traceStart[2];
    traceEnd[2] = traceStart[2] - (float)(viewOrigin[2] - moverOrigin[2]);
    memset(mins, 0, sizeof(mins));
    mins[0] = turret_placement_trace_mins->current.value;
    mins[1] = mins[0];
    mins[2] = mins[0];
    memset(maxs, 0, sizeof(maxs));
    maxs[0] = turret_placement_trace_maxs->current.value;
    maxs[1] = maxs[0];
    maxs[2] = maxs[0];
    if ( turret_placement_debug->current.enabled )
    {
        G_DebugLine(traceStart, traceEnd, colorBlue, 0);
        CG_DebugBox(traceStart, mins, maxs, viewAngles[1], colorYellow, 0, 0);
        CG_DebugBox(traceEnd, mins, maxs, viewAngles[1], colorGreen, 0, 0);
    }
    ////col_context_t::col_context_t(&context);
    Turret_PlaceTurret_Trace(handler, &trace, traceStart, mins, maxs, traceEnd, moverEntNum, 0x2818011, &context);
    Vec3Lerp(traceStart, traceEnd, trace.fraction, outOrigin);
    if ( trace.startsolid
        || trace.allsolid
        || trace.fraction < 0.000099999997
        || turret_placement_trace_min_normal->current.value > trace.normal.vec.v[2] )
    {
        outOrigin[2] = moverOrigin[2];
        return 0;
    }
    else
    {
        outOrigin[2] = outOrigin[2] - turret_placement_trace_maxs->current.value;
        hitEntId = Trace_GetEntityHitId(&trace);
        if ( hitEntId >= 0x400u
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp",
                        280,
                        0,
                        "hitEntId doesn't index MAX_GENTITIES\n\t%i not in [0, %i)",
                        hitEntId,
                        1024) )
        {
            __debugbreak();
        }
        if ( hitEntId == 1022 )
        {
            *(_QWORD *)&axis[0][0] = *(_QWORD *)traceDir;
            axis[0][2] = traceDir[2];
            *(_QWORD *)&axis[2][0] = *(_QWORD *)trace.normal.vec.v;
            LODWORD(axis[2][2]) = trace.normal.vec.u[2];
            Vec3Cross(axis[2], axis[0], axis[1]);
            Vec3Cross(axis[1], axis[2], axis[0]);
            AxisToAngles(axis, outAngles);
            if ( Turret_PlaceTurret_UpdateFooting(handler, moverEntNum, &trace, outOrigin, axis) )
            {
                AxisToAngles(axis, outAngles);
                return 1;
            }
            else
            {
                return 0;
            }
        }
        else
        {
            return 0;
        }
    }
}

void __cdecl Turret_PlaceTurret_Trace(
                turret_handler handler,
                trace_t *results,
                const float *start,
                const float *mins,
                const float *maxs,
                const float *end,
                int passEntityNum,
                int contentmask,
                col_context_t *context)
{
    if ( (unsigned int)handler >= TURRET_HANDLER_COUNT
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp",
                    91,
                    0,
                    "handler doesn't index TURRET_HANDLER_COUNT\n\t%i not in [0, %i)",
                    handler,
                    2) )
    {
        __debugbreak();
    }
    turretTraceHandlers[handler](results, start, mins, maxs, end, passEntityNum, contentmask, context);
}

bool __cdecl Turret_PlaceTurret_UpdateFooting(
                turret_handler handler,
                int moverEntNum,
                trace_t *trace,
                float *outOrigin,
                float (*outAxis)[3])
{
    float v6; // [esp+0h] [ebp-160h]
    float *v7; // [esp+34h] [ebp-12Ch]
    float v8; // [esp+3Ch] [ebp-124h]
    float v9; // [esp+48h] [ebp-118h]
    float v10; // [esp+54h] [ebp-10Ch]
    float v11; // [esp+64h] [ebp-FCh]
    float v12; // [esp+70h] [ebp-F0h]
    float v13; // [esp+78h] [ebp-E8h]
    float v14; // [esp+80h] [ebp-E0h]
    float value; // [esp+84h] [ebp-DCh]
    col_context_t context; // [esp+B8h] [ebp-A8h] BYREF
    float feetNormal[3]; // [esp+E0h] [ebp-80h] BYREF
    float feetAxis[2][3]; // [esp+ECh] [ebp-74h] BYREF
    unsigned int legIndex; // [esp+104h] [ebp-5Ch]
    unsigned int numFailed; // [esp+108h] [ebp-58h]
    float legPosition[4][3]; // [esp+10Ch] [ebp-54h] BYREF
    float feetElevation[4]; // [esp+13Ch] [ebp-24h]
    float feetHeight; // [esp+14Ch] [ebp-14h]
    float traceEnd[3]; // [esp+150h] [ebp-10h] BYREF
    bool feetSuccess[4]; // [esp+15Ch] [ebp-4h]

    if ( !trace
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 139, 0, "%s", "trace") )
    {
        __debugbreak();
    }
    if ( !outOrigin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 140, 0, "%s", "outOrigin") )
    {
        __debugbreak();
    }
    if ( !outAxis
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 141, 0, "%s", "outAxis") )
    {
        __debugbreak();
    }
    nanassertvec3(outOrigin);

    nanassertvec3(outAxis[0]);
    nanassertvec3(outAxis[1]);
    nanassertvec3(outAxis[2]);

    value = turret_placement_feet_offset->current.value;
    legPosition[0][0] = (float)(value * (*outAxis)[0]) + *outOrigin;
    legPosition[0][1] = (float)(value * (*outAxis)[1]) + outOrigin[1];
    legPosition[0][2] = (float)(value * (*outAxis)[2]) + outOrigin[2];
    v14 = turret_placement_feet_offset->current.vector[1];
    legPosition[0][0] = (float)(v14 * (*outAxis)[3]) + legPosition[0][0];
    legPosition[0][1] = (float)(v14 * (*outAxis)[4]) + legPosition[0][1];
    legPosition[0][2] = (float)(v14 * (*outAxis)[5]) + legPosition[0][2];
    v13 = turret_placement_feet_offset->current.vector[2];
    legPosition[0][0] = (float)(v13 * (*outAxis)[6]) + legPosition[0][0];
    legPosition[0][1] = (float)(v13 * (*outAxis)[7]) + legPosition[0][1];
    legPosition[0][2] = (float)(v13 * (*outAxis)[8]) + legPosition[0][2];
    v12 = turret_placement_feet_offset->current.value * -2.0;
    legPosition[1][0] = (float)(v12 * (*outAxis)[0]) + legPosition[0][0];
    legPosition[1][1] = (float)(v12 * (*outAxis)[1]) + legPosition[0][1];
    legPosition[1][2] = (float)(v12 * (*outAxis)[2]) + legPosition[0][2];
    v11 = turret_placement_feet_offset->current.vector[1] * -2.0;
    legPosition[2][0] = (float)(v11 * (*outAxis)[3]) + legPosition[1][0];
    legPosition[2][1] = (float)(v11 * (*outAxis)[4]) + legPosition[1][1];
    legPosition[2][2] = (float)(v11 * (*outAxis)[5]) + legPosition[1][2];
    v10 = turret_placement_feet_offset->current.value * 2.0;
    legPosition[3][0] = (float)(v10 * (*outAxis)[0]) + legPosition[2][0];
    legPosition[3][1] = (float)(v10 * (*outAxis)[1]) + legPosition[2][1];
    legPosition[3][2] = (float)(v10 * (*outAxis)[2]) + legPosition[2][2];
    numFailed = 0;
    for ( legIndex = 0; legIndex < 4; ++legIndex )
    {
        v9 = -1.0 * turret_placement_feet_trace_dist_z->current.value;
        traceEnd[0] = (float)(v9 * (*outAxis)[6]) + legPosition[legIndex][0];
        traceEnd[1] = (float)(v9 * (*outAxis)[7]) + legPosition[legIndex][1];
        traceEnd[2] = (float)(v9 * (*outAxis)[8]) + legPosition[legIndex][2];
        ////col_context_t::col_context_t(&context);
        Turret_PlaceTurret_Trace(
            handler,
            trace,
            legPosition[legIndex],
            vec3_origin,
            vec3_origin,
            traceEnd,
            moverEntNum,
            0x2818011,
            &context);
        if ( trace->hitType )
        {
            feetSuccess[legIndex] = 1;
            feetElevation[legIndex] = (float)(trace->fraction * turret_placement_feet_trace_dist_z->current.value)
                                                            - turret_placement_feet_offset->current.vector[2];
            if ( turret_placement_debug->current.enabled )
                G_DebugLine(legPosition[legIndex], traceEnd, colorGreen, 0);
        }
        else
        {
            ++numFailed;
            feetSuccess[legIndex] = 0;
            feetElevation[legIndex] = 0.0f;
            if ( turret_placement_debug->current.enabled )
                G_DebugLine(legPosition[legIndex], traceEnd, colorRed, 0);
        }
        v7 = legPosition[legIndex];
        v8 = -1.0 * feetElevation[legIndex];
        *v7 = (float)(v8 * (*outAxis)[6]) + *v7;
        v7[1] = (float)(v8 * (*outAxis)[7]) + v7[1];
        v7[2] = (float)(v8 * (*outAxis)[8]) + v7[2];
    }
    for ( legIndex = 0; legIndex < 4; ++legIndex )
    {
        if ( !feetSuccess[legIndex] )
            feetElevation[legIndex] = feetElevation[(legIndex + 2) % 4];
    }
    if ( (float)((float)((float)((float)(1.0 - 0.5) * feetElevation[0]) + (float)(feetElevation[2] * 0.5))
                         - (float)((float)((float)(1.0 - 0.5) * feetElevation[1]) + (float)(feetElevation[3] * 0.5))) < 0.0 )
        v6 = (float)((float)(1.0 - 0.5) * feetElevation[1]) + (float)(feetElevation[3] * 0.5);
    else
        v6 = (float)((float)(1.0 - 0.5) * feetElevation[0]) + (float)(feetElevation[2] * 0.5);
    feetHeight = v6;
    *outOrigin = (float)((float)(-1.0 * v6) * (*outAxis)[6]) + *outOrigin;
    outOrigin[1] = (float)((float)(-1.0 * v6) * (*outAxis)[7]) + outOrigin[1];
    outOrigin[2] = (float)((float)(-1.0 * v6) * (*outAxis)[8]) + outOrigin[2];
    feetAxis[0][0] = legPosition[0][0] - legPosition[2][0];
    feetAxis[0][1] = legPosition[0][1] - legPosition[2][1];
    feetAxis[0][2] = legPosition[0][2] - legPosition[2][2];
    feetAxis[1][0] = legPosition[1][0] - legPosition[3][0];
    feetAxis[1][1] = legPosition[1][1] - legPosition[3][1];
    feetAxis[1][2] = legPosition[1][2] - legPosition[3][2];
    Vec3Cross(feetAxis[0], feetAxis[1], feetNormal);
    Vec3Normalize(feetNormal);
    (*outAxis)[6] = feetNormal[0];
    (*outAxis)[7] = feetNormal[1];
    (*outAxis)[8] = feetNormal[2];
    Vec3Cross(&(*outAxis)[6], (const float *)outAxis, &(*outAxis)[3]);
    Vec3Cross(&(*outAxis)[3], &(*outAxis)[6], (float *)outAxis);
    return numFailed <= 1;
}

int __cdecl Turret_PlaceTurret_Client(int localClientNum, const entityState_s *es, float *outOrigin, float *outAngles)
{
    int clientNum; // eax
    cg_s *cgameGlob; // [esp+2Ch] [ebp-38h]
    centity_s *carrierEnt; // [esp+30h] [ebp-34h]
    float viewOrg[3]; // [esp+34h] [ebp-30h] BYREF
    clientInfo_t *ci; // [esp+40h] [ebp-24h]
    int canPlace; // [esp+44h] [ebp-20h]
    playerState_s *ps; // [esp+48h] [ebp-1Ch]
    float moverOrigin[3]; // [esp+4Ch] [ebp-18h] BYREF
    float viewAngles[3]; // [esp+58h] [ebp-Ch] BYREF

    if ( !es && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 330, 0, "%s", "es") )
        __debugbreak();
    if ( !outOrigin
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 331, 0, "%s", "outOrigin") )
    {
        __debugbreak();
    }
    if ( !outAngles
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 332, 0, "%s", "outAngles") )
    {
        __debugbreak();
    }
    if ( (es->lerp.u.turret.flags & 8) == 0 )
        return 0;
    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( !cgameGlob
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 338, 0, "%s", "cgameGlob") )
    {
        __debugbreak();
    }
    ps = &cgameGlob->predictedPlayerState;
    if ( (unsigned int)es->otherEntityNum >= 0x20
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp",
                    341,
                    0,
                    "es->otherEntityNum doesn't index MAX_CLIENTS\n\t%i not in [0, %i)",
                    es->otherEntityNum,
                    32) )
    {
        __debugbreak();
    }
    if ( es->otherEntityNum == ps->clientNum && (cgameGlob->renderingThirdPerson == TP_OFF || Demo_IsPlaying()) )
    {
        BG_GetPlayerViewOrigin(ps, viewOrg, cgameGlob->time);
        viewOrg[2] = (float)(60.0 - ps->viewHeightCurrent) + viewOrg[2];
        viewAngles[0] = ps->viewangles[0];
        viewAngles[1] = ps->viewangles[1];
        viewAngles[2] = ps->viewangles[2];
        moverOrigin[0] = ps->origin[0];
        moverOrigin[1] = ps->origin[1];
        moverOrigin[2] = ps->origin[2];
    }
    else
    {
        if ( es->otherEntityNum == ps->clientNum )
            carrierEnt = &cgameGlob->predictedPlayerEntity;
        else
            carrierEnt = CG_GetEntity(localClientNum, es->otherEntityNum);
        if ( !carrierEnt
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp",
                        368,
                        1,
                        "%s",
                        "carrierEnt") )
        {
            __debugbreak();
        }
        clientNum = es->clientNum;
        ci = &cgameGlob->bgs.clientinfo[clientNum];
        if ( (cg_s *)((char *)cgameGlob + clientNum * 1480) == (cg_s *)-389096 || !ci->infoValid )
        {
            Com_PrintWarning(
                17,
                "Sentry [%d] isBeingCarried but the client info [%d] is invalid! Script should ensure that this doesn't happen.\n",
                es->number,
                es->otherEntityNum);
            return 0;
        }
        viewAngles[0] = ci->playerAngles[0];
        viewAngles[1] = ci->playerAngles[1];
        viewAngles[2] = ci->playerAngles[2];
        moverOrigin[0] = carrierEnt->pose.origin[0];
        moverOrigin[1] = carrierEnt->pose.origin[1];
        moverOrigin[2] = carrierEnt->pose.origin[2];
        viewOrg[0] = carrierEnt->pose.origin[0];
        viewOrg[1] = carrierEnt->pose.origin[1];
        viewOrg[2] = carrierEnt->pose.origin[2];
        viewOrg[2] = viewOrg[2] + 60.0;
        AddLeanToPosition(viewOrg, viewAngles[1], ci->lerpLean, 16.0, 20.0);
    }
    canPlace = Turret_PlaceTurret_Internal(
                             TURRET_HANDLER_CLIENT,
                             localClientNum,
                             es->otherEntityNum,
                             moverOrigin,
                             viewOrg,
                             viewAngles,
                             outOrigin,
                             outAngles);
    if ( (ps->pm_flags & 4) != 0 )
        return 0;
    return canPlace;
}

int __cdecl Turret_PlaceTurret_Server(playerState_s *ps, float *outOrigin, float *outAngles)
{
    unsigned int v3; // eax
    float viewOrg[3]; // [esp+0h] [ebp-10h] BYREF
    int canPlace; // [esp+Ch] [ebp-4h]

    if ( !ps && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\turret\\turret_placement.cpp", 450, 0, "%s", "ps") )
        __debugbreak();
    BG_GetPlayerViewOrigin(ps, viewOrg, level.time);
    viewOrg[2] = (float)(60.0 - ps->viewHeightCurrent) + viewOrg[2];
    canPlace = Turret_PlaceTurret_Internal(
                             TURRET_HANDLER_SERVER,
                             ps->clientNum,
                             ps->clientNum,
                             ps->origin,
                             viewOrg,
                             ps->viewangles,
                             outOrigin,
                             outAngles);
    if ( (ps->pm_flags & 4) != 0 )
        canPlace = 0;
    if ( canPlace )
        v3 = ps->weapFlags & 0xFF7FFFFF;
    else
        v3 = ps->weapFlags | 0x800000;
    ps->weapFlags = v3;
    return canPlace;
}

