#include "actor.h"

#include "actor_dog_exposed.h"
#include "actor_generic.h"
#include "actor_exposed.h"
#include "actor_zombie_exposed.h"
#include "actor_zombie_dog_exposed.h"
#include "actor_scripted.h"
#include "actor_death.h"
#include "actor_pain.h"
#include "actor_negotiation.h"
#include "actor_badplace.h"
#include <game_sp/actor_sp_ext.h>

// zombies: P6a - SP's AIFuncTable (.data 0x00b750d8) points at four 12-state tables:
//   species 0 human 0x00a519e8, 1 dog 0x00a51e68, 2 zombie 0x00a51b68, 3 zombie_dog 0x00a51ce8
// (MEASURED: the dispatchers 0x005cc800 / 0x006948e0 read [0x00b750d8 + species*4], species at
// actor+8). Each SP row is 8 pointers; the 8th (pfnReact) is recorded in actor.h, not stored.
// A slot points at a KB function only where the SP slot is the same function as the SP dog-family
// slot KB's MP table already maps. Slots SP fills and KB has not ported are Actor_SP_NotPorted_*
// (loud ERR_DROP), with the SP address in the comment. NULL where SP is NULL.
//
// SP pointer -> KB function, shared by every row that uses it:
//   0x006948e0 Actor_Generic_Suspend    0x005cc800 Actor_Generic_Resume
//   0x005fc750 Actor_Generic_Touch      0x0050dd40 Actor_Generic_Pain (both `ret`)
//   0x00565940 Actor_Exposed_Touch      0x00651a30 Actor_BadPlace_Flee_Finish (empty)
//   0x0068b610 / 0x0052f820 Actor_BadPlace_Flee_Start / _Think
//   0x00557ac0 / 0x004eb640 Actor_Death_Start / _Think
//   0x0059e7d0 / 0x00461890 / 0x00583de0 Actor_Pain_Start / _Finish / _Think
//   0x0052be40 / 0x005189a0 Actor_Negotiation_Start / _Think
//   0x005d9430 / 0x0050b5e0 Actor_Dog_Exposed_Start / _Finish (= _Suspend) (bytes compared)
#define AI_NOT_PORTED_ROW(touch, pain) \
  { Actor_SP_NotPorted_Start, Actor_SP_NotPorted_Finish, Actor_SP_NotPorted_Finish, \
    Actor_SP_NotPorted_Resume, Actor_SP_NotPorted_Think, touch, pain }

#define AI_BADPLACE_ROW \
  { Actor_BadPlace_Flee_Start, Actor_BadPlace_Flee_Finish, Actor_Generic_Suspend, Actor_Generic_Resume, \
    Actor_BadPlace_Flee_Think, Actor_Generic_Touch, Actor_Generic_Pain }
#define AI_DEATH_ROW \
  { Actor_Death_Start, Actor_BadPlace_Flee_Finish, Actor_Generic_Suspend, Actor_Generic_Resume, \
    Actor_Death_Think, Actor_Generic_Touch, Actor_Generic_Pain }
#define AI_PAIN_ROW \
  { Actor_Pain_Start, Actor_Pain_Finish, Actor_Generic_Suspend, Actor_Generic_Resume, \
    Actor_Pain_Think, Actor_Generic_Touch, Actor_Generic_Pain }
#define AI_NEGOTIATION_ROW \
  { Actor_Negotiation_Start, Actor_BadPlace_Flee_Finish, Actor_Generic_Suspend, Actor_Generic_Resume, \
    Actor_Negotiation_Think, Actor_Generic_Touch, Actor_Generic_Pain }
// SP 9 scriptedanim: 0x00452e90 / 0x00620a70 / 0x006948e0 / 0x005cc800 / think 0x004f9f30 (P6d)
#define AI_SCRIPTEDANIM_ROW AI_NOT_PORTED_ROW(Actor_Generic_Touch, Actor_Generic_Pain)
// SP 10 customanim: 0x00631220 / 0x00651a30 / 0x006948e0 / 0x005cc800 / think 0x0040d530 (P6d)
#define AI_CUSTOMANIM_ROW AI_NOT_PORTED_ROW(Actor_Generic_Touch, Actor_Generic_Pain)
#define AI_ZOMBIE_SCRIPTEDANIM_ROW \
  { Actor_ScriptedAnim_Start_SP, Actor_ScriptedAnim_Finish_SP, Actor_Generic_Suspend, Actor_Generic_Resume, \
    Actor_ScriptedAnim_Think_SP, Actor_Generic_Touch, Actor_Generic_Pain }
#define AI_ZOMBIE_CUSTOMANIM_ROW \
  { Actor_CustomAnim_Start_SP, Actor_BadPlace_Flee_Finish, Actor_Generic_Suspend, Actor_Generic_Resume, \
    Actor_CustomAnim_Think_SP, Actor_Generic_Touch, Actor_Generic_Pain }
#define AI_NULL_ROW { NULL, NULL, NULL, NULL, NULL, NULL, NULL }

// species 0, SP 0x00a519e8. Every state is filled in SP; the human-only states are not ported.
const ai_funcs_t AIHumanFuncTable[AIS_COUNT] =
{
  AI_NULL_ROW,
  // 1 exposed: 0x0062a110 / 0x005f8970 / 0x006948e0 / 0x004544c0 / think 0x004eec10 / 0x00565940 / 0x0050dd40
  // x6: the zombie row's start/finish/suspend/resume are the same SP functions; the think is the human one.
  {
    Actor_Exposed_Start_SP,
    Actor_Exposed_Finish_SP,
    Actor_Generic_Suspend,
    Actor_Exposed_Resume_SP,
    Actor_Human_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  // 2 turret: 0x004ccb10 / 0x0042cad0 / 0x005731f0 / 0x005cc800 / think 0x00433250 / 0x00565940 / 0x0052a1b0
  AI_NOT_PORTED_ROW(Actor_Exposed_Touch, NULL),
  // 3 grenade_response: 0x0065b9c0 / 0x006589c0 / 0x006589c0 / 0x0043c140 / think 0x0065b2b0 / 0x005fc750 / 0x0050dd40
  AI_NOT_PORTED_ROW(Actor_Generic_Touch, Actor_Generic_Pain),
  AI_BADPLACE_ROW,
  // 5 coverarrival: 0x0046a230 / 0x00651a30 / 0x006948e0 / 0x00674400 / think 0x00681710 / 0x005fc750 / 0x0050dd40
  AI_NOT_PORTED_ROW(Actor_Generic_Touch, Actor_Generic_Pain),
  AI_DEATH_ROW,
  AI_PAIN_ROW,
  // 8 react: 0x00696f30 / 0x005f2d40 / 0x006948e0 / 0x005cc800 / think 0x00573f90 / 0x005fc750 / 0x0050dd40
  AI_NOT_PORTED_ROW(Actor_Generic_Touch, Actor_Generic_Pain),
  AI_ZOMBIE_SCRIPTEDANIM_ROW, // x6: SP row 9/10 are the same functions as the zombie rows (addresses above)
  AI_ZOMBIE_CUSTOMANIM_ROW,
  AI_NEGOTIATION_ROW
};

// species 1, SP 0x00a51e68. KB's MP dog table, unchanged, plus SP's states 9 and 10.
// SP's dog exposed think is 0x004d0e20; KB keeps the MP exe's own dog think.
const ai_funcs_t AIDogFuncTable[AIS_COUNT] =
{
  AI_NULL_ROW,
  {
    Actor_Dog_Exposed_Start,
    Actor_Dog_Exposed_Finish,
    Actor_Dog_Exposed_Suspend,
    Actor_Generic_Resume,
    Actor_Dog_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  AI_NULL_ROW,
  AI_NULL_ROW,
  AI_BADPLACE_ROW,
  AI_NULL_ROW,
  AI_DEATH_ROW,
  AI_PAIN_ROW,
  AI_NULL_ROW,
  AI_SCRIPTEDANIM_ROW,
  AI_CUSTOMANIM_ROW,
  AI_NEGOTIATION_ROW
};

// species 2, SP 0x00a51b68 - Five's zombies (aitype main: self.type = "zombie").
const ai_funcs_t AIZombieFuncTable[AIS_COUNT] =
{
  AI_NULL_ROW,
  // 1 exposed: start 0x0062a110 / finish 0x005f8970 / suspend 0x006948e0 / resume 0x004544c0 /
  // think 0x005591f0 / touch 0x00565940 / pain 0x0050dd40. P6c ports the exposed dispatch;
  // unresolved callee exports still raise a diagnostic with their SP address.
  {
    Actor_Exposed_Start_SP,
    Actor_Exposed_Finish_SP,
    Actor_Generic_Suspend,
    Actor_Exposed_Resume_SP,
    Actor_Zombie_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  AI_NULL_ROW,
  AI_NULL_ROW,
  AI_BADPLACE_ROW,
  AI_NULL_ROW,
  AI_DEATH_ROW,
  AI_PAIN_ROW,
  AI_NULL_ROW,
  AI_ZOMBIE_SCRIPTEDANIM_ROW,
  AI_ZOMBIE_CUSTOMANIM_ROW,
  AI_NEGOTIATION_ROW
};

// species 3, SP 0x00a51ce8. Same as the dog row except the exposed think, 0x004d5950 (lane x3 port).
const ai_funcs_t AIZombieDogFuncTable[AIS_COUNT] =
{
  AI_NULL_ROW,
  {
    Actor_Dog_Exposed_Start,
    Actor_Dog_Exposed_Finish,
    Actor_Dog_Exposed_Suspend,
    Actor_Generic_Resume,
    Actor_ZombieDog_Exposed_Think,
    Actor_Exposed_Touch,
    Actor_Generic_Pain
  },
  AI_NULL_ROW,
  AI_NULL_ROW,
  AI_BADPLACE_ROW,
  AI_NULL_ROW,
  AI_DEATH_ROW,
  AI_PAIN_ROW,
  AI_NULL_ROW,
  AI_ZOMBIE_SCRIPTEDANIM_ROW,
  AI_ZOMBIE_CUSTOMANIM_ROW,
  AI_NEGOTIATION_ROW
};

const ai_funcs_t *AIFuncTable[MAX_AI_SPECIES] =
{
  AIHumanFuncTable,
  AIDogFuncTable,
  AIZombieFuncTable,
  AIZombieDogFuncTable
};
