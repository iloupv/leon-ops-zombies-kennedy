#pragma once

struct pmove_t;
struct entityState_s;
struct clientInfo_t;

void __cdecl BG_LocalEvalPlayerAnimType(pmove_t *pm);
void __cdecl BG_EvalPlayerAnimType(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalNextPlayerAnimType(pmove_t *pm);
void __cdecl BG_LocalEvalWeaponClass(pmove_t *pm);
void __cdecl BG_EvalWeaponClass(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalNextWeaponClass(pmove_t *pm);
void __cdecl BG_LocalEvalMounted(pmove_t *pm);
void __cdecl BG_EvalMounted(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalMoveStatus(pmove_t *pm);
void __cdecl BG_EvalMoveStatus(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalDirection(pmove_t *pm);
void __cdecl BG_EvalDirection(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalDmgDirection(pmove_t *pm);
void __cdecl BG_EvalDmgDirection(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalDmgType(pmove_t *pm);
void __cdecl BG_LocalEvalStance(pmove_t *pm);
void __cdecl BG_EvalStance(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalUnderhand(pmove_t *pm);
void __cdecl BG_EvalUnderhand(const entityState_s *es, const clientInfo_t *ci);
void __cdecl BG_LocalEvalFiring(pmove_t *pm);
void __cdecl BG_EvalFiring(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalWeaponPosition(pmove_t *pm);
void __cdecl BG_EvalWeaponPosition(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalSlope(pmove_t *pm);
void __cdecl BG_EvalSlope(const entityState_s *es, const clientInfo_t *);
void __cdecl BG_LocalEvalPerk(pmove_t *pm);
void __cdecl BG_LocalEvalAttachment(pmove_t *pm);
void __cdecl BG_LocalEvalVehicleName(pmove_t *pm);
void __cdecl BG_LocalEvalVehicleSeatTo(pmove_t *pm);
void __cdecl BG_LocalEvalEvent(pmove_t *pm);
