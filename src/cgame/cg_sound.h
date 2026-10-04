#pragma once
#include <sound/snd.h>
#include <flame/flame_sound.h>

struct CgSoundPlaybackTrackingInfo // sizeof=0x18
{                                       // XREF: .data:cg_SoundPlaybackTracking/r
    int entitynum;
    float origin[3];
    unsigned int aliasname;
    int wasInKillcam;
};

struct snd_fire_manager // sizeof=0x18
{                                       // XREF: .data:g_snd_fires/r
    int active;
    float location[3];
    float level;
    int id;
};

struct WeaponDef;

bool __cdecl CG_ShouldPlaySoundOnLocalClient(
                int localClientNum,
                int entitynum,
                const float *origin,
                unsigned int aliasname);
SndEntHandle __cdecl CG_SndEntHandle(int localClientNum, int entIndex);
SndEntHandle __cdecl SND_EntHandle(
                char localClientNum,
                __int16 entIndex,
                char useCount,
                bool isInKillCam,
                bool isStationary,
                team_t team);
void __cdecl CG_PlaySound(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float attenuation,
                unsigned int id);
void __cdecl CG_PlaySound(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float attenuation,
                char *name);
inline void CG_PlaySound(int localClientNum,
    int entitynum,
    float *origin,
    int fadeMs,
    bool doNotify,
    float attenuation,
    const char *name)
{
    CG_PlaySound(localClientNum,
        entitynum,
        origin,
        fadeMs,
        doNotify,
        attenuation,
        (char*)name);
}
void __cdecl CG_PlaySound(int localClientNum, float *origin, int fadeMs, bool doNotify, float attenuation, char *name);
inline void __cdecl CG_PlaySound(int localClientNum, float *origin, int fadeMs, bool doNotify, float attenuation, const char *name)
{
    CG_PlaySound(localClientNum, origin, fadeMs, doNotify, attenuation, (char *)name);
}

int __cdecl CG_PlaySoundWithHandle(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float volume,
                unsigned int id);
int __cdecl CG_PlaySoundWithHandle(
                int localClientNum,
                int entitynum,
                float *origin,
                int fadeMs,
                bool doNotify,
                float attenuation,
                char *name);
inline int CG_PlaySoundWithHandle(
    int localClientNum,
    int entitynum,
    float *origin,
    int fadeMs,
    bool doNotify,
    float attenuation,
    const char *name)
{
    return CG_PlaySoundWithHandle(
        localClientNum,
        entitynum,
        origin,
        fadeMs,
        doNotify,
        attenuation,
        (char *)name);
}
void __cdecl CG_PumpEntityLoopSound(int localClientNum, const centity_s *cent);
void __cdecl CG_PlayAnimScriptSoundAlias(int clientIndex, snd_alias_list_t *aliasList);
void __cdecl CG_SubtitlePrint(int msec, const char *subtitle);
void __cdecl CG_SubtitleSndLengthNotify(int msec, const char *lengthNotifyData);
void __cdecl CG_ScriptSndLengthNotify(int msec, int ent); // zombies (a1): SP 0x004534C0
int __cdecl CG_SoundGetUseCount(SndEntHandle sndEnt);
bool __cdecl CG_SoundEntInUse(SndEntHandle sndEnt);
void __cdecl CG_GetSoundEntityOrientation(
                SndEntHandle sndEnt,
                float *origin_out,
                float (*axis_out)[3],
                float *velocity_out);
void __cdecl CG_CopySoundEntityOrientation(int localClientNum, int entIndex, float *origin_out, float (*axis_out)[3]);
void __cdecl CG_StopSoundAlias(int localClientNum, int entityNum, unsigned int aliasName);
void __cdecl CG_FireManagerUpdate(unsigned int listener, unsigned int index, const float *location, float level);
void __cdecl CG_SndFireReset();
void __cdecl CG_SndUpdateFire();
void __cdecl CG_FlameSourceUpdate(
                int localClientNum,
                int entityNum,
                float *sourcePosition,
                float *flamePosition,
                snd_flame_source_flags flags,
                char *flameOffLoopSound,
                char *flameIgniteSound,
                char *flameOnLoopSound,
                char *flameCooldownSound);
void __cdecl CG_SoundWhizby(
                int localClientNum,
                const WeaponDef *weapDef,
                const float *shotPosition,
                const float *shotDirection,
                const float *position);
char __cdecl CG_SoundWhizbyPath(const float *p1, const float *p2, const float *p3, float r, float *start, float *end);
void __cdecl CG_SndUpdate();
void __cdecl CG_SndGameReset();
