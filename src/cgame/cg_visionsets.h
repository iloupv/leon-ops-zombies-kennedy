#pragma once
#include <xanim/xanim.h>

struct GfxFilm // sizeof=0x94
{                                                                             // XREF: GfxVisionSet/r
                                                                                // GfxViewInfo/r ...
        float filmMidStart;
        float filmMidEnd;
        float filmDarkFeather;
        float filmLightFeather;
        float filmBleach[3];
        float filmColorTemp[3];
        float filmHue[3];
        float filmSaturation[3];
        float filmDarkTint[3];
        float filmMidTint[3];
        float filmLightTint[3];
        float filmContrast[3];
        bool enabled;
        // padding byte
        // padding byte
        // padding byte
        float filmLut;
        float sCurveEnable;
        float sCurveShoulderStrength;
        float sCurveLinearStrength;
        float sCurveLinearAngle;
        float sCurveToeStrength;
        float sCurveToeNumerator;
        float sCurveToeDenominator;
};

struct GfxBloom // sizeof=0x1B4
{                                                                             // XREF: GfxVisionSet/r
                                                                                // GfxViewInfo/r ...
        float bloomTintWeights[4];
        float bloomColorScale[4];
        float bloomTintScale[4];
        float bloomCurveBreakpoint[4];
        float bloomCurveLoBlack[4];
        float bloomCurveLoGamma[4];
        float bloomCurveLoWhite[4];
        float bloomCurveLoRemapBlack[4];
        float bloomCurveLoRemapWhite[4];
        float bloomCurveHiBlack[4];
        float bloomCurveHiGamma[4];
        float bloomCurveHiWhite[4];
        float bloomCurveHiRemapBlack[4];
        float bloomCurveHiRemapWhite[4];
        float bloomExpansionControl[4];
        float bloomExpansionWeights[4];
        int bloomExpansionSource;
        float bloomBlurRadius;
        float bloomPersistence;
        float bloomStreakXLevels0[4];
        float bloomStreakXLevels1[4];
        float bloomStreakXInnerTint[3];
        float bloomStreakXOuterTint[3];
        float bloomStreakXTintControl[4];
        float bloomStreakXTint[3];
        float bloomStreakYLevels0[4];
        float bloomStreakYLevels1[4];
        float bloomStreakYInnerTint[3];
        float bloomStreakYOuterTint[3];
        float bloomStreakYTintControl[4];
        float bloomStreakYTint[3];
};

struct GfxReviveFx // sizeof=0x38
{                                                                             // XREF: GfxVisionSet/r
                                                                                // GfxViewInfo/r ...
        bool enabled;
        // padding byte
        // padding byte
        // padding byte
        float reviveEdgeColorTemp;
        float reviveEdgeSaturation;
        float reviveEdgeScale[3];
        float reviveEdgeContrast[3];
        float reviveEdgeOffset[3];
        float reviveEdgeMaskAdjust;
        float reviveEdgeAmount;
};

struct GfxLightScale // sizeof=0x8
{                                                                             // XREF: GfxVisionSet/r
                                                                                // GfxViewInfo/r ...
        float diffuseScale;
        float specularScale;
};

struct GfxVisionSet // sizeof=0x288
{                                                                             // XREF: refdef_s/r
        GfxFilm film;
        GfxBloom bloom;
        GfxReviveFx reviveFx;
        GfxLightScale charPrimaryLightScale;
};

struct visionSetVars_t // sizeof=0x2E0
{                                                                             // XREF: cg_s/r cg_s/r ...
        bool filmEnable;                                        // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+5E/r
        // padding byte
        // padding byte
        // padding byte
        float filmMidStart;                                 // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+73/r
        float filmMidEnd;                                     // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+89/r
        float filmDarkFeather;                            // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+9F/r
        float filmLightFeather;                         // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+B5/r
        float filmHue[3];                                     // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+F5/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+FF/r ...
        float filmSaturation[3];                        // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+11F/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+129/r ...
        float filmColorTemp[3];                         // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+149/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+153/r ...
        float filmLightTint[3];                         // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+1C7/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+1D1/r ...
        float filmMidTint[3];                             // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+19D/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+1A7/r ...
        float filmDarkTint[3];                            // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+173/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+17D/r ...
        float filmContrast[3];                            // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+1F1/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+1FB/r ...
        float filmBleach[3];                                // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+CB/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Film(void)+D5/r ...
        float filmLut;                                            // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+48/r
        bool reviveEnable;
        // padding byte
        // padding byte
        // padding byte
        float reviveEdgeColorTemp;
        float reviveEdgeSaturation;
        float reviveEdgeScale[3];
        float reviveEdgeContrast[3];
        float reviveEdgeOffset[3];
        float reviveEdgeMaskAdjust;
        float reviveEdgeAmount;
        bool charPrimaryUseTweaks;                    // XREF: CG_VisionSetUpdateTweaksFromFile_CharPrimary(void):loc_523744/r
        // padding byte
        // padding byte
        // padding byte
        float charPrimaryDiffuseScale;            // XREF: CG_VisionSetUpdateTweaksFromFile_CharPrimary(void)+59/r
        float charPrimarySpecularScale;         // XREF: CG_VisionSetUpdateTweaksFromFile_CharPrimary(void)+6F/r
        float masterRingmod;
        float reverbRingmod;
        float hiFilter;
        float lowFilter;
        int sCurveEnable;                                     // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+21B/r
        float sCurveShoulderStrength;             // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+22F/r
        float sCurveLinearStrength;                 // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+245/r
        float sCurveLinearAngle;                        // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+25B/r
        float sCurveToeStrength;                        // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+271/r
        float sCurveToeNumerator;                     // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+287/r
        float sCurveToeDenominator;                 // XREF: CG_VisionSetUpdateTweaksFromFile_Film(void)+29D/r
        float postEmissiveBrightening;
        float bloomBlurRadius;                            // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+48/r
        float bloomTintWeights[4];                    // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+5E/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+68/r ...
        float bloomColorScale[4];                     // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+92/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+9C/r ...
        float bloomTintScale[4];                        // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+C6/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+D0/r ...
        float bloomCurveBreakpoint[4];            // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+FA/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+104/r ...
        float bloomCurveLoBlack[4];                 // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+12E/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+138/r ...
        float bloomCurveLoGamma[4];                 // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+162/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+16C/r ...
        float bloomCurveLoWhite[4];                 // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+196/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+1A0/r ...
        float bloomCurveLoRemapBlack[4];        // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+1CA/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+1D4/r ...
        float bloomCurveLoRemapWhite[4];        // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+1FE/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+208/r ...
        float bloomCurveHiBlack[4];                 // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+232/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+23C/r ...
        float bloomCurveHiGamma[4];                 // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+266/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+270/r ...
        float bloomCurveHiWhite[4];                 // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+29A/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+2A4/r ...
        float bloomCurveHiRemapBlack[4];        // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+2CE/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+2D8/r ...
        float bloomCurveHiRemapWhite[4];        // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+302/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+30C/r ...
        float bloomExpansionControl[4];         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+336/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+340/r ...
        float bloomExpansionWeights[4];         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+36A/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+374/r ...
        int bloomExpansionSource;                     // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+39E/r
        float bloomPersistence;                         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+3B2/r
        float bloomStreakXLevels0[4];             // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+3C8/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+3D2/r ...
        float bloomStreakXLevels1[4];             // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+3FC/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+406/r ...
        float bloomStreakXInnerTint[3];         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+430/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+43A/r ...
        float bloomStreakXOuterTint[3];         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+45A/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+464/r ...
        float bloomStreakXTintControl[4];     // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+484/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+48E/r ...
        float bloomStreakXTint[3];                    // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+4B8/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+4C2/r ...
        float bloomStreakYLevels0[4];             // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+4E2/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+4EC/r ...
        float bloomStreakYLevels1[4];             // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+516/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+51D/r ...
        float bloomStreakYInnerTint[3];         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+547/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+54E/r ...
        float bloomStreakYOuterTint[3];         // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+568/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+56F/r ...
        float bloomStreakYTintControl[4];     // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+589/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+590/r ...
        float bloomStreakYTint[3];                    // XREF: CG_VisionSetUpdateTweaksFromFile_Bloom(void)+5B1/r
                                                                                // CG_VisionSetUpdateTweaksFromFile_Bloom(void)+5B8/r ...
        char name[64];
};

enum visionSetLerpStyle_t : __int32
{                                                                             // XREF: visionSetLerpData_t/r
                                                                                // LerpBool/r ...
        VISIONSETLERP_UNDEFINED                = 0x0,
        VISIONSETLERP_NONE                         = 0x1,
        VISIONSETLERP_TO_LINEAR                = 0x2,
        VISIONSETLERP_TO_SMOOTH                = 0x3,
        VISIONSETLERP_BACKFORTH_LINEAR = 0x4,
        VISIONSETLERP_BACKFORTH_SMOOTH = 0x5,
        VISIONSETLERP_CUSTOM_VALUE         = 0x6,
};

enum visionSetMode_t : __int32
{                                                                             // XREF: ?VisionSetCurrent@@YA_NHW4visionSetMode_t@@PBD@Z/r
                                                                                // ?CG_VisionSetStartLerp_To@@YA_NHW4visionSetMode_t@@W4visionSetLerpStyle_t@@PBDH@Z/r ...
        VISIONSETMODE_NAKED        = 0x0,
        VISIONSETMODE_NIGHT        = 0x1,
        VISIONSETMODE_FLARE        = 0x2,
        VISIONSETMODE_INFRARED = 0x3,
        VISIONSETMODE_TVGUIDED = 0x4,
        VISIONSETMODE_EXTRACAM = 0x5,
        VISIONSETMODECOUNT         = 0x6,
};

struct visionSetLerpData_t // sizeof=0xC
{                                                                             // XREF: cg_s/r
        int timeStart;
        int timeDuration;
        visionSetLerpStyle_t style;
};

struct visField_t // sizeof=0xC
{                                       // XREF: .data:visionDefFields/r
    const char *name;                   // XREF: LoadVisionSettingsFromBuffer+A6/r
    int offset;
    int fieldType;
};

void __cdecl CG_RegisterVisionSetsDvars();
void __cdecl CG_InitVisionSetsMenu();
void __cdecl CG_AddVisionSetMenuItem(XAssetHeader header);
void __cdecl CG_InitVisionSets(int localClientNum);
double __cdecl CalculateFlareRatio(float flareRatio, float flareRatioLastFrame);
void __cdecl CG_VisionSetsUpdate(int localClientNum);
void __cdecl UpdateVarsLerpCustom(
                float flareRatio,
                const visionSetVars_t *from,
                const visionSetVars_t *to,
                visionSetLerpData_t *lerpData,
                visionSetVars_t *result);
bool __cdecl LerpBool(bool from, bool to, float fraction, visionSetLerpStyle_t style);
int __cdecl LerpInteger(int from, int to, float fraction, visionSetLerpStyle_t style);
double __cdecl LerpStepFloat(float from, float to, float fraction, visionSetLerpStyle_t style);
double __cdecl LerpFloat(float from, float to, float fraction, visionSetLerpStyle_t style);
void __cdecl LerpVec3(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result);
void __cdecl LerpVec4(float *from, float *to, float fraction, visionSetLerpStyle_t style, float *result);
void __cdecl UpdateVarsLerp(
                int time,
                const visionSetVars_t *from,
                const visionSetVars_t *to,
                visionSetLerpData_t *lerpData,
                visionSetVars_t *result);
char __cdecl VisionSetCurrent(int localClientNum, visionSetMode_t mode, const char *name);
char __cdecl GetVisionSet(int localClientNum, const char *name, visionSetVars_t *resultSettings);
char __cdecl LoadVisionFile(const char *name, visionSetVars_t *resultSettings);
char *__cdecl RawBufferOpen(const char *name, const char *formatFullPath);
char __cdecl LoadVisionSettingsFromBuffer(const char *buffer, const char *filename, visionSetVars_t *settings);
bool __cdecl ApplyTokenToField(unsigned int fieldNum, char *token, visionSetVars_t *settings);
bool __cdecl CG_VisionSetStartLerp_To(
                int localClientNum,
                visionSetMode_t mode,
                visionSetLerpStyle_t style,
                const char *nameTo,
                int duration);
void __cdecl SetDefaultVision(int localClientNum);
void __cdecl CG_VisionSetConfigString_Naked(int localClientNum);
void __cdecl CG_VisionSetConfigString_Night(int localClientNum);
void __cdecl CG_VisionSetMyChanges();
void __cdecl CG_VisionSetUpdateTweaksFromFile_Film();
bool __cdecl LoadVisionFileForTweaks(visionSetVars_t *setVars);
void __cdecl CG_VisionSetUpdateTweaksFromFile_CharPrimary();
void __cdecl CG_VisionSetUpdateTweaksFromFile_Bloom();
bool __cdecl CG_InFlareVision(int localClientNum);
char __cdecl CG_LookingThroughNightVision(int localClientNum);
void __cdecl DrawVisionSetDebug(int localClientNum, visionSetMode_t curChannel);
void __cdecl CG_VisionSetApplyToRefdef(int localClientNum, bool isExtracam);
void __cdecl CG_InitExposure(int localClientNum, char *mapname);
