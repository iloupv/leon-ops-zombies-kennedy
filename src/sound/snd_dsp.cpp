#include "snd_dsp.h"
#include <universal/com_math.h>
#include "snd.h"

void __cdecl SND_DspMul(unsigned int count, const float *a, const float *b, float *c)
{
    iassert((count & 3) == 0);
    iassert(((uintptr_t)a & 15) == 0);
    iassert(((uintptr_t)b & 15) == 0);
    iassert(((uintptr_t)c & 15) == 0);

    for (unsigned int i = 0; i < count; i += 4)
    {
        c[i + 0] = a[i + 0] * b[i + 0];
        c[i + 1] = a[i + 1] * b[i + 1];
        c[i + 2] = a[i + 2] * b[i + 2];
        c[i + 3] = a[i + 3] * b[i + 3];
    }
}

void __cdecl SND_DspScale(unsigned int count, float a, float *c)
{
    float *v3; // eax
    unsigned int v4; // ecx

    if ( (count & 3) != 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 160, 0, "%s", "(count&3) == 0") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)c & 0xF) != 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 161, 0, "%s", "(((int)c)&15) == 0") )
    {
        __debugbreak();
    }
    if ( count )
    {
        v3 = c + 2;
        v4 = ((count - 1) >> 2) + 1;
        do
        {
            *(v3 - 2) = *(v3 - 2) * a;
            *(v3 - 1) = *(v3 - 1) * a;
            *v3 = a * *v3;
            v3[1] = v3[1] * a;
            v3 += 4;
            --v4;
        }
        while ( v4 );
    }
}

void __cdecl SND_DspScaleCache(unsigned int count, float a, float *c)
{
    float *v3; // ebx
    unsigned int v4; // esi
    int v5; // eax

    v3 = c;

    iassert((count & 31) == 0);
    iassert((((int)c) & 31) == 0);

    if ( count >> 5 )
    {
        v4 = count >> 5;
        do
        {
            I_dcbt(128, (const char *)v3);
            v5 = 8;
            do
            {
                *v3 = *v3 * a;
                v3[1] = a * v3[1];
                v3[2] = a * v3[2];
                v3[3] = a * v3[3];
                v3 += 4;
                --v5;
            }
            while ( v5 );
            --v4;
        }
        while ( v4 );
    }
}

void __cdecl I_dcbt(int offset, const char *base)
{
    _mm_prefetch(&base[offset], 1);
}

void __cdecl SND_DspScale(unsigned int count, float a, const float *b, float *c)
{
    float *v4; // ebx
    const float *v5; // eax
    const float *v6; // ecx

    v4 = c;
    if ( (count & 3) != 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 200, 0, "%s", "(count&3) == 0") )
    {
        __debugbreak();
    }
    if ( ((unsigned __int8)c & 0xF) != 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 201, 0, "%s", "(((int)c)&15) == 0") )
    {
        __debugbreak();
    }
    v5 = b;
    v6 = &b[count];
    if ( b < v6 )
    {
        do
        {
            *v4 = *v5 * a;
            v4[1] = v5[1] * a;
            v4[2] = v5[2] * a;
            v4[3] = v5[3] * a;
            v5 += 4;
            v4 += 4;
        }
        while ( v5 < v6 );
    }
}

void __cdecl SND_DspSum(unsigned int count, const float *a, float *c)
{
    iassert((count & 3) == 0);
    iassert(((uintptr_t)a & 15) == 0);
    iassert(((uintptr_t)c & 15) == 0);
    iassert(a != c);

    for (unsigned int i = 0; i < count; i += 4)
    {
        c[i + 0] += a[i + 0];
        c[i + 1] += a[i + 1];
        c[i + 2] += a[i + 2];
        c[i + 3] += a[i + 3];
    }
}

void __cdecl SND_DspOnePoleFilterMono(unsigned int count, float *frames, float b0, float a1, float *y1)
{
    float *v5; // ecx
    float *v6; // edx

    v5 = frames + 1;
    v6 = &frames[count];
    *frames = (float)(*frames * b0) - (float)(*y1 * a1);
    if ( frames + 1 != v6 )
    {
        do
        {
            *v5 = (float)(*v5 * b0) - (float)(*(v5 - 1) * a1);
            ++v5;
        }
        while ( v5 != v6 );
    }
    *y1 = *(v6 - 1);
}

void __cdecl SND_OcclusionLpfCoef(float occlusionLevel, float occlusionRatio, float sampleRate, float *b0, float *a1)
{
    float *v7; // eax
    float *v8; // ebx
    float max; // [esp+8h] [ebp-1Ch]
    float maxa; // [esp+8h] [ebp-1Ch]
    float maxb; // [esp+8h] [ebp-1Ch]
    float k; // [esp+18h] [ebp-Ch]
    float y; // [esp+1Ch] [ebp-8h]
    float ya; // [esp+1Ch] [ebp-8h]
    float x; // [esp+20h] [ebp-4h]
    float sampleRatea; // [esp+34h] [ebp+10h]

    if ( sampleRate <= 100.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 424, 0, "%s", "sampleRate > 100.0f") )
    {
        __debugbreak();
    }
    if ( sampleRate >= 100000.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    425,
                    0,
                    "%s",
                    "sampleRate < 100000.0f") )
    {
        __debugbreak();
    }
    x = I_fclamp(occlusionLevel, -80.0, 0.0);
    if ( x >= -0.000001 )
    {
        v7 = b0;
        v8 = a1;
        *b0 = 1.0f;
        *a1 = 0.0;
    }
    else
    {
        // zombies (L16): the exe's single-precision SSE arithmetic with the double CRT pow / cos (SP 0x004B7AA9..0x004B7BA9):
        // ya = 10^((1-y)*x*0.1f), c = cos(31415.928f / rate), v = ((1 - c*ya) - sqrt((1-c)*ya*2 - (1-c*c)*ya*ya)) / (1 - ya),
        // k = 10^(y*x*0.05f), b0 = k*(1-v), a1 = -v - not the double intermediates and powf / cosf of the decompilation.
        y = I_fclamp(occlusionRatio, 0.0, 0.99000001);
        maxa = (float)((float)(1.0f - y) * x) * 0.1f;
        ya = (float)pow(10.0, (double)maxa);
        const float c = (float)cos((double)(31415.928f / sampleRate));
        sampleRatea = 1.0f - (float)(c * ya);
        maxb = (float)((float)((float)(1.0f - c) * ya) * 2.0f) - (float)((float)((float)(1.0f - (float)(c * c)) * ya) * ya);
        const float root = sqrtf(maxb);
        const float v = (float)(sampleRatea - root) / (float)(1.0f - ya);
        max = (float)(y * x) * 0.05f;
        k = (float)pow(10.0, (double)max);
        v7 = b0;
        v8 = a1;
        *b0 = k * (float)(1.0f - v);
        *a1 = -v;
    }
    if ( IS_NAN(*v7)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 449, 0, "%s", "!IS_NAN(*b0)") )
    {
        __debugbreak();
    }
    if ( IS_NAN(*v8) )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 450, 0, "%s", "!IS_NAN(*a1)") )
            __debugbreak();
    }
}

double __cdecl I_fclamp(float val, float min, float max)
{
    float v5; // [esp+4h] [ebp-4h]

    if ( (float)(val - max) < 0.0 )
        v5 = val;
    else
        v5 = max;
    if ( (float)(min - val) < 0.0 )
        return v5;
    else
        return min;
}

void __cdecl SND_DspBiquadInPlace(
                const snd_dsp_biquad_coef *coef,
                snd_dsp_biquad_state *state,
                unsigned int count,
                float *input)
{
    unsigned int v5; // edx
    float v7; // xmm3_4
    float v8; // xmm1_4
    float v9; // xmm2_4
    float v10; // xmm0_4
    float *v11; // ecx
    float v12; // xmm4_4
    float v13; // xmm6_4
    float v14; // [esp+0h] [ebp-10h]
    float v15; // [esp+4h] [ebp-Ch]
    float v16; // [esp+8h] [ebp-8h]
    float v17; // [esp+Ch] [ebp-4h]
    float coefa; // [esp+18h] [ebp+8h]
    float statea; // [esp+1Ch] [ebp+Ch]

    v5 = 0;
    if ( count )
    {
        v7 = state->x[1];
        v8 = state->x[0];
        v9 = state->y[1];
        v17 = coef->s[2];
        statea = coef->s[1];
        coefa = coef->s[0];
        v16 = coef->s[3];
        v15 = coef->s[4];
        v10 = state->y[0];
        do
        {
            v11 = &input[v5];
            v12 = v7;
            v7 = v8;
            v8 = *v11;
            v14 = v9;
            v9 = v10;
            ++v5;
            v13 = (float)((float)((float)((float)(statea * v7) + (float)(coefa * *v11)) + (float)(v17 * v12))
                                    - (float)(v16 * v10))
                    - (float)(v15 * v14);
            v10 = v13;
            *v11 = v13;
        }
        while ( v5 < count );
        state->x[1] = v7;
        state->x[2] = v12;
        state->x[0] = v8;
        state->y[1] = v9;
        state->y[2] = v14;
        state->y[0] = v13;
    }
}

void __cdecl SND_DspBiquadNormalize(float *a, float *b, snd_dsp_biquad_coef *coef)
{
    if ( IS_NAN(*b)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 838, 0, "%s", "!IS_NAN(b[0])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(b[1])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 839, 0, "%s", "!IS_NAN(b[1])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(b[2])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 840, 0, "%s", "!IS_NAN(b[2])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(*a)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 841, 0, "%s", "!IS_NAN(a[0])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(a[1])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 842, 0, "%s", "!IS_NAN(a[1])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(a[2])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 843, 0, "%s", "!IS_NAN(a[2])") )
    {
        __debugbreak();
    }
    coef->s[0] = *b / *a;
    coef->s[1] = b[1] / *a;
    coef->s[2] = b[2] / *a;
    coef->s[3] = a[1] / *a;
    coef->s[4] = a[2] / *a;
    if ( IS_NAN(coef->s[0])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 856, 0, "%s", "!IS_NAN(coef->s[0])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(coef->s[1])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 857, 0, "%s", "!IS_NAN(coef->s[1])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(coef->s[2])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 858, 0, "%s", "!IS_NAN(coef->s[2])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(coef->s[3])
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 859, 0, "%s", "!IS_NAN(coef->s[3])") )
    {
        __debugbreak();
    }
    if ( IS_NAN(coef->s[4]) )
    {
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 860, 0, "%s", "!IS_NAN(coef->s[4])") )
            __debugbreak();
    }
}

void __cdecl SND_DspBiquadHShelve(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef)
{
    float v5; // xmm1_4
    float v6; // xmm3_4
    float v7; // xmm2_4
    float a[3]; // [esp+14h] [ebp-18h] BYREF
    float b[3]; // [esp+20h] [ebp-Ch] BYREF
    float alpha; // [esp+34h] [ebp+8h]
    float alphaa; // [esp+34h] [ebp+8h]
    float alphab; // [esp+34h] [ebp+8h]
    float w0; // [esp+38h] [ebp+Ch]
    float w0a; // [esp+38h] [ebp+Ch]
    float A; // [esp+3Ch] [ebp+10h]
    float sqrtA; // [esp+40h] [ebp+14h]

    if ( Fs <= 1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 871, 0, "%s", "Fs > 1000") )
    {
        __debugbreak();
    }
    if ( Fs >= 100000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 872, 0, "%s", "Fs < 100000") )
    {
        __debugbreak();
    }
    alpha = (float)(Fhz / Fs) * 2.0;
    if ( alpha < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 876, 0, "%s", "f0 >= 0.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(alpha)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 877, 0, "%s", "!IS_NAN(f0)") )
    {
        __debugbreak();
    }
    if ( q <= 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 878, 0, "%s", "q > 0.0f") )
        __debugbreak();
    if ( q >= 32.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 879, 0, "%s", "q < 32.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(q)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 880, 0, "%s", "!IS_NAN(q)") )
    {
        __debugbreak();
    }
    if ( db >= 50.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 881, 0, "%s", "db < 50.0f") )
    {
        __debugbreak();
    }
    if ( db <= -1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 882, 0, "%s", "db > -1000.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(db)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 883, 0, "%s", "!IS_NAN(db)") )
    {
        __debugbreak();
    }
    alphaa = I_fmax(alpha, 0.000001);
    A = powf(10.0, db * 0.025);
    w0 = alphaa * 3.1415927;
    alphab = sinf(w0) / (q + q);
    w0a = cosf(w0);
    sqrtA = sqrtf(A);
    v5 = (float)(sqrtA * alphab) * 2.0;
    v6 = (float)(A - 1.0) * w0a;
    b[0] = (float)((float)((float)(A + 1.0) + v5) + v6) * A;
    b[1] = (float)((float)((float)((float)(A + 1.0) * w0a) + (float)(A - 1.0)) * A) * -2.0;
    v7 = (float)(A + 1.0) - v6;
    b[2] = (float)((float)((float)(A + 1.0) + v6) - v5) * A;
    a[0] = v7 + v5;
    a[1] = (float)((float)(A - 1.0) - (float)((float)(A + 1.0) * w0a)) * 2.0;
    a[2] = v7 - v5;
    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspBiquadLShelve(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef)
{
    float v5; // xmm2_4
    float v6; // xmm1_4
    float v7; // xmm3_4
    float v8; // xmm0_4
    float a[3]; // [esp+14h] [ebp-18h] BYREF
    float b[3]; // [esp+20h] [ebp-Ch] BYREF
    float sqrtA; // [esp+34h] [ebp+8h]
    float sqrtAa; // [esp+34h] [ebp+8h]
    float sqrtAb; // [esp+34h] [ebp+8h]
    float sqrtAc; // [esp+34h] [ebp+8h]
    float A; // [esp+38h] [ebp+Ch]
    float alpha; // [esp+3Ch] [ebp+10h]
    float cosw0; // [esp+40h] [ebp+14h]

    if ( Fs <= 1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 914, 0, "%s", "Fs > 1000") )
    {
        __debugbreak();
    }
    if ( Fs >= 100000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 915, 0, "%s", "Fs < 100000") )
    {
        __debugbreak();
    }
    sqrtA = (float)(Fhz / Fs) * 2.0;
    if ( sqrtA < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 919, 0, "%s", "f0 >= 0.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(sqrtA)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 920, 0, "%s", "!IS_NAN(f0)") )
    {
        __debugbreak();
    }
    if ( q <= 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 921, 0, "%s", "q > 0.0f") )
        __debugbreak();
    if ( q >= 32.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 922, 0, "%s", "q < 32.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(q)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 923, 0, "%s", "!IS_NAN(q)") )
    {
        __debugbreak();
    }
    if ( db >= 50.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 924, 0, "%s", "db < 50.0f") )
    {
        __debugbreak();
    }
    if ( db <= -1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 925, 0, "%s", "db > -1000.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(db)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 926, 0, "%s", "!IS_NAN(db)") )
    {
        __debugbreak();
    }
    sqrtAa = I_fmax(sqrtA, 0.000001);
    A = powf(10.0, db * 0.025);
    sqrtAb = sqrtAa * 3.1415927;
    alpha = sinf(sqrtAb) / (q + q);
    cosw0 = cosf(sqrtAb);
    sqrtAc = sqrtf(A);
    v5 = (float)(sqrtAc * alpha) * 2.0;
    v6 = (float)(A - 1.0) * cosw0;
    v7 = (float)(A + 1.0) - v6;
    b[0] = (float)(v5 + v7) * A;
    b[2] = (float)(v7 - v5) * A;
    v8 = (float)(A + 1.0) * cosw0;
    b[1] = (float)((float)((float)(A - 1.0) - v8) * A) * 2.0;
    a[0] = (float)(v5 + v6) + (float)(A + 1.0);
    a[1] = (float)(v8 + (float)(A - 1.0)) * -2.0;
    a[2] = (float)(v6 + (float)(A + 1.0)) - v5;
    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspBiquadPeak(float Fs, float db, float Fhz, float q, snd_dsp_biquad_coef *coef)
{
    float a[3]; // [esp+14h] [ebp-18h] BYREF
    float b[3]; // [esp+20h] [ebp-Ch] BYREF
    float f0; // [esp+34h] [ebp+8h]
    float f0a; // [esp+34h] [ebp+8h]
    float w0; // [esp+38h] [ebp+Ch]
    float w0a; // [esp+38h] [ebp+Ch]
    float A; // [esp+3Ch] [ebp+10h]
    float alpha; // [esp+40h] [ebp+14h]

    if ( Fs <= 1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 957, 0, "%s", "Fs > 1000") )
    {
        __debugbreak();
    }
    if ( Fs >= 100000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 958, 0, "%s", "Fs < 100000") )
    {
        __debugbreak();
    }
    f0 = (float)(Fhz / Fs) * 2.0;
    if ( f0 < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 962, 0, "%s", "f0 >= 0.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(f0)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 963, 0, "%s", "!IS_NAN(f0)") )
    {
        __debugbreak();
    }
    if ( q <= 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 964, 0, "%s", "q > 0.0f") )
        __debugbreak();
    if ( q >= 32.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 965, 0, "%s", "q < 32.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(q)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 966, 0, "%s", "!IS_NAN(q)") )
    {
        __debugbreak();
    }
    if ( db >= 50.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 967, 0, "%s", "db < 50.0f") )
    {
        __debugbreak();
    }
    if ( db <= -1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 968, 0, "%s", "db > -1000.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(db)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 969, 0, "%s", "!IS_NAN(db)") )
    {
        __debugbreak();
    }
    f0a = I_fmax(f0, 0.000001);
    A = powf(10.0, db * 0.025);
    w0 = f0a * 3.1415927;
    alpha = sinf(w0) / (q + q);
    w0a = cosf(w0);
    b[0] = (float)(alpha * A) + 1.0;
    b[1] = w0a * -2.0;
    b[2] = 1.0 - (float)(alpha * A);
    a[0] = (float)(alpha / A) + 1.0;
    a[1] = w0a * -2.0;
    a[2] = 1.0 - (float)(alpha / A);
    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspBiquadBpf(float Fs, float Fhz, float q, snd_dsp_biquad_coef *coef)
{
    float a[3]; // [esp+14h] [ebp-18h] BYREF
    float b[3]; // [esp+20h] [ebp-Ch] BYREF
    float f0; // [esp+34h] [ebp+8h]
    float f0a; // [esp+34h] [ebp+8h]
    float alpha; // [esp+3Ch] [ebp+10h]

    if ( Fs <= 1000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1000, 0, "%s", "Fs > 1000") )
    {
        __debugbreak();
    }
    if ( Fs >= 100000.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1001, 0, "%s", "Fs < 100000") )
    {
        __debugbreak();
    }
    f0 = (float)(Fhz / Fs) * 2.0;
    if ( f0 < 0.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1005, 0, "%s", "f0 >= 0.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(f0)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1006, 0, "%s", "!IS_NAN(f0)") )
    {
        __debugbreak();
    }
    if ( q <= 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1007, 0, "%s", "q > 0.0f") )
        __debugbreak();
    if ( q > 32.0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1008, 0, "%s", "q <= 32.0f") )
    {
        __debugbreak();
    }
    if ( IS_NAN(q)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1009, 0, "%s", "!IS_NAN(q)") )
    {
        __debugbreak();
    }
    f0a = I_fmax(f0, 0.000001) * 3.1415927;
    alpha = sinf(f0a) / (q + q);
    b[0] = alpha;
    b[2] = -alpha;
    b[1] = 0.0;
    a[0] = alpha + 1.0;
    a[1] = cosf(f0a) * -2.0;
    a[2] = 1.0 - alpha;
    SND_DspBiquadNormalize(a, b, coef);
}

void __cdecl SND_DspPolyDistortion(unsigned int count, float amount, float *frame, float *temp)
{
    float min; // [esp+4h] [ebp-10h]
    float mina; // [esp+4h] [ebp-10h]
    float amounta; // [esp+20h] [ebp+Ch]
    float amountb; // [esp+20h] [ebp+Ch]

    amounta = I_fclamp(amount, 0.0, 1.0);
    SND_DspMul(count, frame, frame, temp);
    amountb = amounta * 1.5707964;
    min = cosf(amountb);
    SND_DspScale(count, min, frame);
    mina = sinf(amountb);
    SND_DspScale(count, mina, temp);
    SND_DspSum(count, temp, frame);
}

void __cdecl SND_DspClip(unsigned int count, float *c, float minimum, float maximum)
{
    float *v4; // esi
    unsigned int v5; // edi
    float *v6; // esi
    unsigned int v7; // edi

    if ( count )
    {
        v4 = c + 2;
        v5 = ((count - 1) >> 2) + 1;
        do
        {
            *(v4 - 2) = I_fmin(maximum, *(v4 - 2));
            *(v4 - 1) = I_fmin(maximum, *(v4 - 1));
            *v4 = I_fmin(maximum, *v4);
            v4[1] = I_fmin(maximum, v4[1]);
            v4 += 4;
            --v5;
        }
        while ( v5 );
        v6 = c + 2;
        v7 = ((count - 1) >> 2) + 1;
        do
        {
            *(v6 - 2) = I_fmax(minimum, *(v6 - 2));
            *(v6 - 1) = I_fmax(minimum, *(v6 - 1));
            *v6 = I_fmax(minimum, *v6);
            v6[1] = I_fmax(minimum, v6[1]);
            v6 += 4;
            --v7;
        }
        while ( v7 );
    }
}

void __cdecl SND_DspSquelch(
                const snd_dsp_squelch_param *param,
                snd_dsp_squelch_state *state,
                float frameRate,
                unsigned int frameCount,
                float *input)
{
    unsigned int v6; // ebx
    float *v7; // esi
    float v8; // xmm0_4
    float v9; // xmm1_4
    float dt; // [esp+4h] [ebp-1Ch]
    float ITG; // [esp+18h] [ebp-8h]
    float TH; // [esp+28h] [ebp+8h]
    float statea; // [esp+2Ch] [ebp+Ch]
    float TG; // [esp+30h] [ebp+10h]

    TG = SND_DspDecayConstant(param->tg, 1.0 / frameRate, 0.0056234132);
    v6 = 0;
    for ( TH = param->th; v6 < frameCount; *v7 = v9 )
    {
        v7 = &input[v6];
        dt = fabs(*v7);
        ++v6;
        statea = 1.0 - I_fmax(dt, TH);
        ITG = 1.0 - TG;
        v8 = (float)((float)((float)((float)((float)((float)((float)((float)(statea * statea) * statea) * statea) * statea)
                                                                             * statea)
                                                             * (float)(1.0 / TH))
                                             + 1.0)
                             * TG)
             + (float)(state->g * ITG);
        v9 = *v7 * v8;
        state->g = v8;
    }

    iassert(!IS_NAN(state->g));
}

double __cdecl SND_DspDecayConstant(float time, float dt, float targetAmplitude)
{
    iassert(time >= 0);
    iassert(!IS_NAN(time));
    iassert(dt >= 0);
    iassert(!IS_NAN(dt));

    if ( dt < time )
        return -(logf(targetAmplitude) * (dt / time));
    else
        return 0.0;
}

//double __cdecl logf(float _X)
//{
//    return (float)log(_X);
//}

void __cdecl SND_DspFutzMono(
                const snd_dsp_futz_param *param,
                snd_dsp_futz_state *state,
                float rate,
                unsigned int count,
                float *input,
                float *tempa,
                float *tempb)
{
    snd_dsp_biquad_coef ls; // [esp+20h] [ebp-28h] BYREF
    snd_dsp_biquad_coef bpf; // [esp+34h] [ebp-14h] BYREF

    if ( param->blend >= 0.0000152879 )
    {
        if ( param->blend < 0.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                        1168,
                        0,
                        "%s",
                        "param->blend >= 0.0f") )
        {
            __debugbreak();
        }
        if ( param->blend > 1.0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                        1169,
                        0,
                        "%s",
                        "param->blend <= 1.0f") )
        {
            __debugbreak();
        }
        SND_DspScale(count, param->pregain, input, tempa);
        SND_DspPolyDistortion(count, param->distortion, tempa, tempb);
        //SND_DspClip(count, tempa, COERCE_FLOAT(LODWORD(param->preclip) ^ _mask__NegFloat_), param->preclip);
        SND_DspClip(count, tempa, -param->preclip, param->preclip);
        SND_DspBiquadBpf(rate, param->bpfF, param->bpfQ, &bpf);
        SND_DspBiquadInPlace(&bpf, &state->bpf, count, tempa);
        SND_DspBiquadLShelve(rate, param->lsG, param->lsF, param->lsQ, &ls);
        SND_DspBiquadInPlace(&ls, &state->ls, count, tempa);
        SND_DspSquelch(&param->squelch, &state->squelch, rate, count, tempa);
        SND_DspScale(count, param->postgain * param->blend, tempa);
        SND_DspScale(count, 1.0 - param->blend, input);
        SND_DspSum(count, tempa, input);
        //SND_DspClip(count, input, COERCE_FLOAT(LODWORD(param->postclip) ^ _mask__NegFloat_), param->postclip);
        SND_DspClip(count, input, -param->postclip, param->postclip);
    }
}

void __cdecl SND_DspInterleave(unsigned int channel_count, unsigned int frame_count, float *in, float *out)
{
    // aislopped for correctness
    for (unsigned int ch = 0; ch < channel_count; ch++)
    {
        float *src = in + ch * frame_count;
        float *dst = out + ch;

        for (unsigned int i = 0; i < frame_count; i++)
        {
            dst[i * channel_count] = src[i];
        }
    }
}

void __cdecl SND_DspUninterleave(unsigned int channel_count, unsigned int frame_count, float *in, float *out)
{
    // aislopped for correctness
    for (unsigned int ch = 0; ch < channel_count; ch++)
    {
        float *dst = out + ch * frame_count;
        float *src = in + ch;

        for (unsigned int i = 0; i < frame_count; i++)
        {
            dst[i] = src[i * channel_count];
        }
    }
}

double __cdecl SND_DspLog(float m)
{
    double v1; // st7

    v1 = (m - 1.0) / (m + 1.0);
    return v1 + v1 * v1 * v1 * 0.33333334;
}

double __cdecl SND_DspExp(float x)
{
    return (float)((float)((float)(x * x) * x) * x) * 0.041666668
             + (float)((float)((float)((float)(x * x) * x) * 0.16666667)
                             + (float)((float)((float)(x * x) * 0.5) + (float)(x + 1.0)))
             + (float)((float)((float)(x * x) * x) * x) * x * 0.0083333338;
}

void __cdecl SND_DspDynamoDenormal(snd_dsp_dynamo_state *state)
{
    if ( state->yfilter < 0.0000152879 )
        state->yfilter = 0.0;
    if ( state->gfilter < 0.0000152879 )
        state->gfilter = 0.0;
    if ( state->g < 0.0000152879 )
        state->g = 0.0;
}

void __cdecl SND_DspDynamo(
                unsigned int frameCount,
                float sampleRate,
                float TH,
                float R,
                float TA,
                float TR,
                snd_dsp_dynamo_state *state,
                float *input,
                float *gain)
{
    unsigned int *p_offset; // ebx
    unsigned int v11; // edx
    float *v12; // ebx
    float g; // xmm0_4
    float gfilter; // xmm3_4
    float v15; // xmm2_4
    float *v16; // ebx
    double v17; // st7
    float v18; // xmm0_4
    float dt_4; // [esp+14h] [ebp-2Ch]
    float dt_4a; // [esp+14h] [ebp-2Ch]
    float dt_4b; // [esp+14h] [ebp-2Ch]
    float *end; // [esp+24h] [ebp-1Ch]
    float v23; // [esp+30h] [ebp-10h]
    float y; // [esp+34h] [ebp-Ch]
    float *output; // [esp+38h] [ebp-8h]
    unsigned int T; // [esp+3Ch] [ebp-4h]
    float iR; // [esp+48h] [ebp+8h]
    float SA; // [esp+4Ch] [ebp+Ch]
    float SR; // [esp+54h] [ebp+14h]
    float *lookahead; // [esp+60h] [ebp+20h]
    float *lookaheada; // [esp+60h] [ebp+20h]
    float Ma; // [esp+64h] [ebp+24h]
    float M; // [esp+64h] [ebp+24h]

    if ( 0x3C0 % frameCount
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    1404,
                    0,
                    "%s",
                    "SND_DSP_DYNAMO_LOOKAHEAD_FRAMES%frameCount == 0") )
    {
        __debugbreak();
    }
    T = frameCount;
    if ( ((4 * (_BYTE)frameCount) & 0x7F) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    1405,
                    0,
                    "%s",
                    "(frameCount*sizeof(float))%128 == 0") )
    {
        __debugbreak();
    }
    p_offset = &state->offset;
    output = input;
    lookahead = &state->lookahead[state->offset];
    if ( lookahead >= (float *)p_offset
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    1410,
                    0,
                    "%s",
                    "lookahead < state->lookahead+SND_DSP_DYNAMO_LOOKAHEAD_FRAMES") )
    {
        __debugbreak();
    }
    if ( lookahead < (float *)state
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    1411,
                    0,
                    "%s",
                    "lookahead >= state->lookahead") )
    {
        __debugbreak();
    }
    memcpy((unsigned __int8 *)lookahead, (unsigned __int8 *)input, T * 4);
    v11 = (frameCount + *p_offset) % 0x3C0;
    *p_offset = v11;
    v12 = &state->lookahead[v11];
    if ( TH == 0.0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1417, 0, "%s", "TH != 0") )
        __debugbreak();
    Ma = 1.0 / sampleRate;
    iR = 1.0 / R;
    SR = SND_DspDecayConstant(TR, Ma, 0.0056234132);
    SA = SND_DspDecayConstant(TA, Ma, 0.0056234132);
    dt_4 = SND_DspLog(TH) * (iR - 1.0);
    M = SND_DspExp(dt_4);
    if ( IS_NAN(iR)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1427, 0, "%s", "!IS_NAN(iR)") )
    {
        __debugbreak();
    }
    if ( IS_NAN(M)
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp", 1428, 0, "%s", "!IS_NAN(M)") )
    {
        __debugbreak();
    }
    if ( (SA < 0.0 || SA > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    1429,
                    0,
                    "SA not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    SA,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    if ( (SR < 0.0 || SR > 1.0)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\sound\\snd_dsp.cpp",
                    1430,
                    0,
                    "SR not in [0.0f, 1.0f]\n\t%g not in [%g, %g]",
                    SR,
                    0.0,
                    1.0) )
    {
        __debugbreak();
    }
    *gain = M;
    for ( end = &v12[T]; v12 != end; output += 2 )
    {
        y = fabs(*lookahead);
        lookaheada = lookahead + 1;
        v23 = (float)(state->yfilter * 0.99000001) + (float)(y * 0.0099999905);
        state->yfilter = v23;
        if ( v23 <= TH )
        {
            state->g = M;
        }
        else
        {
            dt_4a = v23 * (float)(1.0 / TH);
            dt_4b = SND_DspLog(dt_4a) * iR;
            state->g = SND_DspExp(dt_4b) / state->yfilter * M * TH;
        }
        *output = state->gfilter * *v12;
        *gain = I_fmin(*gain, state->gfilter);
        g = state->g;
        gfilter = state->gfilter;
        if ( gfilter <= g )
            v15 = SR;
        else
            v15 = SA;
        state->gfilter = (float)(g * v15) + (float)((float)(1.0 - v15) * gfilter);
        v16 = v12 + 1;
        v17 = fabs(*lookaheada);
        v18 = state->gfilter;
        lookahead = lookaheada + 1;
        state->yfilter = state->yfilter * 0.99000001 + v17 * 0.0099999905;
        output[1] = v18 * *v16;
        v12 = v16 + 1;
        state->gfilter = (float)((float)(1.0 - v15) * state->gfilter) + (float)(state->g * v15);
    }
    SND_DspDynamoDenormal(state);
}

void __cdecl SND_DspFxSourceMono(
                const snd_dsp_futz_param *params,
                snd_dsp_source_state *state,
                unsigned int frameCount,
                float *frames,
                float *tempa,
                float *tempb)
{
    snd_dsp_source_state *v6; // ebx
    const snd_dsp_source_params *v7; // esi
    float blend; // xmm0_4
    float b0; // [esp+20h] [ebp-4h] BYREF

    v6 = state;
    v7 = (const snd_dsp_source_params *)params;
    if ( params->bpfF < -0.0000152879 )
    {
        PROF_SCOPED("lpf");
        SND_OcclusionLpfCoef(v7->lpfAttenuation, v7->lpfRatio, v7->frameRate, &b0, (float *)&params);
        SND_DspOnePoleFilterMono(frameCount, frames, b0, *(float *)&params, &v6->lpfy);
    }
    blend = v7->futz.blend;
    params = &v7->futz;
    if ( blend > 0.0000152879 )
    {
        PROF_SCOPED("futz");
        SND_DspFutzMono(params, &v6->futz, v7->frameRate, frameCount, frames, tempa, tempb);
    }
}

void __cdecl SND_DspBiquadDenormal(snd_dsp_biquad_state *state)
{
    int i; // ebx
    float *v2; // esi
    float f; // [esp+0h] [ebp-18h]
    float fa; // [esp+0h] [ebp-18h]
    float valGE; // [esp+4h] [ebp-14h]
    float valGEa; // [esp+4h] [ebp-14h]

    for ( i = 0; i < 3; ++i )
    {
        v2 = &state->y[i];
        valGE = *v2;
        f = fabs(valGE) - 5.9604643e-12;
        *v2 = I_fsel(f, valGE, 0.0);
        v2 += 4;
        valGEa = *v2;
        fa = fabs(valGEa) - 5.9604643e-12;
        *v2 = I_fsel(fa, valGEa, 0.0);
    }
}

double __cdecl I_fsel(float comparand, float valGE, float valLT)
{
    if ( comparand < 0.0 )
        return valLT;
    return valGE;
}

void __cdecl SND_DspBiquadNanCheck(snd_dsp_biquad_state *state)
{
    for ( int i = 0; i < 3; ++i )
    {
        iassert(!IS_NAN(state->y[i]));
        iassert(!IS_NAN(state->x[i]));
    }
}

void __cdecl SND_DspFxMasterNoVoiceSingleChannel(
                unsigned int frameCount,
                float frameRate,
                float *frames,
                snd_dsp_master_no_voice_params *params,
                snd_dsp_master_no_voice_state *state,
                snd_dsp_meters *meters)
{
    PROF_SCOPED("comp");

    if ( params->compE > 0.0000152879 )
    {
        if ( fabs(params->compPG - 1.0) > 0.0000152879 )
            SND_DspScale(frameCount, params->compPG, frames);
        SND_DspDynamo(
            frameCount,
            frameRate,
            params->compT,
            params->compR,
            params->compTA,
            params->compTR,
            &state->comp,
            frames,
            meters != 0 ? &meters->dyn1Gain : 0);
        if ( fabs(params->compMG - 1.0) > 0.0000152879 )
            SND_DspScale(frameCount, params->compMG, frames);
    }
}

void __cdecl SND_DspFxMasterSingleChannel(
                unsigned int frameCount,
                float frameRate,
                float *frames,
                snd_dsp_master_params *params,
                snd_dsp_master_state *state,
                snd_dsp_meters *meters)
{
    snd_dsp_biquad_coef hi; // [esp+2Ch] [ebp-14h] BYREF

    iassert(params);
    iassert(state);
    iassert(meters);

    SND_DspScaleCache(frameCount, params->eqG, frames);
    
    {
        PROF_SCOPED("eq");

        if (params->lowE <= 0.0000152879)
        {
            memset(state, 0, 0x20u);
        }
        else
        {
            SND_DspBiquadLShelve(frameRate, params->lowG, params->lowF, params->lowQ, &hi);
            SND_DspBiquadInPlace(&hi, &state->low, frameCount, frames);
            SND_DspBiquadDenormal(&state->low);
            SND_DspBiquadNanCheck(&state->low);
        }

        if (params->peak1E <= 0.0000152879)
        {
            memset(&state->peak1, 0, sizeof(state->peak1));
        }
        else
        {
            SND_DspBiquadPeak(frameRate, params->peak1G, params->peak1F, params->peak1Q, &hi);
            SND_DspBiquadInPlace(&hi, &state->peak1, frameCount, frames);
            SND_DspBiquadDenormal(&state->peak1);
            SND_DspBiquadNanCheck(&state->peak1);
        }
        if (params->peak2E <= 0.0000152879)
        {
            memset(&state->peak2, 0, sizeof(state->peak2));
        }
        else
        {
            SND_DspBiquadPeak(frameRate, params->peak2G, params->peak2F, params->peak2Q, &hi);
            SND_DspBiquadInPlace(&hi, &state->peak2, frameCount, frames);
            SND_DspBiquadDenormal(&state->peak2);
            SND_DspBiquadNanCheck(&state->peak2);
        }
        if (params->hiE <= 0.0000152879)
        {
            memset(&state->hi, 0, sizeof(state->hi));
        }
        else
        {
            SND_DspBiquadHShelve(frameRate, params->hiG, params->hiF, params->hiQ, &hi);
            SND_DspBiquadInPlace(&hi, &state->hi, frameCount, frames);
            SND_DspBiquadDenormal(&state->hi);
            SND_DspBiquadNanCheck(&state->hi);
        }
    }
    
    {
        PROF_SCOPED("limit");
        if (params->limitE > 0.0000152879)
        {
            if (fabs(params->limitPG - 1.0) > 0.0000152879)
                SND_DspScale(frameCount, params->limitPG, frames);
            SND_DspDynamo(
                frameCount,
                frameRate,
                params->limitT,
                params->limitR,
                params->limitTA,
                params->limitTR,
                &state->limit,
                frames,
                meters != 0 ? &meters->dyn2Gain : 0);
            if (fabs(params->limitMG - 1.0) > 0.0000152879)
                SND_DspScale(frameCount, params->limitMG, frames);
        }
    }
}

