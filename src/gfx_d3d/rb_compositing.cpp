#include "rb_compositing.h"
#include "rb_draw3d.h"
#include "r_state.h"
#include "r_state_utils.h"
#include "rb_backend.h"

void __cdecl RB_DrawComposites()
{
  const void *cmds; // [esp+14h] [ebp-4h]

  PROF_SCOPED("RB_DrawComposites");
  cmds = backEndData->compositingCmds;
  if ( cmds )
  {
    R_InitLocalCmdBufState(&gfxCmdBufState);
    R_SetRenderTargetSize(&gfxCmdBufSourceState, R_RENDERTARGET_COMPOSITE);
    R_SetRenderTarget(gfxCmdBufContext, R_RENDERTARGET_COMPOSITE);
    R_Set2D(&gfxCmdBufSourceState);
    RB_ExecuteRenderCommandsLoop(cmds, 0);
    memcpy(gfxCmdBufState.refSamplerState, gfxCmdBufState.refSamplerState, sizeof(gfxCmdBufState));
  }
}

