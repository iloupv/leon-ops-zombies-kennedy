#include "r_water_load_obj.h"
#include "r_material.h"
#include <qcommon/common.h>
#include "r_image_load_obj.h"

int sceneWaterMapSetupsCount;
water_t sceneWaterMapSetups[16];

water_t *__cdecl R_LoadWaterSetup(const water_t *water)
{
    int waterMapSetupIndex; // [esp+0h] [ebp-4h]

    if ( (water->N & (water->N - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    136,
                    0,
                    "%s",
                    "IsPowerOf2( water->N )") )
    {
        __debugbreak();
    }
    if ( (water->M & (water->M - 1)) != 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    137,
                    0,
                    "%s",
                    "IsPowerOf2( water->M )") )
    {
        __debugbreak();
    }
    if ( (water->M < 4 || water->M > 64)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    138,
                    0,
                    "water->M not in [MIN_WATER_SIZE, MAX_WATER_SIZE]\n\t%i not in [%i, %i]",
                    water->M,
                    4,
                    64) )
    {
        __debugbreak();
    }
    if ( (water->N < 4 || water->N > 64)
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    139,
                    0,
                    "water->N not in [MIN_WATER_SIZE, MAX_WATER_SIZE]\n\t%i not in [%i, %i]",
                    water->N,
                    4,
                    64) )
    {
        __debugbreak();
    }
    if ( water->Lx <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    140,
                    0,
                    "%s",
                    "water->Lx > 0") )
    {
        __debugbreak();
    }
    if ( water->Lz <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    141,
                    0,
                    "%s",
                    "water->Lz > 0") )
    {
        __debugbreak();
    }
    if ( water->gravity <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    142,
                    0,
                    "%s",
                    "water->gravity > 0") )
    {
        __debugbreak();
    }
    if ( water->windvel <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    143,
                    0,
                    "%s",
                    "water->windvel > 0") )
    {
        __debugbreak();
    }
    if ( water->winddir[0] == 0.0
        && water->winddir[1] == 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    144,
                    0,
                    "%s",
                    "water->winddir[0] || water->winddir[1]") )
    {
        __debugbreak();
    }
    if ( water->amplitude <= 0.0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    145,
                    0,
                    "%s",
                    "water->amplitude > 0") )
    {
        __debugbreak();
    }
    for ( waterMapSetupIndex = 0; waterMapSetupIndex < sceneWaterMapSetupsCount; ++waterMapSetupIndex )
    {
        if ( R_WatersEquivalent(&sceneWaterMapSetups[waterMapSetupIndex], water) )
            return &sceneWaterMapSetups[waterMapSetupIndex];
    }
    if ( waterMapSetupIndex == 16 )
    {
        Com_PrintError(8, "ERROR: map uses more than %i waterMap textures\n", 16);
        return 0;
    }
    else
    {
        R_CreateWaterSetup(water, waterMapSetupIndex, &sceneWaterMapSetups[waterMapSetupIndex]);
        ++sceneWaterMapSetupsCount;
        return &sceneWaterMapSetups[waterMapSetupIndex];
    }
}

bool __cdecl R_WatersEquivalent(const water_t *w0, const water_t *w1)
{
    float v3; // xmm0_4

    if ( !w0 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp", 16, 0, "%s", "w0") )
        __debugbreak();
    if ( !w1 && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp", 17, 0, "%s", "w1") )
        __debugbreak();
    if ( w0->M != w1->M || w0->N != w1->N )
        return 0;
    if ( w0->Lx != w1->Lx || w0->Lz != w1->Lz )
        return 0;
    if ( fabs(w0->amplitude - w1->amplitude) > 0.001 )
        return 0;
    if ( fabs(w0->gravity - w1->gravity) > 0.1 )
        return 0;
    if ( fabs(w0->windvel - w1->windvel) > 0.1 )
        return 0;
    v3 = sqrtf(
                 (float)((float)(w0->winddir[0] * w0->winddir[0]) + (float)(w0->winddir[1] * w0->winddir[1]))
             * (float)((float)(w1->winddir[0] * w1->winddir[0]) + (float)(w1->winddir[1] * w1->winddir[1])))
         + 1.0e-10;
    //return COERCE_FLOAT(COERCE_UNSIGNED_INT((float)((float)((float)(w0->winddir[0] * w1->winddir[0]) + (float)(w0->winddir[1] * w1->winddir[1])) / v3) - 1.0) & _mask__AbsFloat_) <= 0.001;
    return ((fabs((float)((float)((float)(w0->winddir[0] * w1->winddir[0]) + (float)(w0->winddir[1] * w1->winddir[1])) / v3) - 1.0)) <= 0.001);
}

void __cdecl R_CreateWaterSetup(const water_t *source, int waterMapSetupIndex, water_t *destination)
{
    char *v3; // eax
    unsigned __int16 M; // [esp-8h] [ebp-18h]
    unsigned __int16 N; // [esp-4h] [ebp-14h]
    GfxImage *image; // [esp+8h] [ebp-8h]
    int elementCount; // [esp+Ch] [ebp-4h]

    if ( !source
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp", 109, 0, "%s", "source") )
    {
        __debugbreak();
    }
    if ( !destination
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                    110,
                    0,
                    "%s",
                    "destination") )
    {
        __debugbreak();
    }
    elementCount = source->N * source->M;
    memcpy(destination, source, sizeof(water_t));
    destination->H0 = (complex_s *)Material_Alloc(8 * elementCount);
    destination->wTerm = (float *)Material_Alloc(4 * elementCount);
    R_PickWaterFrequencies(destination);
    N = source->N;
    M = source->M;
    v3 = va("watersetup%i", waterMapSetupIndex);
    image = R_CreateWaterMap(v3, M, N);
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp", 128, 1, "%s", "image") )
    {
        __debugbreak();
    }
    destination->image = image;
}

//void __cdecl R_PickWaterFrequencies(water_t *water)
//{
//    long double v1; // [esp+0h] [ebp-50h]
//    int m; // [esp+10h] [ebp-40h]
//    float m_scale; // [esp+14h] [ebp-3Ch]
//    float kz; // [esp+1Ch] [ebp-34h]
//    float windfactor; // [esp+20h] [ebp-30h]
//    float L_sqrd; // [esp+24h] [ebp-2Ch]
//    float windvelsqrd; // [esp+2Ch] [ebp-24h]
//    float n_scale; // [esp+30h] [ebp-20h]
//    int n; // [esp+34h] [ebp-1Ch]
//    float scale; // [esp+38h] [ebp-18h]
//    int i; // [esp+3Ch] [ebp-14h]
//    float kx; // [esp+40h] [ebp-10h]
//    complex_s E; // [esp+44h] [ebp-Ch] BYREF
//    float w_sqrd; // [esp+4Ch] [ebp-4h]
//
//    if ( !water
//        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp", 56, 0, "%s", "water") )
//    {
//        __debugbreak();
//    }
//    windvelsqrd = water->windvel * water->windvel;
//    L_sqrd = (float)(windvelsqrd * windvelsqrd) / water->gravity;
//    n_scale = 6.2831855 / (float)((float)water->N * water->Lx);
//    m_scale = 6.2831855 / (float)((float)water->M * water->Lz);
//    i = 0;
//    for ( n = -water->N / 2; n < water->N / 2; ++n )
//    {
//        kx = (float)n * n_scale;
//        for ( m = -water->M / 2; m < water->M / 2; ++m )
//        {
//            kz = (float)m * m_scale;
//            GaussianRandom(&E.real, &E.imag);
//            w_sqrd = water->gravity * sqrtf((float)(kx * kx) + (float)(kz * kz));
//            windfactor = (float)(water->winddir[0] * kx) + (float)(water->winddir[1] * kz);
//            if ( windfactor > 0.0 )
//            {
//                __libm_sse2_exp(v1);
//                HIDWORD(v1) = sqrtf(
//                                                (float)((float)((float)(water->amplitude
//                                                                                            * (float)(-1.0
//                                                                                                            / (float)((float)((float)(kx * kx) + (float)(kz * kz)) * L_sqrd)))
//                                                                            / (float)((float)((float)((float)(kx * kx) + (float)(kz * kz))
//                                                                                                            * (float)((float)(kx * kx) + (float)(kz * kz)))
//                                                                                            * (float)((float)(kx * kx) + (float)(kz * kz))))
//                                                            * (float)(windfactor * windfactor))
//                                            * 0.5);
//                scale = *((float *)&v1 + 1) * water->amplitude;
//                water->H0[i].real = E.real * scale;
//                water->H0[i].imag = E.imag * scale;
//                LODWORD(v1) = sqrtf(w_sqrd);
//                water->wTerm[i] = *(float *)&v1;
//            }
//            else
//            {
//                water->H0[i].real = 0.0f;
//                water->H0[i].imag = 0.0f;
//                water->wTerm[i] = 0.0f;
//            }
//            ++i;
//        }
//    }
//}

// aislop (exp)
void R_PickWaterFrequencies(water_t *water)
{
    if (!water)
    {
        if (!Assert_MyHandler(
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
            56, 0, "%s", "water"))
        {
            __debugbreak();
        }
        return;
    }

    const float windVelSq = water->windvel * water->windvel;
    const float L_sqrd = (windVelSq * windVelSq) / water->gravity;

    const float n_scale = 6.2831855f / (water->N * water->Lx);
    const float m_scale = 6.2831855f / (water->M * water->Lz);

    int i = 0;

    for (int n = -water->N / 2; n < water->N / 2; ++n)
    {
        const float kx = n * n_scale;

        for (int m = -water->M / 2; m < water->M / 2; ++m)
        {
            const float kz = m * m_scale;

            complex_s E;
            GaussianRandom(&E.real, &E.imag);

            const float k2 = kx * kx + kz * kz;

            if (k2 > 0.0f)
            {
                const float k = sqrtf(k2);

                const float windFactor =
                    water->winddir[0] * kx +
                    water->winddir[1] * kz;

                if (windFactor > 0.0f)
                {
                    /* Phillips spectrum */
                    const float expTerm =
                        expf(-1.0f / (k2 * L_sqrd));

                    const float spectrum =
                        water->amplitude *
                        expTerm *
                        (windFactor * windFactor) /
                        (k2 * k2 * k2);

                    const float scale = sqrtf(spectrum * 0.5f);

                    water->H0[i].real = E.real * scale;
                    water->H0[i].imag = E.imag * scale;
                    water->wTerm[i] = sqrtf(water->gravity * k);
                }
                else
                {
                    water->H0[i].real = 0.0f;
                    water->H0[i].imag = 0.0f;
                    water->wTerm[i] = 0.0f;
                }
            }
            else
            {
                water->H0[i].real = 0.0f;
                water->H0[i].imag = 0.0f;
                water->wTerm[i] = 0.0f;
            }

            ++i;
        }
    }
}


void __cdecl R_InitLoadWater()
{
    if ( sceneWaterMapSetupsCount )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_load_obj.cpp",
                        168,
                        0,
                        "%s",
                        "sceneWaterMapSetupsCount == 0") )
            __debugbreak();
    }
}

void __cdecl R_ShutdownLoadWater()
{
    sceneWaterMapSetupsCount = 0;
}

