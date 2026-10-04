#pragma once
#include <d3d9.h>


enum DxCapsResponse : __int32
{                                       // XREF: DxCapsCheckBits/r
                                        // DxCapsCheckInteger/r ...
    DX_CAPS_RESPONSE_QUIT       = 0x0,  // XREF: .rdata:s_capsCheckBits/s
                                        // .rdata:s_capsCheckInt/s
    DX_CAPS_RESPONSE_WARN       = 0x1,  // XREF: .rdata:s_capsCheckBits/s
    DX_CAPS_RESPONSE_INFO       = 0x2,  // XREF: .rdata:s_capsCheckBits/s
    DX_CAPS_RESPONSE_FORBID_SM3 = 0x3,  // XREF: .rdata:s_capsCheckInt/s
};

struct DxCapsCheckBits // sizeof=0x14
{                                       // XREF: .rdata:s_capsCheckBits/r
    int offset;                         // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+28/r
    unsigned int setBits;               // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+61/r
                                        // R_CheckDxCaps(_D3DCAPS9 const *)+73/r
    unsigned int clearBits;             // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+3D/r
                                        // R_CheckDxCaps(_D3DCAPS9 const *)+51/r
    DxCapsResponse response;            // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+90/r
    const char *msg;                    // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+83/r
};

struct DxCapsCheckInteger // sizeof=0x14
{                                       // XREF: .rdata:s_capsCheckInt/r
    int offset;                         // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+C2/r
    unsigned int min;                   // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+DA/r
    unsigned int max;                   // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+EB/r
    DxCapsResponse response;            // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+108/r
    const char *msg;                    // XREF: R_CheckDxCaps(_D3DCAPS9 const *)+FB/r
};

void __cdecl R_CheckDxCaps(const _D3DCAPS9 *caps);
void __cdecl R_RespondToMissingCaps(DxCapsResponse response, const char *msg);
