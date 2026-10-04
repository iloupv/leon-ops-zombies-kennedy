#pragma once

#include "r_gfx.h"

void __cdecl R_InitDelayedCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
void __cdecl R_EndCmdBuf(GfxDelayedCmdBuf *delayedCmdBuf);
