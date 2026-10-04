#pragma once

enum snd_overlay_type : __int32
{                                       // XREF: ?SND_GetSoundOverlay@@YAHW4snd_overlay_type@@QAUsnd_overlay_info@@H@Z/r
                                        // CG_DrawSoundOverlay/r
    SND_OVERLAY_NONE   = 0x0,
    SND_OVERLAY_3D     = 0x1,
    SND_OVERLAY_STREAM = 0x2,
    SND_OVERLAY_2D     = 0x3,
};

struct snd_overlay_info // sizeof=0xF0
{                                       // XREF: CG_DrawSoundOverlay/r
    char pszSampleName[128];            // XREF: CG_DrawSoundOverlay+90/r
                                        // CG_DrawSoundOverlay+166/r
    char name[64];                      // XREF: CG_DrawSoundOverlay+308/o
    int channel;                        // XREF: CG_DrawSoundOverlay+319/r
    float fGlobalPriority;              // XREF: CG_DrawSoundOverlay+2EE/r
    float fLosOcclusion;                // XREF: CG_DrawSoundOverlay+2DB/r
    float fDry;                         // XREF: CG_DrawSoundOverlay+2C8/r
    float fWet;                         // XREF: CG_DrawSoundOverlay+2B5/r
    float fDistance;                    // XREF: CG_DrawSoundOverlay+29B/r
    float fPitch;                       // XREF: CG_DrawSoundOverlay+281/r
    int iEntity;                        // XREF: CG_DrawSoundOverlay+270/r
    int group;
    int pan;
    __int64 played;
};

struct snd_listener;

void __cdecl SND_DebugInit();
void __cdecl SND_DebugFini();
void __cdecl SND_DebugDrawWorldSounds(int debugDrawStyle);
void __cdecl DebugDrawWorldSound3D(
                unsigned int idx,
                int debugDrawStyle,
                int *offsets,
                int *closestId,
                float *closestIdDotProd);
double __cdecl FontSizeForDistance(float distance);
void __cdecl SND_PlayLocal_f();
void __cdecl RelativeToListener(const snd_listener *listener, float yaw, float pitch, float dist, float *result);
int __cdecl SND_GetSoundOverlay(snd_overlay_info *info, int start, int count);
int __cdecl SND_GetSoundOverlay(snd_overlay_type type, snd_overlay_info *info, int maxcount);
