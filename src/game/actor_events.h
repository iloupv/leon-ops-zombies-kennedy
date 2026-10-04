#pragma once
#include "actor.h"

enum PARM_SUPPRESSION : __int32
{                                       // XREF: Actor_EventBullet/r
    DO_SUPPRESSION = 0x0,
    DONT_SUPPRESS  = 0x1,
};

enum ai_event_t : __int32
{                                       // XREF: ?Actor_EventDefaultRadiusSqrd@@YIMW4ai_event_t@@@Z/r
                                        // ?Actor_EventDefaultHeightDiff@@YIMW4ai_event_t@@@Z/r ...
    AI_EV_BAD                  = 0x0,
    AI_EV_FIRST_POINT_EVENT    = 0x1,
    AI_EV_FOOTSTEP             = 0x2,
    AI_EV_FOOTSTEP_LITE        = 0x3,
    AI_EV_NEW_ENEMY            = 0x4,
    AI_EV_PAIN                 = 0x5,
    AI_EV_REACT                = 0x6,
    AI_EV_DEATH                = 0x7,
    AI_EV_EXPLOSION            = 0x8,
    AI_EV_GRENADE_PING         = 0x9,
    AI_EV_PROJECTILE_PING      = 0xA,
    AI_EV_GUNSHOT              = 0xB,
    AI_EV_SILENCED_SHOT        = 0xC,
    AI_EV_LAST_POINT_EVENT     = 0xD,
    AI_EV_FIRST_LINE_EVENT     = 0xE,
    AI_EV_BULLET               = 0xF,
    AI_EV_BLOCK_FRIENDLIES     = 0x10,
    AI_EV_BULLET_REACT         = 0x11,
    AI_EV_PROJECTILE_IMPACT    = 0x12,
    AI_EV_LAST_LINE_EVENT      = 0x13,
    AI_EV_FIRST_ARC_EVENT      = 0x14,
    AI_EV_BADPLACE_ARC         = 0x15,
    AI_EV_BADPLACE_LIMITED_ARC = 0x16,
    AI_EV_REALLY_BADPLACE_ARC  = 0x17,
    AI_EV_LAST_ARC_EVENT       = 0x18,
    AI_EV_FIRST_VOLUME_EVENT   = 0x19,
    AI_EV_BADPLACE_VOLUME      = 0x1A,
    AI_EV_LAST_VOLUME_EVENT    = 0x1B,
    AI_EV_NUM_EVENTS           = 0x1C,
};

double __fastcall Actor_EventDefaultRadiusSqrd(ai_event_t eType);
double __fastcall Actor_EventDefaultHeightDiff(ai_event_t eType);
void __fastcall Actor_GetPerfectInfo(actor_s *self, sentient_s *pOther);
void __fastcall Actor_WasAttackedBy(actor_s *self, sentient_s *pOther);
void __fastcall Actor_BroadcastTeamEvent(sentient_s *sentient, ai_event_t eType);
void __fastcall Actor_BroadcastPointEvent(
                gentity_s *originator,
                ai_event_t eType,
                int teamFlags,
                float *vOrigin,
                float fRadiusSqrd);
char __cdecl Actor_IsOnSameTeam(gentity_s *originator, actor_s *pActor, int teamFlags);
void __fastcall Actor_BroadcastLineEvent(
                gentity_s *originator,
                gentity_s *hitEnt,
                ai_event_t eType,
                int teamFlags,
                const float *vStart,
                const float *vEnd,
                float fRadiusSqrd);
void __fastcall Actor_ReceivePointEvent(
                actor_s *self,
                gentity_s *originator,
                ai_event_t eType,
                float *vOrigin,
                float fDistSqrd,
                float fRadiusSqrd);
void __cdecl Actor_DumpEvents(actor_s *self, ai_event_t event, gentity_s *originator);
void __fastcall Actor_EventFootstep(actor_s *self, sentient_s *originator, const float *vOrigin);
void __fastcall Actor_EventNewEnemy(actor_s *self, sentient_s *originator);
void __fastcall Actor_EventPain(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker);
void __fastcall Actor_EventDeath(actor_s *self, sentient_s *pCasualty, sentient_s *pAttacker);
void __cdecl Actor_EventExplosion(actor_s *self, gentity_s *originator, float *vOrigin);
void __cdecl Actor_EventGrenadePing(actor_s *self, gentity_s *originator, const float *vOrigin);
void __fastcall Actor_EventGunshot(actor_s *self, sentient_s *originator, const float *vOrigin);
void __fastcall Actor_ReceiveLineEvent(
                actor_s *self,
                gentity_s *originator,
                gentity_s *hitEnt,
                ai_event_t eType,
                const float *vStart,
                const float *vEnd,
                const float *vClosest,
                float fDistSqrd,
                float fRadiusSqrd);
void __fastcall Actor_EventBullet(
                actor_s *self,
                gentity_s *originator,
                const float *vStart,
                const float *vEnd,
                const float *vClosest,
                float fDistSqrd,
                float fRadiusSqrd,
                PARM_SUPPRESSION suppression);
