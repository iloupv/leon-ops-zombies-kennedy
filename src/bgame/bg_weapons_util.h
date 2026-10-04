#pragma once

struct WeaponComponentList;

void __cdecl BG_WeaponComponentListSort(const char *originalName, WeaponComponentList *componentList, char *outputName);
void __cdecl BG_WeaponNameToComponentList(const char *name, WeaponComponentList *componentList);
void __cdecl BG_WeaponComponentListIdentify(WeaponComponentList *componentList);
int __cdecl BG_WeaponComponentListCountAttachments(const WeaponComponentList *componentList);
void __cdecl BG_WeaponComponentListRemove(
                const WeaponComponentList *componentListIn,
                WeaponComponentList *componentListOut,
                bool lookingForAttachements);
void __cdecl BG_WeaponComponentListRemoveAllAttachments(
                const WeaponComponentList *componentListIn,
                WeaponComponentList *componentListOut);
void __cdecl BG_WeaponComponentListNthAttachment(
                const WeaponComponentList *componentListIn,
                int nth,
                WeaponComponentList *componentListOut);
void __cdecl BG_WeaponComponentListToName(const WeaponComponentList *componentList, char *name, int size);
