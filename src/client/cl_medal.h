#pragma once

enum medalTableColumns_t : __int32
{                                       // XREF: ?CG_GetMedalData@@YAPBDHW4medalTableColumns_t@@@Z/r
                                        // ?CG_GetMedalLocalizedXP@@YAPBDMH_N@Z/r
    MEDAL_COLUMN_INDEX            = 0x0,
    MEDAL_COLUMN_STAT_NAME        = 0x1,
    MEDAL_COLUMN_NAME             = 0x2,
    MEDAL_COLUMN_DESC             = 0x3,
    MEDAL_COLUMN_XP_TEAMBASED     = 0x4,
    MEDAL_COLUMN_XP_FFA           = 0x5,
    MEDAL_COLUMN_HARDCORE_ENABLED = 0x6,
};

enum wagerMedalTableColumns_t : __int32
{                                       // XREF: ?CG_GetWagerMedalData@@YAPBDHW4wagerMedalTableColumns_t@@@Z/r
                                        // ?CG_GetWagerMedalLocalizedCP@@YAPBDH_N@Z/r
    WAGER_MEDAL_COLUMN_INDEX        = 0x0,
    WAGER_MEDAL_COLUMN_STAT_NAME    = 0x1,
    WAGER_MEDAL_COLUMN_NAME         = 0x2,
    WAGER_MEDAL_COLUMN_DESC         = 0x3,
    WAGER_MEDAL_COLUMN_CP_TEAMBASED = 0x4,
    WAGER_MEDAL_COLUMN_CP_FFA       = 0x5,
};

const char *__cdecl CG_GetMedalData(int medalIndex, medalTableColumns_t column);
char *__cdecl CG_GetMedalLocalizedXP(float xpScale, int medalIndex, bool teamBased);
const char *__cdecl CG_GetWagerMedalData(int wagerMedalIndex, wagerMedalTableColumns_t column);
char *__cdecl CG_GetWagerMedalLocalizedCP(int wagerMedalIndex, bool teamBased);
