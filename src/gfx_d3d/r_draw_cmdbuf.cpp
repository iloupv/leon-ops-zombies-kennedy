#include "r_draw_cmdbuf.h"

const unsigned int *__cdecl R_ReadPrimDrawSurfData(GfxReadCmdBuf *cmdBuf, unsigned int count)
{
    const unsigned int *result; // [esp+0h] [ebp-4h]

    result = cmdBuf->primDrawSurfPos;
    cmdBuf->primDrawSurfPos += count;
    return result;
}

unsigned int __cdecl R_ReadPrimDrawSurfInt(GfxReadCmdBuf *cmdBuf)
{
    return *cmdBuf->primDrawSurfPos++;
}

