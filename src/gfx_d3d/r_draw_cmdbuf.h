#pragma once

struct GfxReadCmdBuf // sizeof=0x4
{                                       // XREF: ?R_DrawBspDrawSurfsLitPreTess@@YAXPBIUGfxCmdBufContext@@@Z/r
    const unsigned int *primDrawSurfPos;
};

const unsigned int *__cdecl R_ReadPrimDrawSurfData(GfxReadCmdBuf *cmdBuf, unsigned int count);
unsigned int __cdecl R_ReadPrimDrawSurfInt(GfxReadCmdBuf *cmdBuf);
