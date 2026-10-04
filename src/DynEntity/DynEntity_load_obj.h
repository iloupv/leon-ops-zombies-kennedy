#pragma once

#include "DynEntity_gamestate.h"
#include "DynEntity_client.h"
#include <universal/q_parse.h>
#include <physics/physconstraints_load_obj.h>

struct DynEntityConstraintCreateParams // sizeof=0x18C
{                                       // XREF: ?DynEnt_LoadEntities@@YAXXZ/r
    char classname[64];
    char targetname[64];
    char target[64];
    char target2[64];
    float origin[3];
    float origin2[3];
    float angles[3];
    int health;
    float minAngle;
    float maxAngle;
    float damp;
    float distance;
    float scale;
    float force_scale;
    float width;
    int spawnflags;
    float timeout;
    char materialname[64];
};

struct __declspec(align(4)) DynEntityCreateParams // sizeof=0x2BC
{
    char typeName[64];
    char modelName[64];
    char physModelName[64];
    char destroyFxFile[64];
    char destroySound[64];
    char destroyPiecesFile[64];
    char physPresetFile[64];
    char targetname[64];
    char target[64];
    char destroyedModelName[64];
    float origin[3];
    float angles[3];
    int health;
    float mass;
    float bounce;
    float friction;
    float centerOfMass[3];
    bool startInPhysics;
    bool autoActivate;
    bool noBulletDamage;
    bool noExplosiveDamage;
    bool alwaysFloats;
    // padding byte
    // padding byte
    // padding byte
};

struct DynEntityProps // sizeof=0x8
{                                       // XREF: .rdata:dynEntProps/r
    const char *name;                   // XREF: DynEnt_GetType+4B/r
    bool clientOnly;                    // XREF: DynEnt_CompareEntities+102/r
                                        // DynEnt_CompareEntities+118/r
    bool clipMove;
    bool usePhysics;
    bool destroyable;
};

struct DynEntityServer // sizeof=0x8
{
    unsigned __int16 flags;
    // padding byte
    // padding byte
    int health;
};

struct node_params_t // sizeof=0x120
{                                       // XREF: .data:node_params_t * dents_params/r
                                        // .data:node_params_t * rope_params/r ...
    char classname[64];
    char targetname[64];                // XREF: setup_gjkcc_input(pmove_t *,gjkcc_input_t *)+37/o
                                        // PM_trace(pmove_t *,trace_t *,float const * const,float const * const,float const * const,float const * const,int,int)+4C/o ...
    unsigned __int16 targetname_hash;   // XREF: LoadRopeParams+2FA/w
                                        // GetAttachPointsInWorldSpace+8B/r ...
    char target[64];                    // XREF: LoadRopeParams+327/o
                                        // .data:00E0AED2/o
    char target2[64];                   // XREF: LoadRopeParams+3A5/o
                                        // R_ChangeState_1(GfxCmdBufState *,uint)+2E1/o
    // padding byte
    // padding byte
    float origin[3];                    // XREF: DynEnt_LoadEntities(void)+F2A/o
                                        // LoadRopeParams+1F9/o ...
    float angles[3];                    // XREF: DynEnt_LoadEntities(void)+F18/o
                                        // LoadRopeParams+23C/o ...
    int used;
};

struct DynEntityDef;
struct DynEntityClient;
struct PhysPreset;
struct XModelPieces;
struct XModel;
struct DynEntityCreateParams;
struct DynEntityPose;

DynEntityClient *__cdecl DynEnt_GetClientEntity(unsigned __int16 dynEntId, DynEntityDrawType drawType);
PhysPreset *__cdecl DynEnt_PhysPresetPrecache(const char *name);
unsigned __int8 *__cdecl DynEnt_AllocPhysPreset(int size);
XModelPieces *__cdecl DynEnt_XModelPiecesPrecache(const char *name);
unsigned __int8 *__cdecl DynEnt_AllocXModelPieces(int size);
void __cdecl DynEnt_LoadEntities();
char __cdecl DynEnt_IsValidClassName(const char *className);
void __cdecl DynEnt_GetEntityCountFromString(const char *entityString, int *entCount, int *constraintCount);
char __cdecl DynEnt_IsValidConstraintName(const char *className);
parseInfo_t *LoadRopeParams();
void __cdecl DynEnt_GetEntityConstraintParams(
                DynEntityConstraintCreateParams *constraintParams,
                PhysConstraint *constraints,
                int dynEntConstraintCount);
char __cdecl DynEnt_CreateConstraint(PhysConstraint *constraint, DynEntityConstraintCreateParams *params);
bool __cdecl FindConstraints(
                const char *targetname,
                const float *origin,
                const float *angles,
                AttachPointType attach_type,
                DynEntityConstraintCreateParams *constraintParams,
                int numConstraints,
                int *outConstraints);
char __cdecl DynEnt_Create(
                DynEntityDef *dynEntDef,
                const DynEntityCreateParams *params,
                DynEntityConstraintCreateParams *constraintParams,
                int numConstraints);
int __cdecl DynEnt_GetType(const char *typeName);
XModel *__cdecl DynEnt_XModelPrecache(char *modelName);
unsigned __int8 *__cdecl DynEnt_AllocXModel(int size);
unsigned __int8 *__cdecl DynEnt_AllocXModelColl(int size);
PhysPreset *__cdecl DynEnt_AutoCreatePhysPreset(const DynEntityDef *dynEntDef, const DynEntityCreateParams *params);
void __cdecl DynEnt_Strcpy(char **memberString, const char *keyValue);
int __cdecl DynEnt_CompareEntities(unsigned int *arg0, unsigned int *arg1);
unsigned __int8 *__cdecl DynEnt_Alloc(int count, int size);
void __cdecl GetAttachPointsInWorldSpace(PhysConstraint *constraint, float *p0, float *p1);
const DynEntityProps *__cdecl DynEnt_GetEntityProps(DynEntityType dynEntType);
unsigned __int16 __cdecl DynEnt_GetId(const DynEntityDef *dynEntDef, DynEntityDrawType drawType);
unsigned __int16 __cdecl DynEnt_GetAbsId(const DynEntityDef *dynEntDef);
bool __cdecl DynEnt_Valid(unsigned __int16 absDynEntId);
unsigned __int16 __cdecl DynEnt_GetEntityCount(DynEntityCollType collType);
const DynEntityDef *__cdecl DynEnt_GetEntityDef(unsigned __int16 absDynEntId);
const DynEntityPose *__cdecl DynEnt_GetClientPose(unsigned __int16 absDynEntId);
DynEntityPose *__cdecl DynEnt_GetClientModelPoseList();
DynEntityPose *__cdecl DynEnt_GetClientPose(unsigned __int16 dynEntId, DynEntityDrawType drawType);
DynEntityServer *__cdecl DynEnt_GetServerEntity(unsigned __int16 dynEntId, DynEntityDrawType drawType);
void __cdecl DynEnt_GetClientIdDrawType(unsigned __int16 absId, unsigned __int16 *outId, DynEntityDrawType *drawType);
DynEntityClient *__cdecl DynEnt_GetClientEntity(unsigned __int16 id);
unsigned __int16 __cdecl DynEnt_GetClientAbsId(unsigned __int16 dynEntId, DynEntityDrawType drawType);
int __cdecl DynEnt_GetXModelUsageCount(const XModel *xModel);
