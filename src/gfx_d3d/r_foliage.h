#pragma once
#include "r_bsp.h"
#include "rb_state.h"
#include "r_state.h"
#include "r_rendercmds.h"
#include <tl/jobqueue/jobqueue_all.h>

struct ForceBoundsInfo // sizeof=0x40
{                                       // XREF: DrawStateWorkerSharedBuffer/r
    float persistForceBounds[2][3];
    float instantForceBounds[2][3];
    float windForceBounds[2][2];
};

struct ActiveModelNode // sizeof=0x8
{                                       // XREF: ModelHashTable<ActiveModelNode,63,128>/r
                                        // ?R_FoliageNotifyBurn@@YAXQAMMPAHH_N@Z/r ...
    float curSquashValue;               // XREF: R_FoliageNotifyBurn(float * const,float,int *,int,bool)+1D5/w
                                        // R_FoliageNotifyBurn(float * const,float,int *,int,bool)+3D0/w
    unsigned __int16 modelIndex;        // XREF: R_FoliageNotifyBurn(float * const,float,int *,int,bool)+1DE/w
                                        // R_FoliageNotifyBurn(float * const,float,int *,int,bool)+3DF/w
    unsigned __int16 nextNode;
};

struct GrassWindForce // sizeof=0x10
{                                       // XREF: PerFrameFoliageInfo/r
    float worldPosition[2];
    float inverseRadiusSquared;
    float strength;
};

struct GrassForce // sizeof=0x18
{                                       // XREF: PerFrameFoliageInfo/r
                                        // PerFrameFoliageInfo/r ...
    float worldPosition[3];             // XREF: R_FoliageNotifyVehiclePosition(int,float const * const)+D/w
                                        // R_FoliageNotifyVehiclePosition(int,float const * const)+1A/w ...
    float radius;                       // XREF: R_FoliageNotifyVehiclePosition(int,float const * const)+34/w
    float strength;                     // XREF: R_FoliageNotifyVehiclePosition(int,float const * const)+41/w
    float frequency;                    // XREF: R_FoliageNotifyVehiclePosition(int,float const * const)+4E/w
};

struct PerFrameFoliageInfo // sizeof=0x34C
{                                       // XREF: DrawStateWorkerSharedBuffer/r
    GrassForce grassInstantForces[16];
    GrassForce grassPersistantForces[8];
    GrassWindForce grassWindForces[16];
    unsigned int numGrassInstantForces;
    unsigned int numGrassPersistantForces;
    unsigned int numGrassWindForces;

    PerFrameFoliageInfo()
    {
        numGrassInstantForces = 0;
        numGrassPersistantForces = 0;
        numGrassWindForces = 0;
    }
};

struct __declspec(align(4)) DrawStateWorkerSharedBuffer // sizeof=0x1390
{                                       // XREF: .data:DrawStateWorkerSharedBuffer g_drawStateWorkerSharedBuffer/r
    unsigned __int16 visibleDynSModelList[2048];
    PerFrameFoliageInfo frameInfo;      // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+62/o
    // _dynamic_initializer_for__g_drawStateWorkerSharedBuffer__+4/o
    ForceBoundsInfo forceBounds;        // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+69/o
    bool inUse;                         // XREF: R_DynSModelWaitWorker(void)+22/w
    // R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+2A/r ...
    // padding byte
    // padding byte
    // padding byte
};

struct FoliageShaderConstantBlock // sizeof=0x40
{                                       // XREF: ?R_FoliageSetStaticModelShaderConstants@@YAXPAUGfxCmdBufContext@@IPBUDynSModelClientView@@PBUDynSModelGfxState@@@Z/r
    float grassParms[4];
    float grassForce0[4];               // XREF: R_FoliageSetStaticModelShaderConstants(GfxCmdBufContext *,uint,DynSModelClientView const *,DynSModelGfxState const *)+2DD/w
                                        // R_FoliageSetStaticModelShaderConstants(GfxCmdBufContext *,uint,DynSModelClientView const *,DynSModelGfxState const *)+2ED/w ...
    float grassForce1[4];               // XREF: R_FoliageSetStaticModelShaderConstants(GfxCmdBufContext *,uint,DynSModelClientView const *,DynSModelGfxState const *)+336/w
                                        // R_FoliageSetStaticModelShaderConstants(GfxCmdBufContext *,uint,DynSModelClientView const *,DynSModelGfxState const *)+346/w ...
    float windForce0[4];                // XREF: R_FoliageSetStaticModelShaderConstants(GfxCmdBufContext *,uint,DynSModelClientView const *,DynSModelGfxState const *)+3B9/w
                                        // R_FoliageSetStaticModelShaderConstants(GfxCmdBufContext *,uint,DynSModelClientView const *,DynSModelGfxState const *)+3C6/w ...
};

struct GrassPersistForce // sizeof=0x3C
{                                       // XREF: ClientFoliageInfo/r
    enum ForceType : __int32
    {                                       // XREF: GrassPersistForce/r
        GRENADE_EXPLOSION = 0x0,
        NEAR_MUZZLE_SHAKE = 0x1,
    };
    enum ShakeForceState : __int32
    {                                       // XREF: GrassPersistForce/r
        GAINING     = 0x0,
        MAINTAINING = 0x1,
        DECAYING    = 0x2,
    };

    float worldPosition[3];
    float radius;
    float strength;
    float frequency;
    float age;
    float maxAge;
    float maxStrength;
    bool isActive;
    // padding byte
    // padding byte
    // padding byte
    GrassPersistForce::ForceType forceType;
    unsigned int id;
    float curForcePercent;
    float trgForcePercent;
    GrassPersistForce::ShakeForceState shakeForceState;
};

struct __declspec(align(4)) ClientFoliageInfo // sizeof=0x888
{                                       // XREF: .data:ClientFoliageInfo * g_foliageInfo/r
    PerFrameFoliageInfo perFrame[2];
    GrassPersistForce persistantForces[8];
                                        // XREF: AddNearMuzzleShakeForce(uint,float const * const,float const * const,FoliageShakeParams const &)+F9/o
    float clientPlayerPos[3];           // XREF: AddNearMuzzleShakeForce(uint,float const * const,float const * const,FoliageShakeParams const &)+41/o
                                        // AddInstantForce+41/o
    bool clientIsActive;                // XREF: AddNearMuzzleShakeForce(uint,float const * const,float const * const,FoliageShakeParams const &)+2B/r
                                        // AddInstantForce+2B/r
    // padding byte
    // padding byte
    // padding byte

    ClientFoliageInfo();
};

struct FoliageShakeParams // sizeof=0x14
{                                       // XREF: .data:FoliageShakeParams fsp_WeaponFiring/r
                                        // .data:FoliageShakeParams fsp_FlameThrower/r ...
    float maxAge;
    float radius;
    float strength;
    float frequency;
    float locForwardExtension;
};

struct DynSModelDrawStateCmd // sizeof=0x24
{                                       // XREF: ?R_DynSModelBuildClientView@@YAXPBUDynSModelClient@@PAUDynSModelClientView@@PAUDynSModelGfxState@@@Z/r
    PerFrameFoliageInfo *frameInfo;     // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+A95/w
    unsigned __int16 *smodelList;       // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+AD3/w
    unsigned int listBegin;             // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+ADC/w
    unsigned int listEnd;               // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+AEB/w
    DynSModelClientView *view;          // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+AF4/w
    DynSModelGfxState *dynData;         // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+AFD/w
    unsigned int persistForceIndexOffset;
                                        // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+B03/w
    unsigned int instantForceIndexOffset;
                                        // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+B0D/w
    ForceBoundsInfo *boundsInfo;        // XREF: R_DynSModelBuildClientView(DynSModelClient const *,DynSModelClientView *,DynSModelGfxState *)+B1D/w
};

bool __cdecl ModelIsFullySquashed(unsigned int drawInstIndex);
void __cdecl SetModelIsFullySquashed(unsigned int drawInstIndex);
double __cdecl GetFoliageSquashFromHash(unsigned int drawInstIndex);
void __cdecl R_FoliageSetPerFramePersistantForces(
                PerFrameFoliageInfo *frameInfo,
                const ClientFoliageInfo *clientFoliageInfo);
bool __cdecl GrassPersistForceIsActive(const GrassPersistForce *force);
void __cdecl R_FoliageInitModels(GfxWorld *gfxWorld);
void __cdecl R_MarkDynFoliageModels(GfxWorld *gfxWorld);
void __cdecl R_FoliageShutdown();
void __cdecl R_FoliageSetInitialBurnState();
float __cdecl R_GetWindBestStrength(
                const GrassWindForce *windForces,
                unsigned int numWindForces,
                const float *modelPlacement);
bool __cdecl R_StaticModelNeedsCharredTech(unsigned int modelDrawInstIndex);
void __cdecl R_FoliageNotifyBurn(float *loc, float rad, int *models, int models_count, bool instant);
bool __cdecl AABBOverlapsSphere(
                const float *boxMin,
                const float *boxMax,
                float sphereRadiusSquared,
                const float *sphereCenter);
void __cdecl AddNearMuzzleShakeForce(
                unsigned int id,
                const float *forcePos,
                const float *forwardVector,
                const FoliageShakeParams *params);
void __cdecl SetGrassPersistForceToActive(GrassPersistForce *force);
void __cdecl R_FoliageNotifyGrenadeExplosion(int localClientNum, float *loc);
void __cdecl R_FoliageNofityWeaponFiring(
                int localClientNum,
                unsigned int id,
                const float *loc,
                const float *forwardVector,
                bool isFlameThrower);
void __cdecl R_FoliageSetGrassWindForces(int clientIdx, float *pos, float radius, float strength);
void __cdecl DrawDebugAxis(const float *pos);
void __cdecl AddInstantForce(const GrassForce *force);
void __cdecl R_FoliageNotifyVehiclePosition(int localClientNum, const float *vehiclePos);
void __cdecl R_FoliageSetDefaultShaderConstants(GfxCmdBufSourceState *source);
void __cdecl R_SetCodeConstantFromVec4(GfxCmdBufSourceState *source, CodeConstant constant, const float *value);
void __cdecl R_DirtyCodeConstant(GfxCmdBufSourceState *source, CodeConstant constant);
void __cdecl R_DynSModelInitGfxState(DynSModelGfxState *dynData);
int __cdecl dyn_smodel_drawstateCallback(jqBatch *batch);
void __cdecl R_CalcVisDynSModelDrawState(
                const unsigned __int16 *modelIndexList,
                unsigned int listBegin,
                unsigned int listEnd,
                const PerFrameFoliageInfo *frameInfo,
                DynSModelClientView *view,
                DynSModelGfxState *dynData,
                unsigned int persistForceIndexOffset,
                unsigned int instantForceIndexOffset,
                const ForceBoundsInfo *boundsInfo);
void __cdecl R_DynSModelWaitWorker();
void __cdecl R_DynSModelBuildClientView(
                const DynSModelClient *clientVis,
                DynSModelClientView *view,
                DynSModelGfxState *dynData);
void __cdecl R_FoliageSetStaticModelShaderConstants(
                GfxCmdBufContext *context,
                unsigned int smodelDrawIndex,
                const DynSModelClientView *smodelClientView,
                const DynSModelGfxState *smodelState);
int __cdecl R_UpdateCodeConstant(
                GfxCmdBufSourceState *source,
                CodeConstant constant,
                float x,
                float y,
                float z,
                float w);
void __cdecl R_SetCodeConstant(GfxCmdBufSourceState *source, CodeConstant constant, float x, float y, float z, float w);
int __cdecl R_UpdateCodeConstantFromVec4(GfxCmdBufSourceState *source, CodeConstant constant, float *value);
void __cdecl R_GetGrassParmShaderConstant(float curSquashAmt, float *out);
void __cdecl R_DynSModelGetCounts(unsigned int *numDynSModelInMap, unsigned int *maxNumDynSModel);


// aislop used for member funcs
template <typename T, int TABLE_SIZE, int NUM_NODES>
struct ModelHashTable//<ActiveModelNode,63,128> // sizeof=0x480
{                                       // XREF: .data:ModelHashTable<ActiveModelNode,63,128> g_activeModelsHash/r
                                        // ActiveModelsHash/r
    T m_nodes[NUM_NODES];
    unsigned __int16 m_table[TABLE_SIZE];
    unsigned __int16 m_firstFreeNode;

    static constexpr uint16_t INVALID = 0xFFFF;

    void Init()
    {
        // Build free list
        for (uint16_t i = 0; i < NUM_NODES - 1; ++i)
            m_nodes[i].nextNode = i + 1;

        m_nodes[NUM_NODES - 1].nextNode = INVALID;
        m_firstFreeNode = 0;

        // Clear hash buckets
        for (uint16_t i = 0; i < TABLE_SIZE; ++i)
            m_table[i] = INVALID;
    }

    bool Add(const T &node)
    {
        if (m_firstFreeNode == INVALID)
            return false; // table full

        // Pop free node
        uint16_t nodeIndex = m_firstFreeNode;
        T &dst = m_nodes[nodeIndex];
        m_firstFreeNode = dst.nextNode;

        // Copy payload
        dst = node;

        // Insert into hash bucket (push front)
        uint16_t hash = node.modelIndex % TABLE_SIZE;
        dst.nextNode = m_table[hash];
        m_table[hash] = nodeIndex;

        return true;
    }


    T *FindByKey(unsigned short key)
    {
        for (uint16_t index = m_table[key % TABLE_SIZE];
            index != INVALID;
            index = m_nodes[index].nextNode)
        {
            if (m_nodes[index].modelIndex == key)
                return &m_nodes[index];
        }

        return nullptr;
    }
};