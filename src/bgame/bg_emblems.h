#pragma once
#include <qcommon/cmd.h>
#include <qcommon/msg_mp.h>
#include <bgame/bg_emblems.h>

struct CompositeEmblemLayer;
struct GfxImage;
struct Material;

struct BackgroundPurchasedCount // sizeof=0x8
{                                       // XREF: .data:s_backgroundPurchasedCount/r
    unsigned int flush;
    int count;
};

struct ResultCache // sizeof=0x818
{                                       // XREF: .data:s_resultCache/r
    int controllerIndex;
    unsigned int flush;
    unsigned int use;
    unsigned int include;
    unsigned int exclude;
    int count;
    __int16 ids[1024];
};

struct EmblemLayer // sizeof=0xC
{
    int cost;
    int unlockLevel;
    int unlockPLevel;
};

struct EmblemCategory // sizeof=0x8
{
    const char *name;
    const char *description;
};

struct EmblemIcon // sizeof=0x28
{
    GfxImage *image;
    const char *description;
    float outlineSize;
    int defaultColor;
    int cost;
    int unlockLevel;
    int unlockPLevel;
    int unclassifyAt;
    int sortKey;
    unsigned int category;
};

struct EmblemBackground // sizeof=0x18
{
    Material *material;
    const char *description;
    int cost;
    int unlockLevel;
    int unlockPLevel;
    int unclassifyAt;
};

struct EmblemSet // sizeof=0x2C
{                                       // XREF: XAssetPoolEntry<EmblemSet>/r
    int colorCount;
    int layerCount;
    EmblemLayer *layers;
    int categoryCount;
    EmblemCategory *categories;
    int iconCount;
    EmblemIcon *icons;
    int backgroundCount;
    EmblemBackground *backgrounds;
    int backgroundLookupCount;
    __int16 *backgroundLookup;
};

struct __declspec(align(4)) CompositeEmblemLayer // sizeof=0x20
{                                       // XREF: .data:s_emblem/r
                                        // PCachePlayerEmblem/r ...
    __int16 icon;                       // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+9D/w
    // UI_DrawEmblemIconThumbnail(int,rectDef_s const *,float const * const)+66/w ...
// padding byte
// padding byte
    int colorIdx;                       // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+96/w
    // UI_DrawEmblemIconThumbnail(int,rectDef_s const *,float const * const)+5A/w
    float pos[2];                       // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+48/w
    // UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+55/w ...
    float scale[2];                     // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+62/w
    // UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+6F/w ...
    float angle;                        // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+7C/w
    // UI_DrawEmblemIconThumbnail(int,rectDef_s const *,float const * const)+4D/w
    bool outline;                       // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+81/w
    bool flip;                          // XREF: UI_DrawEmblemSelector(int,int,itemDef_s *,rectDef_s const *,float const * const)+85/w
    // padding byte
    // padding byte
};

struct BGDefaultEmblem // sizeof=0x18C
{                                       // XREF: .data:s_defaultEmblems/r
    int minlevel;                       // XREF: BG_EmblemsClearDefaults(void)+38/w
                                        // BG_EmblemsSetDefaultForLevel(int,int,CompositeEmblemLayer *,int)+AF/r ...
    int maxlevel;                       // XREF: BG_EmblemsClearDefaults(void)+25/w
                                        // BG_EmblemsSetDefaultForLevel(int,int,CompositeEmblemLayer *,int)+C5/r ...
    CompositeEmblemLayer layers[12];    // XREF: BG_EmblemsSetDefaultForLevel(int,int,CompositeEmblemLayer *,int)+13E/o
                                        // BG_EmblemsGenerateDefault(int,int,CompositeEmblemLayer *,int)+B2/o
    int layerCount;                     // XREF: BG_EmblemsSetDefaultForLevel(int,int,CompositeEmblemLayer *,int)+124/w
                                        // BG_EmblemsGenerateDefault(int,int,CompositeEmblemLayer *,int)+7D/r
};

int __cdecl BG_EmblemsGetColorCount();
int __cdecl BG_EmblemsGetUnlockedLayerCount(int controllerIndex);
int __cdecl BG_EmblemsGetLayerCost(int layer);
int __cdecl BG_EmblemsGetLayerUnlockLevel(int layer);
int __cdecl BG_EmblemsGetPurchasedLayerCount(int controllerIndex);
char __cdecl BG_EmblemsPurchaseLayer(int controllerIndex);
void __cdecl BG_EmblemsFlushResults();
ResultCache *__cdecl BG_EmblemsGetResults(int controllerIndex, unsigned int include, unsigned int exclude);
int __cdecl ResultSort(__int16 *a, __int16 *b);
int __cdecl BG_EmblemsGetIconCount(int controllerIndex, unsigned int include, unsigned int exclude);
int __cdecl BG_EmblemsGetIconCount();
__int16 __cdecl BG_EmblemsGetIconID(int controllerIndex, int index, unsigned int include, unsigned int exclude);
int __cdecl BG_EmblemsGetPurchasedIconCount(int controllerIndex);
const char *__cdecl BG_EmblemsGetIconDesc(int controllerIndex, __int16 id);
const char *__cdecl BG_EmblemsGetIconUnlockDesc(int controllerIndex, __int16 id);
int __cdecl BG_EmblemsGetIconCost(__int16 id);
int __cdecl BG_EmblemsGetIconUnlockLevel(__int16 id);
int __cdecl BG_EmblemsGetIconUnlockPLevel(__int16 id);
int __cdecl BG_EmblemsGetIconUnclassifyAt(__int16 id);
int __cdecl BG_EmblemsGetIconDefaultColor(__int16 id);
GfxImage *__cdecl BG_EmblemsGetIconImage(__int16 id);
double __cdecl BG_EmblemsGetIconOutlineSize(__int16 id);
bool __cdecl BG_EmblemsIsIconLocked(int controllerIndex, __int16 id);
bool __cdecl BG_EmblemsIsIconClassified(int controllerIndex, __int16 id);
bool __cdecl BG_EmblemsIsIconPurchased(int controllerIndex, __int16 id);
char __cdecl BG_EmblemsPurchaseIcon(int controllerIndex, __int16 id);
int __cdecl BG_EmblemsGetCategoryCount();
const char *__cdecl BG_EmblemsGetCategoryDesc(int index);
int __cdecl BG_EmblemsGetBackgroundCount();
int __cdecl BG_EmblemsGetTotalBackgroundCount();
int __cdecl BG_EmblemsGetPurchasedBackgroundCount(int controllerIndex);
__int16 __cdecl BG_EmblemsGetBackgroundID(int index);
int __cdecl BG_EmblemsGetBackgroundIndex(__int16 id);
const char *__cdecl BG_EmblemsGetBackgroundDesc(int controllerIndex, __int16 id);
const char *__cdecl BG_EmblemsGetBackgroundUnlockDesc(int controllerIndex, __int16 id);
int __cdecl BG_EmblemsGetBackgroundCost(__int16 id);
int __cdecl BG_EmblemsGetBackgroundUnlockLevel(__int16 id);
int __cdecl BG_EmblemsGetBackgroundUnlockPLevel(__int16 id);
int __cdecl BG_EmblemsGetBackgroundUnclassifyAt(__int16 id);
Material *__cdecl BG_EmblemsGetBackgroundMaterial(__int16 id);
char __cdecl BG_EmblemsIsBackgroundLocked(int controllerIndex, __int16 id);
bool __cdecl BG_EmblemsIsBackgroundClassified(int controllerIndex, __int16 id);
bool __cdecl BG_EmblemsIsBackgroundPurchased(int controllerIndex, __int16 id);
char __cdecl BG_EmblemsPurchaseBackground(int controllerIndex, __int16 id);
void __cdecl BG_EmblemsWriteMsg(msg_t *msg, __int16 background, CompositeEmblemLayer *layers, int layerCount);
void __cdecl BG_EmblemsReadMsg(msg_t *msg, __int16 *background, CompositeEmblemLayer *layers, int layerCount);
char *__cdecl BG_EmblemsWriteString(__int16 background, CompositeEmblemLayer *layers, int layerCount);
bool __cdecl BG_EmblemsReadString(const char *str, __int16 *background, CompositeEmblemLayer *layers, int layerCount);
void __cdecl BG_EmblemsInit();
void __cdecl BG_EmblemsPurchaseLayer_f();
void __cdecl BG_EmblemsPurchaseIcon_f();

void __cdecl BG_EmblemsPurchaseBackgroundByID_f();
void __cdecl BG_EmblemsPurchaseBackgroundByIndex_f();
void __cdecl BG_EmblemsClearDefaults();
void __cdecl BG_EmblemsSetDefaultForLevel(int minlevel, int maxlevel, CompositeEmblemLayer *s_emblem, int layerCount);
void __cdecl BG_EmblemsGenerateDefault(int controllerIndex, int level, CompositeEmblemLayer *layers, int layerCount);
unsigned int __cdecl SV_CACValidate_GetPurchasedEmblemLayers(unsigned __int8 *buffer);
bool __cdecl SV_CACValidate_IsBackgroundPurchased(unsigned __int8 *buffer, int rank, __int16 id);
bool __cdecl SV_CACValidate_IsBackgroundLockedForRank(unsigned __int8 *buffer, int rank, __int16 id);
bool __cdecl SV_CACValidate_IsIconPurchased(unsigned __int8 *buffer, int rank, __int16 id);
bool __cdecl SV_CACValidate_IsIconLockedForRank(unsigned __int8 *buffer, int rank, __int16 id);

extern const dvar_s *allEmblemsUnlocked;
extern const dvar_s *allEmblemsPurchased;
extern const dvar_s *bg_overridePlayerEmblemColor;
extern const dvar_s *bg_overridePlayerEmblemIcon;