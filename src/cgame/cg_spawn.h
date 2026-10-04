#pragma once

enum fieldtype_t : __int32;

struct cent_field_t // sizeof=0xC
{                                       // XREF: .rdata:client_spawn_fields/r
    const char *name;
    int ofs;
    fieldtype_t type;
};

struct centity_s;
struct fake_centity_s;
struct SpawnVar;

int __cdecl CG_SpawnFloat(const char *key, const char *defaultString, float *out);
const char *__cdecl CG_GetEntityTypeName(centity_s *cent);
void __cdecl CG_PrintEntities(unsigned int localClientNum);
centity_s *__cdecl CG_Spawn(unsigned int localClientNum);
void __cdecl CG_Free(int localClientNum, int entNum);
void __cdecl CG_ClearFakeEntInUse(int localClientNum, fake_centity_s *ent);
void __cdecl CG_ShutdownFakeEntities(int localClientNum);
void __cdecl CG_InitFakeEntities(int localClientNum, bool spawnEntsFromMap);
int __cdecl CG_GetModelIndex(const char *name, int localClientNum);
void __cdecl CScr_FreeFields(const cent_field_t *fields, unsigned __int8 *base);
void __cdecl CScr_FreeEntityFields(centity_s *cent);
void __cdecl CScr_SetDynamicEntityField(unsigned __int16 clientNum, centity_s *ent, unsigned int index);
void __cdecl CG_InitScriptMover(centity_s *cent);
// local variable allocation has failed, the output may be wrong!
char __cdecl CG_SetTriggerBrushModel(int localClientNum, centity_s *ent);
char __cdecl CG_InitTrigger(int localClientNum, centity_s *self);
void __cdecl CG_InitSentientTrigger(centity_s *self, char spawnflags);
void __cdecl CG_SP_trigger_multiple(int localClientNum, centity_s *ent, char spawnFlags);
void __cdecl CG_InitTriggerWait(int localClientNum, centity_s *ent, int *spawnflags, int waitSpawnFlag);
void __cdecl CG_SP_trigger_once(int localClientNum, centity_s *ent, char spawnFlags);
void __cdecl CG_SP_trigger_radius(int localClientNum, centity_s *ent, char spawnFlags);
void __cdecl CG_CallSpawn(int localClientNum, SpawnVar *spawnVar);
void __cdecl CG_ParseEntityFields(int localClientNum, centity_s *ent, SpawnVar *spawnVar, int ignoreModel);
void __cdecl CG_ParseEntityField(int localClientNum, const char *key, char *value, centity_s *ent, int ignoreModel);
void __cdecl CG_SetEntityScriptVariable(unsigned __int16 clientNum, const char *key, char *value, centity_s *ent);
void __cdecl CG_SpawnEntitiesFromString(int localClientNum);
