#pragma once
#include <xanim/xanim.h>
#include "rb_state.h"

void __cdecl R_SetStreamsForXModelSurface(
                const XSurface *localSurf,
                GfxCmdBufPrimState *primState,
                GfxCmdBufPrimState *prepassPrimState);
