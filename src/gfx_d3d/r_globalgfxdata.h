#pragma once

#include "r_gfx.h"

enum GfxDisplayGamma : __int32
{                                       // XREF: GfxGamutInfo/r
    GAMMA_SMPTE240M = 0x0,
    GAMMA_REC709    = 0x1,
    GAMMA_SRGB      = 0x2,
    GAMMA_USER      = 0x3,
};

enum GfxDisplayStandard : __int32
{                                       // XREF: GfxDisplayInfo/r
    VIDEO_STANDARD_NTSC      = 0x0,
    VIDEO_STANDARD_PAL_SECAM = 0x1,
    VIDEO_STANDARD_HDTV      = 0x2,
    VIDEO_STANDARD_SRGB      = 0x3,
};

enum GfxDisplayMode : __int32
{                                       // XREF: GfxDisplayInfo/r
    SINGLE_PLAYER_FULLSCENE_WIDE = 0x0,
    TWO_PLAYER_COOP_1UP_WIDE     = 0x1,
    TWO_PLAYER_COOP_2UP_WIDE     = 0x2,
    FOUR_PLAYER_1UP_WIDE         = 0x3,
    FOUR_PLAYER_2UP_WIDE         = 0x4,
    FOUR_PLAYER_3UP_WIDE         = 0x5,
    FOUR_PLAYER_4UP_WIDE         = 0x6,
    SINGLE_PLAYER_FULLSCENE      = 0x7,
    TWO_PLAYER_COOP_1UP          = 0x8,
    TWO_PLAYER_COOP_2UP          = 0x9,
    FOUR_PLAYER_1UP              = 0xA,
    FOUR_PLAYER_2UP              = 0xB,
    FOUR_PLAYER_3UP              = 0xC,
    FOUR_PLAYER_4UP              = 0xD,
    DISPLAY_MODE_TYPES           = 0xE,
};

enum GfxEDIDMode : __int32
{                                       // XREF: GfxDisplayInfo/r
    EDID_FALSE  = 0x0,
    EDID_TRUE   = 0x1,
    EDID_ACTIVE = 0x2,
};

struct __declspec(align(4)) GfxSceneSize // sizeof=0x20
{                                       // XREF: GfxDisplayInfo/r
    int width;
    int height;
    int compositeX;
    int compositeY;
    int compositeW;
    int compositeH;
    float aspect;
    bool panAndScan;
    // padding byte
    // padding byte
    // padding byte
};

struct GfxGamutInfo // sizeof=0x30
{                                       // XREF: GfxDisplayInfo/r
                                        // GfxDisplayInfo/r ...
    float xRed;
    float yRed;
    float xGreen;
    float yGreen;
    float xBlue;
    float yBlue;
    float xWhite;
    float yWhite;
    float gammaRed;
    float gammaGreen;
    float gammaBlue;
    GfxDisplayGamma gammaType;
};

struct GfxDisplayInfo // sizeof=0x278
{                                       // XREF: .data:GfxDisplayInfo g_displayInfo/r
    bool isHiDef;
    bool isWideScreen;
    // padding byte
    // padding byte
    GfxDisplayStandard outputDisplayVideoStandard;
    unsigned __int16 outputDisplayWidth;
    unsigned __int16 outputDisplayHeight;
    float outputDisplayHz;
    GfxDisplayMode renderDisplayMode;
    unsigned __int16 renderDisplayWidth;
    unsigned __int16 renderDisplayHeight;
    GfxSceneSize sceneSize[14];
    float displayfade;
    float contrastAdjust;
    float brightnessAdjust;
    GfxEDIDMode hasEDID;
    GfxGamutInfo referenceGamut;
    GfxGamutInfo outputGamut;
    GfxGamutInfo EDIDGamut;
};

struct GfxExposureHeader // sizeof=0x10
{                                       // XREF: GfxExposureSettings/r
    unsigned int identifier;
    unsigned int date;
    unsigned int typeVersion;
    unsigned int numberSize;
};

struct GfxExposureSettings // sizeof=0xB010
{                                       // XREF: .data:GfxExposureSettings exposureSettings/r
    GfxExposureHeader header;
    GfxExposureValue exposureValues[256];
};

extern GfxExposureSettings exposureSettings;
extern GfxDisplayInfo g_displayInfo;
extern bool g_HDRmode;
