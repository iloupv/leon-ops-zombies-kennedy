#include "snd_utils.h"
#include "snd.h"
#include <universal/assertive.h>
#include <universal/com_math.h>
#include <cfloat>
#include <qcommon/common.h>
#include <string.h>
#include <ctype.h>

snd_speaker_config snd_speaker_configs[3] =
{
  {
    "Stereo",
    (snd_speaker_flag)(SND_SPEAKER_FLAG_LEFT | SND_SPEAKER_FLAG_RIGHT),
    2u,
    2u,
    2u,
    {
      { SND_SPEAKER_FLAG_LEFT, 5.4977875 },
      { SND_SPEAKER_FLAG_RIGHT, 0.78539819 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 }
    }
  },
  {
    "5.1",
    (snd_speaker_flag)(SND_SPEAKER_FLAG_LEFT | SND_SPEAKER_FLAG_RIGHT | SND_SPEAKER_FLAG_CENTER | SND_SPEAKER_FLAG_LFE | SND_SPEAKER_FLAG_LEFT_SURROUND | SND_SPEAKER_FLAG_RIGHT_SURROUND),
    6u,
    6u,
    4u,
    {
      { SND_SPEAKER_FLAG_LEFT, 5.4977875 },
      { SND_SPEAKER_FLAG_RIGHT, 0.78539819 },
      { SND_SPEAKER_FLAG_RIGHT_SURROUND, 2.0420351 },
      { SND_SPEAKER_FLAG_LEFT_SURROUND, 3.926991 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 }
    }
  },
  {
    "7.1",
    (snd_speaker_flag)(SND_SPEAKER_FLAG_LEFT | SND_SPEAKER_FLAG_RIGHT | SND_SPEAKER_FLAG_CENTER | SND_SPEAKER_FLAG_LFE | SND_SPEAKER_FLAG_LEFT_SURROUND | SND_SPEAKER_FLAG_RIGHT_SURROUND | SND_SPEAKER_FLAG_LEFT_REAR_SURROUND | SND_SPEAKER_FLAG_RIGHT_REAR_SURROUND),
    8u,
    6u,
    6u,
    {
      { SND_SPEAKER_FLAG_LEFT, 5.4977875 },
      { SND_SPEAKER_FLAG_RIGHT, 0.78539819 },
      { SND_SPEAKER_FLAG_RIGHT_SURROUND, 1.9198623 },
      { SND_SPEAKER_FLAG_RIGHT_REAR_SURROUND, 2.0420351 },
      { SND_SPEAKER_FLAG_LEFT_REAR_SURROUND, 3.926991 },
      { SND_SPEAKER_FLAG_LEFT_SURROUND, 4.3633232 },
      { (snd_speaker_flag)0, 0.0 },
      { (snd_speaker_flag)0, 0.0 }
    }
  }
};



unsigned int __cdecl SND_GetSpeakerConfigCount()
{
    return 3;
}

const snd_speaker_config *__cdecl Snd_GetSpeakerConfig(unsigned int index)
{
    if ( index >= 3
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    86,
                    0,
                    "%s",
                    "index < SND_SPEAKER_CONFIG_COUNT") )
    {
        __debugbreak();
    }
    return &snd_speaker_configs[index];
}

unsigned int __cdecl Snd_GetMixChannelCount(unsigned int speakerConfig)
{
    return Snd_GetSpeakerConfig(speakerConfig)->speakerCount;
}

double __cdecl Snd_PanMono(float angle)
{
    float v2; // [esp+8h] [ebp-24h]
    float D; // [esp+18h] [ebp-14h]
    float s; // [esp+28h] [ebp-4h]
    float anglea; // [esp+34h] [ebp+8h]

    anglea = Snd_NormalizeAngle(angle);
    D = (float)(5.4977875 - 0.78539819) / 2.0;
    if ( anglea > 0.78539819 || anglea < 5.4977875 )
        return 1.0;
    if ( (float)((float)(anglea - 0.78539819) - (float)(5.4977875 - anglea)) < 0.0 )
        v2 = anglea - 0.78539819;
    else
        v2 = 5.4977875 - anglea;
    s = v2 / D;
    if ( ((float)(v2 / D) < 0.0 || s > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    152,
                    0,
                    "%s\n\t(s) = %g",
                    "(s >= 0.0f && s <= 1.0f)",
                    s) )
    {
        __debugbreak();
    }
    return 0.2 * 0.69999999 + (1.0 - 0.69999999) * s;
}

double __cdecl Snd_NormalizeAngle(float x)
{
    while ( x >= 6.2831855 )
        x = x - 6.2831855;
    while ( x < 0.0 )
        x = x + 6.2831855;
    return x;
}

// aislop cleanup
void __cdecl Snd_PanStereo(float angle, float boost, float *left, float *right)
{
    // Float literals throughout: anglea is float, and using double constants
    // (e.g. `3.1415927`) would promote anglea to double for comparison. The
    // float bit-pattern of pi (0x40490FDB = 3.14159274...) is strictly greater
    // than the double 3.1415927 (= 3.14159270...), so an anglea exactly equal
    // to pi-as-float would take the wrong branch and trip the (v6 > pi) assert.
    const float A = Snd_PanMono(angle);
    const float anglea = Snd_NormalizeAngle(angle + 1.5707964f);
    const float v6 = (anglea > 3.1415927f) ? (6.2831855f - anglea) : anglea;
    iassert(v6 <= 3.1415927f);

    SND_EqualPowerFadeCoefs(v6 * 0.31830987f, left, right); // zombies (L16): SP 0x004484C0 mulss [0x00A157F8]
    *left  = I_fmin(1.0f, I_fmax(0.0f, *left  * A + boost));
    *right = I_fmin(1.0f, I_fmax(0.0f, *right * A + boost));
}

void __cdecl Snd_Pan(unsigned int speakerCount, const float *angles, float toSound, float *levels)
{
    unsigned int k; // [esp+14h] [ebp-1Ch]
    unsigned int j; // [esp+18h] [ebp-18h]
    unsigned int i; // [esp+1Ch] [ebp-14h]
    float spread; // [esp+20h] [ebp-10h]
    unsigned int rightIndex; // [esp+24h] [ebp-Ch]
    unsigned int leftIndex; // [esp+28h] [ebp-8h]
    float sound; // [esp+2Ch] [ebp-4h]

    leftIndex = -1;
    rightIndex = -1;
    for ( i = 0; i < speakerCount; ++i )
    {
        if ( angles[i] < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 182, 0, "%s", "angles[i] >= 0") )
        {
            __debugbreak();
        }
        if ( angles[i] >= 6.2831855
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                        183,
                        0,
                        "%s",
                        "angles[i] < SND_2PI") )
        {
            __debugbreak();
        }
        levels[i] = 0.0f;
    }
    for ( j = 0; j < speakerCount; ++j )
    {
        if ( Snd_AngleInInterval(toSound, angles[j], angles[(j + 1) % speakerCount]) )
        {
            leftIndex = j;
            rightIndex = (j + 1) % speakerCount;
            break;
        }
    }
    if ( leftIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    197,
                    0,
                    "%s",
                    "(leftIndex != 0xFFFFFFFF)") )
    {
        __debugbreak();
    }
    if ( rightIndex == -1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    198,
                    0,
                    "%s",
                    "(rightIndex != 0xFFFFFFFF)") )
    {
        __debugbreak();
    }
    spread = Snd_NormalizeAngle(angles[rightIndex] - angles[leftIndex]);
    sound = Snd_NormalizeAngle(toSound - angles[leftIndex]);
    if ( (float)(spread - sound) <= -0.0000152879
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    203,
                    0,
                    "%s",
                    "spread-sound > -SND_EPSILON") )
    {
        __debugbreak();
    }
    SND_EqualPowerFadeCoefs(sound / spread, &levels[leftIndex], &levels[rightIndex]);
    for ( k = 0; k < speakerCount; ++k )
    {
        //if ( (LODWORD(levels[k]) & 0x7F800000) == 0x7F800000
        //    && !Assert_MyHandler(
        //                "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
        //                209,
        //                0,
        //                "%s",
        //                "!IS_NAN(levels[i])") )
        //{
        //    __debugbreak();
        //}
        if ( levels[k] < 0.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 210, 0, "%s", "levels[i] >= 0.0f") )
        {
            __debugbreak();
        }
        if ( levels[k] > 1.0
            && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 211, 0, "%s", "levels[i] <= 1.0f") )
        {
            __debugbreak();
        }
    }
}

bool __cdecl Snd_AngleInInterval(float angle, float left, float right)
{
    float anglea; // [esp+14h] [ebp+8h]
    float lefta; // [esp+18h] [ebp+Ch]
    float righta; // [esp+1Ch] [ebp+10h]

    anglea = Snd_NormalizeAngle(angle);
    lefta = Snd_NormalizeAngle(left);
    righta = Snd_NormalizeAngle(right);
    if ( righta <= lefta )
        return anglea >= lefta || righta >= anglea;
    else
        return righta >= anglea && anglea >= lefta;
}

void __cdecl Snd_Pan3d(
                const snd_speaker_config *config,
                const float *position,
                const float *forward,
                const float *listener,
                float centerSend,
                float lfeSend,
                float aliasOmni,
                snd_speaker_map *pan)
{
    double v8; // st7
    //double v9; // xmm0_8
    //double v10; // xmm0_8
    //long double v11; // [esp+10h] [ebp-E8h]
    //long double v12; // [esp+10h] [ebp-E8h]
    float v13; // [esp+10h] [ebp-E8h]
    float v14; // [esp+14h] [ebp-E4h]
    //long double v15; // [esp+18h] [ebp-E0h]
    //long double v16; // [esp+18h] [ebp-E0h]
    float v17; // [esp+20h] [ebp-D8h]
    float v18; // [esp+28h] [ebp-D0h]
    float v19; // [esp+2Ch] [ebp-CCh]
    float v20; // [esp+30h] [ebp-C8h]
    float v21; // [esp+38h] [ebp-C0h]
    //float v22; // [esp+48h] [ebp-B0h]
    unsigned int j; // [esp+78h] [ebp-80h]
    float scale; // [esp+80h] [ebp-78h]
    signed int out; // [esp+84h] [ebp-74h]
    unsigned int m; // [esp+88h] [ebp-70h]
    unsigned int k; // [esp+8Ch] [ebp-6Ch]
    unsigned int i; // [esp+90h] [ebp-68h]
    float d; // [esp+94h] [ebp-64h]
    float angle; // [esp+98h] [ebp-60h]
    float omniBaseVolume; // [esp+9Ch] [ebp-5Ch]
    float levels[8]; // [esp+A0h] [ebp-58h] BYREF
    float angles[8]; // [esp+C0h] [ebp-38h] BYREF
    float d2; // [esp+E0h] [ebp-18h]
    float totalVolume; // [esp+E4h] [ebp-14h]
    float to[3]; // [esp+E8h] [ebp-10h]
    float omni; // [esp+F4h] [ebp-4h]

    iassert(config);
    iassert(!IS_NAN((position)[0]) && !IS_NAN((position)[1]) && !IS_NAN((position)[2]));
    iassert(!IS_NAN((listener)[0]) && !IS_NAN((listener)[1]) && !IS_NAN((listener)[2]));
    iassert(centerSend >= 0.0f && centerSend <= 1.0f);
    iassert(lfeSend >= 0.0f && lfeSend <= 1.0f);
    iassert(aliasOmni >= 0.0f && aliasOmni <= 1.0f);
    iassert(Vec3Length(forward) > 1.0f - (1.52879e-5f) && Vec3Length(forward) < 1.0f + (1.52879e-5f));
    iassert(Vec3Length(forward));
    iassert(pan);
    iassert(pan->input_channel_count == 1);

    to[0] = *position - *listener;
    to[1] = position[1] - listener[1];
    to[2] = position[2] - listener[2];
    d2 = (float)(to[0] * to[0]) + (float)(to[1] * to[1]);
    d = sqrtf(d2);
    angle = 0.0f;
    omni = 0.0f;

    for ( i = 0; i < config->angleCount; ++i )
    {
        levels[i] = 0.0f;
        angles[i] = config->angles[i].angle;
    }

    if ( d2 <= 0.0000152879 )
    {
        omni = 1.0f;
    }
    else
    {
        // aislop
        // The engine's angle convention is CW from +X (forward): 0=forward,
        // pi/2=right, pi=behind, 3pi/2=left — matching the speaker table where
        // LEFT sits at 315 deg and RIGHT at 45 deg. The original binary calls
        // __libm_sse2_atan2 with xmm0=X, xmm1=Y, which produces atan2(X, Y) =
        // pi/2 - atan2(Y, X). Standard atan2f(Y, X) here would invert the
        // angle and swap L/R, so we keep the (X, Y) arg order.
        // zombies (L16): SP Snd_Pan3d 0x004C2050 takes the double atan2 (0x00967B8A, cvtss2sd / cvtsd2ss), not atan2f,
        // and scales the distance by 0.1f (0x004C2144 mulss [0x00A2A2F8]), not / 10.0.
        const float azimuthTo      = (float)atan2((double)to[0],      (double)to[1]);
        const float azimuthForward = (float)atan2((double)forward[0], (double)forward[1]);
        angle = Snd_NormalizeAngle(azimuthTo - azimuthForward);

        const float dScaled = d * 0.1f;
        if ( (float)(dScaled - 1.0f) < 0.0f )
            v21 = dScaled;
        else
            v21 = 1.0f;
        if ( (float)(0.0f - dScaled) < 0.0f )
            v17 = v21;
        else
            v17 = 0.0f;
        omni = 1.0 - v17;
    }

    v20 = 1.0 / (double)config->speakerCount;
    omniBaseVolume = sqrtf(v20);
    if ( omni != 1.0 )
    {
        if ( config->angleCount == 1 )
        {
            levels[0] = Snd_PanMono(angle);
        }
        else if ( config->angleCount == 2 )
        {
            Snd_PanStereo(angle, centerSend, levels, &levels[1]);
        }
        else
        {
            Snd_Pan(config->angleCount, angles, angle, levels);
        }
    }
    totalVolume = 0.0f;
    for ( k = 0; k < config->angleCount; ++k )
    {
        levels[k] = (float)((float)(1.0 - omni) * levels[k]) + (float)(omniBaseVolume * omni);
        totalVolume = (float)(levels[k] * levels[k]) + totalVolume;
    }
    v19 = sqrtf(totalVolume);
    if ( (float)(v19 - 1.0) < 0.0 )
        v18 = v19;
    else
        v18 = 1.0f;
    if ( (float)(0.0 - v19) < 0.0 )
        v14 = v18;
    else
        v14 = 0.0f;
    totalVolume = v14;
    for ( m = 0; m < config->angleCount; ++m )
        levels[m] = (float)((float)(1.0 - aliasOmni) * levels[m]) + (float)(totalVolume * aliasOmni);
    if ( pan->output_channel_count != config->speakerCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    296,
                    0,
                    "%s",
                    "pan->output_channel_count == (int)config->speakerCount") )
    {
        __debugbreak();
    }
    for ( out = 0; out < (signed int)config->speakerCount; ++out )
    {
        scale = 0.0f;
        if ( (config->outputFormat & (1 << out)) != 0 )
        {
            for ( j = 0; j < config->angleCount; ++j )
            {
                if ( config->angles[j].speaker == 1 << out )
                {
                    scale = levels[j];
                    break;
                }
            }
            if ( out == 2 )
            {
                if ( (float)(centerSend - scale) < 0.0 )
                    v13 = scale;
                else
                    v13 = centerSend;
                scale = v13;
            }
            else if ( out == 3 )
            {
                scale = lfeSend;
            }
        }
        iassert(!IS_NAN(scale));
        iassert(scale >= 0.0f);
        iassert(scale <= 1.0f);
        Snd_SpeakerMapSetVolume(pan, 0, out, scale);
    }
}

void __cdecl SND_EqualPowerFadeCoefs(float t, float *a, float *b)
{
    iassert(!IS_NAN(t));

    if (t < 0.0f) t = 0.0f;
    else if (t > 1.0f) t = 1.0f;

    float angle = t * (3.1415927f / 2.0f);

    // KB decompilation fix (MP and zombies): SP 0x004B43C0 / MP 0x006066B0 take the double cos/sin, round to float and
    // clamp each coefficient to [0,1] (x - 1 >= 0 -> 1, then -x >= 0 -> 0; SP 0x004B441C..0x004B447E), so t = 1
    // gives a = 0 exactly, not cos(pi/2 float) = -4.4e-8.
    auto clamp01 = [](float x) -> float
    {
        const float hi = (x - 1.0f >= 0.0f) ? 1.0f : x;
        return (-x >= 0.0f) ? 0.0f : hi;
    };
    *a = clamp01((float)cos((double)angle));
    *b = clamp01((float)sin((double)angle));

    iassert(!IS_NAN(*a));
    iassert(!IS_NAN(*b));
}

void __cdecl SND_GetNearestPointOnSegment(
                const float *P,
                const float *segmentA,
                const float *segmentB,
                float *nearPoint)
{
    float BA; // [esp+30h] [ebp-20h]
    float BA_4; // [esp+34h] [ebp-1Ch]
    float BA_8; // [esp+38h] [ebp-18h]
    float fraction; // [esp+48h] [ebp-8h]
    float segmentLengthSq; // [esp+4Ch] [ebp-4h]

    nanassertvec3(P);
    nanassertvec3(segmentA);
    nanassertvec3(segmentB);

    *nearPoint = 0.0f;
    nearPoint[1] = 0.0f;
    nearPoint[2] = 0.0f;
    BA = *segmentB - *segmentA;
    BA_4 = segmentB[1] - segmentA[1];
    BA_8 = segmentB[2] - segmentA[2];
    segmentLengthSq = (float)((float)(BA * BA) + (float)(BA_4 * BA_4)) + (float)(BA_8 * BA_8);
    if ( segmentLengthSq >= 1.0 )
    {
        fraction = (float)((float)((float)(BA * (float)(*P - *segmentA)) + (float)(BA_4 * (float)(P[1] - segmentA[1])))
                                         + (float)(BA_8 * (float)(P[2] - segmentA[2])))
                         / segmentLengthSq;
        if ( fraction >= 0.0 )
        {
            if ( fraction <= 1.0 )
            {
                *nearPoint = (float)(fraction * BA) + *segmentA;
                nearPoint[1] = (float)(fraction * BA_4) + segmentA[1];
                nearPoint[2] = (float)(fraction * BA_8) + segmentA[2];
            }
            else
            {
                *nearPoint = *segmentB;
                nearPoint[1] = segmentB[1];
                nearPoint[2] = segmentB[2];
            }
        }
        else
        {
            *nearPoint = *segmentA;
            nearPoint[1] = segmentA[1];
            nearPoint[2] = segmentA[2];
        }
        nanassertvec3(nearPoint);

    }
    else
    {
        *nearPoint = *segmentA;
        nearPoint[1] = segmentA[1];
        nearPoint[2] = segmentA[2];
    }
}

void __cdecl SND_GetNearestPointOnStrip(
                const float *point,
                const float (*verts)[3],
                unsigned int count,
                float *position)
{
    float d; // [esp+Ch] [ebp-1Ch]
    unsigned int i; // [esp+10h] [ebp-18h]
    bool copied; // [esp+17h] [ebp-11h]
    float nearestDistance; // [esp+18h] [ebp-10h]
    float nearestOnSegment[3]; // [esp+1Ch] [ebp-Ch] BYREF

    nearestDistance = FLT_MAX;
    copied = 0;
    for ( i = 0; i < count - 1; ++i )
    {
        SND_GetNearestPointOnSegment(point, &(*verts)[3 * i], &(*verts)[3 * i + 3], nearestOnSegment);
        d = Vec3DistanceSq(point, nearestOnSegment);
        if ( nearestDistance > d )
        {
            nearestDistance = d;
            *position = nearestOnSegment[0];
            position[1] = nearestOnSegment[1];
            position[2] = nearestOnSegment[2];
            copied = 1;
        }
    }
    if ( !copied && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 411, 0, "%s", "copied") )
        __debugbreak();
}

unsigned int __cdecl SND_HashAlias(const snd_alias_list_t *alias)
{
    return alias->id;
}

const char *__cdecl SND_GetAliasName(const snd_alias_list_t *alias)
{
    if ( alias && alias->name )
        return alias->name;
    else
        return "null";
}

// aislop cleanup
float __cdecl SND_dBToLinear(float db)
{
    // Compute 10^(db / 20)
    // zombies (L16): SP 0x0067D9B0: pow(10.0, (double)(db * 0.05f)) - the exponent is a float product (0.05f at 0x009EFB40)
    float x = static_cast<float>(pow(10.0, (double)(db * 0.05f)));

    // Clamp to minimum value
    if (x < 0.0000152879f)
        x = 0.0f;

    // First branch: clamp max at 1.0
    float v = (x < 1.0f) ? x : 1.0f;

    // Second branch: ensure non-negative
    if (x <= 0.0f)
        v = 0.0f;

    return v;
}
double __cdecl SND_LinearToDb(float linear)
{
    float v2; // [esp+0h] [ebp-4h]

    if ( linear < 0.0000152879 )
        linear = 0.0000152879f;
    v2 = log10(linear);
    return v2 * 20.0;
}

double __cdecl SND_LinearToDbSpl(float linear)
{
    float db; // [esp+4h] [ebp-4h]

    db = SND_LinearToDb(linear);
    if ( db > -95.0 )
        return (float)(db + 100.0);
    else
        return 0.0f;
}

double __cdecl SND_dBSPLToLinear(float value)
{
    return SND_dBToLinear(value);
}

int __cdecl SND_HashName(const char *name)
{
    unsigned int v2; // kr00_4
    unsigned int i; // [esp+10h] [ebp-Ch]
    int hash; // [esp+14h] [ebp-8h]

    if ( !name || !*name )
        return 0;
    v2 = strlen(name);
    hash = 5381;
    for ( i = 0; i < v2; ++i )
        hash = (hash << 16) + (hash << 6) + tolower(name[i]) - hash;
    if ( !hash )
        return 1;
    return hash;
}

void __cdecl Snd_SpeakerMapSetVolume(snd_speaker_map *map, int in, int out, float volume)
{
    if ( (volume < 0.0 || volume > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    523,
                    0,
                    "%s\n\t(volume) = %g",
                    "(volume >= 0.0f && volume <= 1.0f)",
                    volume) )
    {
        __debugbreak();
    }
    map->volumes[Snd_SpeakerMapGetIndex(map, in, out)] = volume;
}

void __cdecl Snd_SpeakerMapZero(snd_speaker_map *map)
{
    int i; // [esp+0h] [ebp-4h]

    for ( i = 0; i < 16; ++i )
        map->volumes[i] = 0.0f;
}

int __cdecl Snd_SpeakerMapGetIndex(const snd_speaker_map *map, int in, int out)
{
    int i; // [esp+0h] [ebp-4h]

    if ( in >= map->input_channel_count
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    568,
                    0,
                    "%s",
                    "in < map->input_channel_count") )
    {
        __debugbreak();
    }
    if ( out >= map->output_channel_count
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    569,
                    0,
                    "%s",
                    "out < map->output_channel_count") )
    {
        __debugbreak();
    }
    i = out + map->output_channel_count * in;
    if ( i >= 16
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    571,
                    0,
                    "%s",
                    "i < SND_CHANNEL_MAP_VOLUME_COUNT") )
    {
        __debugbreak();
    }
    return i;
}

double __cdecl Snd_SpeakerMapGetVolume(const snd_speaker_map *map, int in, int out)
{
    float volume; // [esp+0h] [ebp-4h]

    volume = map->volumes[Snd_SpeakerMapGetIndex(map, in, out)];
    if ( volume < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 579, 0, "%s", "volume >= 0.0f") )
    {
        __debugbreak();
    }
    if ( volume > 1.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 580, 0, "%s", "volume <= 1.0f") )
    {
        __debugbreak();
    }
    return volume;
}

void __cdecl SND_PanToSpeakermap(
                unsigned int inputChannelCount,
                unsigned int outputChannelCount,
                const snd_pan *pan,
                float centerSend,
                snd_speaker_map *map)
{
    float v5; // [esp+4h] [ebp-2Ch]
    float v6; // [esp+Ch] [ebp-24h]
    float v7; // [esp+14h] [ebp-1Ch]
    float volume; // [esp+18h] [ebp-18h]
    float center; // [esp+2Ch] [ebp-4h]
    float centera; // [esp+2Ch] [ebp-4h]
    float centerb; // [esp+2Ch] [ebp-4h]
    float centerc; // [esp+2Ch] [ebp-4h]
    float centerd; // [esp+2Ch] [ebp-4h]
    float centere; // [esp+2Ch] [ebp-4h]

    if ( !inputChannelCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 698, 0, "%s", "inputChannelCount") )
    {
        __debugbreak();
    }
    if ( !outputChannelCount
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp", 699, 0, "%s", "outputChannelCount") )
    {
        __debugbreak();
    }
    Snd_SpeakerMapZero(map);
    map->input_channel_count = inputChannelCount;
    map->output_channel_count = outputChannelCount;
    if ( inputChannelCount == 1 )
    {
        switch ( outputChannelCount )
        {
            case 1u:
                SND_PanToSpeakermap11(pan, map);
                return;
            case 2u:
                SND_PanToSpeakermap12(pan, map);
                return;
            case 6u:
                SND_PanToSpeakermap16(pan, map);
                center = Snd_SpeakerMapGetVolume(map, 0, 2);
                if ( (float)(center - centerSend) < 0.0 )
                    volume = centerSend;
                else
                    volume = center;
                Snd_SpeakerMapSetVolume(map, 0, 2, volume);
                return;
            case 8u:
                SND_PanToSpeakermap18(pan, map);
                centera = Snd_SpeakerMapGetVolume(map, 0, 2);
                if ( (float)(centera - centerSend) < 0.0 )
                    v7 = centerSend;
                else
                    v7 = centera;
                Snd_SpeakerMapSetVolume(map, 0, 2, v7);
                return;
        }
LABEL_44:
        Com_PrintError(9, "invalid channel count %d %d\n", inputChannelCount, outputChannelCount);
        return;
    }
    if ( inputChannelCount != 2 )
        goto LABEL_44;
    switch ( outputChannelCount )
    {
        case 1u:
            SND_PanToSpeakermap21(pan, map);
            return;
        case 2u:
            SND_PanToSpeakermap22(pan, map);
            return;
        case 6u:
            SND_PanToSpeakermap26(pan, map);
            centerb = Snd_SpeakerMapGetVolume(map, 0, 2);
            if ( (float)(centerb - (float)(centerSend * 0.70710677)) < 0.0 )
                Snd_SpeakerMapSetVolume(map, 0, 2, centerSend * 0.70710677);
            else
                Snd_SpeakerMapSetVolume(map, 0, 2, centerb);
            centerc = Snd_SpeakerMapGetVolume(map, 1, 2);
            if ( (float)(centerc - (float)(centerSend * 0.70710677)) < 0.0 )
                v6 = centerSend * 0.70710677;
            else
                v6 = centerc;
            Snd_SpeakerMapSetVolume(map, 1, 2, v6);
            break;
        case 8u:
            SND_PanToSpeakermap28(pan, map);
            centerd = Snd_SpeakerMapGetVolume(map, 0, 2);
            if ( (float)(centerd - (float)(centerSend * 0.70710677)) < 0.0 )
                Snd_SpeakerMapSetVolume(map, 0, 2, centerSend * 0.70710677);
            else
                Snd_SpeakerMapSetVolume(map, 0, 2, centerd);
            centere = Snd_SpeakerMapGetVolume(map, 1, 2);
            if ( (float)(centere - (float)(centerSend * 0.70710677)) < 0.0 )
                v5 = centerSend * 0.70710677;
            else
                v5 = centere;
            Snd_SpeakerMapSetVolume(map, 1, 2, v5);
            break;
        default:
            goto LABEL_44;
    }
}

void __cdecl SND_PanToSpeakermap11(const snd_pan *pan, snd_speaker_map *map)
{
    float volume; // [esp+4h] [ebp-Ch]
    float v3; // [esp+8h] [ebp-8h]
    float v4; // [esp+Ch] [ebp-4h]

    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    588,
                    0,
                    "%s",
                    "map->input_channel_count == 1") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    589,
                    0,
                    "%s",
                    "map->output_channel_count == 1") )
    {
        __debugbreak();
    }
    v3 = pan->front + pan->back;
    if ( (float)(v3 - 1.0) < 0.0 )
        v4 = pan->front + pan->back;
    else
        v4 = 1.0f;
    if ( (float)(0.0 - v3) < 0.0 )
        volume = v4;
    else
        volume = 0.0f;
    Snd_SpeakerMapSetVolume(map, 0, 0, volume);
}

void __cdecl SND_PanToSpeakermap12(const snd_pan *pan, snd_speaker_map *map)
{
    float v2; // [esp+4h] [ebp-18h]
    float v3; // [esp+8h] [ebp-14h]
    float v4; // [esp+Ch] [ebp-10h]
    float v5; // [esp+10h] [ebp-Ch]
    float v6; // [esp+14h] [ebp-8h]
    float v7; // [esp+18h] [ebp-4h]

    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    597,
                    0,
                    "%s",
                    "map->input_channel_count == 1") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    598,
                    0,
                    "%s",
                    "map->output_channel_count == 2") )
    {
        __debugbreak();
    }
    v6 = pan->front + pan->back;
    if ( (float)(v6 - 1.0) < 0.0 )
        v7 = pan->front + pan->back;
    else
        v7 = 1.0f;
    if ( (float)(0.0 - v6) < 0.0 )
        v3 = v7;
    else
        v3 = 0.0f;
    Snd_SpeakerMapSetVolume(map, 0, 0, v3 * pan->left);
    v4 = pan->front + pan->back;
    if ( (float)(v4 - 1.0) < 0.0 )
        v5 = pan->front + pan->back;
    else
        v5 = 1.0f;
    if ( (float)(0.0 - v4) < 0.0 )
        v2 = v5;
    else
        v2 = 0.0f;
    Snd_SpeakerMapSetVolume(map, 0, 1, v2 * pan->right);
}

void __cdecl SND_PanToSpeakermap16(const snd_pan *pan, snd_speaker_map *map)
{
    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    607,
                    0,
                    "%s",
                    "map->input_channel_count == 1") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    608,
                    0,
                    "%s",
                    "map->output_channel_count == 6") )
    {
        __debugbreak();
    }
    Snd_SpeakerMapSetVolume(map, 0, 0, pan->front * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 1, pan->front * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 2, pan->center);
    Snd_SpeakerMapSetVolume(map, 0, 3, pan->lfe);
    Snd_SpeakerMapSetVolume(map, 0, 4, pan->back * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 5, pan->back * pan->right);
}

void __cdecl SND_PanToSpeakermap18(const snd_pan *pan, snd_speaker_map *map)
{
    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    621,
                    0,
                    "%s",
                    "map->input_channel_count == 1") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    622,
                    0,
                    "%s",
                    "map->output_channel_count == 8") )
    {
        __debugbreak();
    }
    Snd_SpeakerMapSetVolume(map, 0, 0, pan->front * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 1, pan->front * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 2, pan->center);
    Snd_SpeakerMapSetVolume(map, 0, 3, pan->lfe);
    Snd_SpeakerMapSetVolume(map, 0, 4, (float)(pan->back * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 5, (float)(pan->back * 0.5) * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 6, (float)(pan->back * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 7, (float)(pan->back * 0.5) * pan->right);
}

void __cdecl SND_PanToSpeakermap21(const snd_pan *pan, snd_speaker_map *map)
{
    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    637,
                    0,
                    "%s",
                    "map->input_channel_count == 2") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 1
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    638,
                    0,
                    "%s",
                    "map->output_channel_count == 1") )
    {
        __debugbreak();
    }
    Snd_SpeakerMapSetVolume(map, 0, 0, pan->front * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 0, pan->front * pan->right);
}

void __cdecl SND_PanToSpeakermap22(const snd_pan *pan, snd_speaker_map *map)
{
    float total; // [esp+4h] [ebp-4h]

    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    647,
                    0,
                    "%s",
                    "map->input_channel_count == 2") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    648,
                    0,
                    "%s",
                    "map->output_channel_count == 2") )
    {
        __debugbreak();
    }
    total = (float)(pan->front + pan->back) / 2.0;
    Snd_SpeakerMapSetVolume(map, 0, 0, total * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 1, total * pan->right);
}

void __cdecl SND_PanToSpeakermap26(const snd_pan *pan, snd_speaker_map *map)
{
    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    658,
                    0,
                    "%s",
                    "map->input_channel_count == 2") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    659,
                    0,
                    "%s",
                    "map->output_channel_count == 6") )
    {
        __debugbreak();
    }
    Snd_SpeakerMapSetVolume(map, 0, 0, pan->front * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 1, pan->front * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 4, pan->back * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 5, pan->back * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 2, (float)(pan->center * 0.69999999) * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 3, (float)(pan->lfe * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 2, (float)(pan->center * 0.69999999) * pan->right);
    Snd_SpeakerMapSetVolume(map, 1, 3, (float)(pan->lfe * 0.5) * pan->right);
}

void __cdecl SND_PanToSpeakermap28(const snd_pan *pan, snd_speaker_map *map)
{
    Snd_SpeakerMapZero(map);
    if ( map->input_channel_count != 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    675,
                    0,
                    "%s",
                    "map->input_channel_count == 2") )
    {
        __debugbreak();
    }
    if ( map->output_channel_count != 8
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_utils.cpp",
                    676,
                    0,
                    "%s",
                    "map->output_channel_count == 8") )
    {
        __debugbreak();
    }
    Snd_SpeakerMapSetVolume(map, 0, 0, pan->front * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 1, pan->front * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 4, (float)(pan->back * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 5, (float)(pan->back * 0.5) * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 6, (float)(pan->back * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 7, (float)(pan->back * 0.5) * pan->right);
    Snd_SpeakerMapSetVolume(map, 0, 2, (float)(pan->center * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 0, 3, (float)(pan->lfe * 0.5) * pan->left);
    Snd_SpeakerMapSetVolume(map, 1, 2, (float)(pan->center * 0.5) * pan->right);
    Snd_SpeakerMapSetVolume(map, 1, 3, (float)(pan->lfe * 0.5) * pan->right);
}

