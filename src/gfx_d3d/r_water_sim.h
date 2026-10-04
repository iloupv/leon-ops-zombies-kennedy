#pragma once

#include "r_gfx.h"
#include "rb_state.h"
#include <universal/physicalmemory.h>
#include <universal/dvar.h>

struct meshExpire_t // sizeof=0x8
{                                       // XREF: fifo_t<meshExpire_t,1024>/r
    unsigned int framesLeft;            // XREF: ExpireMesh:loc_A81A8A/w
    unsigned int baseVertex;            // XREF: ExpireMesh+67/w
};

struct waterconfig_t // sizeof=0x54
{                                       // XREF: .data:config/r
    int framerate;
    float waveDamping;                  // XREF: R_InitWaterSimulation(void)+3FD/w
                                        // R_WaterSimulationUpdateFrame+15B/r
    float waveSeedHeight;               // XREF: R_InitWaterSimulation(void)+3DD/w
                                        // R_WaterSimulationApplyWaveSeed+F1/r
    float waveSeedRadius;               // XREF: R_InitWaterSimulation(void)+3ED/w
                                        // R_WaterSimulationApplyWaveSeed+107/r
    float waveSeedDelay;                // XREF: R_WaterSimulationUpdateConfig+10/w
                                        // R_WaterSimulationApplyWaveSeed+3B/r
    float gridScale;                    // XREF: R_InitWaterSimulation(void)+3CD/w
                                        // R_SetWaterSimulationConstants(GfxCmdBufSourceState *,float)+47/r ...
    float curlAmount;                   // XREF: R_WaterSimulationUpdateConfig+23/w
                                        // R_WaterSimulationUpdateFrame+1D4/r
    float curlMax;                      // XREF: R_WaterSimulationUpdateConfig+36/w
                                        // R_WaterSimulationUpdateFrame+1481/r
    float curlReduce;                   // XREF: R_WaterSimulationUpdateConfig+48/w
                                        // R_WaterSimulationUpdateFrame+148A/r
    float minShoreHeight;               // XREF: R_WaterSimulationUpdateConfig+5B/w
                                        // R_WaterSimulationUpdateFrame+19C/r
    float foamAppear;                   // XREF: R_WaterSimulationUpdateConfig+6E/w
                                        // R_WaterSimulationUpdateFrame+224/r
    float foamDisappear;                // XREF: R_WaterSimulationUpdateConfig+80/w
                                        // R_WaterSimulationUpdateFrame+2611/r
    float windAmount;                   // XREF: R_WaterSimulationUpdateConfig+93/w
                                        // R_WaterSimulationUpdateFrame+576/r
    float windMax;                      // XREF: R_WaterSimulationUpdateConfig+A6/w
                                        // R_WaterSimulationUpdateFrame+1657/r
    float windDir[3];                   // XREF: R_WaterSimulationUpdateConfig+11E/w
                                        // R_WaterSimulationUpdateConfig+12E/w ...
    float particleGravity;              // XREF: R_WaterSimulationUpdateConfig+B8/w
    float particleLimit;                // XREF: R_WaterSimulationUpdateConfig+CB/w
    float particleLength;               // XREF: R_WaterSimulationUpdateConfig+DE/w
    float particleWidth;                // XREF: R_WaterSimulationUpdateConfig+F0/w
};

struct WaterSimulationCmdUpdate // sizeof=0xC
{                                       // XREF: WaterSimulationCmd/r
    unsigned int startTile;             // XREF: R_WaterSimulationRender(float const * const,int,uint)+222/r
    unsigned int srcBuf;
    unsigned int destBuf;               // XREF: R_WaterSimulationRender(float const * const,int,uint):loc_A820B9/r
};

struct GfxWaterVertex // sizeof=0x10
{
    unsigned __int8 pos[4];
    unsigned __int8 normal[4];
    GfxColor color;
    unsigned __int8 texCoord[4];
};

struct tilemesh_t // sizeof=0x20
{                                       // XREF: fifo_t<tilemesh_t,4>/r
    int srcU;
    int srcV;
    int worldX;
    int worldY;
    int worldZ;
    int lastSkinTime;
    unsigned int baseVertex;
    GfxWaterVertex *lockedData;
};

struct WaterSimulationCmdSkin // sizeof=0x48
{                                       // XREF: WaterSimulationCmd/r
    unsigned int startTile;
    unsigned int srcBuf;
    //$23E4FC536F9FEE55F47DDD857C0ED32E ___u2;
    union //$23E4FC536F9FEE55F47DDD857C0ED32E // sizeof=0x40
    {                                       // XREF: WaterSimulationCmdSkin/r
        tilemesh_t *meshesEA[16];
        tilemesh_t *meshes[16];
    };
};

struct WaterSimulationCmd // sizeof=0x54
{                                       // XREF: .data:prevCmd/r
                                        // ?R_WaterSimulationRender@@YAXQBMHI@Z/r
    WaterSimulationCmdUpdate update;    // XREF: R_WaterSimulationRender(float const * const,int,uint):loc_A820B9/r
                                        // R_WaterSimulationRender(float const * const,int,uint)+222/r
    WaterSimulationCmdSkin skin;
};

template <typename T, int SIZE>
struct fifo_t//<tilemesh_t,4> // sizeof=0x88
{                                       // XREF: tile_t/r
    unsigned int head;
    unsigned int tail;
    T data[SIZE];

    fifo_t()
    {
        tail = 0;
        head = 0;
    }

    void add(const T *t)
    {
        data[tail] = *t;

        tail = (tail + 1) & (SIZE - 1);

        if (head == tail)
        {
            head = (head + 1) & (SIZE - 1);
        }
    }

};

struct tile_t // sizeof=0xBC
{                                       // XREF: waterdata_t/r
    int srcU;
    int srcV;
    int worldX;
    int worldY;
    int worldZ;
    bool occupied;
    // padding byte
    // padding byte
    // padding byte
    float minMax[2][3];
    unsigned int lastUpdateTime;
    fifo_t<tilemesh_t,4> meshes;
};

struct ComWaterCell;

struct debugpoint_t // sizeof=0xC
{                                       // XREF: fifo_t<debugpoint_t,16>/r
    float pos[3];                       // XREF: R_WaterSimulationNotifyBullet(float const * const)+1A/w
};

// aislop used
template <typename T>
struct __declspec(align(16)) channel_t
{
    channel_t()
    {
        int v2; // [esp+4h] [ebp-8h]
        cache_t *i; // [esp+8h] [ebp-4h]

        v2 = 4;
        for (i = this->cache; --v2 >= 0; ++i)
        {
            i->y = -1;
            i->modified = 0;
        }
        this->v = 0;
        this->bufferSize = 0;
        this->cache_now = 0;
        //return this;
    }

    void Alloc(unsigned int location)
    {
        if (this->v
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_sim.cpp",
                179,
                0,
                "%s",
                "v == NULL"))
        {
            __debugbreak();
        }

        // 256 * 256 elements + padding
        this->bufferSize = sizeof(T) * 0x10000 + 0x80;

        this->v = (T *)_PMem_Alloc(
            this->bufferSize,
            0x10u,          // 16-byte alignment
            4u,
            location,
            TRACK_WATERSIM,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_sim.cpp",
            189);
    }

    T *Cache(unsigned int y, bool modify, bool flush)
    {
        unsigned int i;
        cache_t *c;

        if (y > 256
            && !Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_water_sim.cpp",
                216,
                0,
                "y not in [0, GRIDSIZE]\n\t%i not in [%i, %i]",
                y,
                0,
                256))
        {
            __debugbreak();
        }

        // Try cache hit
        if (!flush)
        {
            for (i = 0; i < 4; ++i)
            {
                if (cache[i].y == (int)y)
                {
                    cache[i].modified |= modify;
                    return cache[i].data;
                }
            }
        }

        // Evict next cache line (round-robin)
        c = &cache[cache_now++];
        cache_now &= 3;

        // Write back if dirty
        if (c->modified)
        {
            memcpy(
                &v[256 * c->y],
                c->data,
                sizeof(T) * 256);
            c->modified = false;
        }

        // Flush-only path
        if (flush)
        {
            c->y = -1;
            return nullptr;
        }

        // Fill cache
        c->modified = modify;
        c->y = y;

        memcpy(
            c->data,
            &v[256 * y],
            sizeof(T) * 256);

        // Halo copies for SIMD safety
        c->pad0 = c->data[255];
        c->pad1 = c->data[0];

        return c->data;
    }


    struct __declspec(align(16)) cache_t
    {
        cache_t()
        {
            this->y = -1;
            this->modified = 0;
        }

        T pad0;

        // pad to 0x10
        //char _pad0[16 - (sizeof(T) % 16 == 0 ? 16 : sizeof(T) % 16)];

        T data[256];
        T pad1;

        int  y;
        bool modified;

        char _pad1[
            (16 - ((sizeof(T)
                + (16 - (sizeof(T) % 16 == 0 ? 16 : sizeof(T) % 16))
                + sizeof(T) * 256
                + sizeof(T)
                + sizeof(int)
                + sizeof(bool)) % 16)) % 16
        ];
    };

    void Free()
    {
        iassert(v);
        v = 0;
    }

    T *v;
    unsigned int bufferSize;
    char         _pad0[8];

    cache_t cache[4];

    int  cache_now;
    char _pad1[12];
};

struct WaterSimInfo // sizeof=0x3C
{                                       // XREF: ?R_WaterSimulationRender@@YAXQBMHI@Z/r
    float pos[3];
    float tangent[3];
    float binormal[3];
    float normal[3];
    float waveVelocity[3];
};

struct waterdata_t // sizeof=0x160A0
{                                       // XREF: .data:data/r
    waterdata_t() = default;

    float waveTimeleft;                 // XREF: R_WaterSimulationRestart(void)+E/w
                                        // R_WaterSimulationApplyWaveSeed+6/r ...
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    channel_t<float4> buffer[2];        // XREF: R_InitWaterSimulation(void)+405/r
                                        // R_InitWaterSimulation(void)+40D/r ...
    channel_t<short> waterheight;       // XREF: R_InitWaterSimulation(void)+43A/r
                                        // R_InitWaterSimulation(void)+442/r ...
    channel_t<char> flooroffset;        // XREF: R_InitWaterSimulation(void)+451/r
                                        // R_InitWaterSimulation(void)+45A/r ...
    channel_t<char> shoredist;          // XREF: R_InitWaterSimulation(void)+468/r
                                        // R_InitWaterSimulation(void)+471/r ...
    channel_t<GfxColor> colors;         // XREF: R_InitWaterSimulation(void)+480/r
                                        // R_InitWaterSimulation(void)+488/r ...
    unsigned int frame;                 // XREF: R_WaterSimulationTeleport+2B/w
                                        // R_WaterSimulationRender(float const * const,int,uint)+15A/r ...
    bool enabled;                       // XREF: R_WaterSimulationRestart(void)+20/w
                                        // R_SetWaterSimulationConstants(GfxCmdBufSourceState *,float)+2C/r ...
    // padding byte
    // padding byte
    // padding byte
    int timeDelta;                      // XREF: R_WaterSimulationRestart(void)+16/w
                                        // R_WaterSimulationRender(float const * const,int,uint):loc_A81FC8/r ...
    Material *sprayMaterial;            // XREF: R_InitWaterSimulation(void)+55B/w
    int gridX;                          // XREF: R_WaterSimulationTeleport+9/w
                                        // R_WaterSimulationTeleport+BB/r ...
    int gridY;                          // XREF: R_WaterSimulationTeleport+11/w
                                        // R_WaterSimulationTeleport+D1/r ...
    int centerU;                        // XREF: R_WaterSimulationTeleport+17/w
                                        // R_WaterSimulationTrackView+2A4/r ...
    int centerV;                        // XREF: R_WaterSimulationTeleport+21/w
                                        // R_WaterSimulationTrackView+2B8/r ...
    float localSurfaceDistance;         // XREF: R_WaterSimulationRestart(void)+2F/w
                                        // R_SetWaterSimulationConstants(GfxCmdBufSourceState *,float)+7F/r ...
    float localEyeHeight;               // XREF: R_WaterSimulationRestart(void)+3F/w
                                        // R_WaterSimulationTrackView:loc_A8265C/r ...
    float speedScale;                   // XREF: R_WaterSimulationRestart(void)+4F/w
                                        // R_SetWaterSimulationConstants(GfxCmdBufSourceState *,float)+3F/r ...
    float oldEyePos[3];                 // XREF: R_WaterSimulationRestart(void)+5F/w
                                        // R_WaterSimulationRestart(void)+6F/w ...
    int tileTriCount[2];                // XREF: R_RenderWaterModel+272/r
                                        // R_InitWaterSimulationVertexBuffers(uint)+4B/w ...
    IDirect3DIndexBuffer9 *indexBuffer[2];
                                        // XREF: R_RenderWaterModel+259/r
                                        // R_InitWaterSimulationVertexBuffers(uint)+6F/o ...
    IDirect3DVertexBuffer9 *vertexBuffer;
                                        // XREF: R_RenderWaterModel+17E/r
                                        // R_RenderWaterModel+18A/r ...
    GfxWaterVertex *lockedData;         // XREF: R_WaterSimulationRender(float const * const,int,uint)+3ED/r
                                        // R_InitWaterSimulationVertexBuffers(uint)+3C/w
    tile_t tiles[256];                  // XREF: R_WaterSimulationRestart(void)+AB/o
                                        // R_WaterSimulationTeleport+82/o ...
};

void __cdecl R_InitWaterSimulation();
void __cdecl R_WaterSimulationRestart();
void __cdecl ExpireAllMeshes(tile_t *tile);
void __cdecl ExpireMesh(tile_t *tile);
void __cdecl R_WaterSimulationTeleport(int posX, int posY);
void __cdecl R_InitWaterSimulationBuffers(unsigned int location);
void __cdecl R_FreeWaterSimulationBuffers();
void __cdecl R_SetWaterSimulationConstants(GfxCmdBufSourceState *state, float interp);
void    R_WaterSimulationRender(const float *eyePos, int time, unsigned int viewIndex);
void __cdecl R_WaterSimulationTrackView(const float *eyePos);
bool __cdecl R_WaterSimulationFillTile(tile_t *tile);
char __cdecl R_WaterFindCells(int gridX, int gridY, ComWaterCell **cells);
unsigned int __cdecl AllocMeshVerts();
unsigned int __cdecl NumMeshVertsFree();
void __cdecl FreeMeshVerts(unsigned int baseVertex);
void R_WaterSimulationUpdateConfig();
void __cdecl R_RenderWaterModel(unsigned int viewIndex);
char *__cdecl R_SurfaceAlloc(unsigned int size);
void __cdecl R_WaterSimulationSwitchOff();
void *FlushBuffers();
void    R_WaterSimulationUpdateFrame(const WaterSimulationCmd *cmd);
void __cdecl Float4LoadFloats(unsigned int x, unsigned int y, unsigned int z, unsigned int w, float4 *result);
void *__cdecl Water_AllocVertexBuffer(IDirect3DVertexBuffer9 **vb, unsigned int buffersize);
void *__cdecl Water_AllocIndexBuffer(IDirect3DIndexBuffer9 **ib, unsigned int buffersize);
void __cdecl R_InitWaterSimulationVertexBuffers();
unsigned int __cdecl FillIndexBuffer(unsigned __int16 *lockedindex, unsigned int lodMask, unsigned __int16 *verts);
void __cdecl R_FreeWaterSimulationVertexBuffers();
void __cdecl R_WaterSimulationNotifyBullet(const float *hitpos);
void __cdecl R_WaterSimulationSetHeight(const float *center, float radius, float targetHeight, float lerpRate);
void __cdecl GetBuffersForPoint(int u, int v, int *bufId, float *interpolate);
char __cdecl ConvertWorldToGrid(const float *worldPos, int *u, int *v);
void __cdecl R_WaterSimulationSimpleDisplace(const float *hitpos, float radius, float height);
void __cdecl R_WaterSimulationNotifyExplosionEx(const float *hitpos, int size, float height);
void __cdecl R_WaterSimulationNotifyExplosion(const float *hitpos);
void R_WaterSimulationApplyWaveSeed();
int __cdecl water_rand();
void __cdecl R_WaterSimulationCmd(const WaterSimulationCmd *rawdata);
unsigned int LockUpdateMutex();
bool UnlockUpdateMutex();
void    R_WaterSimulationSkinMesh(const WaterSimulationCmd *cmd);
float4 *__cdecl ReadValue(float4 *result, unsigned __int8 u, int v, unsigned int bufnum);
double __cdecl R_WaterSimulationGetBaseHeight(const float *queryPos);
double __cdecl R_WaterSimulationGetHeight(const float *queryPos);
void __cdecl R_WaterSimulationGetGridPosition(unsigned __int8 sx, unsigned __int8 sy, int timeOffset, float *v);
void __cdecl R_WaterSimulationGetInfo(const float *queryPos, WaterSimInfo *info);
double __cdecl R_WaterSimulationGetHeightGlobal(const float *queryPos);
char __cdecl R_WaterSimulationGetAverageHeight(float *pHeight);

extern const dvar_t *r_watersim_enabled;
extern const dvar_t *r_watersim_debug;
extern const dvar_t *r_watersim_flatten;
extern const dvar_t *r_watersim_waveSeedDelay;
extern const dvar_t *r_watersim_curlAmount;
extern const dvar_t *r_watersim_curlMax;
extern const dvar_t *r_watersim_curlReduce;
extern const dvar_t *r_watersim_minShoreHeight;
extern const dvar_t *r_watersim_foamAppear;
extern const dvar_t *r_watersim_foamDisappear;
extern const dvar_t *r_watersim_windAmount;
extern const dvar_t *r_watersim_windDir;
extern const dvar_t *r_watersim_windMax;
extern const dvar_t *r_watersim_particleGravity;
extern const dvar_t *r_watersim_particleLimit;
extern const dvar_t *r_watersim_particleLength;
extern const dvar_t *r_watersim_particleWidth;
extern const dvar_t *r_watersim_scroll;