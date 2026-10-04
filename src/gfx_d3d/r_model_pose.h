#pragma once
#include <xanim/dobj.h>
#include "r_scene.h"

enum
{
    CULL_STATE_OUT = 0,
    CULL_STATE_BOUNDED_PENDING = 1,
    CULL_STATE_BOUNDED = 2,
    CULL_STATE_SKINNED_PENDING = 3,
    CULL_STATE_DONE = 4,
};

// local variable allocation has failed, the output may be wrong!
DObjAnimMat * R_UpdateSceneEntBounds(
                GfxSceneEntity *sceneEnt,
                GfxSceneEntity **pLocalSceneEnt,
                const DObj **pObj,
                int waitForCullState);
DObjAnimMat *__cdecl R_DObjCalcPose(const GfxSceneEntity *sceneEnt, DObj *obj, int *partBits);
void __cdecl R_SetNoDraw(GfxSceneEntity *sceneEnt);
void __cdecl R_UpdateGfxEntityBoundsCmd(GfxSceneEntity **data);
