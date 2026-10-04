#pragma once
#include <xanim/dobj.h>
#include <xanim/xanim.h>
#include "r_scene.h"



struct r_SkinConditions // sizeof=0x4
{                                       // XREF: .data:r_SkinConditions g_skinErrorFlags/r
    union //$17997E609355AF3513DE5D7A94073E88 // sizeof=0x4
    {                                       // XREF: G_UpdateActorCorpses(void)+6/r
                                            // G_UpdateActorCorpses(void):loc_8ECA9C/r ...
        //$4BAAEF421A7DA1E87559C5E7C38D0061 __s0;
        struct //$4BAAEF421A7DA1E87559C5E7C38D0061 // sizeof=0x4
        {                                       // XREF: $17997E609355AF3513DE5D7A94073E88/r
            unsigned __int32 outOfSkinnedCachedVerts : 1;
            unsigned __int32 almostoutOfSkinnedCachedVerts : 1;
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
            // padding bit
        };
        unsigned int allbits;
    };
};

struct __declspec(align(2)) GfxModelSurfaceInfo // sizeof=0xC
{                                       // XREF: GfxModelSkinnedSurface/r
    //$428C0DFD0D9970E1A3749A5754AD806F ___u0;
    union //$428C0DFD0D9970E1A3749A5754AD806F // sizeof=0x4
    {                                       // XREF: GfxModelSurfaceInfo/r
        const DObjAnimMat *baseMat;
        const DObjAnimMat *baseMat_local;
    };
    unsigned __int8 boneIndex;
    unsigned __int8 boneCount;
    unsigned __int16 gfxEntIndex;
    unsigned __int16 lightingHandle;
    unsigned __int8 dobjModelIndex;
    // padding byte
};

struct GfxModelSkinnedSurface // sizeof=0x18
{                                       // XREF: GfxModelRigidSurface/r
    int skinnedCachedOffset;
    XSurface *xsurf;
    GfxModelSurfaceInfo info;
    //$BFB86953EFC7B86272823EA22BE25520 ___u3;
    union //$BFB86953EFC7B86272823EA22BE25520 // sizeof=0x4
    {                                       // XREF: GfxModelSkinnedSurface/r
        GfxPackedVertex *skinnedVert;
        int oldSkinnedCachedOffset;
    };
};

struct GfxModelRigidSurface // sizeof=0x38
{
    GfxModelSkinnedSurface surf;
    GfxScaledPlacement placement;
};

struct __declspec(align(8)) SkinXModelCmd // sizeof=0x30
{
    void *modelSurfs;
    const DObjAnimMat *mat;
    int surfacePartBits[5];
    float viewoffset[3];
    unsigned __int16 surfCount;
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
    // padding byte
};

void __cdecl R_FlagXModelAsSkinned(GfxSceneEntity *sceneEnt, unsigned int surfaceCount);
void __cdecl R_SkinSceneDObj(
                GfxSceneEntity *sceneEnt,
                GfxSceneEntity *localSceneEnt,
                const DObj *obj,
                DObjAnimMat *boneMatrix,
                int waitForCullState);
unsigned int    R_SkinSceneDObjModels(
                GfxSceneEntity *sceneEnt,
                const DObj *obj,
                DObjAnimMat *boneMatrix);
int __cdecl R_AllocSkinnedCachedVerts(int vertCount, bool highPriority);
int    R_PreSkinXSurface(
                const DObj *obj,
                XSurface *surf,
                const GfxModelSurfaceInfo *surfaceInfo,
                unsigned int *numSkinnedVerts,
                GfxModelRigidSurface *surfPos);
void __cdecl R_TransformSkelMat(const float *in1, const float4 *in2, float *out);
void __cdecl R_SkinGfxEntityCmd(GfxSceneEntity **data);

extern r_SkinConditions g_skinErrorFlags;