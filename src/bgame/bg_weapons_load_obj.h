#pragma once

#include "bg_weapons.h"
#include "bg_weapons_attachment.h"

struct flameTable;
struct WeaponVariantDef;

enum eWeaponComponentType : __int32
{                                       // XREF: WeaponComponent/r
    WEAPON_COMPONENT_UNKNOWN    = 0x0,
    WEAPON_COMPONENT_ATTACHMENT = 0x1,
    WEAPON_COMPONENT_BASE       = 0x2,
};

struct WeaponFullDef // sizeof=0xC48
{                                       // XREF: .data:bg_defaultWeaponFullDefs/r
    WeaponVariantDef weapVariantDef;
    WeaponDef weapDef;                  // XREF: BG_WeaponUpdateField(char const *,char const *)+58/o
                                        // BG_WeaponUpdateField(char const *,char const *)+AC/o
    XModel *gunXModel[16];
    const char *szXAnims[66];
    unsigned __int16 hideTags[32];
    unsigned __int16 notetrackSoundMapKeys[20];
    unsigned __int16 notetrackSoundMapValues[20];
    XModel *worldModel[16];
    float parallelBounce[31];
    float perpendicularBounce[31];
    float locationDamageMultipliers[19];
};

struct WeaponComponent // sizeof=0x2C
{                                       // XREF: WeaponComponentList/r
                                        // ??$_Insertion_sort1@PAUWeaponComponent@@Vcompare_weapon_component_sort@@U1@@std@@YAXPAUWeaponComponent@@0Vcompare_weapon_component_sort@@0@Z/r ...
    char name[32];                      // XREF: BG_MergeWeaponDefClipName(char * * const,char *,int)+D4/o
                                        // BG_MergeWeaponDefClipName(char * * const,char *,int)+F5/o ...
    eWeaponComponentType type;          // XREF: std::_Insertion_sort1<WeaponComponent *,compare_weapon_component_sort,WeaponComponent>(WeaponComponent *,WeaponComponent *,compare_weapon_component_sort,WeaponComponent *)+4F/r
                                        // std::_Insertion_sort1<WeaponComponent *,compare_weapon_component_sort,WeaponComponent>(WeaponComponent *,WeaponComponent *,compare_weapon_component_sort,WeaponComponent *)+5A/r ...
    eAttachment attachment;             // XREF: WeaponComponentListAddAllPoints(WeaponComponentList *)+92/w
                                        // PlayerCmd_giveWeaponNextAttachmentDEVONLY(scr_entref_t)+1BB/r ...
    eAttachmentPoint attachmentPoint;   // XREF: std::_Insertion_sort1<WeaponComponent *,compare_weapon_component_sort,WeaponComponent>(WeaponComponent *,WeaponComponent *,compare_weapon_component_sort,WeaponComponent *)+79/r
                                        // std::_Insertion_sort1<WeaponComponent *,compare_weapon_component_sort,WeaponComponent>(WeaponComponent *,WeaponComponent *,compare_weapon_component_sort,WeaponComponent *)+FF/r ...
};

struct WeaponComponentList // sizeof=0x138
{                                       // XREF: BG_MergeWeaponDefClipName(char * * const,char *,int)+AE/r
                                        // ?WeaponComponentListAddAllPoints@@YAXPAUWeaponComponentList@@@Z/r ...
    int numComponents;                  // XREF: BG_MergeWeaponDefClipName(char * * const,char *,int)+9C/r
                                        // BG_MergeWeaponDefClipName(char * * const,char *,int)+102/r ...
    WeaponComponent components[7];      // XREF: BG_MergeWeaponDefClipName(char * * const,char *,int)+C3/o
                                        // BG_MergeWeaponDefClipName(char * * const,char *,int)+D4/o ...
};

struct SurfaceTypeSoundList // sizeof=0x8
{                                       // XREF: .data:SurfaceTypeSoundList * surfaceTypeSoundLists/r
    char *surfaceSoundBase;             // XREF: BG_RegisterSurfaceTypeSounds+74/r
                                        // BG_RegisterSurfaceTypeSounds+25E/w ...
    char **soundAliasList;              // XREF: BG_RegisterSurfaceTypeSounds+8B/r
                                        // BG_RegisterSurfaceTypeSounds+2DC/w
};

char *__cdecl BG_GetPlayerAnimTypeName(int index);
int __cdecl BG_GetPlayerAnimTypeIndex(int typeEnum);
const char *__cdecl BG_GetWeaponTypeName(weapType_t type);
const char *__cdecl BG_GetWeaponClassName(weapClass_t type);
const char *__cdecl BG_GetWeaponInventoryTypeName(weapInventoryType_t type);
void __cdecl BG_LoadWeaponStrings();
void __cdecl BG_LoadPlayerAnimTypes();
void __cdecl BG_ClearWeaponDefInternal();
void __cdecl SetConfigString(char **ppszConfigString, const char *pszKeyValue);
flameTable *__cdecl BG_LoadFlameTableInternal(const char *folder, char *name);
void __cdecl SetConfigString2(unsigned __int8 *pMember, const char *pszKeyValue);
void __cdecl InitFlameTable(flameTable *fTable);
flameTable *__cdecl BG_GetFlameTable(const char *folder, char *name);
char __cdecl BG_LoadWeaponFile(char *szFileName, char *szBuffer, int iBufferSize);
void __cdecl BG_LoadWeaponMergeSupport();
char __cdecl BG_SplitWeaponDefNames(
                const char *name,
                char **sources,
                WeaponComponentList *componentAll,
                char *outputName);
int __cdecl BG_MergeWeaponDefClipName(char **value, char *mergedValue, int size);
int __cdecl BG_MergeWeaponDefFloatValue(char **value, char *mergedValue);
int __cdecl BG_MergeWeaponDefHidetags(char **value, char *mergedValue, int size);
bool __cdecl BG_IsAnimationField(const char *fieldName);
bool __cdecl BG_IsADSField(const char *fieldName);
char __cdecl BG_IsGripAnimationName(const char *name);
int __cdecl BG_MergeWeaponDefAnimations(const char *fieldName, char **value, char *mergedValue, unsigned int size);
int __cdecl BG_MergeWeaponDefSpecialCases(const char *fieldName, char **value, char *mergedValue, int size);
char __cdecl BG_LoadWeaponVariantDefFile(WeaponFullDef *weapFullDef, const char *folder, char *name);
int __cdecl BG_ParseWeaponDefSpecificFieldType(WeaponFullDef *pStruct, const char *pValue, int iFieldType);
int __cdecl Weapon_GetStringArrayIndex(const char *value, const char **stringArray, int arraySize);
char **__cdecl BG_RegisterSurfaceTypeSounds(const char *surfaceSoundBase);
WeaponFullDef *__cdecl BG_LoadWeaponVariantDefInternal(const char *folder, char *name);
void __cdecl BG_SetupTransitionTimes(WeaponVariantDef *weapVariantDef);
void __cdecl BG_CheckWeaponDamageRanges(WeaponDef *weapDef);
void __cdecl BG_CheckProjectileValues(WeaponFullDef *weaponFullDef);
void __cdecl InitWeaponDef(WeaponFullDef *weapFullDef);
char __cdecl BG_WeaponUpdateField(const char *weaponName, char *keyValue);
bool __cdecl BG_FlameTableUpdateField(const char *flameTableName, char *keyValue);
WeaponVariantDef *__cdecl BG_LoadDefaultWeaponVariantDef();
WeaponFullDef *__cdecl BG_LoadDefaultWeaponVariantDef_LoadObj();
WeaponVariantDef *__cdecl BG_LoadDefaultWeaponVariantDef_FastFile();
