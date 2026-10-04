#pragma once
// p1: headless player-feel evidence (TEST SWITCH bo1_feeltrace, headless local client only). See cg_sp_feeltrace.cpp.
struct weaponState_t;

bool CG_SP_FeelTraceEnabled();
void CG_SP_FeelTraceWeapon(int localClientNum, const weaponState_t *ws);
void CG_SP_FeelTraceKick(int localClientNum, int stage);
void CG_SP_FeelTraceView(int localClientNum, const float *gunOrigin, const float *gunAngles);
void CG_SP_FeelTraceAutoMelee(int localClientNum, bool meleeing, int meleeChargeDist, float meleeChargeYaw);
