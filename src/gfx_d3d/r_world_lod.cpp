#include "r_world_lod.h"
#include <universal/q_shared.h>
#include "r_init.h"
#include <universal/com_memory.h>
#include "r_dvars.h"

unsigned __int8 *s_lodVals;
LodChainState *s_lodState[1];

void __thiscall LodChainState::Init(const GfxWorldLodChain *lodChain, int localClientNum)
{
    int i; // [esp+10h] [ebp-4h]

    this->m_lodChain = lodChain;
    this->m_fade = 0.0f;
    this->m_curLevel = 1;
    this->m_lastLevel = 1;
    for ( i = 1; i < this->m_lodChain->lodInfoCount; ++i )
        SetFadeForSortedSurfaces(0.0, i + this->m_lodChain->firstLodInfo, localClientNum);
}

void __thiscall LodChainState::UpdateLevel(int newLevel, int localClientNum)
{
    __int64 v3; // rax
    float val; // [esp+Ch] [ebp-10h]
    unsigned int i; // [esp+18h] [ebp-4h]

    if ( newLevel != this->m_curLevel )
    {
        if ( this->m_lastLevel != this->m_curLevel )
            LodChainState::UpdateFade(1000.0, localClientNum);
        this->m_lastLevel = this->m_curLevel;
        this->m_curLevel = newLevel;
        v3 = this->m_curLevel - this->m_lastLevel;
        if ( (int)((HIDWORD(v3) ^ v3) - HIDWORD(v3)) <= 1 )
        {
            if ( this->m_curLevel <= (int)this->m_lastLevel )
            {
                if ( this->m_lastLevel == (unsigned __int8)(this->m_lodChain->lodInfoCount + 1) )
                {
                    this->m_fade = 0.0f;
                }
                else
                {
                    this->m_fade = 1.0f;
                    if ( this->m_curLevel )
                        SetFadeForSortedSurfaces(1.0, this->m_lodChain->firstLodInfo + this->m_curLevel - 1, localClientNum);
                }
            }
            else if ( this->m_curLevel == (unsigned __int8)(this->m_lodChain->lodInfoCount + 1) )
            {
                this->m_fade = 1.0f;
            }
            else
            {
                this->m_fade = 0.0f;
            }
        }
        else
        {
            for ( i = 0; i < this->m_lodChain->lodInfoCount; ++i )
            {
                if ( i == this->m_curLevel )
                    val = 1.0f;
                else
                    val = 0.0f;
                if ( i )
                    SetFadeForSortedSurfaces(val, this->m_lodChain->firstLodInfo + i - 1, localClientNum);
            }
            this->m_lastLevel = this->m_curLevel;
        }
    }
}

void __thiscall LodChainState::UpdateFade(float dt, int localClientNum)
{
    int m_lastLevel; // [esp+10h] [ebp-10h]
    int savedLastLevel; // [esp+14h] [ebp-Ch]
    unsigned int firstLodInfo; // [esp+18h] [ebp-8h]
    unsigned int lodInfoIndex; // [esp+1Ch] [ebp-4h]

    if ( this->m_curLevel != this->m_lastLevel )
    {
        if ( this->m_lastLevel && this->m_lastLevel != this->m_lodChain->lodInfoCount + 1 )
        {
            if ( this->m_curLevel && this->m_curLevel != this->m_lodChain->lodInfoCount + 1 )
            {
                if ( this->m_curLevel <= (int)this->m_lastLevel )
                {
                    m_lastLevel = this->m_lastLevel;
                    this->m_fade = this->m_fade - (float)(dt * 1.0);
                    if ( this->m_fade <= 0.0 )
                    {
                        this->m_fade = 0.0f;
                        this->m_lastLevel = this->m_curLevel;
                    }
                    SetFadeForSortedSurfaces(this->m_fade, this->m_lodChain->firstLodInfo + m_lastLevel - 1, localClientNum);
                }
                else
                {
                    savedLastLevel = this->m_lastLevel;
                    this->m_fade = (float)(dt * 1.0) + this->m_fade;
                    if ( this->m_fade >= 1.0 )
                    {
                        this->m_fade = 1.0f;
                        this->m_lastLevel = this->m_curLevel;
                    }
                    if ( this->m_fade >= 1.0 )
                        SetFadeForSortedSurfaces(0.0, this->m_lodChain->firstLodInfo + savedLastLevel - 1, localClientNum);
                    SetFadeForSortedSurfaces(this->m_fade, this->m_lodChain->firstLodInfo + this->m_curLevel - 1, localClientNum);
                }
            }
            else
            {
                firstLodInfo = this->m_lodChain->firstLodInfo;
                if ( this->m_curLevel )
                    firstLodInfo = firstLodInfo + this->m_lodChain->lodInfoCount - 1;
                this->m_fade = this->m_fade - (float)(dt * 1.0);
                if ( this->m_fade <= 0.0 )
                {
                    this->m_fade = 0.0f;
                    this->m_lastLevel = this->m_curLevel;
                }
                SetFadeForSortedSurfaces(this->m_fade, firstLodInfo, localClientNum);
            }
        }
        else
        {
            lodInfoIndex = this->m_lodChain->firstLodInfo;
            if ( this->m_lastLevel )
                lodInfoIndex = lodInfoIndex + this->m_lodChain->lodInfoCount - 1;
            this->m_fade = (float)(dt * 1.0) + this->m_fade;
            if ( this->m_fade >= 1.0 )
            {
                this->m_fade = 1.0f;
                this->m_lastLevel = this->m_curLevel;
            }
            SetFadeForSortedSurfaces(this->m_fade, lodInfoIndex, localClientNum);
        }
    }
}

void __cdecl SetFadeForSortedSurfaces(float val, int lodInfoIndex, int localClientNum)
{
    unsigned int index; // [esp+0h] [ebp-18h]
    const GfxWorldLodInfo *lodInfo; // [esp+4h] [ebp-14h]
    unsigned int offset; // [esp+Ch] [ebp-Ch]
    unsigned int *surf; // [esp+10h] [ebp-8h]
    unsigned int *last; // [esp+14h] [ebp-4h]

    lodInfo = &rgp.world->worldLodInfos[lodInfoIndex];
    if ( !rgp.world->worldLodSurfaces
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_world_lod.cpp",
                    209,
                    0,
                    "%s",
                    "rgp.world->worldLodSurfaces") )
    {
        __debugbreak();
    }
    surf = &rgp.world->worldLodSurfaces[lodInfo->firstSurf];
    last = &surf[lodInfo->surfCount];
    offset = rgp.world->surfaceCount * localClientNum;
    while ( surf != last )
    {
        index = offset + rgp.world->dpvs.sortedSurfIndex[*surf];
        if ( index >= rgp.world->surfaceCount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_world_lod.cpp",
                        217,
                        0,
                        "index doesn't index rgp.world->surfaceCount * MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                        index,
                        rgp.world->surfaceCount) )
        {
            __debugbreak();
        }
        s_lodVals[index] = (int)(float)(val * 255.0);
        ++surf;
    }
}

void __cdecl R_WorldLod_Init()
{
    unsigned int i; // [esp+0h] [ebp-8h]
    int clientIdx; // [esp+4h] [ebp-4h]

    s_lodVals = Hunk_Alloc(rgp.world->surfaceCount, "R_WorldLod_Init", 23);
    memset(s_lodVals, 0xFFu, rgp.world->surfaceCount);
    for ( clientIdx = 0; clientIdx < 1; ++clientIdx )
    {
        s_lodState[clientIdx] = (LodChainState *)Hunk_Alloc(12 * rgp.world->worldLodChainCount, "R_WorldLod_Init", 23);
        for (i = 0; i < rgp.world->worldLodChainCount; ++i)
        {
            //LodChainState::Init(&s_lodState[clientIdx][i], &rgp.world->worldLodChains[i], clientIdx);
            s_lodState[clientIdx][i].Init(&rgp.world->worldLodChains[i], clientIdx);
        }
    }
}

float s_lastTime = -c_fadeRate * 2.0f;;
void __cdecl R_WorldLod_FrameUpdate(float curTime, float *camPos, int localClientNum)
{
    float v3; // [esp+8h] [ebp-10h]
    float v4; // [esp+Ch] [ebp-Ch]
    unsigned int i; // [esp+10h] [ebp-8h]
    float dt; // [esp+14h] [ebp-4h]
    int savedregs; // [esp+18h] [ebp+0h] BYREF

    dt = curTime - s_lastTime;
    s_lastTime = curTime;
    if ( (float)(dt - 1.0) < 0.0 )
        v4 = dt;
    else
        v4 = 1.0f;
    if ( (float)(0.0 - dt) < 0.0 )
        v3 = v4;
    else
        v3 = 0.0f;
    for ( i = 0; i < rgp.world->worldLodChainCount; ++i )
        UpdateChain(i, camPos, v3, localClientNum);
}

// local variable allocation has failed, the output may be wrong!
void    UpdateChain(int index, const float *inputCamPos, float dt, int localClientNum)
{
    int v5; // [esp+Ch] [ebp-A8h]
    bool v6; // [esp+13h] [ebp-A1h]
    float lodInfos; // [esp+14h] [ebp-A0h]
    int curLevel; // [esp+18h] [ebp-9Ch] BYREF
    float v9; // [esp+1Ch] [ebp-98h] OVERLAPPED
    float camDist; // [esp+20h] [ebp-94h]
    float v11; // [esp+24h] [ebp-90h]
    float v12; // [esp+28h] [ebp-8Ch]
    float v13; // [esp+2Ch] [ebp-88h]
    float v14; // [esp+30h] [ebp-84h]
    float v15; // [esp+34h] [ebp-80h]
    float v16; // [esp+38h] [ebp-7Ch]
    float v17; // [esp+3Ch] [ebp-78h]
    float v18; // [esp+40h] [ebp-74h]
    math::Position3 lodPos; // [esp+44h] [ebp-70h]
    int v20; // [esp+54h] [ebp-60h]
    float v21; // [esp+5Ch] [ebp-58h]
    float v22; // [esp+60h] [ebp-54h]
    float v23; // [esp+64h] [ebp-50h]
    float v24; // [esp+68h] [ebp-4Ch]
    float v25; // [esp+6Ch] [ebp-48h]
    float v26; // [esp+70h] [ebp-44h]
    math::Position3 camPos; // [esp+74h] [ebp-40h]
    int v28; // [esp+84h] [ebp-30h]
    float v29; // [esp+94h] [ebp-20h]
    float v30; // [esp+98h] [ebp-1Ch]
    float v31; // [esp+9Ch] [ebp-18h]
    GfxWorldLodChain *v32; // [esp+A0h] [ebp-14h]
    LodChainState *v33; // [esp+A4h] [ebp-10h]
    //int v34; // [esp+A8h] [ebp-Ch]
    //const GfxWorldLodChain *lodChain; // [esp+ACh] [ebp-8h]
    //const GfxWorldLodChain *retaddr; // [esp+B4h] [ebp+0h]

    //v34 = a1;
    //lodChain = retaddr;
    v33 = &s_lodState[localClientNum][index];
    v32 = &rgp.world->worldLodChains[index];
    v31 = *inputCamPos;
    v30 = inputCamPos[1];
    v29 = inputCamPos[2];
    camPos.v.y = v31;
    camPos.v.z = v30;
    camPos.v.w = v29;
    v28 = 0;
    v24 = v31;
    v25 = v30;
    v26 = v29;
    camPos.v.x = 0.0f;
    v23 = v32->origin[0];
    v22 = v32->origin[1];
    v21 = v32->origin[2];
    lodPos.v.y = v23;
    lodPos.v.z = v22;
    lodPos.v.w = v21;
    v20 = 0;
    v16 = v23;
    v17 = v22;
    v18 = v21;
    lodPos.v.x = 0.0f;
    v12 = v31 - v23;
    v13 = v30 - v22;
    v14 = v29 - v21;
    v15 = 0.0 - 0.0;
    *(float *)&curLevel = v31 - v23;
    v9 = v30 - v22;
    camDist = v29 - v21;
    v11 = 0.0 - 0.0;
    lodInfos = math::Abs((const math::Dir3 *)&curLevel);
    v6 = !r_reflectionProbeGenerate->current.enabled && r_worldLod->current.enabled;
    v5 = 0;
    if ( v6 )
    {
        while ( lodInfos > rgp.world->worldLodInfos[v32->firstLodInfo + v5].dist && v5 < v32->lodInfoCount )
            ++v5;
        if ( lodInfos > v32->lastDist )
            ++v5;
    }
    else
    {
        v5 = 1;
    }
    v33->UpdateLevel(v5, localClientNum);
    v33->UpdateFade(dt, localClientNum);
    //LodChainState::UpdateLevel(v33, v5, localClientNum);
    //LodChainState::UpdateFade(v33, dt, localClientNum);
}

double __cdecl math::Abs(const math::Dir3 *_v)
{
    return (float)sqrt((float)((float)((float)(_v->v.x * _v->v.x) + (float)(_v->v.y * _v->v.y))
                                                     + (float)(_v->v.z * _v->v.z)));
}

