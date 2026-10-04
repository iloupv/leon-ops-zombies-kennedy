#pragma once

struct __declspec(align(4)) GfxWorldLodChain // sizeof=0x18
{
    float origin[3];
    float lastDist;
    unsigned int firstLodInfo;
    unsigned __int16 lodInfoCount;
    // padding byte
    // padding byte
};

struct LodChainState // sizeof=0xC
{
    float m_fade;
    unsigned __int8 m_lastLevel;
    unsigned __int8 m_curLevel;
    // padding byte
    // padding byte
    const GfxWorldLodChain *m_lodChain;

    void __thiscall Init(const GfxWorldLodChain *lodChain, int localClientNum);
    void __thiscall UpdateLevel(int newLevel, int localClientNum);
    void __thiscall UpdateFade(float dt, int localClientNum);
};

struct __declspec(align(4)) GfxWorldLodInfo // sizeof=0xC
{
    float dist;
    unsigned int firstSurf;
    unsigned __int16 surfCount;
    // padding byte
    // padding byte
};

void __cdecl SetFadeForSortedSurfaces(float val, int lodInfoIndex, int localClientNum);
void __cdecl R_WorldLod_Init();
void __cdecl R_WorldLod_FrameUpdate(float curTime, float *camPos, int localClientNum);
// local variable allocation has failed, the output may be wrong!
void    UpdateChain(int index, const float *inputCamPos, float dt, int localClientNum);

namespace math // BO1TODO: absolutely retarded
{
    struct Float4 // sizeof=0x10
    {                                       // XREF: .rdata:Float4_Zero/r
        float x;                            // XREF: math::Abs(math::Dir3 const &)+5A/w
        float y;                            // XREF: math::Abs(math::Dir3 const &)+50/w
        float z;                            // XREF: math::Abs(math::Dir3 const &)+46/w
        float w;                            // XREF: math::Abs(math::Dir3 const &)+3C/w
    };
    struct Dir3 // sizeof=0x10
    {                                       // XREF: math::Mat43/r
                                            // math::Mat43/r ...
        math::Float4 v;
    };
    struct Position3 // sizeof=0x10
    {                                       // XREF: math::Mat43/r
        math::Float4 v;
    };
    double Abs(const math::Dir3 *_v);
}

extern unsigned __int8 *s_lodVals;
