#include "console_mp.h"
#include <qcommon/common.h>

int __cdecl Live_GetNecessaryBandwidth()
{
    return band_dedicated->current.integer;
}

