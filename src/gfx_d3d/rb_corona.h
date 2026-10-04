#pragma once
#include "rb_backend.h"

struct Corona // sizeof=0x48
{                                       // XREF: CoronaState/r
                                        // CoronaState/r
    bool inUse;
    // padding byte
    // padding byte
    // padding byte
    GfxLightCorona *info;
    int timeSinceSpawned;
    float currentVisibility;
    float targetVisibility;
    int hitNum;
    bool wasEverVisible;
    bool queryIssued[5];
    // padding byte
    // padding byte
    IDirect3DQuery9 *query[5];
    IDirect3DQuery9 *queryMax[5];
};

struct CoronaState // sizeof=0x908
{                                       // XREF: .data:CoronaState * coronaState/r
    unsigned int numActive;
    unsigned int numPending;
    Corona active[16];
    Corona pending[16];
};

void __cdecl RB_DrawCoronas(unsigned int localClientNum);
GfxLightCorona *__cdecl RB_FindBestCoronaToSpawn(CoronaState *state);
void __cdecl RB_SpawnCorona(Corona *corona, GfxLightCorona *info);
void __cdecl RB_ResetCorona(Corona *corona);
void __cdecl RB_UnspawnCorona(Corona *corona);
void __cdecl RB_PromoteCorona(CoronaState *state, Corona *corona);
void __cdecl RB_DrawCorona(Corona *corona, int frameTime, const Material *coronaMat);
double __cdecl R_UpdateOverTime(float fCurrent, float fGoal, int frametime);
void __cdecl RB_DrawCoronaSprite(Corona *corona, const Material *material, GfxColor color, float radius, int query);
void __cdecl RB_TessCoronaBillboard(Corona *corona, GfxColor color, float radius, int query);
void __cdecl Vec4DivideByW(const float *in, float *out);
GfxVertex *__cdecl RB_SetTessQuad(GfxColor color);
void __cdecl RB_DrawCoronaQuerySprite(Corona *corona);
void __cdecl RB_HW_BeginOcclusionQuery(IDirect3DQuery9 *query);
unsigned int __cdecl RB_HW_ReadOcclusionQuery(IDirect3DQuery9 *query);
void __cdecl RB_ResetCoronas();
void __cdecl RB_AllocCoronaSpriteQueries();
void __cdecl RB_AllocCoronaSpriteQuery(Corona *corona);
void __cdecl RB_FreeCoronaSpriteQueries();
void __cdecl RB_FreeCoronaSpriteQuery(Corona *corona);
void __cdecl RB_DrawWaypoint(
                Material *material,
                GfxColor color,
                float *origin,
                float radius,
                bool screenSize,
                bool alignBottom,
                bool forceNear);
