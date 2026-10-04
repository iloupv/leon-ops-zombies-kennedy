#include "r_hw_nvidia.h"
#include "r_init.h"

bool __cdecl NV_UseShadowNullColorRenderTarget()
{
    return dx.nvInitialized;
}

