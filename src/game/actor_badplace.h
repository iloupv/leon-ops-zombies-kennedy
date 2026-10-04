#pragma once

enum ai_badplace_t : __int32
{                                       // XREF: actor_s/r
    AI_BADPLACE_NONE      = 0x0,
    AI_BADPLACE_NORMAL    = 0x1,
    AI_BADPLACE_REALLYBAD = 0x2,
};

struct badplace_brush_t // sizeof=0x8
{                                       // XREF: badplace_parms_t/r
    struct gentity_s *volume;
    float radius;
};

struct badplace_arc_t // sizeof=0x1C
{                                       // XREF: badplace_parms_t/r
    float origin[3];
    float radius;
    float halfheight;
    float angle0;
    float angle1;
};

union badplace_parms_t // sizeof=0x1C
{                                       // XREF: badplace_t/r
                                        // _pendedBadPlace/r
    badplace_arc_t arc;
    badplace_brush_t brush;
};

struct badplace_t // sizeof=0x2C
{
    int endtime;
    int pingTime;
    unsigned __int16 name;
    unsigned __int8 type;
    unsigned __int8 teamflags;
    int depth;
    badplace_parms_t parms;
};

struct actor_s;
struct pathsort_t;
struct pathnode_t;
enum ai_state_t : __int32;
enum actor_think_result_t : __int32;

void __cdecl Path_UpdateBadPlaceCount(badplace_t *place, int delta);
void __cdecl Path_UpdateBadPlaces();
void __cdecl Path_FreeBadPlace(int index);
int __cdecl Path_FindBadPlace(unsigned int name);
void __cdecl Path_MakeBadPlaceEx(unsigned int name, int duration, int teamflags, int type, badplace_parms_t *parms);
badplace_t *__cdecl Path_AllocBadPlace(unsigned int name, int duration);
void __cdecl Path_DrawBadPlaces();
void __cdecl Path_DrawBadPlace(badplace_t *place);
void __cdecl Path_InitBadPlaces();
void __cdecl Path_ShutdownBadPlaces();
bool __cdecl Actor_IsInAnyBadPlace(actor_s *self);
actor_s *Actor_BadPlace_UpdateFleeingActors();
bool __fastcall Actor_BadPlace_Flee_Start(actor_s *self, ai_state_t ePrevState);
char __cdecl Actor_BadPlace_AttemptEscape(actor_s *self);
double __cdecl Actor_BadPlace_GetMaximumFleeRadius();
int __cdecl Actor_BadPlace_FindSafeNodeOutsideBadPlace(actor_s *self, pathsort_t *potentialNodes, float maxFleeDist);
int __cdecl Path_IsCoverNode(const pathnode_t *node);
char __cdecl Actor_BadPlace_HasPotentialNodeDuplicates(
                pathsort_t *potentialNodes,
                int potentialNodeCount,
                pathnode_t *checkNode);
bool __cdecl Actor_BadPlace_IsNodeInAnyBadPlace(pathnode_t *node);
pathnode_t *__cdecl Actor_BadPlace_FindSafeNodeAlongPath(actor_s *self);
actor_think_result_t __fastcall Actor_BadPlace_Flee_Think(actor_s *self);
void __fastcall Actor_BadPlace_Flee_Finish(actor_s *self, ai_state_t eNextState);
