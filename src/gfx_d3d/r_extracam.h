#pragma once

enum eExtraCamResolution : __int32
{                                       // XREF: ?R_ExtraCam_Init@@YAXH@Z/r
                                        // CreateExtraCamRenderTargets/r
    EXTRACAM_RES_OFF     = 0x0,
    EXTRACAM_RES_NORMAL  = 0x1,
    EXTRACAM_RES_HIGH    = 0x2,
    EXTRACAM_RES_DEFAULT = 0x1,
};

struct __declspec(align(4)) ExtraCamConfig // sizeof=0x10
{                                       // XREF: .data:g_extraCamConfig/r
    unsigned int renderTargetWidth;     // XREF: CreateExtraCamRenderTargets+100/w
                                        // R_ExtraCam_RenderTargetWidth(void)+3/r ...
    unsigned int renderTargetHeight;    // XREF: CreateExtraCamRenderTargets+109/w
                                        // R_ExtraCam_RenderTargetHeight(void)+3/r ...
    float aspectRatio;                  // XREF: R_ExtraCam_AspectRatio(void)+3/r
                                        // CreateExtraCamRenderTargets+131/w ...
    bool initialized;                   // XREF: R_ExtraCam_Init(int)+6/r
                                        // R_ExtraCam_Init(int)+A7/w ...
    // padding byte
    // padding byte
    // padding byte

    ExtraCamConfig()
    {
        renderTargetWidth = 0;
        renderTargetHeight = 0;
        aspectRatio = 0.0f;
        initialized = 0;
    }
};

struct XGlobals // sizeof=0x28
{                                       // XREF: XAssetPoolEntry<XGlobals>/r
    const char *name;
    int xanimStreamBufferSize;
    int cinematicMaxWidth;
    int cinematicMaxHeight;
    int extracamResolution;
    int gumpReserve;
    float screenClearColor[4];
};

double __cdecl R_ExtraCam_AspectRatio();
void __cdecl R_ExtraCam_Init(int location);
void __cdecl CreateExtraCamRenderTargets(eExtraCamResolution resConfig, int location);
void __cdecl R_ExtraCam_Shutdown();
unsigned int __cdecl R_ExtraCam_RenderTargetWidth();
unsigned int __cdecl R_ExtraCam_RenderTargetHeight();
bool __cdecl R_ExtraCam_GfxResourcesReady();
bool __cdecl R_ExtraCam_UseShadowmap();
