#pragma once

#include <physics/phys_colgeom.h>
#include <physics/phys_gjk.h>

enum aiphys_t : __int32
{                                       // XREF: actor_physics_t/r
    AIPHYS_BAD                    = 0x0,
    AIPHYS_NORMAL_ABSOLUTE        = 0x1,
    AIPHYS_NORMAL_RELATIVE        = 0x2,
    AIPHYS_NOCLIP                 = 0x3,
    AIPHYS_NOGRAVITY              = 0x4,
    AIPHYS_ZONLY_PHYSICS_RELATIVE = 0x5,
};

enum SlideMoveResult : __int32
{                                       // XREF: AIPhys_StepSlideMove/r
    SLIDEMOVE_COMPLETE = 0x0,
    SLIDEMOVE_CLIPPED  = 0x1,
    SLIDEMOVE_FAIL     = 0x2,
};

struct actor_physics_t // sizeof=0x7D0
{                                       // XREF: actor_s/r
    //actor_physics_t *__thiscall actor_physics_t::operator=(actor_physics_t *this, const actor_physics_t *__that);
    
                                        // Path_CanLinkNodes/r ...
    float vOrigin[3];                   // XREF: AIPhys_StepSlideMove+48B/r
                                        // AIPhys_StepSlideMove+4A3/r ...
    float vVelocity[3];                 // XREF: Path_CanLinkNodes+281/w
                                        // Path_CanLinkNodes+291/w ...
    unsigned __int16 groundEntNum;      // XREF: Path_CanLinkNodes+2AE/w
    // padding byte
    // padding byte
    int iFootstepTimer;
    int bHasGroundPlane;                // XREF: Path_CanLinkNodes:loc_92E42B/r
    float groundplaneSlope;
    int iSurfaceType;
    float vWishDelta[3];                // XREF: Path_CanLinkNodes+2C2/w
                                        // Path_CanLinkNodes+2D7/w ...
    int bIsAlive;                       // XREF: Path_CanLinkNodes+2EF/w
    int iEntNum;                        // XREF: Path_CanLinkNodes+2F9/w
    aiphys_t ePhysicsType;              // XREF: Path_CanLinkNodes+303/w
    float fGravity;                     // XREF: Path_CanLinkNodes+318/w
    int iMsec;                          // XREF: Path_CanLinkNodes+320/w
    float vMins[3];                     // XREF: Path_CanLinkNodes+332/w
                                        // Path_CanLinkNodes+342/w ...
    float vMaxs[3];                     // XREF: Path_CanLinkNodes+362/w
                                        // Path_CanLinkNodes+372/w ...
    bool prone;
    // padding byte
    // padding byte
    // padding byte
    int iTraceMask;                     // XREF: Path_CanLinkNodes+3EA/w
    int foliageSoundTime;
    int iNumTouch;
    int iTouchEnts[32];
    int iHitEntnum;                     // XREF: Path_CanLinkNodes:loc_92E476/r
                                        // Path_CanLinkNodes:loc_92E56F/r
    float vHitOrigin[3];
    float vHitNormal[3];
    unsigned __int8 bStuck;
    unsigned __int8 bDeflected;
    // padding byte
    // padding byte
    const gjkcc_input_t *m_gjkcc_input;
    colgeom_visitor_inlined_t<200> proximity_data;
                                        // XREF: AIPhys_StepSlideMove+49/o
                                        // AIPhys_StepSlideMove+133/w ...
};

struct actor_physics_local_t // sizeof=0x68
{                                       // XREF: .data:g_apl/r
                                        // AIPhys_StepSlideMove/r
    float fFrameTime;                   // XREF: setup_ai_gjk_slide_move_input(actor_physics_t *,ai_gjk_slide_move_input_t *,int)+19/r
                                        // Actor_Physics_1(actor_physics_t *)+D2/w ...
    int bIsWalking;                     // XREF: Actor_Physics_1(actor_physics_t *):loc_913056/r
                                        // AIPhys_StepSlideMove+1BB/r ...
    int bGroundPlane;                   // XREF: Actor_Physics_1(actor_physics_t *):loc_91307F/r
                                        // AIPhys_AirMove+3/r ...
    trace_t groundTrace;                // XREF: Actor_Physics_1(actor_physics_t *)+250/r
                                        // Actor_Physics_1(actor_physics_t *)+27F/r ...
    float fImpactSpeed;                 // XREF: AIPhys_SlideMove+8FD/r
                                        // AIPhys_SlideMove+915/w
    float vPrevOrigin[3];               // XREF: Actor_Physics_1(actor_physics_t *)+62/w
                                        // Actor_Physics_1(actor_physics_t *)+72/w ...
    float vPrevVelocity[3];             // XREF: Actor_Physics_1(actor_physics_t *)+9A/w
                                        // Actor_Physics_1(actor_physics_t *)+AA/w ...
    int iTraceMask;                     // XREF: setup_ai_gjk_slide_move_input(actor_physics_t *,ai_gjk_slide_move_input_t *,int)+61/r
                                        // Actor_Physics_1(actor_physics_t *)+55/w ...
    float stepheight;                   // XREF: Actor_Physics_1(actor_physics_t *)+11C/w
                                        // Actor_Physics_1(actor_physics_t *)+12E/w ...
};

void __cdecl AIPhys_AddTouchEnt(actor_physics_t *pPhys, int entityNum);
void __cdecl setup_gjkcc_input(actor_physics_t *pPhys, gjkcc_input_t *gjkcc_in);
void __cdecl ai_physics_trace(
                trace_t *trace,
                float *start,
                float *mins,
                float *maxs,
                float *end,
                int passEntityNum,
                int contentmask,
                actor_physics_t *pPhys);
void __cdecl setup_ai_gjk_slide_move_input(actor_physics_t *pPhys, ai_gjk_slide_move_input_t *input, int gravity);
void __cdecl AIPhys_FoliageSounds(actor_physics_t *pPhys);
bool __cdecl Actor_Physics_1(actor_physics_t *pPhys);
bool __cdecl AIPhys_AirMove(actor_physics_t *pPhys);
void __cdecl AIPhys_ClipVelocity(const float *in, const float *normal, bool isWalkable, float *out, float overbounce);
bool __cdecl AIPhys_StepSlideMove(actor_physics_t *pPhys, int gravity, int zonly);
int __cdecl AIPhys_SlideMove(actor_physics_t *pPhys, int gravity, int zonly); // BO1TODO: SlideMoveResult retval

bool __cdecl AIPhys_WalkMove(actor_physics_t *pPhys);
bool __cdecl AIPhys_ZOnlyPhysicsMove(actor_physics_t *pPhys);
void __cdecl AIPhys_NoClipMove(actor_physics_t *pPhys);
void __cdecl AIPhys_NoGravityMove(actor_physics_t *pPhys);
void __cdecl AIPhys_GroundTrace(actor_physics_t *pPhys);
int __cdecl Actor_Physics(actor_physics_t *pPhys);
int __cdecl Actor_Physics_z(actor_physics_t *pPhys);
void __cdecl Actor_PostPhysics(actor_physics_t *pPhys);
void __cdecl AIPhys_Footsteps(actor_physics_t *pPhys);
//colgeom_visitor_inlined_t<200> *__thiscall colgeom_visitor_inlined_t<200>::colgeom_visitor_inlined_t<200>(colgeom_visitor_inlined_t<200> *this);
//colgeom_visitor_inlined_t<200> *__thiscall colgeom_visitor_inlined_t<200>::operator=(
//    colgeom_visitor_inlined_t<200> *this,
//    const colgeom_visitor_inlined_t<200> *__that);
//colgeom_visitor_t *__thiscall colgeom_visitor_t::operator=(colgeom_visitor_t *this, const colgeom_visitor_t *__that);
