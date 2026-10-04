#include "r_draw_model_util.h"
#include "rb_shade.h"

void __cdecl R_SetStreamsForXModelSurface(
                const XSurface *localSurf,
                GfxCmdBufPrimState *primState,
                GfxCmdBufPrimState *prepassPrimState)
{
    IDirect3DVertexBuffer9 *vb0; // [esp+0h] [ebp-4h]

    vb0 = localSurf->vb0;
    if ( prepassPrimState )
        R_SetStreamSource(prepassPrimState, vb0, 0, 0x20u);
    R_SetStreamSource(primState, vb0, 0, 0x20u);
}

