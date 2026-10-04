// p1: headless player-feel evidence (TEST SWITCH bo1_feeltrace, headless local client only, no gameplay change).
// Every rendered frame that draws the view weapon prints the ADS fraction, the rendered FOV and the view-model sway
// state as a "bo1_feel:" line (after CG_SaveWeaponState, so the values are the ones this frame drew with);
// tools/p1_feel_check.mjs recomputes them with the SP formulas (CG_GetViewFov 0x00437EF0, CG_ViewModelSway
// 0x0076D800 / LerpTowards 0x00479EE0 / LerpAngleTowards 0x005E8EB0) and compares.
// bo1_feel_knife <range> turns the harness "player" toward the nearest live actor (plus bo1_feel_yawofs degrees,
// so the SP auto-melee convergence has something to do) and presses +melee when it is inside <range>: a player's
// mouse and knife key, nothing else. The aim assist, lunge, melee trace and damage are the game's own.
#include "cg_sp_feeltrace.h"
#include <cgame_mp/cg_local_mp.h>
#include <client/client.h>
#include <aim_assist/aim_assist.h>
#include <bgame/bg_weapons.h>
#include <bgame/bg_weapons_def.h>
#include <bgame/bg_local.h>
#include <qcommon/common.h>
#include <qcommon/cmd.h>
#include <universal/dvar.h>
#include <universal/com_math.h>
#include <win32/win_main.h>

extern AimAssistGlobals aaGlobArray[1]; // aim_assist.cpp

static const dvar_s *s_feelTrace;
static const dvar_s *s_feelKnife;
static const dvar_s *s_feelYawOfs;

bool CG_SP_FeelTraceEnabled()
{
    if (!Sys_IsHeadlessClient())
        return false;
    if (!s_feelTrace)
    {
        s_feelTrace = _Dvar_RegisterBool("bo1_feeltrace", false, 0,
            "TEST SWITCH (headless client): log rendered ADS/FOV/sway and auto-melee state every frame");
        s_feelKnife = _Dvar_RegisterFloat("bo1_feel_knife", 0.0f, 0.0f, 1000.0f, 0,
            "TEST SWITCH (headless client): face the nearest actor and press +melee inside this range (0 = off)");
        s_feelYawOfs = _Dvar_RegisterFloat("bo1_feel_yawofs", 0.0f, -45.0f, 45.0f, 0,
            "TEST SWITCH (headless client): yaw offset in degrees from the nearest actor while facing it");
    }
    return s_feelTrace->current.enabled;
}

static void CG_SP_FeelAsset(int weapon)
{
    const WeaponDef *def = BG_GetWeaponDef(weapon);
    const WeaponVariantDef *variant = BG_GetWeaponVariantDef(weapon);
    const dvar_s *fov = Dvar_FindVar("cg_fov");
    const dvar_s *fovScale = Dvar_FindVar("cg_fovScale");
    const dvar_s *fovMin = Dvar_FindVar("cg_fovMin");
    const dvar_s *toggleStyle = Dvar_FindVar("cg_adsZoomToggleStyle");
    Com_Printf(16, "bo1_feel_asset: w=%d name=%s ads=%d reticle=%d zoom=%.3f,%.3f,%.3f frac=%.3f,%.3f"
        " sway=%.3f,%.3f,%.3f,%.3f,%.3f,%.3f adssway=%.3f,%.3f,%.3f,%.3f,%.3f,%.3f ss=%.3f cgfov=%.3f,%.3f,%.3f toggle=%d\n",
        weapon, variant->szInternalName, def->aimDownSight ? 1 : 0, (int)def->overlayReticle,
        variant->fAdsZoomFov1, variant->fAdsZoomFov2, variant->fAdsZoomFov3, variant->fAdsZoomInFrac, variant->fAdsZoomOutFrac,
        def->swayMaxAngle, def->swayLerpSpeed, def->swayPitchScale, def->swayYawScale, def->swayHorizScale, def->swayVertScale,
        def->adsSwayMaxAngle, def->adsSwayLerpSpeed, def->adsSwayPitchScale, def->adsSwayYawScale,
        variant->fAdsSwayHorizScale, variant->fAdsSwayVertScale, def->swayShellShockScale,
        fov ? fov->current.value : -1.0f, fovScale ? fovScale->current.value : -1.0f, fovMin ? fovMin->current.value : -1.0f,
        toggleStyle ? toggleStyle->current.integer : -1);
}

// Nearest valid ET_ACTOR in the current snapshot (the aim target list's source).
static const centity_s *CG_SP_FeelNearestActor(int localClientNum, const cg_s *cgameGlob, float *distOut)
{
    const centity_s *best = nullptr;
    float bestDist = 1.0e9f;
    if (!cgameGlob->nextSnap)
        return nullptr;
    for (int i = 0; i < cgameGlob->nextSnap->numEntities; ++i)
    {
        const centity_s *cent = CG_GetEntity(localClientNum, cgameGlob->nextSnap->entities[i].number);
        if (cent->nextState.eType != ET_ACTOR || (cent->nextState.lerp.eFlags & 0x40000) != 0
            || ((*((const unsigned int *)cent + 201) >> 1) & 1) == 0)
            continue;
        const float dx = cent->pose.origin[0] - cgameGlob->predictedPlayerState.origin[0];
        const float dy = cent->pose.origin[1] - cgameGlob->predictedPlayerState.origin[1];
        const float dz = cent->pose.origin[2] - cgameGlob->predictedPlayerState.origin[2];
        const float dist = sqrtf(dx * dx + dy * dy + dz * dz);
        if (dist < bestDist)
        {
            bestDist = dist;
            best = cent;
        }
    }
    *distOut = bestDist;
    return best;
}

// Harness input: a mouse turn toward the nearest actor and a knife key press. Never touches playerState.
static void CG_SP_FeelKnifeInput(int localClientNum, const cg_s *cgameGlob)
{
    static int s_pressTime = -100000;
    static bool s_down;
    if (s_down && cgameGlob->time - s_pressTime >= 100)
    {
        Cbuf_AddText(0, "-melee\n");
        s_down = false;
    }
    if (!s_feelKnife || s_feelKnife->current.value <= 0.0f)
        return;
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    const AimAssistGlobals *aaGlob = &aaGlobArray[localClientNum];
    if (ps->weaponstate >= 17 && ps->weaponstate <= 19)
        return; // meleeing: the SP auto-melee convergence owns the view now
    if (aaGlob->autoMeleeState != AMS_NOT_ACTIVE)
        return;
    float dist;
    const centity_s *actor = CG_SP_FeelNearestActor(localClientNum, cgameGlob, &dist);
    if (!actor || dist > 600.0f)
        return;
    float dir[3] = {actor->pose.origin[0] - aaGlob->viewOrigin[0], actor->pose.origin[1] - aaGlob->viewOrigin[1],
        actor->pose.origin[2] + 40.0f - aaGlob->viewOrigin[2]};
    float angles[3];
    vectoangles(dir, angles);
    clientActive_t *cl = CL_GetLocalClientGlobals(localClientNum);
    cl->viewangles[0] = AngleNormalize360(angles[0] - ps->delta_angles[0]);
    cl->viewangles[1] = AngleNormalize360(angles[1] + s_feelYawOfs->current.value - ps->delta_angles[1]);
    if (dist <= s_feelKnife->current.value && !s_down && cgameGlob->time - s_pressTime >= 1500 && ps->weaponstate == 0)
    {
        Com_Printf(16, "bo1_feel_knife: t=%d press ent=%d dist=%.3f yawofs=%.3f org=%.3f,%.3f,%.3f\n",
            cgameGlob->time, actor->nextState.number, dist, s_feelYawOfs->current.value,
            ps->origin[0], ps->origin[1], ps->origin[2]);
        Cbuf_AddText(0, "+melee\n");
        s_down = true;
        s_pressTime = cgameGlob->time;
    }
}

void CG_SP_FeelTraceWeapon(int localClientNum, const weaponState_t *ws)
{
    if (!CG_SP_FeelTraceEnabled())
        return;
    static int s_lastWeapon = -1;
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    const int weapon = BG_GetViewmodelWeaponIndex(ps);
    if (weapon != s_lastWeapon)
    {
        s_lastWeapon = weapon;
        CG_SP_FeelAsset(weapon);
    }
    Com_Printf(16, "bo1_feel: t=%d ft=%.6f w=%d st=%d pos=%.6f toads=%d zs=%d fov=%.5f va=%.5f,%.5f,%.5f"
        " sa=%.6f,%.6f so=%.6f,%.6f sv=%.5f,%.5f,%.5f ss=%d,%d,%d,%d eflags=0x%x\n",
        cgameGlob->time, ws->frametime, weapon, ps->weaponstate, ps->fWeaponPosFrac,
        cgameGlob->playerEntity.bPositionToADS, ps->adsZoomSelect, cgameGlob->refdef.fov_x,
        ps->viewangles[0], ps->viewangles[1], ps->viewangles[2], ws->swayAngles[0], ws->swayAngles[1],
        ws->swayOrigin[1], ws->swayOrigin[2], ws->swayViewAngles[0], ws->swayViewAngles[1], ws->swayViewAngles[2],
        ws->shellShockStart, ws->shellShockDuration, ws->time, ws->shellShockFadeTime, ps->eFlags);
    CG_SP_FeelKnifeInput(localClientNum, cgameGlob);
}

// p1 c29: the first-person view, every drawn frame (TEST SWITCH bo1_viewtrace, headless local client only): the
// inputs of the view chain (pm type/flags, stance target and current view height, bob cycle, speed, ADS fraction,
// land and damage state, kick angles) and its outputs (camera offset from the player origin, refdef view angles,
// fov, gun origin minus the camera in view axes, and the gun's local angles before they are composed with the view). tools/p1_view_check.mjs reads it.
void CG_SP_FeelTraceView(int localClientNum, const float *gunOrigin, const float *gunAngles)
{
    static const dvar_s *s_viewTrace;
    if (!Sys_IsHeadlessClient())
        return;
    if (!s_viewTrace)
        s_viewTrace = _Dvar_RegisterBool("bo1_viewtrace", false, 0,
            "TEST SWITCH (headless client): log the first-person camera, fov and gun placement every drawn frame");
    if (!s_viewTrace->current.enabled)
        return;
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    const refdef_s *rd = &cgameGlob->refdef;
    float d[3], g[3] = { 0.0f, 0.0f, 0.0f }, ga[3] = { 0.0f, 0.0f, 0.0f };
    Vec3Sub(rd->vieworg, ps->origin, d);
    if (gunOrigin)
    {
        float r[3];
        Vec3Sub(gunOrigin, rd->vieworg, r);
        for (int i = 0; i < 3; ++i)
        {
            g[i] = Vec3Dot(r, rd->viewaxis[i]);
            ga[i] = gunAngles[i];
        }
    }
    Com_Printf(16, "bo1_view: t=%d ft=%d pmt=%d pmf=0x%x ef=0x%x vht=%d vh=%.4f bob=%d fb=%.5f xy=%.3f pos=%.4f"
        " org=%.3f,%.3f,%.3f vel=%.2f,%.2f,%.2f d=%.4f,%.4f,%.4f rva=%.4f,%.4f,%.4f pva=%.4f,%.4f,%.4f"
        " land=%.3f,%d dmg=%d,%.3f,%.3f kick=%.4f,%.4f,%.4f step=%.3f,%d fov=%.4f,%.5f,%.5f"
        " g=%.4f,%.4f,%.4f ga=%.4f,%.4f,%.4f w=%d\n",
        cgameGlob->time, cgameGlob->frametime, ps->pm_type, ps->pm_flags, ps->eFlags, ps->viewHeightTarget,
        ps->viewHeightCurrent, ps->bobCycle, cgameGlob->fBobCycle, cgameGlob->xyspeed, ps->fWeaponPosFrac,
        ps->origin[0], ps->origin[1], ps->origin[2], ps->velocity[0], ps->velocity[1], ps->velocity[2],
        d[0], d[1], d[2], cgameGlob->refdefViewAngles[0], cgameGlob->refdefViewAngles[1],
        cgameGlob->refdefViewAngles[2], ps->viewangles[0], ps->viewangles[1], ps->viewangles[2],
        cgameGlob->landChange, cgameGlob->landTime, cgameGlob->damageTime, cgameGlob->v_dmg_pitch,
        cgameGlob->v_dmg_roll, cgameGlob->kickAngles[0], cgameGlob->kickAngles[1], cgameGlob->kickAngles[2],
        cgameGlob->stepViewChange, cgameGlob->stepViewStart, rd->fov_x, rd->tanHalfFovX, rd->tanHalfFovY,
        g[0], g[1], g[2], ga[0], ga[1], ga[2], BG_GetViewmodelWeaponIndex(ps));
    // L10: the link / freeze inputs of the same frame (thief grab: FreezeControls + PlayerLinkTo a rotating
    // script_origin): delta and link angles, link flags, clamp, the snapshot pair and interpolation fraction the
    // predicted state was built from, the position error being decayed. tools/L10_steal_view.mjs reads it.
    static const dvar_s *s_viewTraceLink;
    if (!s_viewTraceLink)
        s_viewTraceLink = _Dvar_RegisterBool("bo1_viewtrace_link", false, 0,
            "TEST SWITCH (headless client, with bo1_viewtrace): also log the link / freeze state every drawn frame");
    if (!s_viewTraceLink->current.enabled)
        return;
    Com_Printf(16, "bo1_viewlink: t=%d pmt=%d pmf=0x%x lf=0x%x da=%.4f,%.4f,%.4f la=%.4f,%.4f,%.4f"
        " cb=%.3f,%.3f cr=%.3f,%.3f sn=%d nsn=%d fi=%.4f pe=%.3f,%.3f,%.3f pet=%d cmd=%d cva=%.4f,%.4f spf=0x%x\n",
        cgameGlob->time, ps->pm_type, ps->pm_flags, ps->linkFlags, ps->delta_angles[0], ps->delta_angles[1],
        ps->delta_angles[2], ps->linkAngles[0], ps->linkAngles[1], ps->linkAngles[2], ps->viewAngleClampBase[0],
        ps->viewAngleClampBase[1], ps->viewAngleClampRange[0], ps->viewAngleClampRange[1],
        cgameGlob->snap ? cgameGlob->snap->serverTime : -1, cgameGlob->nextSnap ? cgameGlob->nextSnap->serverTime : -1,
        cgameGlob->frameInterpolation, cgameGlob->predictedError[0], cgameGlob->predictedError[1],
        cgameGlob->predictedError[2], cgameGlob->predictedErrorTime, ps->commandTime,
        CL_GetLocalClientGlobals(localClientNum)->viewangles[0], CL_GetLocalClientGlobals(localClientNum)->viewangles[1],
        CL_GetLocalClientGlobals(localClientNum)->snap.ps.pm_flags);
}

// w1: view kick evidence. stage 0 = before CG_KickAngles (its inputs), 1 = after it, 2 = after BG_WeaponFireRecoil
// (the shot's kick impulse). tools/w1_kick_check.mjs recomputes SP CG_ViewKickUpdate (0x007919F0) from stage 0 and
// checks each impulse against SP Weapon_ViewKick (0x005676C0) ranges from the bo1_kick_asset line.
void CG_SP_FeelTraceKick(int localClientNum, int stage)
{
    if (!CG_SP_FeelTraceEnabled())
        return;
    const cg_s *cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    const playerState_s *ps = &cgameGlob->predictedPlayerState;
    const int weapon = BG_GetViewmodelWeaponIndex(ps);
    static bool s_stage0Printed;
    if (stage == 0)
        s_stage0Printed = cgameGlob->kickAVel[0] != 0.0f || cgameGlob->kickAVel[1] != 0.0f || cgameGlob->kickAVel[2] != 0.0f
            || cgameGlob->kickAngles[0] != 0.0f || cgameGlob->kickAngles[1] != 0.0f || cgameGlob->kickAngles[2] != 0.0f;
    if (stage != 2 && !s_stage0Printed)
        return; // nothing to integrate: CG_KickAngles leaves both zero
    static int s_lastWeapon = -1;
    if (stage == 2 && weapon != s_lastWeapon)
    {
        s_lastWeapon = weapon;
        const WeaponDef *def = BG_GetWeaponDef(weapon);
        const WeaponVariantDef *variant = BG_GetWeaponVariantDef(weapon);
        Com_Printf(16, "bo1_kick_asset: w=%d name=%s type=%d dual=%d hip=%.9g,%.9g,%.9g,%.9g ads=%.9g,%.9g,%.9g,%.9g"
            " center=%.9g,%.9g reduce=%.9g,%.9g\n",
            weapon, variant->szInternalName, (int)def->weapType, def->bDualWield ? 1 : 0,
            def->fHipViewKickPitchMin, def->fHipViewKickPitchMax, def->fHipViewKickYawMin, def->fHipViewKickYawMax,
            def->fAdsViewKickPitchMin, def->fAdsViewKickPitchMax, def->fAdsViewKickYawMin, def->fAdsViewKickYawMax,
            variant->fHipViewKickCenterSpeed, variant->fAdsViewKickCenterSpeed,
            def->hipGunKickReducedKickPercent, def->adsGunKickReducedKickPercent);
        if (def->bDualWield)
        {
            const WeaponDef *left = BG_GetWeaponDef(def->dualWieldWeaponIndex);
            Com_Printf(16, "bo1_kick_asset: w=%d left=%d lhip=%.9g,%.9g,%.9g,%.9g lcenter=%.9g\n", weapon,
                def->dualWieldWeaponIndex, left->fHipViewKickPitchMin, left->fHipViewKickPitchMax, left->fHipViewKickYawMin,
                left->fHipViewKickYawMax, BG_GetWeaponVariantDef(def->dualWieldWeaponIndex)->fHipViewKickCenterSpeed);
        }
    }
    Com_Printf(16, "bo1_kick: t=%d s=%d ft=%d w=%d ws=%d wsl=%d pos=%.9g rk=%d ka=%.9g,%.9g,%.9g kv=%.9g,%.9g,%.9g"
        " va=%.5f,%.5f\n",
        cgameGlob->time, stage, cgameGlob->frametime, weapon, ps->weaponstate, ps->weaponstateLeft, ps->fWeaponPosFrac,
        ps->weaponRestrictKickTime, cgameGlob->kickAngles[0], cgameGlob->kickAngles[1], cgameGlob->kickAngles[2],
        cgameGlob->kickAVel[0], cgameGlob->kickAVel[1], cgameGlob->kickAVel[2], ps->viewangles[0], ps->viewangles[1]);
}

void CG_SP_FeelTraceAutoMelee(int localClientNum, bool meleeing, int meleeChargeDist, float meleeChargeYaw)
{
    if (!CG_SP_FeelTraceEnabled())
        return;
    const AimAssistGlobals *aaGlob = &aaGlobArray[localClientNum];
    if (aaGlob->autoMeleeState == AMS_NOT_ACTIVE && !meleeing)
        return;
    Com_Printf(16, "bo1_feel_am: t=%d edge=%d state=%d ent=%d ws=%d targets=%d dist=%d yaw=%.4f"
        " p=%.4f,%.4f y=%.4f,%.4f view=%.4f,%.4f org=%.3f,%.3f,%.3f\n",
        CG_GetLocalClientGlobals(localClientNum)->time, meleeing ? 1 : 0, (int)aaGlob->autoMeleeState,
        aaGlob->autoMeleeTargetEnt, aaGlob->ps.weaponstate, aaGlob->screenTargetCount, meleeChargeDist, meleeChargeYaw,
        aaGlob->autoMeleePitch, aaGlob->autoMeleePitchTarget, aaGlob->autoMeleeYaw, aaGlob->autoMeleeYawTarget,
        aaGlob->viewAngles[0], aaGlob->viewAngles[1], aaGlob->viewOrigin[0], aaGlob->viewOrigin[1], aaGlob->viewOrigin[2]);
}
