#include "rb_stream.h"

#include <cstring>

StreamBackendGlob streamBackendGlob;

void __cdecl RB_StreamInit()
{
    memset(streamBackendGlob.imageDirty, 0, sizeof(streamBackendGlob.imageDirty));
    memset(streamBackendGlob.maxLodBiasDirty, 0, sizeof(streamBackendGlob.maxLodBiasDirty));
}

