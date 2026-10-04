#pragma once
#include <gfx_d3d/fxprimitives.h>
#include "fx_load_obj.h"
#include "FxCurve.h"

enum FxSampleChannel : __int32
{                                       // XREF: ?FX_Convert@@YAPBUFxEffectDef@@PBUFxEditorEffectDef@@P6APAXH@Z@Z/r
                                        // FX_SampleVisualState/r ...
    FX_CHAN_RGBA     = 0x0,
    FX_CHAN_SIZE_0   = 0x1,
    FX_CHAN_SIZE_1   = 0x2,
    FX_CHAN_SCALE    = 0x3,
    FX_CHAN_ROTATION = 0x4,
    FX_CHAN_COUNT    = 0x5,
    FX_CHAN_NONE     = 0x6,
};

const FxEffectDef *__cdecl FX_Convert(const FxEditorEffectDef *editorEffect, void *(__cdecl *Alloc)(int));
int __cdecl FX_DecideVelocitySampleCount(const FxEditorElemDef *edElem, int intervalLimit);
int __cdecl FX_DecideSampleCount(int curveCount, const FxCurve **curves, int intervalLimit);
double __cdecl FX_MaxErrorForIntervalCount(
                int dimensions,
                int sampleCount,
                const float *samples,
                int intervalCount,
                float errorCutoff);
void __cdecl FX_InterpolateSamples(
                int dimensions,
                float time0,
                const float *samples0,
                float time1,
                const float *samples1,
                float timeEval,
                float *result);
void __cdecl FX_GetVisualSampleRouting(const FxEditorElemDef *edElem, FxSampleChannel *routing);
int __cdecl FX_DecideVisualSampleCount(
                const FxEditorElemDef *edElem,
                const FxSampleChannel *routing,
                int intervalLimit);
int __cdecl FX_DecideIntervalLimit(const FxEditorElemDef *edElemDef);
int __cdecl FX_AdditionalBytesNeededForGeomTrail(const FxEditorElemDef *elemDef);
int __cdecl FX_FindEmission(const FxEffectDef *emission, const FxEditorEffectDef *editorEffect);
int __cdecl FX_ConvertElemDefsOfType(
                FxElemDef *elemDefArray,
                const FxEditorEffectDef *editorEffect,
                unsigned int loopingFlagState,
                const int *velStateCount,
                const int *visStateCount,
                const int *emitIndex,
                unsigned __int8 **memPool);
void __cdecl FX_ConvertElemDef(
                FxElemDef *elemDef,
                const FxEditorElemDef *edElemDef,
                int velStateCount,
                int visStateCount,
                int emitIndex,
                unsigned __int8 **memPool);
void __cdecl FX_SampleVelocity(FxElemDef *elemDef, const FxEditorElemDef *edElemDef);
void __cdecl FX_SampleVelocityInFrame(
                FxElemDef *elemDef,
                const float (*velScale)[3],
                FxElemVelStateInFrame *velState,
                int velStateStride,
                int useGraphBit,
                const FxEditorElemDef *edElemDef);
double __cdecl FX_SampleCurve1D(const FxCurve *curve, float scale, float time);
double __cdecl FxCurveIterator_SampleTime(FxCurveIterator *source, float time);
double __cdecl FxCurve_Interpolate1d(float *key, float intermediateTime);
void __cdecl FxCurveIterator_MoveToTime(FxCurveIterator *source, float time);
void __cdecl FX_SampleVisualState(FxElemDef *elemDef, const FxEditorElemDef *edElemDef);
void __cdecl Byte4PackVertexColor(const float *from, unsigned __int8 *to);
void __cdecl FX_SampleCurve3D(const FxCurve *curve, float scale, float time, float *value);
void __cdecl FxCurveIterator_SampleTimeVec3(FxCurveIterator *source, float *replyVector, float time);
void __cdecl FxCurve_Interpolate3d(const float *key, float intermediateTime, float *result);
void __cdecl FX_SampleVisualStateScalar(
                const FxEditorElemDef *edElemDef,
                float sampleTime,
                FxSampleChannel routing,
                float scaleFactor,
                float *base,
                float *amplitude);
void __cdecl FX_ConvertAtlas(FxElemDef *elemDef, const FxEditorElemDef *edElemDef);
bool __cdecl FX_ElemUsesMaterial(const FxEditorElemDef *edElemDef);
void __cdecl FX_ScaleFloatRange(FxFloatRange *to, const FxFloatRange *from, float scale);
void __cdecl FX_ReserveElemDefMemory(FxElemDef *elemDef, unsigned __int8 **memPool);
void __cdecl FX_CopyMarkVisuals(const FxEditorElemDef *edElemDef, FxElemMarkVisuals *markVisualsArray);
void __cdecl FX_CopyVisuals(const FxEditorElemDef *edElemDef, FxElemVisuals *visualsArray);
void __cdecl FX_ConvertEffectDefRef(FxEffectDefRef *ref, const FxEffectDef *effectDef);
void __cdecl FX_BoundFloatRange(FxFloatRange *range, float lower, float upper);
void __cdecl FX_ConvertTrail(FxTrailDef **outTrailDef, const FxEditorElemDef *edElemDef, unsigned __int8 **mempool);
void __cdecl FX_ConvertTrail_CompileVertices(
                const FxEditorElemDef *edElemDef,
                FxTrailDef *outTrailDef,
                unsigned __int8 **mempool);
void __cdecl FX_ConvertTrail_CalcNormForSegment(const float *vert0, const float *vert1, float *outNormal);
void __cdecl FX_ConvertElemSpawnSound(FxElemSpawnSound *dest, const FxElemSpawnSound *src, unsigned __int8 **memPool);
int __cdecl FX_CopyEmittedElemDefs(
                FxElemDef *elemDefArray,
                const FxEditorEffectDef *editorEffect,
                unsigned __int8 **memPool);
int __cdecl FX_AdditionalBytesNeededForElemDef(
                unsigned __int8 elemType,
                int velStateSampleCount,
                int visStateSampleCount,
                int visualCount,
                const FxElemSpawnSound *elemSpawnSound);
int __cdecl FX_AdditionalBytesNeededForEmission(const FxEffectDef *emission);
int __cdecl FX_GetLoopingLife(const FxEffectDef *effectDef);
bool __cdecl FX_Validate(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef);
char __cdecl FX_ValidateAtlasSettings(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef);
char __cdecl FX_ValidateFlags(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef);
char __cdecl FX_ValidateColor(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef);
char __cdecl FX_ValidateVisuals(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef);
char __cdecl FX_ValidatePhysics(const FxEditorEffectDef *editorEffect, const FxEditorElemDef *edElemDef);
