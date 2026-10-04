#include "cl_medal.h"
#include <universal/com_stringtable.h>
#include <universal/assertive.h>
#include <universal/q_shared.h>
#include <ui/ui_main.h>
#include <stdlib.h>

const char *__cdecl CG_GetMedalData(int medalIndex, medalTableColumns_t column)
{
    const StringTable *medalTable; // [esp+0h] [ebp-18h] BYREF
    char index[8]; // [esp+4h] [ebp-14h] BYREF
    int medalTableRow; // [esp+14h] [ebp-4h]

    StringTable_GetAsset("mp/medalTable.csv", (XAssetHeader *)&medalTable);
    if ( !medalTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_medal.cpp", 17, 0, "%s", "medalTable") )
    {
        __debugbreak();
    }
    if ( !medalTable )
        return 0;
    if ( medalIndex > 999
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_medal.cpp",
                    21,
                    0,
                    "%s",
                    "medalIndex <= MEDAL_MAX_COUNT") )
    {
        __debugbreak();
    }
    if ( medalIndex <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_medal.cpp", 22, 0, "%s", "medalIndex > 0") )
    {
        __debugbreak();
    }
    if ( (unsigned int)medalIndex >= 0x3E8 )
        return 0;
    Com_sprintf(index, 5u, "%i", medalIndex);
    medalTableRow = StringTable_LookupRowNumForValue(medalTable, 0, index);
    return StringTable_GetColumnValueForRow(medalTable, medalTableRow, column);
}

char *__cdecl CG_GetMedalLocalizedXP(float xpScale, int medalIndex, bool teamBased)
{
    const char *MedalData; // eax
    int xpAmount; // [esp+4h] [ebp-8h]
    char *localizedPopUpXP; // [esp+8h] [ebp-4h]

    if ( teamBased )
        MedalData = CG_GetMedalData(medalIndex, MEDAL_COLUMN_XP_TEAMBASED);
    else
        MedalData = CG_GetMedalData(medalIndex, MEDAL_COLUMN_XP_FFA);
    xpAmount = (int)(float)((float)atoi(MedalData) * xpScale);
    localizedPopUpXP = UI_SafeTranslateString("MEDAL_XP_POPUP");
    if ( xpAmount <= 0 )
        return (char *)"";
    else
        return UI_ReplaceConversionInt(localizedPopUpXP, xpAmount);
}

const char *__cdecl CG_GetWagerMedalData(int wagerMedalIndex, wagerMedalTableColumns_t column)
{
    int wagerMedalTableRow; // [esp+0h] [ebp-18h]
    char index[8]; // [esp+4h] [ebp-14h] BYREF
    const StringTable *wagerMedalTable; // [esp+10h] [ebp-8h] BYREF

    StringTable_GetAsset("mp/medalTable.csv", (XAssetHeader *)&wagerMedalTable);
    if ( !wagerMedalTable
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_medal.cpp", 68, 0, "%s", "wagerMedalTable") )
    {
        __debugbreak();
    }
    if ( !wagerMedalTable )
        return 0;
    if ( wagerMedalIndex > 999
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_medal.cpp",
                    72,
                    0,
                    "%s",
                    "wagerMedalIndex <= MEDAL_MAX_COUNT") )
    {
        __debugbreak();
    }
    if ( wagerMedalIndex <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client_mp\\cl_medal.cpp",
                    73,
                    0,
                    "%s",
                    "wagerMedalIndex > 0") )
    {
        __debugbreak();
    }
    if ( (unsigned int)wagerMedalIndex >= 0x3E8 )
        return 0;
    Com_sprintf(index, 5u, "%i", wagerMedalIndex);
    wagerMedalTableRow = StringTable_LookupRowNumForValue(wagerMedalTable, 0, index);
    return StringTable_GetColumnValueForRow(wagerMedalTable, wagerMedalTableRow, column);
}

char *__cdecl CG_GetWagerMedalLocalizedCP(int wagerMedalIndex, bool teamBased)
{
    const char *WagerMedalData; // eax
    int Int; // eax
    int cpAmount; // [esp+4h] [ebp-Ch]
    char *localizedPopUpCP; // [esp+Ch] [ebp-4h]

    if ( teamBased )
        WagerMedalData = CG_GetWagerMedalData(wagerMedalIndex, WAGER_MEDAL_COLUMN_CP_TEAMBASED);
    else
        WagerMedalData = CG_GetWagerMedalData(wagerMedalIndex, WAGER_MEDAL_COLUMN_CP_FFA);
    cpAmount = atoi(WagerMedalData);
    localizedPopUpCP = UI_SafeTranslateString("MEDAL_CP_POPUP");
    Int = Dvar_GetInt("scr_wagerBet");
    return UI_ReplaceConversionInt(localizedPopUpCP, (int)(float)((float)(cpAmount * Int) * 0.0099999998));
}

