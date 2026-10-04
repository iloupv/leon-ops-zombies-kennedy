#pragma once

struct DObj;
struct DObjAnimMat;
struct XBoneInfo;
struct XModel;

DObjAnimMat *__cdecl DObjGetRotTransArray(const DObj *obj);
int __cdecl DObjGetNumModels(const DObj *obj);
unsigned __int16 __cdecl DObjGetEntNum(const DObj *obj);
int __cdecl DObjGetSurfaces(const DObj *obj, int *partBits, const char *lods);
void __cdecl DObjGetSurfaceData(const DObj *obj, const float *origin, float scale, bool offscreen, char *lods);
void __cdecl DObjGetBoneInfo(const DObj *obj, XBoneInfo **boneInfo);
int __cdecl DObjNumBones(const DObj *obj);
int __cdecl DObjGetLodForDist(const DObj *obj, int modelIndex, float dist, float base_dist);
void __cdecl DObjGetSetBones(const DObj *obj, int *setPartBits);
unsigned int __cdecl DObjGetRootBoneCount(const DObj *obj);
int __cdecl DObjSetRotTransIndex(const DObj *obj, const int *partBits, int boneIndex);
char __cdecl DObjSetSkelRotTransIndex(const DObj *obj, const int *partBits, int boneIndex);
void __cdecl DObjSetControlTagAngles(const DObj *obj, int *partBits, unsigned int boneIndex, float *angles);
XModel *__cdecl DObjGetModel(const DObj *obj, int modelIndex);
int __cdecl DObjGetModelBoneIndex(const DObj *obj, int model_index, unsigned int name, unsigned __int8 *index);
void __cdecl DObjSetLocalTag(
                const DObj *obj,
                int *partBits,
                unsigned int boneIndex,
                const float *trans,
                const float *angles);
void __cdecl DObjSetLocalTagInternal(const DObj *obj, const float *trans, const float *angles, int boneIndex);
void __cdecl DObjSetAngles(DObjAnimMat *rotTrans, const float *angles);
void __cdecl DObjClearAngles(DObjAnimMat *rotTrans);
void __cdecl DObjSetTrans(DObjAnimMat *rotTrans, const float *trans);
void __cdecl DObjCompleteHierarchyBits(DObj *obj, int *partBits);
int __cdecl DObjSetControlRotTransIndex(const DObj *obj, const int *partBits, int boneIndex);
bool __cdecl DObjSkelExists(const DObj *obj, int timeStamp);
void __cdecl DObjClearSkel(const DObj *obj);
int __cdecl DObjSkelAreBonesUpToDate(const DObj *obj, int *partBits);
void __cdecl DObjSkelSetSkel(const DObj *obj, int *partBits);
int __cdecl DObjGetAllocSkelSize(const DObj *obj);
void __cdecl DObjCreateSkel(DObj *obj, char *buf, int timeStamp);
DObjAnimMat *__cdecl I_dmaGetDObjSkel(const DObj *obj);
void __cdecl DObjGetHidePartBits(const DObj *obj, unsigned int *partBits);
int __cdecl DObjGetBoneIndex(const DObj *obj, unsigned int name, unsigned __int8 *index, int modelNum);
int __cdecl DObjGetChildBones(const DObj *obj, unsigned __int8 parentBone, unsigned __int8 *children, int maxChildren);
void __cdecl DObjLock(const DObj *obj);
void __cdecl DObjUnlock(const DObj *obj);
bool __cdecl DObjIsRecorded(const DObj *obj);
