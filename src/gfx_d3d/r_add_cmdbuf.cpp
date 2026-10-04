#include "r_add_cmdbuf.h"
#include <universal/q_shared.h>
#include "r_rendercmds.h"

void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf)
{
    delayedCmdBuf->primDrawSurfPos = -1;
    delayedCmdBuf->primDrawSurfSize = 0;
    delayedCmdBuf->drawSurfKey.packed = 0xFFFFFFFFFFFFFFFF;
    //*(unsigned int *)&delayedCmdBuf->drawSurfKey.fields = -1;
    //HIDWORD(delayedCmdBuf->drawSurfKey.packed) = -1;
}

void __cdecl R_EndCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf)
{
    if ((HIDWORD(delayedCmdBuf->drawSurfKey.packed) & *(unsigned int *)&delayedCmdBuf->drawSurfKey.fields) != -1)
    {
        if (!delayedCmdBuf->primDrawSurfSize
            && !Assert_MyHandler(
                "c:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_add_cmdbuf.h",
                62,
                0,
                "%s\n\t(delayedCmdBuf->primDrawSurfSize) = %i",
                "(delayedCmdBuf->primDrawSurfSize > 0)",
                delayedCmdBuf->primDrawSurfSize))
        {
            __debugbreak();
        }
        *(unsigned int *)&delayedCmdBuf->drawSurfKey.fields = -1;
        HIDWORD(delayedCmdBuf->drawSurfKey.packed) = -1;
        frontEndDataOut->primDrawSurfsBuf[delayedCmdBuf->primDrawSurfPos++] = 0;
        --delayedCmdBuf->primDrawSurfSize;
    }
}