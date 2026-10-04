#pragma once

struct __declspec(align(4)) StreamBackendGlob // sizeof=0x404
{                                       // XREF: .data:StreamBackendGlob streamBackendGlob/r
    unsigned int imageDirty[128];
    unsigned int maxLodBiasDirty[128];  // XREF: RB_StreamInit(void)+1E/o
    bool imagesDirty;
    bool maxLodDirty;
    // padding byte
    // padding byte
};

void __cdecl RB_StreamInit();