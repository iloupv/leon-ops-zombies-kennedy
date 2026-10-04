#pragma once
#include <qcommon/common.h>
#include <bgame/bg_public.h>
#include <game/g_items.h>
#include <game/g_hudelem.h>

struct SpawnFuncEntry // sizeof=0x8
{                                       // XREF: .rdata:s_bspOnlySpawns/r
                                        // .rdata:s_bspOrDynamicSpawns/r
    const char *classname;
    void (__cdecl *callback)(gentity_s *, SpawnVar *);
};

int __cdecl G_SpawnFloat(const SpawnVar *spawnVar, const char *key, const char *defaultString, float *out);
int __cdecl G_SpawnInt(const SpawnVar *spawnVar, const char *key, const char *defaultString, int *out);
void __cdecl Scr_ReadOnlyField(gentity_s *ent, int);
void __cdecl G_FreeEntityWrapper(gentity_s *ent, SpawnVar *v = NULL);
unsigned int __cdecl G_SetEntityScriptVariableInternal(const char *key, char *value);
// zombies: ignoreModel as SP's G_ParseEntityFields (SP 0x004219a0) / G_ParseEntityField (SP 0x00812770): 1 skips F_MODEL
void __cdecl G_ParseEntityFields(const SpawnVar *spawnVar, gentity_s *ent, int radiant_update, int ignoreModel = 0);
void __cdecl G_ParseEntityField(const char *key, char *value, gentity_s *ent, int radiant_update, int ignoreModel = 0);
void __cdecl G_SetEntityScriptVariable(const char *key, char *value, gentity_s *ent);
unsigned int __cdecl G_SpawnStruct(SpawnVar *spawnVar);
void __cdecl G_DuplicateEntityFields(gentity_s *dest, const gentity_s *source);
void __cdecl G_DuplicateScriptFields(gentity_s *dest, const gentity_s *source);
bool __cdecl IsClientOnlyEntity(const SpawnVar *spawnVar);
void __cdecl G_SetupDestructible(gentity_s *ent, char *destructibleName);
void __cdecl G_SetupDestructible(gentity_s *ent, SpawnVar *spawnVar);
void __cdecl G_CallSpawn(SpawnVar *spawnVar);
const gitem_s *__cdecl G_GetItemForClassname(const char *classname);
void (__cdecl *__cdecl G_FindSpawnFunc(
                const char *classname,
                const SpawnFuncEntry *spawnFuncArray,
                int spawnFuncCount))(gentity_s *, SpawnVar *);
int __cdecl G_CallSpawnEntity(gentity_s *ent);
void __cdecl GScr_AddFieldsForEntity();
void __cdecl GScr_AddFieldsForRadiant();
void __cdecl GScr_SetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs, unsigned int whichbits);
int __cdecl Scr_SetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset);
int __cdecl Scr_SetEntityField(unsigned int entnum, unsigned int offset);
void __cdecl Scr_GetEntityField(unsigned int entnum, unsigned int offset);
void __cdecl GScr_GetGenericField(unsigned __int8 *b, fieldtype_t type, int ofs, unsigned int whichbits);
void __cdecl Scr_GetObjectField(unsigned int classnum, unsigned int entnum, unsigned int offset);
void __cdecl Scr_FreeEntityConstStrings(gentity_s *pEnt);
void __cdecl Scr_FreeEntity(gentity_s *ent);
void __cdecl Scr_AddEntity(gentity_s *ent, scriptInstance_t inst);
gentity_s *__cdecl Scr_GetEntityAllowNull(unsigned int index, scriptInstance_t inst);
gentity_s *__cdecl Scr_GetEntity(unsigned int index);
void __cdecl Scr_FreeHudElem(game_hudelem_s *hud);
void __cdecl Scr_AddHudElem(game_hudelem_s *hud);
unsigned int __cdecl Scr_ExecEntThread(gentity_s *ent, int handle, unsigned int paramcount);
void __cdecl Scr_Notify(gentity_s *ent, unsigned __int16 stringValue, unsigned int paramcount);
void __cdecl Scr_GetEnt();
void __cdecl Scr_GetEntArray();
void __cdecl GScr_SetDynamicEntityField(gentity_s *ent, unsigned int index);
void __cdecl SP_worldspawn(SpawnVar *spawnVar);
void __cdecl G_SpawnEntitiesFromString();
void __cdecl G_LoadStructs();
