#pragma once
#include <physics/physpreset_load_obj.h>
#include <gfx_d3d/fxprimitives.h>

struct FxEditorElemAtlas // sizeof=0x20
{                                       // XREF: FxEditorElemDef/r
    int behavior;
    int index;
    int fps;
    int loopCount;
    int indexRange;
    int colIndexBits;
    int rowIndexBits;
    int entryCount;
};

struct FxCurve // sizeof=0xC
{
    int dimensionCount;
    int keyCount;
    float keys[1];
};

struct FxEditorTrailDef // sizeof=0x608
{                                       // XREF: FxEditorElemDef/r
    FxTrailVertex verts[64];
    int vertCount;
    unsigned __int16 inds[128];
    int indCount;
};

struct FxEditorElemDef // sizeof=0x974
{                                       // XREF: FxEditorEffectDef/r
    char name[256];
    int editorFlags;
    int flags;
    FxFloatRange spawnRange;
    FxFloatRange fadeInRange;
    FxFloatRange fadeOutRange;
    float spawnFrustumCullRadius;
    FxSpawnDefLooping spawnLooping;
    FxSpawnDefOneShot spawnOneShot;
    FxIntRange spawnDelayMsec;
    FxIntRange lifeSpanMsec;
    FxFloatRange spawnOrigin[3];
    FxFloatRange spawnOffsetRadius;
    FxFloatRange spawnOffsetHeight;
    FxFloatRange spawnAngles[3];
    FxFloatRange angularVelocity[3];
    FxFloatRange initialRotation;
    float rotationAxis[4];
    FxFloatRange gravity;
    FxFloatRange elasticity;
    FxEditorElemAtlas atlas;
    float timeScale;
    float windInfluence;
    float velScale[2][3];
    const FxCurve *velShape[2][3][2];
    float rotationScale;
    const FxCurve *rotationShape[2];
    float sizeScale[2];
    const FxCurve *sizeShape[2][2];
    float scaleScale;
    const FxCurve *scaleShape[2];
    const FxCurve *color[2];
    const FxCurve *alpha[2];
    float lightingFrac;
    float collOffset[3];
    float collRadius;
    const FxEffectDef *effectOnImpact;
    const FxEffectDef *effectOnDeath;
    int sortOrder;
    const FxEffectDef *emission;
    FxFloatRange emitDist;
    FxFloatRange emitDistVariance;
    const FxEffectDef *attachment;
    unsigned __int8 elemType;
    // padding byte
    // padding byte
    // padding byte
    int visualCount;
    //$02AC52774A03C1B6F49B7C17F56F8A85 ___u45;
    union //$02AC52774A03C1B6F49B7C17F56F8A85 // sizeof=0x80
    {                                       // XREF: FxEditorElemDef/r
        FxElemVisuals visuals[32];
        FxElemMarkVisuals markVisuals[16];
    };
    int trailSplitDist;
    int trailRepeatDist;
    float trailScrollTime;
    FxEditorTrailDef trailDef;
    int alphaFadeTimeMsec;
    int maxWindMagnitude;
    int spawnIntervalAtMaxWind;
    int lifespanAtMaxWind;
    float billboardTopWidth;
    float billboardBottomWidth;
    FxElemSpawnSound spawnSound;
    FxIntRange cloudDensityRange;
    float billboardPivot[2];
};

struct FxFlagOutputSet // sizeof=0x10
{                                       // XREF: FX_ParseNonAtlasFlags/r
    int *flags[4];                      // XREF: FX_ParseNonAtlasFlags+6/w
};

struct FxEditorEffectDef // sizeof=0x12EE0
{
    char name[64];
    int elemCount;
    unsigned __int8 flags;
    unsigned __int8 editorFlags;
    unsigned __int8 edPriority;
    // padding byte
    float boundingBoxDim[3];
    float boundingBoxCentre[3];
    FxEditorElemDef elems[32];
};

struct FxFlagDef // sizeof=0x10
{
    const char *name;
    int flagType;
    int mask;
    int value;
};

struct FxElemField // sizeof=0x8
{                                       // XREF: .rdata:s_elemFields/r
    const char *keyName;                // XREF: FX_ParseEditorElemField+25/r
                                        // FX_ParseEditorElemField+F8/r
    bool (__cdecl *handler)(const char **, FxEditorElemDef *);
                                        // XREF: FX_ParseEditorElemField+93/r
};

struct FxEffectField // sizeof=0x8
{                                       // XREF: .rdata:s_effectFields/r
    const char *keyName;                // XREF: FX_ParseEditorEffectField+25/r
                                        // FX_ParseEditorEffectField+F8/r
    bool (__cdecl *handler)(const char **, FxEditorEffectDef *);
                                        // XREF: FX_ParseEditorEffectField+93/r
};

void __cdecl FX_UnregisterAll();
unsigned __int8 *__cdecl FX_AllocMem(unsigned int size);
PhysPreset *__cdecl FX_RegisterPhysPreset(const char *name);
XModel *__cdecl FX_RegisterModel(char *modelName);
bool __cdecl FX_RegisterMarkMaterials(const char *materialName, Material **materials);
Material *__cdecl FX_RegisterMaterial(char *material);
bool __cdecl FX_ParseName(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseNonAtlasFlags(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseFlagsField(const char **parse, FxFlagOutputSet *flagOutputSet);
bool __cdecl FX_ParseSingleFlag(const char *token, FxFlagOutputSet *flagOutputSet);
bool __cdecl FX_ParseSpawnRange(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVector2(const char **parse, float *value);
bool __cdecl FX_ParseFloat(const char **parse, float *value);
bool __cdecl FX_ParseFadeInRange(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseFadeOutRange(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnFrustumCullRadius(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnDefLooping(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseInt(const char **parse, int *value);
bool __cdecl FX_ParseSpawnDefOneShot(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseIntRange(const char **parse, FxIntRange *range);
bool __cdecl FX_ParseSpawnDelayMsec(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseLifeSpanMsec(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnOrgX(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnOrgY(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnOrgZ(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnOffsetRadius(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnOffsetHeight(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnAnglePitch(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnAngleYaw(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSpawnAngleRoll(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAngleVelPitch(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAngleVelYaw(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAngleVelRoll(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseInitialRot(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseGravity(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseElasticity(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseWindinfluence(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasBehavior(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasIndex(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasIndexRange(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasFps(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasLoopCount(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasColIndexBits(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasRowIndexBits(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAtlasEntryCount(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVelGraph0X(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseGraphRange(
                const char **parse,
                int dimCount,
                float minValue,
                float maxValue,
                float *scale,
                const FxCurve **shape);
bool __cdecl FX_ParseCurve(const char **parse, int dimCount, float minValue, float maxValue, const FxCurve **shape);
bool __cdecl FX_ParseVelGraph0Y(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVelGraph0Z(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVelGraph1X(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVelGraph1Y(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVelGraph1Z(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseRotGraph(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSizeGraph0(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSizeGraph1(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseScaleGraph(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseColorGraph(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAlphaGraph(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseCollOffset(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVector(const char **parse, float *value);
bool __cdecl FX_ParseLightingFrac(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseCollRadius(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseFxOnImpact(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEffectRef(const char **parse, const FxEffectDef **fx);
bool __cdecl FX_ParseFxOnDeath(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseSortOrder(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParsePriority(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAlphaFadeTimeMsec(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseMaxWindMagnitude(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseMaxWindLife(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseMaxWindInterval(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEmission(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEmitDist(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEmitDistVariance(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseAttachment(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTrailRepeatTime(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTrailSplitDist(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTrailScrollTime(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTrailRepeatDist(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseBillboardSprite(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_RegisterAsset_Material(char *name, FxElemVisuals *visuals);
bool __cdecl FX_ParseOrientedSprite(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseRotatedSprite(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseCloud(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTail(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseLine(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTrail(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseTrailDef(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseDecal(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_RegisterAsset_DecalMaterials(const char *name, FxElemMarkVisuals *visuals);
bool __cdecl FX_ParseModel(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_RegisterAsset_Model(char *name, FxElemVisuals *visuals);
bool __cdecl FX_ParseLight(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_SetEditorElemType(FxEditorElemDef *edElemDef, unsigned __int8 type);
bool __cdecl FX_ParseSpotLight(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseRunner(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_RegisterAsset_EffectDef(char *name, FxElemVisuals *visuals);
bool __cdecl FX_ParseSound(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_RegisterAsset_SoundAliasName(char *name, FxElemVisuals *visuals);
bool __cdecl FX_ParseElemSpawnSound(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseElemSpawnSoundAsset(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseBillboardTopWidth(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseBillboardBottomWidth(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseRotationAxis(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseVector4(const char **parse, float *value);
bool __cdecl FX_ParseCloudDensityRange(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEffectBoundingBoxDim(const char **parse, FxEditorEffectDef *edEffectDef);
bool __cdecl FX_ParseEffectBoundingBoxCentre(const char **parse, FxEditorEffectDef *edEffectDef);
bool __cdecl FX_ParseEffectFlags(const char **parse, FxEditorEffectDef *edEffectDef);
bool __cdecl FX_ParseEffectPriority(const char **parse, FxEditorEffectDef *edEffectDef);
bool __cdecl FX_ParseBillboardPivot(const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEditorElem(int version, const char **parse, FxEditorElemDef *edElemDef);
bool __cdecl FX_ParseEditorElemField(const char **parse, FxEditorElemDef *edElemDef, const char *token);
bool __cdecl FX_LoadEditorEffectFromBuffer(
                const char *buffer,
                const char *parseSessionName,
                FxEditorEffectDef *edEffectDef);
bool __cdecl FX_ParseEditorEffect(const char **parse, FxEditorEffectDef *edEffectDef);
bool __cdecl FX_ParseEditorEffectField(const char **parse, FxEditorEffectDef *edEffectDef, const char *token);
bool __cdecl FX_LoadEditorEffect(const char *name, FxEditorEffectDef *edEffectDef);
const FxEffectDef *__cdecl FX_Load(const char *name);
const FxEffectDef *__cdecl FX_LoadFailed(const char *name);
const FxEffectDef *__cdecl FX_Register(char *name);
inline const FxEffectDef *__cdecl FX_Register(const char *name)
{
    return FX_Register((char *)name);
}
const FxEffectDef *__cdecl FX_Register_FastFile(const char *name);
const FxEffectDef *__cdecl FX_Register_LoadObj(char *name);
int __cdecl FX_GetHashIndex(char *name, bool *exists);
int __cdecl FX_HashName(char *name);
void __cdecl FX_RegisterDefaultEffect();
void *__cdecl Hunk_AllocPhysPresetPrecache(unsigned int size);
void *__cdecl Hunk_AllocPhysPresetPrecache(int size);
