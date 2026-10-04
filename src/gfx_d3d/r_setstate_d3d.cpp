#include "r_setstate_d3d.h"
#include "r_init.h"

void __cdecl R_FinishGpuFence()
{
    ///while ( LODWORD(dx.gpuSyncDelay)
    ///         && (*(int (__stdcall **)(unsigned int, unsigned int, unsigned int, int))(*(unsigned int *)LODWORD(dx.gpuSyncDelay) + 28))(
    ///                    dx.gpuSyncDelay,
    ///                    0,
    ///                    0,
    ///                    1) == 1 )
    ///    ;

    //while (dx.gpuSyncDelay->GetData(NULL, 0, D3DGETDATA_FLUSH) == S_FALSE)
    while (dx.flushGpuQuery->GetData(NULL, 0, D3DGETDATA_FLUSH) == S_FALSE)
    {
        /* spin until GPU reaches the fence */
    }
}

