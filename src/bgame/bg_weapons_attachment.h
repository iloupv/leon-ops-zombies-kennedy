#pragma once
#include <universal/com_stringtable.h>

enum eAttachment : __int32
{                                       // XREF: WeaponComponent/r
                                        // AttachmentTableEntry/r ...
    ATTACHMENT_NONE         = 0x0,
    ATTACHMENT_REFLEX       = 0x1,
    ATTACHMENT_ELBIT        = 0x2,
    ATTACHMENT_ACOG         = 0x3,
    ATTACHMENT_LPS          = 0x4,
    ATTACHMENT_VZOOM        = 0x5,
    ATTACHMENT_IR           = 0x6,
    ATTACHMENT_GL           = 0x7,
    ATTACHMENT_MK           = 0x8,
    ATTACHMENT_SILENCER     = 0x9,
    ATTACHMENT_GRIP         = 0xA,
    ATTACHMENT_EXTCLIP      = 0xB,
    ATTACHMENT_DUALCLIP     = 0xC,
    ATTACHMENT_SADDLE       = 0xD,
    ATTACHMENT_FMJ          = 0xE,
    ATTACHMENT_HP           = 0xF,
    ATTACHMENT_RF           = 0x10,
    ATTACHMENT_FT           = 0x11,
    ATTACHMENT_BAYONET      = 0x12,
    ATTACHMENT_AUTO         = 0x13,
    ATTACHMENT_SPEED        = 0x14,
    ATTACHMENT_UPGRADESIGHT = 0x15,
    ATTACHMENT_SNUB         = 0x16,
    ATTACHMENT_LAST_REAL    = 0x16,
    ATTACHMENT_DW           = 0x17,
    ATTACHMENT_COUNT        = 0x18,
    ATTACHMENT_INVALID      = 0x18,
};

enum eAttachmentPoint : __int32
{                                       // XREF: WeaponComponent/r
                                        // AttachmentTableEntry/r ...
    ATTACHMENT_POINT_NONE = 0x0,
    ATTACHMENT_POINT_TOP = 0x1,
    ATTACHMENT_POINT_FIRST = 0x1,
    ATTACHMENT_POINT_BOTTOM = 0x2,
    ATTACHMENT_POINT_TRIGGER = 0x3,
    ATTACHMENT_POINT_MUZZLE = 0x4,
    ATTACHMENT_POINT_COUNT = 0x5,
    ATTACHMENT_POINT_INVALID = 0x5,
};
inline eAttachmentPoint &operator++(eAttachmentPoint &t)
{
    t = static_cast<eAttachmentPoint>((static_cast<int>(t) + 1));
    return t;
}
inline eAttachmentPoint operator++(eAttachmentPoint &t, int)
{
    eAttachmentPoint old = t;
    t = static_cast<eAttachmentPoint>((static_cast<int>(t) + 1));
    return old;
}


enum eWeaponOptionGroup : __int32
{                                       // XREF: WeaponOptionTableEntry/r
                                        // GetWeaponOptionGroupIndex/r ...
    WEAPONOPTION_GROUP_FIRST         = 0x0,
    WEAPONOPTION_GROUP_CAMO          = 0x0,
    WEAPONOPTION_GROUP_RETICLE       = 0x1,
    WEAPONOPTION_GROUP_LENS          = 0x2,
    WEAPONOPTION_GROUP_TAG           = 0x3,
    WEAPONOPTION_GROUP_EMBLEM        = 0x4,
    WEAPONOPTION_GROUP_RETICLE_COLOR = 0x5,
    WEAPONOPTION_GROUP_COUNT         = 0x6,
    WEAPONOPTION_GROUP_INVALID       = 0x6,
};

struct WeaponAttachmentTableEntry // sizeof=0x24
{                                       // XREF: AttachmentTableEntry/r
                                        // WeaponOptionTableEntry/r ...
    const char *displayName;            // XREF: BG_GetAttachmentDisplayName(eAttachment)+38/r
                                        // BG_GetWeaponOptionDisplayName(int)+38/r
    const char *referenceName;          // XREF: BG_GetAttachmentReference(eAttachment)+38/r
                                        // BG_GetWeaponOptionReference(int)+38/r ...
    const char *pointName;
    const char *imageName;              // XREF: BG_GetAttachmentImage(eAttachment)+38/r
                                        // BG_GetWeaponOptionImage(int)+38/r
    const char *description;            // XREF: BG_GetAttachmentDesc(eAttachment)+38/r
    int cost;                           // XREF: BG_GetAttachmentCost(eAttachment)+38/r
                                        // BG_GetAttachmentPointCost(eAttachmentPoint)+28/r ...
    int count;
    int unlockLvl;                      // XREF: BG_GetWeaponOptionUnlockLvl(int)+48/r
    int unlockPLevel;                   // XREF: BG_GetWeaponOptionUnlockPLevel(int)+48/r
};

struct __declspec(align(4)) AttachmentTableEntry // sizeof=0x48
{                                       // XREF: .data:s_weaponTableAttachments/r
    eAttachment attachmentIndex;        // XREF: BG_GetAttachmentArrayIndex+3A/r
                                        // BG_GetAttachmentPointIndexFromAttachment(eAttachment)+3A/r ...
    eAttachmentPoint attachmentPointIndex;
                                        // XREF: BG_GetAttachmentPointIndexFromAttachment(eAttachment)+4B/r
    WeaponAttachmentTableEntry values;  // XREF: BG_GetAttachmentDisplayName(eAttachment)+38/r
                                        // BG_GetAttachmentReference(eAttachment)+38/r ...
    bool compatibleAttachments[24];     // XREF: BG_AreAttachmentsCompatible(eAttachment,eAttachment)+A0/r
                                        // BG_AreAttachmentsCompatible(eAttachment,eAttachment)+B5/r
    bool isValid;                       // XREF: BG_GetAttachmentDisplayName(eAttachment)+23/r
                                        // BG_GetAttachmentArrayIndex+27/r ...
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) AttachmentPointTableEntry // sizeof=0x2C
{                                       // XREF: .data:s_weaponTableAttachmentPoints/r
    eAttachmentPoint attachmentPointIndex;
                                        // XREF: BG_LoadWeaponAttachmentTable(void)+1B0/r
    WeaponAttachmentTableEntry values;  // XREF: BG_GetAttachmentPointCost(eAttachmentPoint)+28/r
    bool isValid;                       // XREF: BG_GetAttachmentPointCost(eAttachmentPoint)+13/r
    // padding byte
    // padding byte
    // padding byte
};

struct __declspec(align(4)) WeaponOptionTableEntry // sizeof=0x30
{                                       // XREF: .data:s_weaponTableWeaponOptions/r
    int weaponOptionIndex;              // XREF: BG_GetWeaponOptionArrayIndex+49/r
                                        // BG_GetWeaponOptionFromName(char const *)+57/r
    eWeaponOptionGroup weaponOptionGroup;
                                        // XREF: BG_GetWeaponOptionGroup(int)+3E/r
    WeaponAttachmentTableEntry values;  // XREF: BG_GetWeaponOptionDisplayName(int)+38/r
                                        // BG_GetWeaponOptionReference(int)+38/r ...
    bool isValid;                       // XREF: BG_GetWeaponOptionDisplayName(int)+23/r
                                        // BG_GetWeaponOptionArrayIndex+36/r ...
    // padding byte
    // padding byte
    // padding byte
};

const char **__cdecl BG_GetAttachmentNames();
const char *__cdecl BG_GetAttachmentName(eAttachment index);
eAttachment __cdecl BG_GetAttachmentIndex(const char *name);
const char *__cdecl BG_GetAttachmentPointName(eAttachmentPoint index);
eAttachmentPoint __cdecl BG_GetAttachmentPointIndex(const char *name);
int __cdecl BG_GetWeaponOptionGroup(const char *name);
const char *__cdecl BG_GetAttachmentDisplayName(eAttachment attachment);
int __cdecl BG_GetAttachmentArrayIndex(eAttachment attachment);
const char *__cdecl BG_GetAttachmentReference(eAttachment attachment);
const char *__cdecl BG_GetAttachmentImage(eAttachment attachment);
const char *__cdecl BG_GetAttachmentDesc(eAttachment attachment);
int __cdecl BG_GetAttachmentCost(eAttachment attachment);
int __cdecl BG_GetAttachmentPointCost(eAttachmentPoint attachmentPoint);
eAttachmentPoint __cdecl BG_GetAttachmentPointIndexFromAttachment(eAttachment attachment);
bool __cdecl BG_AreAttachmentsCompatible(eAttachment attachment1, eAttachment attachment2);
const AttachmentTableEntry *__cdecl BG_GetAttachmentTableEntry(unsigned int index);
const AttachmentPointTableEntry *__cdecl BG_GetAttachmentPointTableEntry(unsigned int index);
int __cdecl BG_GetWeaponAttachmentCount();
int __cdecl BG_GetWeaponOptionCountForGroup(eWeaponOptionGroup optionGroup);
int __cdecl BG_GetWeaponOptionCount();
const char *__cdecl BG_GetWeaponOptionDisplayName(int weaponOption);
int __cdecl BG_GetWeaponOptionArrayIndex(int weaponOption);
const char *__cdecl BG_GetWeaponOptionReference(int weaponOption);
const char *__cdecl BG_GetWeaponOptionImage(int weaponOption);
int __cdecl BG_GetWeaponOptionCost(int weaponOption);
eWeaponOptionGroup __cdecl BG_GetWeaponOptionGroup(int weaponOption);
int __cdecl BG_GetWeaponOptionUnlockLvl(int weaponOption);
int __cdecl BG_GetWeaponOptionUnlockPLevel(int weaponOption);
int __cdecl BG_GetWeaponOptionNumFromIndexAndGroup(int index, eWeaponOptionGroup weaponOptionGroup);
int __cdecl BG_GetWeaponOptionFromName(const char *optionName);
void __cdecl BG_LoadWeaponAttachmentTable();
void __cdecl BG_LoadAttachmentRow(const StringTable *attachmentTable, int row, AttachmentTableEntry *entry, int *count);
void __cdecl BG_LoadWeaponAttachmentTableRow(
                const StringTable *attachmentTable,
                int row,
                WeaponAttachmentTableEntry *entry);
void __cdecl BG_LoadAttachmentPointRow(
                const StringTable *attachmentTable,
                int row,
                AttachmentPointTableEntry *entry,
                int *count);
void __cdecl BG_LoadWeaponOptions(const StringTable *attachmentTable);
void __cdecl BG_LoadWeaponOptionRow(
                const StringTable *attachmentTable,
                int row,
                WeaponOptionTableEntry *entry,
                int *count);
