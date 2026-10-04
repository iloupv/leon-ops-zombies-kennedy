#include <game_sp/g_sp_loadgame.h>
#include "cl_keys.h"
#include <gfx_d3d/r_font.h>
#include <client_mp/cl_main_mp.h>
#include <stringed/stringed_hooks.h>
#include "cl_console.h"
#include <win32/win_main.h>
#include <client_mp/cl_scrn_mp.h>
#include <universal/com_files.h>
#include "splitscreen.h"
#include <universal/com_memory.h>
#include <win32/win_gamepad.h>
#include "cl_main.h"
#include <devgui/devgui.h>
#include <cgame_mp/cg_newDraw_mp.h>
#include <server_mp/sv_init_mp.h>
#include "client.h"
#include <cgame_mp/cg_main_mp.h>
#include <ui_mp/ui_main_mp.h>
#include <cgame_mp/cg_consolecmds_mp.h>
#include <cgame_mp/cg_scoreboard_mp.h>
#include <demo/demo_playback.h>
#include <ui/ui_shared.h>
#include <qcommon/com_clients.h>
#include <client_mp/cl_ui_mp.h>
#include <client_mp/cl_cgame_mp.h>
#include <cgame/cg_sp_playtrace.h>

struct keyname_t // sizeof=0x8
{
    const char *name;
    int keynum;
};

keyname_t keynames[116] =
{
  { "TAB", 9 },
  { "ENTER", 13 },
  { "ESCAPE", 27 },
  { "SPACE", 32 },
  { "BACKSPACE", 127 },
  { "UPARROW", 154 },
  { "DOWNARROW", 155 },
  { "LEFTARROW", 156 },
  { "RIGHTARROW", 157 },
  { "ALT", 158 },
  { "CTRL", 159 },
  { "SHIFT", 160 },
  { "CAPSLOCK", 151 },
  { "F1", 167 },
  { "F2", 168 },
  { "F3", 169 },
  { "F4", 170 },
  { "F5", 171 },
  { "F6", 172 },
  { "F7", 173 },
  { "F8", 174 },
  { "F9", 175 },
  { "F10", 176 },
  { "F11", 177 },
  { "F12", 178 },
  { "INS", 161 },
  { "DEL", 162 },
  { "PGDN", 163 },
  { "PGUP", 164 },
  { "HOME", 165 },
  { "END", 166 },
  { "MOUSE1", 200 },
  { "MOUSE2", 201 },
  { "MOUSE3", 202 },
  { "MOUSE4", 203 },
  { "MOUSE5", 204 },
  { "MWHEELUP", 206 },
  { "MWHEELDOWN", 205 },
  { "BUTTON_A", 1 },
  { "BUTTON_B", 2 },
  { "BUTTON_X", 3 },
  { "BUTTON_Y", 4 },
  { "BUTTON_LSHLDR", 5 },
  { "BUTTON_RSHLDR", 6 },
  { "BUTTON_START", 14 },
  { "BUTTON_BACK", 15 },
  { "BUTTON_LSTICK", 16 },
  { "BUTTON_RSTICK", 17 },
  { "BUTTON_RTRIG", 19 },
  { "BUTTON_LTRIG", 18 },
  { "DPAD_UP", 20 },
  { "DPAD_DOWN", 21 },
  { "DPAD_LEFT", 22 },
  { "DPAD_RIGHT", 23 },
  { "APAD_UP", 28 },
  { "APAD_DOWN", 29 },
  { "APAD_LEFT", 30 },
  { "APAD_RIGHT", 31 },
  { "AUX1", 207 },
  { "AUX2", 208 },
  { "AUX3", 209 },
  { "AUX4", 210 },
  { "AUX5", 211 },
  { "AUX6", 212 },
  { "AUX7", 213 },
  { "AUX8", 214 },
  { "AUX9", 215 },
  { "AUX10", 216 },
  { "AUX11", 217 },
  { "AUX12", 218 },
  { "AUX13", 219 },
  { "AUX14", 220 },
  { "AUX15", 221 },
  { "AUX16", 222 },
  { "KP_HOME", 182 },
  { "KP_UPARROW", 183 },
  { "KP_PGUP", 184 },
  { "KP_LEFTARROW", 185 },
  { "KP_5", 186 },
  { "KP_RIGHTARROW", 187 },
  { "KP_END", 188 },
  { "KP_DOWNARROW", 189 },
  { "KP_PGDN", 190 },
  { "KP_ENTER", 191 },
  { "KP_INS", 192 },
  { "KP_DEL", 193 },
  { "KP_SLASH", 194 },
  { "KP_MINUS", 195 },
  { "KP_PLUS", 196 },
  { "KP_NUMLOCK", 197 },
  { "KP_STAR", 198 },
  { "KP_EQUALS", 199 },
  { "PAUSE", 153 },
  { "SEMICOLON", 59 },
  { "COMMAND", 150 },
  { "181", 128 },
  { "191", 129 },
  { "223", 130 },
  { "224", 131 },
  { "225", 132 },
  { "228", 133 },
  { "229", 134 },
  { "230", 135 },
  { "231", 136 },
  { "232", 137 },
  { "233", 138 },
  { "236", 139 },
  { "241", 140 },
  { "242", 141 },
  { "243", 142 },
  { "246", 143 },
  { "248", 144 },
  { "249", 145 },
  { "250", 146 },
  { "252", 147 },
  { NULL, 0 }
};

keyname_t keynames_localized[116] =
{
  { "KEY_TAB", 9 },
  { "KEY_ENTER", 13 },
  { "KEY_ESCAPE", 27 },
  { "KEY_SPACE", 32 },
  { "KEY_BACKSPACE", 127 },
  { "KEY_UPARROW", 154 },
  { "KEY_DOWNARROW", 155 },
  { "KEY_LEFTARROW", 156 },
  { "KEY_RIGHTARROW", 157 },
  { "KEY_ALT", 158 },
  { "KEY_CTRL", 159 },
  { "KEY_SHIFT", 160 },
  { "KEY_CAPSLOCK", 151 },
  { "KEY_F1", 167 },
  { "KEY_F2", 168 },
  { "KEY_F3", 169 },
  { "KEY_F4", 170 },
  { "KEY_F5", 171 },
  { "KEY_F6", 172 },
  { "KEY_F7", 173 },
  { "KEY_F8", 174 },
  { "KEY_F9", 175 },
  { "KEY_F10", 176 },
  { "KEY_F11", 177 },
  { "KEY_F12", 178 },
  { "KEY_INS", 161 },
  { "KEY_DEL", 162 },
  { "KEY_PGDN", 163 },
  { "KEY_PGUP", 164 },
  { "KEY_HOME", 165 },
  { "KEY_END", 166 },
  { "KEY_MOUSE1", 200 },
  { "KEY_MOUSE2", 201 },
  { "KEY_MOUSE3", 202 },
  { "KEY_MOUSE4", 203 },
  { "KEY_MOUSE5", 204 },
  { "KEY_MWHEELUP", 206 },
  { "KEY_MWHEELDOWN", 205 },
  { "KEY_BUTTON_A", 1 },
  { "KEY_BUTTON_B", 2 },
  { "KEY_BUTTON_X", 3 },
  { "KEY_BUTTON_Y", 4 },
  { "KEY_BUTTON_LSHLDR", 5 },
  { "KEY_BUTTON_RSHLDR", 6 },
  { "KEY_BUTTON_START", 14 },
  { "KEY_BUTTON_BACK", 15 },
  { "KEY_BUTTON_LSTICK", 16 },
  { "KEY_BUTTON_RSTICK", 17 },
  { "KEY_BUTTON_RTRIG", 19 },
  { "KEY_BUTTON_LTRIG", 18 },
  { "KEY_DPAD_UP", 20 },
  { "KEY_DPAD_DOWN", 21 },
  { "KEY_DPAD_LEFT", 22 },
  { "KEY_DPAD_RIGHT", 23 },
  { "KEY_APAD_UP", 28 },
  { "KEY_APAD_DOWN", 29 },
  { "KEY_APAD_LEFT", 30 },
  { "KEY_APAD_RIGHT", 31 },
  { "KEY_AUX1", 207 },
  { "KEY_AUX2", 208 },
  { "KEY_AUX3", 209 },
  { "KEY_AUX4", 210 },
  { "KEY_AUX5", 211 },
  { "KEY_AUX6", 212 },
  { "KEY_AUX7", 213 },
  { "KEY_AUX8", 214 },
  { "KEY_AUX9", 215 },
  { "KEY_AUX10", 216 },
  { "KEY_AUX11", 217 },
  { "KEY_AUX12", 218 },
  { "KEY_AUX13", 219 },
  { "KEY_AUX14", 220 },
  { "KEY_AUX15", 221 },
  { "KEY_AUX16", 222 },
  { "KEY_KP_HOME", 182 },
  { "KEY_KP_UPARROW", 183 },
  { "KEY_KP_PGUP", 184 },
  { "KEY_KP_LEFTARROW", 185 },
  { "KEY_KP_5", 186 },
  { "KEY_KP_RIGHTARROW", 187 },
  { "KEY_KP_END", 188 },
  { "KEY_KP_DOWNARROW", 189 },
  { "KEY_KP_PGDN", 190 },
  { "KEY_KP_ENTER", 191 },
  { "KEY_KP_INS", 192 },
  { "KEY_KP_DEL", 193 },
  { "KEY_KP_SLASH", 194 },
  { "KEY_KP_MINUS", 195 },
  { "KEY_KP_PLUS", 196 },
  { "KEY_KP_NUMLOCK", 197 },
  { "KEY_KP_STAR", 198 },
  { "KEY_KP_EQUALS", 199 },
  { "KEY_PAUSE", 153 },
  { "KEY_SEMICOLON", 59 },
  { "KEY_COMMAND", 150 },
  { " ", 128 },
  { " ", 129 },
  { " ", 130 },
  { " ", 131 },
  { " ", 132 },
  { " ", 133 },
  { " ", 134 },
  { " ", 135 },
  { " ", 136 },
  { " ", 137 },
  { " ", 138 },
  { " ", 139 },
  { " ", 140 },
  { " ", 141 },
  { " ", 142 },
  { " ", 143 },
  { " ", 144 },
  { " ", 145 },
  { " ", 146 },
  { " ", 147 },
  { NULL, 0 }
};





PlayerKeyState playerKeys[1];

const char *s_completionString;
int s_matchCount;
int s_prefixMatchCount;
char s_shortestMatch[1024];
bool s_hasExactMatch;

bool s_shouldCompleteCmd;
int nextHistoryLine;
int historyLine;

void __cdecl Field_DrawTextOverride(
                int localClientNum,
                const field_t *edit,
                int x,
                int y,
                int horzAlign,
                int vertAlign,
                char *str,
                int drawLen,
                int cursorPos)
{
    int v9; // eax
    float v10; // [esp+2Ch] [ebp-2Ch]
    float v11; // [esp+30h] [ebp-28h]
    Font_s *font; // [esp+34h] [ebp-24h]
    float xScale; // [esp+38h] [ebp-20h]
    const ScreenPlacement *scrPlace; // [esp+40h] [ebp-18h]
    float fontScalea; // [esp+48h] [ebp-10h]
    float fontScale; // [esp+48h] [ebp-10h]
    int fontStyle; // [esp+4Ch] [ebp-Ch]
    float yScale; // [esp+50h] [ebp-8h]
    char cursorChar; // [esp+57h] [ebp-1h]

    if ( drawLen <= 0
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp", 414, 0, "%s", "drawLen > 0") )
    {
        __debugbreak();
    }
    scrPlace = &scrPlaceView[localClientNum];
    if ( edit->fixedSize )
    {
        font = cls.consoleFont;
        fontStyle = 0;
        xScale = 1.0f;
        yScale = 1.0f;
        if ( Key_GetOverstrikeMode(localClientNum) )
            cursorChar = 95;
        else
            cursorChar = 124;
    }
    else
    {
        fontScalea = edit->charHeight / 48.0;
        font = UI_GetFontHandle(scrPlace, 0, fontScalea);
        fontScale = R_NormalizedTextScale(font, fontScalea);
        if ( vertAlign == 5 )
        {
            v11 = ScrPlace_HiResGetScale() * fontScale;
            xScale = v11 * scrPlace->scaleVirtualToReal[0];
        }
        else
        {
            xScale = fontScale;
        }
        if ( vertAlign == 5 )
        {
            v10 = ScrPlace_HiResGetScale() * fontScale;
            yScale = v10 * scrPlace->scaleVirtualToReal[1];
        }
        else
        {
            yScale = fontScale;
        }
        fontStyle = 3;
        if ( Key_GetOverstrikeMode(localClientNum) )
            cursorChar = 95;
        else
            cursorChar = 124;
    }
    v9 = R_TextHeight(font);
    CL_DrawTextWithCursor(
        scrPlace,
        str,
        drawLen,
        font,
        (float)x,
        (float)y + (float)((float)v9 * yScale),
        horzAlign,
        vertAlign,
        xScale,
        yScale,
        colorWhite,
        fontStyle,
        cursorPos,
        cursorChar);
}

void __cdecl Field_Draw(int localClientNum, field_t *edit, int x, int y, int horzAlign, int vertAlign)
{
    char str[1024]; // [esp+0h] [ebp-408h] BYREF

    if ( !edit->drawWidth )
        edit->drawWidth = 256;
    if ( edit->scroll < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    476,
                    0,
                    "%s\n\t(edit->scroll) = %i",
                    "(edit->scroll >= 0)",
                    edit->scroll) )
    {
        __debugbreak();
    }
    I_strncpyz(str, &edit->buffer[edit->scroll], 256 - edit->scroll);
    Field_DrawTextOverride(
        localClientNum,
        edit,
        x,
        y,
        horzAlign,
        vertAlign,
        str,
        edit->drawWidth,
        edit->cursor - edit->scroll);
}

void __cdecl Field_AdjustScroll(const ScreenPlacement *scrPlace, field_t *edit)
{
    double v2; // st7
    char *v3; // [esp+Ch] [ebp-2Ch]
    char *v4; // [esp+10h] [ebp-28h]
    float v5; // [esp+14h] [ebp-24h]
    float v6; // [esp+18h] [ebp-20h]
    Font_s *font; // [esp+1Ch] [ebp-1Ch]
    float len; // [esp+20h] [ebp-18h]
    float lena; // [esp+20h] [ebp-18h]
    float lenb; // [esp+20h] [ebp-18h]
    float actualScale; // [esp+24h] [ebp-14h]
    float lineWidth; // [esp+2Ch] [ebp-Ch]
    float fullLen; // [esp+30h] [ebp-8h]
    float fontScale; // [esp+34h] [ebp-4h]
    float fontScalea; // [esp+34h] [ebp-4h]

    fontScale = edit->charHeight / 48.0;
    lineWidth = (float)edit->widthInPixels;
    if ( edit->fixedSize )
    {
        v6 = fontScale / ScrPlace_HiResGetScale();
        fontScalea = v6 * scrPlace->scaleRealToVirtual[0];
        v5 = lineWidth / ScrPlace_HiResGetScale();
        lineWidth = v5 * scrPlace->scaleRealToVirtual[1];
        font = cls.consoleFont;
        v2 = R_NormalizedTextScale(cls.consoleFont, fontScalea);
    }
    else
    {
        font = UI_GetFontHandle(scrPlace, 0, fontScale);
        v2 = R_NormalizedTextScale(font, fontScale);
    }
    actualScale = v2;
    if ( lineWidth <= (float)((float)R_TextWidth(edit->buffer, 0, font) * actualScale) )
    {
        len = 0.0f;
        while ( lineWidth > len )
        {
            if ( edit->scroll <= 0 )
                break;
            len = (float)R_TextWidth((const char *)&edit->fixedSize + edit->scroll + 3, 0, font) * actualScale;
            if ( len >= lineWidth )
                break;
            --edit->scroll;
        }
        do
        {
            fullLen = (float)R_TextWidth(&edit->buffer[edit->scroll], 0, font) * actualScale;
            lena = fullLen - (float)((float)R_TextWidth(&edit->buffer[edit->cursor], 0, font) * actualScale);
            if ( lena < 0.0 || lena >= lineWidth )
            {
                if ( lena >= 0.0 )
                {
                    ++edit->scroll;
                }
                else if ( edit->scroll )
                {
                    --edit->scroll;
                }
                else
                {
                    lena = 0.0f;
                }
            }
        }
        while ( lena < 0.0 || lena >= lineWidth );
        v3 = &edit->buffer[edit->scroll + 1];
        v4 = &edit->buffer[edit->scroll + 1 + strlen(&edit->buffer[edit->scroll])];
        edit->drawWidth = edit->cursor - edit->scroll;
        lenb = 0.0f;
        while ( lineWidth > lenb && edit->drawWidth < v4 - v3 )
        {
            lenb = (float)R_TextWidth(&edit->buffer[edit->scroll], edit->drawWidth + 1, font) * actualScale;
            if ( lineWidth > lenb )
                ++edit->drawWidth;
        }
    }
    else
    {
        edit->scroll = 0;
        edit->drawWidth = SEH_PrintStrlen(edit->buffer);
    }
}

void __cdecl Console_Key(int localClientNum, int key)
{
    bool v2; // [esp+8h] [ebp-1Ch]
    bool v3; // [esp+Ch] [ebp-18h]
    char v4; // [esp+12h] [ebp-12h]
    char v5; // [esp+13h] [ebp-11h]
    int isShiftDown; // [esp+14h] [ebp-10h]
    int isCtrlDown; // [esp+18h] [ebp-Ch]
    int shouldCompleteCmd; // [esp+1Ch] [ebp-8h]
    int isAltDown; // [esp+20h] [ebp-4h]

    shouldCompleteCmd = s_shouldCompleteCmd;
    s_shouldCompleteCmd = 1;
    isShiftDown = playerKeys[localClientNum].keys[160].down;
    isCtrlDown = playerKeys[localClientNum].keys[159].down;
    isAltDown = playerKeys[localClientNum].keys[158].down;
    if ( key == 108 && isCtrlDown )
    {
        Cbuf_AddText(localClientNum, "clear\n");
        return;
    }
    if ( key == 13 || key == 191 )
    {
        if ( Con_CommitToAutoComplete() )
            return;
        Com_Printf(0, "]%s\n", g_consoleField.buffer);
        if ( g_consoleField.buffer[0] == 92 || g_consoleField.buffer[0] == 47 )
        {
            Con_Restricted_AddBuf(&g_consoleField.buffer[1]);
        }
        else if ( Console_IsClientDisconnected()
                     && I_strnicmp(g_consoleField.buffer, "quit", 4)
                     && I_strnicmp(g_consoleField.buffer, "kill", 4) )
        {
            Con_Restricted_AddBuf(g_consoleField.buffer);
        }
        else
        {
            if ( !g_consoleField.buffer[0] )
                return;
            if ( !Console_IsRconCmd(g_consoleField.buffer) )
            {
                Cbuf_AddText(localClientNum, "cmd say ");
                Cbuf_AddText(localClientNum, g_consoleField.buffer);
                Cbuf_AddText(localClientNum, "\n");
            }
        }
        if ( g_consoleField.buffer[0] )
        {
            memcpy(&historyEditLines[nextHistoryLine % 32], &g_consoleField, sizeof(field_t));
            historyLine = ++nextHistoryLine;
        }
        Field_Clear(&g_consoleField);
        g_consoleField.widthInPixels = g_console_field_width;
        g_consoleField.charHeight = g_console_char_height;
        g_consoleField.fixedSize = 1;
        if ( Console_IsClientDisconnected() )
            SCR_UpdateScreen();
    }
    else if ( key == 9 )
    {
        if ( shouldCompleteCmd )
            CompleteCommand();
        else
            Con_CycleAutoComplete(2 * (isShiftDown == 0) - 1);
        s_shouldCompleteCmd = 0;
    }
    else
    {
        if ( key == 154 && isCtrlDown )
        {
            Con_CycleAutoComplete(-1);
            return;
        }
        if ( key == 155 && isCtrlDown )
        {
            Con_CycleAutoComplete(1);
            return;
        }
        if ( key == 206 && isShiftDown )
        {
            v5 = 1;
        }
        else if ( key == 183 )
        {
            v5 = 1;
        }
        else
        {
            v3 = key == 154 || tolower(key) == 112 && isCtrlDown;
            v5 = v3;
        }
        if ( v5 )
        {
            if ( nextHistoryLine - historyLine < 32 && historyLine > 0 )
                --historyLine;
            memcpy(&g_consoleField, &historyEditLines[historyLine % 32], sizeof(g_consoleField));
            Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
            Con_AllowAutoCompleteCycling(0);
        }
        else
        {
            if ( key == 205 && isShiftDown )
            {
                v4 = 1;
            }
            else if ( key == 189 )
            {
                v4 = 1;
            }
            else
            {
                v2 = key == 155 || tolower(key) == 110 && isCtrlDown;
                v4 = v2;
            }
            if ( v4 )
            {
                if ( !Con_CycleAutoComplete(1) && historyLine != nextHistoryLine )
                {
                    memcpy(&g_consoleField, &historyEditLines[++historyLine % 32], sizeof(g_consoleField));
                    Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
                }
            }
            else
            {
                switch ( key )
                {
                    case 164:
                        Con_PageUp();
                        return;
                    case 163:
                        Con_PageDown();
                        return;
                    case 206:
                        Con_PageUp();
                        if ( isCtrlDown )
                        {
                            Con_PageUp();
                            Con_PageUp();
                        }
                        break;
                    case 205:
                        Con_PageDown();
                        if ( isCtrlDown )
                        {
                            Con_PageDown();
                            Con_PageDown();
                        }
                        break;
                    default:
                        if ( key == 165 && isCtrlDown )
                        {
                            Con_Top();
                            return;
                        }
                        if ( key == 166 && isCtrlDown )
                        {
                            Con_Bottom();
                            return;
                        }
                        if ( key == 162 || key == 27 )
                        {
                            if ( Con_CancelAutoComplete() )
                                return;
                        }
                        else if ( key == 157
                                     || key == 187
                                     || key == 156
                                     || key == 185
                                     || key != 127 && !isCtrlDown && !isAltDown && !isShiftDown )
                        {
                            Con_CommitToAutoComplete();
                        }
                        if ( Field_KeyDownEvent(localClientNum, &scrPlaceFull, &g_consoleField, key) )
                            Con_AllowAutoCompleteCycling(1);
                        break;
                }
            }
        }
    }
}

char __cdecl Field_KeyDownEvent(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int key)
{
    int OverstrikeMode; // eax
    signed int len; // [esp+14h] [ebp-Ch]
    int isCtrlDown; // [esp+18h] [ebp-8h]
    char isModified; // [esp+1Fh] [ebp-1h]

    isCtrlDown = playerKeys[localClientNum].keys[159].down;
    isModified = 0;
    len = strlen(edit->buffer);
    if ( (key == 161 || key == 192) && playerKeys[localClientNum].keys[160].down )
    {
        isModified = Field_Paste(localClientNum, scrPlace, edit);
    }
    else
    {
        switch ( key )
        {
            case 162:
                if ( edit->cursor < len )
                    memmove(
                        (unsigned __int8 *)&edit->buffer[edit->cursor],
                        (unsigned __int8 *)&edit->buffer[edit->cursor + 1],
                        len - edit->cursor);
                break;
            case 157:
                if ( edit->cursor < len )
                    ++edit->cursor;
                if ( isCtrlDown )
                {
                    while ( edit->cursor < len && isalnum(edit->buffer[edit->cursor]) )
                        ++edit->cursor;
                    while ( edit->cursor < len && !isalnum(edit->buffer[edit->cursor]) )
                        ++edit->cursor;
                }
                break;
            case 156:
                if ( edit->cursor > 0 )
                    --edit->cursor;
                if ( isCtrlDown )
                {
                    while ( edit->cursor > 0 && isalnum(*((char *)&edit->fixedSize + edit->cursor + 3)) )
                        --edit->cursor;
                }
                if ( edit->cursor < edit->scroll )
                    edit->scroll = edit->cursor;
                break;
            default:
                if ( key == 165 || tolower(key) == 97 && isCtrlDown )
                {
                    edit->cursor = 0;
                }
                else if ( key == 166 || tolower(key) == 101 && isCtrlDown )
                {
                    edit->cursor = len;
                }
                else if ( key == 161 )
                {
                    OverstrikeMode = Key_GetOverstrikeMode(localClientNum);
                    Key_SetOverstrikeMode(localClientNum, OverstrikeMode == 0);
                }
                break;
        }
    }
    if ( cls.uiStarted )
        Field_AdjustScroll(scrPlace, edit);
    return isModified;
}

char __cdecl Field_Paste(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit)
{
    signed int v4; // [esp+0h] [ebp-1Ch]
    int i; // [esp+14h] [ebp-8h]
    char *cbd; // [esp+18h] [ebp-4h]

    cbd = Sys_GetClipboardData();
    if ( !cbd )
        return 0;
    v4 = strlen(cbd);
    for ( i = 0; i < v4; ++i )
        Field_CharEvent(localClientNum, scrPlace, edit, cbd[i]);
    Com_FreeEvent(cbd);
    return 1;
}

bool __cdecl Field_CharEvent(int localClientNum, const ScreenPlacement *scrPlace, field_t *edit, int ch)
{
    unsigned int len; // [esp+10h] [ebp-8h]
    bool isModified; // [esp+17h] [ebp-1h]

    len = strlen(edit->buffer);
    switch ( ch )
    {
        case 22:
            isModified = Field_Paste(localClientNum, scrPlace, edit);
            break;
        case 3:
            Field_Clear(edit);
            isModified = 1;
            break;
        case 8:
            isModified = edit->cursor > 0;
            if ( edit->cursor > 0 )
            {
                memmove(
                    (unsigned __int8 *)&edit->fixedSize + edit->cursor + 3,
                    (unsigned __int8 *)&edit->buffer[edit->cursor],
                    len + 1 - edit->cursor);
                --edit->cursor;
            }
            break;
        case 1:
            edit->cursor = 0;
            edit->scroll = 0;
            return 0;
        case 5:
            edit->cursor = len;
            isModified = 0;
            break;
        default:
            if ( ch < 32 )
                return 0;
            if ( Key_GetOverstrikeMode(localClientNum) )
            {
                if ( edit->cursor == 255 )
                    return 0;
                edit->buffer[edit->cursor++] = ch;
            }
            else
            {
                if ( len == 255 )
                    return 0;
                memmove(
                    (unsigned __int8 *)&edit->buffer[edit->cursor + 1],
                    (unsigned __int8 *)&edit->buffer[edit->cursor],
                    len + 1 - edit->cursor);
                edit->buffer[edit->cursor++] = ch;
            }
            if ( edit->cursor == len + 1 )
                edit->buffer[edit->cursor] = 0;
            isModified = 1;
            break;
    }
    Field_AdjustScroll(scrPlace, edit);
    return isModified;
}

void CompleteCommand()
{
    bool v0; // [esp+28h] [ebp-288h]
    bool v1; // [esp+2Ch] [ebp-284h]
    bool v2; // [esp+40h] [ebp-270h]
    int matchLenAfterCmds; // [esp+64h] [ebp-24Ch] BYREF
    char pbbuf[272]; // [esp+68h] [ebp-248h] BYREF
    field_t savedField; // [esp+178h] [ebp-138h] BYREF
    bool isDvarCommand; // [esp+29Bh] [ebp-15h]
    int offset; // [esp+29Ch] [ebp-14h]
    const char *originalCommand; // [esp+2A0h] [ebp-10h]
    int matchLenAfterDvars; // [esp+2A4h] [ebp-Ch] BYREF
    bool useExactMatch; // [esp+2ABh] [ebp-5h]
    field_t *edit; // [esp+2ACh] [ebp-4h]

    v2 = g_consoleField.buffer[0] == 47 || g_consoleField.buffer[0] == 92;
    offset = v2;
    // remove punkbuster crap
    //if ( !strnicmp(&g_consoleField.buffer[v2], "pb_", 3u) )
    //{
    //    strncpy((unsigned __int8 *)pbbuf, (unsigned __int8 *)&g_consoleField.buffer[offset], 0xFFu);
    //    pbbuf[255] = 0;
    //    if ( !strnicmp(pbbuf, "pb_sv_", 6u) )
    //        PbServerCompleteCommand(pbbuf, 255);
    //    else
    //        PbClientCompleteCommand(pbbuf, 255);
    //    Com_sprintf(g_consoleField.buffer, 0x100u, "\\%s", pbbuf);
    //    g_consoleField.cursor = strlen(g_consoleField.buffer);
    //    Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
    //}
    //else
    {
        s_completionString = Con_TokenizeInput();
        s_matchCount = 0;
        s_prefixMatchCount = 0;
        s_shortestMatch[0] = 0;
        if ( *s_completionString )
        {
            originalCommand = s_completionString;
            v1 = Cmd_Argc() > 1 && Con_IsDvarCommand(s_completionString);
            isDvarCommand = v1;
            if ( v1 )
                s_completionString = Cmd_Argv(1);
            matchLenAfterCmds = 0;
            matchLenAfterDvars = 0;
            if ( con_matchPrefixOnly->current.enabled )
            {
                con_ignoreMatchPrefixOnly = 1;
                UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
                if ( s_matchCount > con_inputMaxMatchesShown )
                {
                    con_ignoreMatchPrefixOnly = 0;
                    UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
                    if ( !s_matchCount )
                    {
                        con_ignoreMatchPrefixOnly = 1;
                        UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
                    }
                }
            }
            else
            {
                con_ignoreMatchPrefixOnly = 0;
                UpdateMatches(!isDvarCommand, &matchLenAfterCmds, &matchLenAfterDvars);
            }
            if ( s_matchCount )
            {
                edit = &g_consoleField;
                memcpy(&savedField, &g_consoleField, sizeof(savedField));
                v0 = isDvarCommand || s_matchCount == 1 || s_hasExactMatch && Con_AnySpaceAfterCommand();
                useExactMatch = v0;
                if ( isDvarCommand )
                    Com_sprintf(edit->buffer, 0x100u, "\\%s %s", originalCommand, s_shortestMatch);
                else
                    Com_sprintf(edit->buffer, 0x100u, "\\%s", s_shortestMatch);
                edit->cursor = strlen(edit->buffer);
                ConcatRemaining(savedField.buffer, (char *)s_completionString);
                if ( useExactMatch )
                {
                    if ( !isDvarCommand )
                    {
                        if ( Cmd_Argc() == 1 )
                        {
                            I_strncat(g_consoleField.buffer, 256, " ");
                        }
                        else if ( Cmd_Argc() == 2 )
                        {
                            if ( matchLenAfterCmds == matchLenAfterDvars )
                                CompleteCmdArgument();
                            else
                                CompleteDvarArgument();
                        }
                    }
                    edit->cursor = strlen(edit->buffer);
                }
                else if ( Con_HasTooManyMatchesToShow() )
                {
                    Com_Printf(0, "]%s\n", g_consoleField.buffer);
                    Con_ForEach(0, PrintMatches, PrintMatches);
                }
                Cmd_EndTokenizedString();
                Field_AdjustScroll(&scrPlaceFull, &g_consoleField);
            }
            else
            {
                Cmd_EndTokenizedString();
            }
        }
        else
        {
            Cmd_EndTokenizedString();
        }
    }
}

void __cdecl PrintMatches(const char *s)
{
    if ( con_ignoreMatchPrefixOnly && con_matchPrefixOnly->current.enabled
        || !I_strnicmp(s, s_shortestMatch, strlen(s_shortestMatch)) )
    {
        if ( I_stristr(s, s_shortestMatch) )
            Com_Printf(0, "        %s\n", s);
    }
}

void __cdecl ConcatRemaining(char *src, char *start)
{
    char *v2; // eax

    v2 = strstr(src, start);
    if ( v2 )
        I_strncat(g_consoleField.buffer, 256, (const char *)(strlen(start) + v2));
    else
        keyConcatArgs();
}

int keyConcatArgs()
{
    int result; // eax
    const char *v1; // eax
    int i; // [esp+18h] [ebp-8h]
    const char *arg; // [esp+1Ch] [ebp-4h]

    for ( i = 1; ; ++i )
    {
        result = Cmd_Argc();
        if ( i >= result )
            break;
        I_strncat(g_consoleField.buffer, 256, " ");
        for ( arg = Cmd_Argv(i); *arg; ++arg )
        {
            if ( *arg == 32 )
            {
                I_strncat(g_consoleField.buffer, 256, "\"");
                break;
            }
        }
        v1 = Cmd_Argv(i);
        I_strncat(g_consoleField.buffer, 256, v1);
        if ( *arg == 32 )
            I_strncat(g_consoleField.buffer, 256, "\"");
    }
    return result;
}

void CompleteCmdArgument()
{
    const char *cmdArgPrefix; // [esp+18h] [ebp-114h]
    char arg[256]; // [esp+1Ch] [ebp-110h] BYREF
    const char *cmdName; // [esp+120h] [ebp-Ch]
    const char **files; // [esp+124h] [ebp-8h]
    int fileCount; // [esp+128h] [ebp-4h] BYREF

    cmdName = Con_TokenizeInput();
    cmdArgPrefix = Cmd_Argv(1);
    if ( *cmdArgPrefix )
    {
        files = Cmd_GetAutoCompleteFileList(cmdName, &fileCount);
        Cmd_EndTokenizedString();
        if ( fileCount )
        {
            Con_AutoCompleteFromList(files, fileCount, cmdArgPrefix, arg, 0x100u);
            FS_FreeFileList(files);
            Com_StripExtension(arg, arg);
            ReplaceConsoleInputArgument(strlen(cmdArgPrefix), arg);
        }
    }
    else
    {
        Cmd_EndTokenizedString();
    }
}

void __cdecl ReplaceConsoleInputArgument(int replaceCount, const char *replacement)
{
    char *v3; // eax
    int cmdLineLen; // [esp+10h] [ebp-8h]

    if (!replacement
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp", 861, 0, "%s", "replacement"))
    {
        __debugbreak();
    }
    if (*replacement)
    {
        for (cmdLineLen = strlen(g_consoleField.buffer);
            cmdLineLen && isspace(g_consoleField.buffer[cmdLineLen - 1]);
            --cmdLineLen)
        {
            ;
        }
        if (replaceCount >= cmdLineLen)
        {
            v3 = va("%s: %i, %i", g_consoleField.buffer, replaceCount, cmdLineLen);
            if (!Assert_MyHandler(
                "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                868,
                0,
                "%s\n\t%s",
                "replaceCount < cmdLineLen",
                v3))
                __debugbreak();
        }
        I_strncpyz(&g_consoleField.buffer[cmdLineLen - replaceCount], replacement, 256 - (cmdLineLen - replaceCount));
    }
}

void CompleteDvarArgument()
{
    const char *dvarName; // [esp+18h] [ebp-114h]
    const dvar_s *dvar; // [esp+1Ch] [ebp-110h]
    const char *dvarValuePrefix; // [esp+20h] [ebp-10Ch]
    char dvarValue[260]; // [esp+24h] [ebp-108h] BYREF

    dvarName = Con_TokenizeInput();
    dvar = Dvar_FindVar(dvarName);
    if ( !dvar && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp", 914, 0, "%s", "dvar") )
        __debugbreak();
    if ( dvar->type == DVAR_TYPE_ENUM )
    {
        dvarValuePrefix = Cmd_Argv(1);
        if ( *dvarValuePrefix )
        {
            Con_AutoCompleteFromList(
                dvar->domain.enumeration.strings,
                dvar->domain.enumeration.stringCount,
                dvarValuePrefix,
                dvarValue,
                0x100u);
            ReplaceConsoleInputArgument(strlen(dvarValuePrefix), dvarValue);
        }
        Cmd_EndTokenizedString();
    }
    else
    {
        Cmd_EndTokenizedString();
    }
}

void __cdecl UpdateMatches(bool searchCmds, int *matchLenAfterCmds, int *matchLenAfterDvars)
{
    s_matchCount = 0;
    s_prefixMatchCount = 0;
    s_shortestMatch[0] = 0;
    if ( searchCmds )
    {
        Cmd_ForEachConsoleAccessName(FindMatches);
        *matchLenAfterCmds = strlen(s_shortestMatch);
    }
    else
    {
        *matchLenAfterCmds = 0;
    }
    Dvar_ForEachConsoleAccessName(FindMatches);
    *matchLenAfterDvars = strlen(s_shortestMatch);
}

void __cdecl FindMatches(const char *s)
{
    int v1; // esi
    bool v2; // [esp+4h] [ebp-28h]
    bool v3; // [esp+8h] [ebp-24h]
    int v4; // [esp+Ch] [ebp-20h]
    int i; // [esp+28h] [ebp-4h]

    v4 = strlen(s_completionString);
    if ( Con_IsAutoCompleteMatch(s, s_completionString, v4) )
    {
        v3 = !con_ignoreMatchPrefixOnly && con_matchPrefixOnly->current.enabled || !I_strnicmp(s, s_completionString, v4);
        if ( v3 )
            ++s_prefixMatchCount;
        if ( ++s_matchCount == 1 || v3 && s_prefixMatchCount == 1 )
        {
            I_strncpyz(s_shortestMatch, s, 1024);
            s_hasExactMatch = 1;
        }
        else if ( s_prefixMatchCount )
        {
            if ( v3 )
            {
                for ( i = 0; s[i]; ++i )
                {
                    v1 = tolower(s_shortestMatch[i]);
                    if ( v1 != tolower(s[i]) )
                        break;
                }
                v2 = !s[i] || s_hasExactMatch && !s_shortestMatch[i];
                s_hasExactMatch = v2;
                s_shortestMatch[i] = 0;
            }
        }
        else
        {
            I_strncpyz(s_shortestMatch, s_completionString, 1024);
            s_hasExactMatch = 0;
        }
    }
}

bool __cdecl Console_IsRconCmd(const char *commandString)
{
    return I_strncmp(commandString, "rcon", strlen("rcon")) == 0;
}

char __cdecl Console_IsClientDisconnected()
{
    return CL_AllLocalClientsDisconnected();
}

int __cdecl Key_GetOverstrikeMode(int localClientNum)
{
    return playerKeys[localClientNum].overstrikeMode;
}

void __cdecl Key_SetOverstrikeMode(int localClientNum, int state)
{
    playerKeys[localClientNum].overstrikeMode = state;
}

int __cdecl Key_IsDown(int localClientNum, int keynum)
{
    if ( keynum == -1 )
        return 0;
    else
        return playerKeys[localClientNum].keys[keynum].down;
}

int __cdecl Key_StringToKeynum(const char *str)
{
    int n2; // [esp+10h] [ebp-Ch]
    int n2a; // [esp+10h] [ebp-Ch]
    int n1; // [esp+14h] [ebp-8h]
    int n1a; // [esp+14h] [ebp-8h]
    keyname_t *kn; // [esp+18h] [ebp-4h]

    if ( !str || !*str )
        return -1;
    if ( !str[1] )
        return *str;
    if ( *str == 48 && str[1] == 120 && strlen(str) == 4 )
    {
        n1 = str[2];
        if ( I_isdigit(n1) )
        {
            n1a = n1 - 48;
        }
        else if ( n1 < 97 || n1 > 102 )
        {
            n1a = 0;
        }
        else
        {
            n1a = n1 - 87;
        }
        n2 = str[3];
        if ( I_isdigit(n2) )
        {
            n2a = n2 - 48;
        }
        else if ( n2 < 97 || n2 > 102 )
        {
            n2a = 0;
        }
        else
        {
            n2a = n2 - 87;
        }
        return n2a + 16 * n1a;
    }
    else
    {
        for ( kn = keynames; kn->name; ++kn )
        {
            if ( !I_stricmp(str, kn->name) )
                return kn->keynum;
        }
        return -1;
    }
}

bool __cdecl Key_IsValidGamePadChar(char key)
{
    return key >= 1 && key <= 6 || key >= 14 && key <= 25 || key >= 28 && key <= 31;
}

char tinystr[5];
const char *__cdecl Key_KeynumToString(int keynum, int translate)
{
    char v3; // [esp+0h] [ebp-14h]
    char v4; // [esp+4h] [ebp-10h]
    char j; // [esp+8h] [ebp-Ch]
    keyname_t *kn; // [esp+Ch] [ebp-8h]
    int i; // [esp+10h] [ebp-4h]

    if (keynum == -1)
        return "<KEY NOT FOUND>";
    if ((unsigned int)keynum >= 0x100)
        return "<OUT OF RANGE>";
    if (translate && (SEH_GetCurrentLanguage() == 1 || SEH_GetCurrentLanguage() == 2) && keynum >= 48 && keynum <= 57)
        return (const char *)*((_DWORD *)&keynames_localized[92].name + keynum);
    if (keynum > 32 && keynum < 127 && keynum != 34)
    {
        tinystr[0] = toupper(keynum);
        tinystr[1] = 0;
        if (keynum != 59 || translate)
            return tinystr;
    }
    if (translate)
    {
        if (!Key_IsValidGamePadChar(keynum))
        {
            kn = keynames_localized;
            goto LABEL_22;
        }
        tinystr[0] = keynum;
        tinystr[1] = 0;
        return tinystr;
    }
    else
    {
        kn = keynames;
    LABEL_22:
        while (kn->name)
        {
            if (keynum == kn->keynum)
                return kn->name;
            ++kn;
        }
        i = keynum >> 4;
        j = keynum & 0xF;
        tinystr[0] = 48;
        tinystr[1] = 120;
        if (keynum >> 4 <= 9)
            v4 = i + 48;
        else
            v4 = i + 87;
        tinystr[2] = v4;
        if ((keynum & 0xFu) <= 9)
            v3 = j + 48;
        else
            v3 = j + 87;
        tinystr[3] = v3;
        tinystr[4] = 0;
        return tinystr;
    }
}

void __cdecl Key_SetBinding(int localClientNum, int keynum, const char *binding, int bindNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    1578,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( keynum != -1 )
    {
        if ( bindNum )
            ReplaceString(
                &playerKeys[localClientNum].keys[keynum].binding2,
                (char*)binding,
                "Key_SetBinding",
                11,
                SCRIPTINSTANCE_SERVER);
        else
            ReplaceString(
                &playerKeys[localClientNum].keys[keynum].binding,
                (char*)binding,
                "Key_SetBinding",
                11,
                SCRIPTINSTANCE_SERVER);
        dvar_modifiedFlags |= 1u;
    }
}

void __cdecl Key_SetBindings(int localClientNum, int *twokeys, const char *binding, int bindNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    1604,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( *twokeys != -1 )
        Key_SetBinding(localClientNum, *twokeys, binding, bindNum);
    if ( twokeys[1] != -1 )
        Key_SetBinding(localClientNum, twokeys[1], binding, bindNum);
}

const char *__cdecl Key_GetBinding(int localClientNum, unsigned int keynum, int bindNum)
{
    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    1619,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    if ( keynum >= 0x100
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    1620,
                    0,
                    "keynum doesn't index MAX_KEYS\n\t%i not in [0, %i)",
                    keynum,
                    256) )
    {
        __debugbreak();
    }
    if ( bindNum )
        return playerKeys[localClientNum].keys[keynum].binding2;
    else
        return playerKeys[localClientNum].keys[keynum].binding;
}

int __cdecl Key_GetCommandAssignment(int localClientNum, const char *command, int *twokeys, int bindNum)
{
    return Key_GetCommandAssignmentInternal(localClientNum, command, twokeys, 0, bindNum);
}

int __cdecl Key_GetCommandAssignmentInternal(
                int localClientNum,
                const char *command,
                int *twokeys,
                int gamePadOnly,
                int bindNum)
{
    int keynum; // [esp+0h] [ebp-Ch]
    const char *binding; // [esp+4h] [ebp-8h]
    int count; // [esp+8h] [ebp-4h]

    twokeys[1] = -1;
    *twokeys = -1;
    count = 0;
    for ( keynum = 0; keynum < 256; ++keynum )
    {
        if ( gamePadOnly != 1 || Key_IsValidGamePadChar(keynum) )
        {
            binding = Key_GetBinding(localClientNum, keynum, bindNum);
            if ( binding
                && !I_stricmp(binding, command)
                && (gpad_enabled && gpad_enabled->current.enabled || !Key_IsValidGamePadChar(keynum)) )
            {
                twokeys[count++] = keynum;
                if ( count == 2 )
                    break;
            }
        }
    }
    return count;
}

bool __cdecl Key_IsCommandBound(int localClientNum, const char *command, int bindNum)
{
    int keys[2]; // [esp+0h] [ebp-Ch] BYREF
    int bindCount; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_KEY_BINDINGS);
    bindCount = Key_GetCommandAssignment(localClientNum, command, keys, bindNum);
    Sys_LeaveCriticalSection(CRITSECT_KEY_BINDINGS);
    return bindCount > 0;
}

void __cdecl Key_Unbind_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int b; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        b = Key_StringToKeynum(v0);
        if ( b == -1 )
        {
            v1 = Cmd_Argv(1);
            Com_Printf(0, "\"%s\" isn't a valid key\n", v1);
        }
        else
        {
            Key_SetBinding(0, b, (char *)"", 0);
        }
    }
    else
    {
        Com_Printf(0, "unbind <key> : remove commands from a key\n");
    }
}

void __cdecl Key_Unbind2_f()
{
    const char *v0; // eax
    const char *v1; // eax
    int b; // [esp+18h] [ebp-4h]

    if ( Cmd_Argc() == 2 )
    {
        v0 = Cmd_Argv(1);
        b = Key_StringToKeynum(v0);
        if ( b == -1 )
        {
            v1 = Cmd_Argv(1);
            Com_Printf(0, "\"%s\" isn't a valid key\n", v1);
        }
        else
        {
            Key_SetBinding(0, b, (char *)"", 1);
        }
    }
    else
    {
        Com_Printf(0, "unbind2 <key> : remove commands from a key\n");
    }
}

void __cdecl Key_Unbindall_f()
{
    int keynum; // [esp+0h] [ebp-Ch]

    for ( keynum = 0; keynum < 256; ++keynum )
    {
        if ( playerKeys[0].keys[keynum].binding )
            Key_SetBinding(0, keynum, (char *)"", 0);
        if ( playerKeys[0].keys[keynum].binding2 )
            Key_SetBinding(0, keynum, (char *)"", 1);
    }
}

void __cdecl Key_Unbindall2_f()
{
    int keynum; // [esp+0h] [ebp-Ch]

    for ( keynum = 0; keynum < 256; ++keynum )
    {
        if ( playerKeys[0].keys[keynum].binding2 )
            Key_SetBinding(0, keynum, (char *)"", 1);
    }
}

void __cdecl Key_Bind_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    unsigned __int8 *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    bool v7; // [esp+0h] [ebp-45Ch]
    int keynum; // [esp+3Ch] [ebp-420h]
    int keynuma; // [esp+3Ch] [ebp-420h]
    int i; // [esp+48h] [ebp-414h]
    const char *binding; // [esp+4Ch] [ebp-410h]
    int argc; // [esp+50h] [ebp-40Ch]
    char cmd[1028]; // [esp+54h] [ebp-408h] BYREF

    argc = Cmd_Argc();
    if ( argc >= 2 )
    {
        v0 = Cmd_Argv(1);
        keynum = Key_StringToKeynum(v0);
        if ( keynum == -1 )
        {
            v1 = Cmd_Argv(1);
            Com_Printf(0, "\"%s\" isn't a valid key\n", v1);
        }
        else
        {
            keynuma = tolower(keynum);
            if ( argc == 2 )
            {
                binding = Key_GetBinding(0, keynuma, 0);
                if ( binding )
                {
                    v2 = Cmd_Argv(1);
                    Com_Printf(0, "\"%s\" = \"%s\"\n", v2, binding);
                }
                else
                {
                    v3 = Cmd_Argv(1);
                    Com_Printf(0, "\"%s\" is not bound\n", v3);
                }
            }
            else
            {
                cmd[0] = 0;
                for ( i = 2; i < argc; ++i )
                {
                    v7 = 0;
                    if ( argc > 3 )
                    {
                        v4 = (unsigned __int8 *)Cmd_Argv(i);
                        v5 = strchr((char*)v4, 0x20u);
                        if ( v5 )
                            v7 = 1;
                    }
                    if ( v7 )
                        I_strncat(cmd, 1024, "\"");
                    v6 = Cmd_Argv(i);
                    I_strncat(cmd, 1024, v6);
                    if ( v7 )
                        I_strncat(cmd, 1024, "\"");
                    if ( i != argc - 1 )
                        I_strncat(cmd, 1024, " ");
                }
                Key_SetBinding(0, keynuma, cmd, 0);
            }
        }
    }
    else
    {
        Com_Printf(0, "bind <key> [command] : attach a command to a key\n");
    }
}

void __cdecl Key_Bind2_f()
{
    const char *v0; // eax
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    unsigned __int8 *v4; // eax
    char *v5; // eax
    const char *v6; // eax
    bool v7; // [esp+0h] [ebp-45Ch]
    int keynum; // [esp+3Ch] [ebp-420h]
    int keynuma; // [esp+3Ch] [ebp-420h]
    int i; // [esp+48h] [ebp-414h]
    const char *binding; // [esp+4Ch] [ebp-410h]
    int argc; // [esp+50h] [ebp-40Ch]
    char cmd[1028]; // [esp+54h] [ebp-408h] BYREF

    argc = Cmd_Argc();
    if ( argc >= 2 )
    {
        v0 = Cmd_Argv(1);
        keynum = Key_StringToKeynum(v0);
        if ( keynum == -1 )
        {
            v1 = Cmd_Argv(1);
            Com_Printf(0, "\"%s\" isn't a valid key\n", v1);
        }
        else
        {
            keynuma = tolower(keynum);
            if ( argc == 2 )
            {
                binding = Key_GetBinding(0, keynuma, 1);
                if ( binding )
                {
                    v2 = Cmd_Argv(1);
                    Com_Printf(0, "\"%s\" bind2 = \"%s\"\n", v2, binding);
                }
                else
                {
                    v3 = Cmd_Argv(1);
                    Com_Printf(0, "\"%s\" is not bound\n", v3);
                }
            }
            else
            {
                cmd[0] = 0;
                for ( i = 2; i < argc; ++i )
                {
                    v7 = 0;
                    if ( argc > 3 )
                    {
                        v4 = (unsigned __int8 *)Cmd_Argv(i);
                        v5 = strchr((char*)v4, 0x20u);
                        if ( v5 )
                            v7 = 1;
                    }
                    if ( v7 )
                        I_strncat(cmd, 1024, "\"");
                    v6 = Cmd_Argv(i);
                    I_strncat(cmd, 1024, v6);
                    if ( v7 )
                        I_strncat(cmd, 1024, "\"");
                    if ( i != argc - 1 )
                        I_strncat(cmd, 1024, " ");
                }
                Key_SetBinding(0, keynuma, cmd, 1);
            }
        }
    }
    else
    {
        Com_Printf(0, "bind2 <key> [command2]: attach a command to a key\n");
    }
}

char myTempBuff[4096];
void __cdecl Key_WriteBindingsToTempBuf(int localClientNum)
{
    const char *v1; // eax
    const char *v2; // eax
    const char *v3; // eax
    int bytesUsed; // [esp+0h] [ebp-38h]
    int bytesUseda; // [esp+0h] [ebp-38h]
    int bytesUsedb; // [esp+0h] [ebp-38h]
    const char *bind; // [esp+4h] [ebp-34h]
    const char *binda; // [esp+4h] [ebp-34h]
    KeyState *keys; // [esp+8h] [ebp-30h]
    int len; // [esp+10h] [ebp-28h]
    int lena; // [esp+10h] [ebp-28h]
    int lenb; // [esp+10h] [ebp-28h]
    int keyIndex; // [esp+14h] [ebp-24h]
    const char *dvars[6]; // [esp+18h] [ebp-20h]
    int i; // [esp+30h] [ebp-8h]
    const dvar_s *dvar; // [esp+34h] [ebp-4h]

    keys = playerKeys[localClientNum].keys;
    bytesUsed = 0;
    for (keyIndex = 0; keyIndex < 256; ++keyIndex)
    {
        if (keys[keyIndex].binding && *keys[keyIndex].binding || keys[keyIndex].binding2 && *keys[keyIndex].binding2)
        {
            if (keys[keyIndex].binding && *keys[keyIndex].binding)
            {
                v1 = Key_KeynumToString(keyIndex, 0);
                len = Com_sprintf(&myTempBuff[bytesUsed], 4092 - bytesUsed, "bind %s \"", v1);
                if (len < 0)
                    break;
                bytesUseda = len + bytesUsed;
                for (bind = keys[keyIndex].binding; *bind && bytesUseda < 4092; ++bind)
                {
                    if (*bind == 34)
                        myTempBuff[bytesUseda++] = 92;
                    myTempBuff[bytesUseda++] = *bind;
                }
                myTempBuff[bytesUseda] = 34;
                myTempBuff[bytesUseda + 1] = 10;
                bytesUsed = bytesUseda + 2;
            }
            if (keys[keyIndex].binding2 && *keys[keyIndex].binding2)
            {
                v2 = Key_KeynumToString(keyIndex, 0);
                lena = Com_sprintf(&myTempBuff[bytesUsed], 4092 - bytesUsed, "bind2 %s \"", v2);
                if (lena < 0)
                    break;
                bytesUsedb = lena + bytesUsed;
                for (binda = keys[keyIndex].binding2; *binda && bytesUsedb < 4092; ++binda)
                {
                    if (*binda == 34)
                        myTempBuff[bytesUsedb++] = 92;
                    myTempBuff[bytesUsedb++] = *binda;
                }
                myTempBuff[bytesUsedb] = 34;
                myTempBuff[bytesUsedb + 1] = 10;
                bytesUsed = bytesUsedb + 2;
            }
        }
    }
    dvars[0] = "sensitivity";
    dvars[1] = "cl_freelook";
    dvars[2] = "ui_mousePitch";
    dvars[3] = "m_pitch";
    dvars[4] = "m_filter";
    dvars[5] = "cl_mouseAccel";
    for (i = 0; (unsigned int)i < 6; ++i)
    {
        dvar = Dvar_FindVar(dvars[i]);
        if (dvar)
        {
            v3 = Dvar_DisplayableValue(dvar);
            lenb = Com_sprintf(&myTempBuff[bytesUsed], 4092 - bytesUsed, "set %s \"%s\"\n", dvar->name, v3);
            if (lenb < 0)
                break;
            bytesUsed += lenb;
        }
    }
    myTempBuff[bytesUsed] = 0;
}

void __cdecl Key_RestoreBindingsFromTempBuf(int localClientNum)
{
    Cbuf_AddText(localClientNum, myTempBuff);
    Cbuf_AddText(localClientNum, "\n");
}

void __cdecl Key_WriteBindings(int localClientNum, int f)
{
    char buffer[8196]; // [esp+0h] [ebp-2008h] BYREF

    Key_WriteBindingsToBuffer(localClientNum, buffer, 0x2000);
    FS_Printf(f, (char*)"%s", buffer);
}

int __cdecl Key_WriteBindingsToBuffer(int localClientNum, char *buffer, int bufferSize)
{
    const char *v3; // eax
    const char *v5; // eax
    int bytesUsed; // [esp+0h] [ebp-14h]
    int bytesUseda; // [esp+0h] [ebp-14h]
    int bytesUsedb; // [esp+0h] [ebp-14h]
    const char *bind; // [esp+4h] [ebp-10h]
    const char *binda; // [esp+4h] [ebp-10h]
    KeyState *keys; // [esp+8h] [ebp-Ch]
    int len; // [esp+Ch] [ebp-8h]
    int lena; // [esp+Ch] [ebp-8h]
    int keyIndex; // [esp+10h] [ebp-4h]
    int bufferSizea; // [esp+24h] [ebp+10h]

    keys = playerKeys[localClientNum].keys;
    bufferSizea = bufferSize - 4;
    bytesUsed = 0;
    for ( keyIndex = 0; keyIndex < 256; ++keyIndex )
    {
        if ( keys[keyIndex].binding && *keys[keyIndex].binding || keys[keyIndex].binding2 && *keys[keyIndex].binding2 )
        {
            if ( keys[keyIndex].binding && *keys[keyIndex].binding )
            {
                v3 = Key_KeynumToString(keyIndex, 0);
                len = Com_sprintf(&buffer[bytesUsed], bufferSizea - bytesUsed, "bind %s \"", v3);
                if ( len < 0 )
                    return bytesUsed;
                bytesUseda = len + bytesUsed;
                for ( bind = keys[keyIndex].binding; *bind && bytesUseda < bufferSizea; ++bind )
                {
                    if ( *bind == 34 )
                        buffer[bytesUseda++] = 92;
                    buffer[bytesUseda++] = *bind;
                }
                buffer[bytesUseda] = 34;
                buffer[bytesUseda + 1] = 10;
                bytesUsed = bytesUseda + 2;
            }
            if ( keys[keyIndex].binding2 && *keys[keyIndex].binding2 )
            {
                v5 = Key_KeynumToString(keyIndex, 0);
                lena = Com_sprintf(&buffer[bytesUsed], bufferSizea - bytesUsed, "bind2 %s \"", v5);
                if ( lena < 0 )
                    return bytesUsed;
                bytesUsedb = lena + bytesUsed;
                for ( binda = keys[keyIndex].binding2; *binda && bytesUsedb < bufferSizea; ++binda )
                {
                    if ( *binda == 34 )
                        buffer[bytesUsedb++] = 92;
                    buffer[bytesUsedb++] = *binda;
                }
                buffer[bytesUsedb] = 34;
                buffer[bytesUsedb + 1] = 10;
                bytesUsed = bytesUsedb + 2;
            }
        }
    }
    buffer[bytesUsed] = 0;
    return bytesUsed;
}

void __cdecl Key_Bindlist_f()
{
    const char *v0; // eax
    int keynum; // [esp+0h] [ebp-Ch]
    const char *binding; // [esp+8h] [ebp-4h]

    for ( keynum = 0; keynum < 256; ++keynum )
    {
        binding = Key_GetBinding(0, keynum, 0);
        if ( binding )
        {
            if ( *binding )
            {
                v0 = Key_KeynumToString(keynum, 0);
                Com_Printf(0, "%s \"%s\"\n", v0, binding);
            }
        }
    }
}

cmd_function_s Key_Bind_f_VAR;
cmd_function_s Key_Bind2_f_VAR;
cmd_function_s Key_Unbind_f_VAR;
cmd_function_s Key_Unbind2_f_VAR;
cmd_function_s Key_Unbindall_f_VAR;
cmd_function_s Key_Unbindall2_f_VAR;
cmd_function_s Key_Bindlist_f_VAR;

void __cdecl CL_InitKeyCommands()
{
    Cmd_AddCommandInternal("bind", Key_Bind_f, &Key_Bind_f_VAR);
    Cmd_AddCommandInternal("bind2", Key_Bind2_f, &Key_Bind2_f_VAR);
    Cmd_AddCommandInternal("unbind", Key_Unbind_f, &Key_Unbind_f_VAR);
    Cmd_AddCommandInternal("unbind2", Key_Unbind2_f, &Key_Unbind2_f_VAR);
    Cmd_AddCommandInternal("unbindall", Key_Unbindall_f, &Key_Unbindall_f_VAR);
    Cmd_AddCommandInternal("unbindall2", Key_Unbindall2_f, &Key_Unbindall2_f_VAR);
    Cmd_AddCommandInternal("bindlist", Key_Bindlist_f, &Key_Bindlist_f_VAR);
}

// mod: bo1_mod_quickrestart. A key bound to +reload pressed after game over goes to SP_QuickRestart (the same
// function as the console command bo1_quickrestart); true = restarted, the key's binding is not run.
static bool CL_ModQuickRestartKey(int localClientNum, int key)
{
    const char *binding = playerKeys[localClientNum].keys[key].binding;
    const char *binding2 = playerKeys[localClientNum].keys[key].binding2;

    if ( !zombiemode || !zombiemode->current.enabled )
        return false;
    if ( !(binding && !I_stricmp(binding, "+reload")) && !(binding2 && !I_stricmp(binding2, "+reload")) )
        return false;
    return SP_QuickRestart(false);
}

// zombies mouse-1: SP CL_KeyEvent's routing for a focused 3D menu (UI_IsFocusedMenu3D, SP 0x0069AA20: the front end's TV
// menus). These menus have no cursor, so retail scrolls them with the wheel and selects with a left click by turning the
// mouse keys into menu keys before UI_KeyEvent.
static bool CL_KeyIsMoveBinding(const char *binding)
{
    return binding && (!I_stricmp(binding, "+forward") || !I_stricmp(binding, "+back") || !I_stricmp(binding, "+moveleft")
        || !I_stricmp(binding, "+moveright"));
}

// SP 0x007A08A0: true = the key skips the 3D menu and runs its game binding. Enter, the arrow keys (154-157), 191,
// mouse 1/2 and the wheel stay with the menu, and so do keys bound to the four move commands.
static bool CL_Key3DMenuShouldBypass(int localClientNum, int key, const KeyState *keyState)
{
    if ( !UI_IsFocusedMenu3D(localClientNum) )
        return false;
    switch ( key )
    {
    case 13:
    case 154:
    case 155:
    case 156:
    case 157:
    case 191:
    case 200:
    case 201:
    case 205:
    case 206:
        return false;
    default:
        return !CL_KeyIsMoveBinding(keyState->binding);
    }
}

// SP 0x007A0A00: while the key is down, mouse 1 -> Enter, mouse 2 -> Escape, wheel down / up -> down / up arrow, and a key
// bound to +forward / +back / +moveright / +moveleft -> up / down / Enter / Escape.
static int CL_Key3DMenuTranslate(int localClientNum, int key, const KeyState *keyState)
{
    if ( !UI_IsFocusedMenu3D(localClientNum) )
        return key;
    if ( key == 200 && keyState->down )
        return 13;
    if ( key == 201 && keyState->down )
        return 27;
    if ( key == 205 && keyState->down )
        return 155;
    if ( key == 206 && keyState->down )
        return 154;
    if ( !keyState->binding )
        return key;
    if ( !I_stricmp(keyState->binding, "+forward") )
        return 154;
    if ( !I_stricmp(keyState->binding, "+back") )
        return 155;
    if ( !I_stricmp(keyState->binding, "+moveright") )
        return 13;
    if ( !I_stricmp(keyState->binding, "+moveleft") )
        return 27;
    return key;
}

void __cdecl CL_KeyEvent(int localClientNum, int key, int down, unsigned int time)
{
    const char *v4; // eax
    const char *v5; // eax
    bool v7; // [esp+93h] [ebp-425h]
    clientUIActive_t *LocalClientUIGlobals; // [esp+94h] [ebp-424h]
    KeyState *keys; // [esp+98h] [ebp-420h]
    cg_s *cgameGlob; // [esp+9Ch] [ebp-41Ch]
    const char *kb; // [esp+A0h] [ebp-418h]
    const char *kba; // [esp+A0h] [ebp-418h]
    const char *kbb; // [esp+A0h] [ebp-418h]
    const char *kbc; // [esp+A0h] [ebp-418h]
    const char *kbd; // [esp+A0h] [ebp-418h]
    connstate_t clcState; // [esp+A8h] [ebp-410h]
    LocSelInputState *locSelInputState; // [esp+ACh] [ebp-40Ch]
    char cmd[1028]; // [esp+B0h] [ebp-408h] BYREF

    keys = playerKeys[localClientNum].keys;
    kb = keys[key].binding;
    keys[key].down = down;
    if ( down )
    {
        if ( ++keys[key].repeats == 1 )
            ++playerKeys[localClientNum].anyKeyDown;
    }
    else
    {
        keys[key].repeats = 0;
        if ( --playerKeys[localClientNum].anyKeyDown < 0 )
            playerKeys[localClientNum].anyKeyDown = 0;
    }
    CG_SP_PlayTraceKey(key, down, keys[key].repeats, kb); // j1: bo1_playtrace F8 marker (tool, off by default)
    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
    if ( key == 96 || key == 126 || (LocalClientUIGlobals->keyCatchers & 1) != 0 )
    {
        if ( DevGui_IsActive() )
            DevGui_Toggle();
    }
    else
    {
        if ( kb && !I_stricmp(kb, "devgui") && down && keys[key].repeats == 1 )
        {
            Cbuf_AddText(localClientNum, keys[key].binding);
            Cbuf_AddText(localClientNum, "\n");
            return;
        }
        if ( DevGui_IsActive() )
        {
            if ( down )
            {
                if ( keys[key].repeats == 1 )
                    DevGui_KeyPressed(key);
            }
            return;
        }
    }
    if ( !down || keys[key].repeats <= 1 )
    {
LABEL_32:
        if ( !con_restricted->current.enabled || (LocalClientUIGlobals->keyCatchers & 1) != 0 )
        {
            if ( key == 96 || key == 126 )
            {
                if ( !down )
                    return;
                if ( (LocalClientUIGlobals->keyCatchers & 1) == 0
                    && !com_sv_running->current.enabled
                    && sv_disableClientConsole->current.enabled )
                {
                    return;
                }
                if ( keys[160].down )
                {
                    if ( !Con_IsActive(localClientNum) )
                        Con_ToggleConsole();
                    Con_ToggleConsoleOutput();
                    return;
                }
LABEL_37:
                Con_ToggleConsole();
                return;
            }
        }
        else
        {
            if ( key == 165 && down && keys[127].down )
                goto LABEL_37;
            if ( key == 96 || key == 126 )
                return;
        }
        cgameGlob = CG_GetLocalClientGlobals(localClientNum);
        locSelInputState = &playerKeys[localClientNum].locSelInputState;
        if ( (LocalClientUIGlobals->keyCatchers & 8) != 0 && down > 0 )
        {
            if ( key == 27 )
            {
                *locSelInputState = LOC_SEL_INPUT_CANCEL;
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "+attack") )
            {
                if ( cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_NAPALM
                    || cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_AIRSTRIKE )
                {
                    *locSelInputState = LOC_SEL_INPUT_YAW;
                }
                else
                {
                    *locSelInputState = LOC_SEL_INPUT_CONFIRM;
                }
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "+actionslot 1") )
            {
                *locSelInputState = LOC_SEL_INPUT_CANCEL;
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "weapnext") )
            {
                *locSelInputState = LOC_SEL_INPUT_REGROUP;
            }
            else if ( keys[key].binding && !strcmp(keys[key].binding, "+reload") )
            {
                *locSelInputState = LOC_SEL_INPUT_DEFEND;
            }
            else if ( keys[key].binding
                         && (!strcmp(keys[key].binding, "+sprint") || !strcmp(keys[key].binding, "+breath_sprint")) )
            {
                *locSelInputState = LOC_SEL_INPUT_SQUAD_CANCEL;
            }
        }
        else
        {
            if ( (LocalClientUIGlobals->keyCatchers & 8) != 0 && !down && *locSelInputState == LOC_SEL_INPUT_YAW )
            {
                if ( cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_NAPALM
                    || cgameGlob->predictedPlayerState.locationSelectionType == LOC_SEL_AIRSTRIKE )
                {
                    *locSelInputState = LOC_SEL_INPUT_CONFIRM;
                }
            }
            else
            {
                *locSelInputState = LOC_SEL_INPUT_NONE;
            }
            clcState = CL_GetLocalClientConnectionState(localClientNum);
            if ( down )
            {
                v7 = key == 200 || key < 128;
                if ( v7
                    && (clcState >= CA_CONNECTING && CL_GetLocalClientConnection(localClientNum)->demoplaying
                     || clcState == CA_CINEMATIC
                     || clcState == CA_LOGO)
                    && !LocalClientUIGlobals->keyCatchers )
                {
                    Dvar_SetString((dvar_s *)nextdemo, "");
                    key = 27;
                }
            }
            if ( key == 27 && down )
            {
                if ( (LocalClientUIGlobals->keyCatchers & 0x20) != 0 )
                {
                    Message_Key(localClientNum, 27);
                }
                else
                {
                    if ( (LocalClientUIGlobals->keyCatchers & 1) != 0 )
                        Con_CancelAutoComplete();
                    if ( (LocalClientUIGlobals->keyCatchers & 0x100) == 0 )
                    {
                        if ( (LocalClientUIGlobals->keyCatchers & 0x50) != 0 )
                        {
                            UI_KeyEvent(localClientNum, 27, down);
                        }
                        else
                        {
                            switch ( clcState )
                            {
                                case CA_CINEMATIC:
                                case CA_LOGO:
                                    CL_StopLogoOrCinematic(localClientNum);
                                    break;
                                case CA_CONNECTING:
                                case CA_CHALLENGING:
                                case CA_CONNECTED:
                                    if ( !com_sv_running->current.enabled )
                                    {
                                        Com_Printf(14, "Client cancelled connecting to server\n");
                                        CL_DisconnectLocalClient(localClientNum);
                                    }
                                    break;
                                case CA_ACTIVE:
                                    if ( CL_GetLocalClientConnection(localClientNum)->demoplaying )
                                    {
                                        CL_DisconnectLocalClient(localClientNum);
                                    }
                                    else
                                    {
                                        if ( cl_waitingOnServerToLoadMap[localClientNum] )
                                            goto LABEL_110;
                                        UI_SetActiveMenu(localClientNum, UIMENU_INGAME);
                                    }
                                    break;
                                default:
                                    if ( cls.uiStarted )
                                        Com_Error(ERR_DISCONNECT, "Bad server address");
LABEL_110:
                                    UI_SetActiveMenu(localClientNum, UIMENU_MAIN);
                                    break;
                            }
                        }
                    }
                }
            }
            else if ( !CL_LocalClient_IsCUIFlagSet(localClientNum, 32)
                         || !CG_IsScoreboardDisplayed(localClientNum)
                         || !down
                         || !Scoreboard_HandleInput(localClientNum, key) )
            {
                if ( (LocalClientUIGlobals->keyCatchers & 1) == 0 && (LocalClientUIGlobals->keyCatchers & 0x10) == 0 )
                    Demo_HandleInput(localClientNum, 0, key, down);
                if ( down )
                {
                    if ( (LocalClientUIGlobals->keyCatchers & 1) != 0 )
                    {
                        Console_Key(localClientNum, key);
                    }
                    else if ( (LocalClientUIGlobals->keyCatchers & 0x100) == 0 )
                    {
                        if ( (LocalClientUIGlobals->keyCatchers & 0x50) == 0 || CL_MouseInputShouldBypassMenus(localClientNum, key)
                            || CL_Key3DMenuShouldBypass(localClientNum, key, &keys[key]) ) // zombies mouse-1 (SP 0x00587BA8)
                        {
                            if ( (LocalClientUIGlobals->keyCatchers & 0x20) != 0 )
                            {
                                Message_Key(localClientNum, key);
                            }
                            // mod: bo1_mod_quickrestart - the reload key after game over restarts the map
                            else if ( clcState && !CL_ModQuickRestartKey(localClientNum, key) )
                            {
                                kbc = keys[key].binding;
                                if ( kbc )
                                {
                                    if ( *kbc == 43 )
                                    {
                                        Com_sprintf(cmd, 0x400u, "%s %i %i\n", kbc, key, time);
                                        Cbuf_AddText(localClientNum, cmd);
                                    }
                                    else
                                    {
                                        Cbuf_AddText(localClientNum, kbc);
                                        Cbuf_AddText(localClientNum, "\n");
                                    }
                                }
                                else if ( key >= 207 )
                                {
                                    v4 = Key_KeynumToString(key, 0);
                                    Com_Printf(14, "%s is unbound, use controls menu to set.\n", v4);
                                }
                                kbd = keys[key].binding2;
                                if ( kbd )
                                {
                                    if ( *kbd == 43 )
                                    {
                                        Com_sprintf(cmd, 0x400u, "%s %i %i\n", kbd, key, time);
                                        Cbuf_AddText(localClientNum, cmd);
                                    }
                                    else
                                    {
                                        Cbuf_AddText(localClientNum, kbd);
                                        Cbuf_AddText(localClientNum, "\n");
                                    }
                                }
                                else if ( key >= 207 )
                                {
                                    v5 = Key_KeynumToString(key, 0);
                                    Com_Printf(14, "%s is unbound, use controls menu to set.\n", v5);
                                }
                            }
                            else
                            {
                                Console_Key(localClientNum, key);
                            }
                        }
                        else if ( cls.uiStarted )
                        {
                            // zombies mouse-1: SP translates the key for a focused 3D menu first (SP 0x00587BF6)
                            UI_KeyEvent(localClientNum, CL_Key3DMenuTranslate(localClientNum, key, &keys[key]), down);
                        }
                        if ( cgameGlob
                            && (cgameGlob->predictedPlayerState.pm_type == 4 || (cgameGlob->predictedPlayerState.otherFlags & 2) != 0) )
                        {
                            CL_HandleKeySpectatorInput(localClientNum, key);
                        }
                    }
                }
                else
                {
                    kba = keys[key].binding;
                    if ( kba && *kba == 43 )
                    {
                        Com_sprintf(cmd, 0x400u, "-%s %i %i\n", kba + 1, key, time);
                        Cbuf_AddText(localClientNum, cmd);
                    }
                    kbb = keys[key].binding2;
                    if ( kbb && *kbb == 43 )
                    {
                        Com_sprintf(cmd, 0x400u, "-%s %i %i\n", kbb + 1, key, time);
                        Cbuf_AddText(localClientNum, cmd);
                    }
                    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 && cls.uiStarted )
                        UI_KeyEvent(localClientNum, key, 0);
                }
            }
        }
        return;
    }
    if ( (LocalClientUIGlobals->keyCatchers & 0x121) != 0 )
    {
LABEL_28:
        if ( key == 96 || key == 126 || key == 27 )
            return;
        goto LABEL_32;
    }
    if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 )
    {
        switch ( key )
        {
            case 154:
            case 155:
            case 163:
            case 164:
                goto LABEL_28;
            default:
                return;
        }
    }
}

void __cdecl Message_Key(int localClientNum, int key)
{
    clientUIActive_t *LocalClientUIGlobals; // [esp+0h] [ebp-414h]
    char buffer[1028]; // [esp+4h] [ebp-410h] BYREF
    field_t *chatField; // [esp+40Ch] [ebp-8h]
    connstate_t clcState; // [esp+410h] [ebp-4h]

    chatField = &playerKeys[localClientNum].chatField;
    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
    if ( key == 27 )
    {
        LocalClientUIGlobals->keyCatchers &= ~0x20u;
        Field_Clear(chatField);
    }
    else if ( key == 13 || key == 191 )
    {
        clcState = CL_GetLocalClientConnectionState(localClientNum);
        if ( chatField->buffer[0] && clcState == CA_ACTIVE )
        {
            if ( playerKeys[localClientNum].chat_team )
                Com_sprintf(buffer, 0x400u, "say_team \"%s\"", chatField->buffer);
            else
                Com_sprintf(buffer, 0x400u, "say \"%s\"", chatField->buffer);
            CL_AddReliableCommand(localClientNum, buffer);
        }
        LocalClientUIGlobals->keyCatchers &= ~0x20u;
        Field_Clear(chatField);
    }
    else
    {
        Field_KeyDownEvent(localClientNum, &scrPlaceView[localClientNum], chatField, key);
    }
}

bool __cdecl CL_MouseInputShouldBypassMenus(int localClientNum, int key)
{
    if ( UI_GetActiveMenu(localClientNum) == UIMENU_SCOREBOARD )
        return 1;
    if ( !cl_bypassMouseInput || !cl_bypassMouseInput->current.enabled )
        return 0;
    if ( key == 200 || key == 201 || key == 202 )
        return 1;
    return cls.uiStarted && !UI_CheckExecKey(localClientNum, key);
}

void __cdecl CL_HandleKeySpectatorInput(int localClientNum, int key)
{
    const char *v2; // eax
    const char *v3; // eax
    const char *v4; // eax
    const cg_s *cgameGlob; // [esp+0h] [ebp-4h]

    cgameGlob = CG_GetLocalClientGlobals(localClientNum);
    if ( cgameGlob->predictedPlayerState.pm_type != 4
        && (cgameGlob->predictedPlayerState.otherFlags & 2) == 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    2178,
                    0,
                    "%s",
                    "(cgameGlob->predictedPlayerState.pm_type == PM_SPECTATOR) || (cgameGlob->predictedPlayerState.otherFlags & POF_FOLLOW)") )
    {
        __debugbreak();
    }
    switch ( key )
    {
        case 200:
            v2 = va("+specNext %i\n-specNext %i\n", 200, 200);
            Cbuf_AddText(localClientNum, v2);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "CycleNext", "Flash", 1);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "SpectatorName", "Flash", 1);
            UI_AnimateItem(
                localClientNum,
                &cgDC[localClientNum],
                "spectatingPlayerCard",
                "spectatingPlayerSlide",
                "StartLeft",
                1);
            break;
        case 201:
            v3 = va("+specPrev %i\n-specPrev %i\n", 201, 201);
            Cbuf_AddText(localClientNum, v3);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "CyclePrev", "Flash", 1);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "SpectatorName", "Flash", 1);
            UI_AnimateItem(
                localClientNum,
                &cgDC[localClientNum],
                "spectatingPlayerCard",
                "spectatingPlayerSlide",
                "StartRight",
                1);
            break;
        case 32:
            v4 = va("+toggleSpec %i\n-toggleSpec %i\n", 32, 32);
            Cbuf_AddText(localClientNum, v4);
            UI_AnimateItem(localClientNum, &cgDC[localClientNum], "hud_spectator", "SpecViewToggle", "Flash", 1);
            break;
    }
}

void __cdecl CL_CharEvent(int localClientNum, int key)
{
    clientUIActive_t *LocalClientUIGlobals; // [esp+0h] [ebp-8h]

    if ( DevGui_IsActive() || key == 96 || key == 126 )
        return;
    LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
    if ( (LocalClientUIGlobals->keyCatchers & 1) != 0 )
    {
        if ( key == 8 && Con_CancelAutoComplete() )
            return;
LABEL_15:
        CL_ConsoleCharEvent(localClientNum, key);
        return;
    }
    if ( (LocalClientUIGlobals->keyCatchers & 0x20) != 0 )
    {
        Field_CharEvent(localClientNum, &scrPlaceView[localClientNum], &playerKeys[localClientNum].chatField, key);
        return;
    }
    if ( (LocalClientUIGlobals->keyCatchers & 0x100) == 0 )
    {
        if ( (LocalClientUIGlobals->keyCatchers & 0x10) != 0 )
        {
            UI_KeyEvent(localClientNum, key | 0x400, 1);
            return;
        }
        if ( !CL_GetLocalClientConnectionState(localClientNum) )
            goto LABEL_15;
    }
}

void __cdecl CL_ConsoleCharEvent(int localClientNum, int key)
{
    if ( Field_CharEvent(localClientNum, &scrPlaceFull, &g_consoleField, key) )
        Con_AllowAutoCompleteCycling(1);
}

void __cdecl Key_ClearStates(int localClientNum)
{
    int ControllerIndex; // eax
    int keynum; // [esp+0h] [ebp-8h]
    KeyState *keys; // [esp+4h] [ebp-4h]

    playerKeys[localClientNum].anyKeyDown = 0;
    keys = playerKeys[localClientNum].keys;
    for ( keynum = 0; keynum < 256; ++keynum )
    {
        if ( keys[keynum].down )
        {
            ControllerIndex = Com_LocalClient_GetControllerIndex(localClientNum);
            CL_GamepadButtonEvent(localClientNum, ControllerIndex, keynum, 0, 0, GPAD_NONE);
        }
        keys[keynum].down = 0;
        keys[keynum].repeats = 0;
    }
}

int __cdecl CL_GetKeyBinding(int localClientNum, const char *command, char (*keyNames)[128], int bindNum)
{
    return CL_GetKeyBindingInternal(localClientNum, command, keyNames, 0, bindNum);
}

int __cdecl CL_GetKeyBindingInternal(
                int localClientNum,
                const char *command,
                char (*keyNames)[128],
                int gamePadOnly,
                int bindNum)
{
    int keys[2]; // [esp+0h] [ebp-Ch] BYREF
    int bindCount; // [esp+8h] [ebp-4h]

    Sys_EnterCriticalSection(CRITSECT_KEY_BINDINGS);
    (*keyNames)[128] = 0;
    bindCount = Key_GetCommandAssignmentInternal(localClientNum, command, keys, gamePadOnly, bindNum);
    if ( (unsigned int)bindCount > 2
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    2847,
                    0,
                    "bindCount not in [0, 2]\n\t%i not in [%i, %i]",
                    bindCount,
                    0,
                    2) )
    {
        __debugbreak();
    }
    if ( bindCount )
    {
        Key_KeynumToStringBuf(keys[0], (char *)keyNames, 128);
        if ( bindCount == 2 )
            Key_KeynumToStringBuf(keys[1], &(*keyNames)[128], 128);
    }
    else
    {
        strcpy((char *)keyNames, "KEY_UNBOUND");
    }
    Sys_LeaveCriticalSection(CRITSECT_KEY_BINDINGS);
    return bindCount;
}

int __cdecl CL_GetGamePadBinding(int localClientNum, const char *command, char (*keyNames)[128], int bindNum)
{
    return CL_GetKeyBindingInternal(localClientNum, command, keyNames, 1, bindNum);
}

int __cdecl CL_IsKeyPressed(int localClientNum, const char *keyName)
{
    int key; // [esp+0h] [ebp-4h]

    if ( localClientNum
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                    2881,
                    0,
                    "localClientNum doesn't index MAX_LOCAL_CLIENTS\n\t%i not in [0, %i)",
                    localClientNum,
                    1) )
    {
        __debugbreak();
    }
    key = Key_StringToKeynum(keyName);
    if ( key < 0 )
        return 0;
    if ( localClientNum )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                        2888,
                        0,
                        "%s\n\t(localClientNum) = %i",
                        "(localClientNum >= 0 && localClientNum < 1)",
                        localClientNum) )
            __debugbreak();
    }
    return playerKeys[localClientNum].keys[key].down;
}

void __cdecl Key_Shutdown()
{
    int client; // [esp+0h] [ebp-8h]
    int keynum; // [esp+4h] [ebp-4h]

    for ( client = 0; client < 1; ++client )
    {
        for ( keynum = 0; keynum < 256; ++keynum )
        {
            if ( playerKeys[client].keys[keynum].binding )
            {
                FreeString(playerKeys[client].keys[keynum].binding, 11, SCRIPTINSTANCE_SERVER);
                playerKeys[client].keys[keynum].binding = 0;
            }
            if ( playerKeys[client].keys[keynum].binding2 )
            {
                FreeString(playerKeys[client].keys[keynum].binding2, 11, SCRIPTINSTANCE_SERVER);
                playerKeys[client].keys[keynum].binding2 = 0;
            }
        }
    }
}

bool __cdecl Key_IsCatcherActive(int localClientNum, int mask)
{
    return localClientNum != -1 && (mask & CL_GetLocalClientUIGlobals(localClientNum)->keyCatchers) != 0;
}

void __cdecl Key_AddCatcher(int localClientNum, int orMask)
{
    clientUIActive_t *LocalClientUIGlobals; // [esp+0h] [ebp-4h]

    if ( localClientNum != -1 )
    {
        LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
        LocalClientUIGlobals->keyCatchers |= orMask;
    }
}

void __cdecl Key_RemoveCatcher(int localClientNum, int andMask)
{
    clientUIActive_t *LocalClientUIGlobals; // eax

    if ( localClientNum != -1 )
    {
        if ( (andMask & (andMask - 1)) == 0
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\client\\cl_keys.cpp",
                        2965,
                        0,
                        "%s",
                        "!IsPowerOf2( andMask )") )
        {
            __debugbreak();
        }
        LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
        LocalClientUIGlobals->keyCatchers &= andMask;
        if ( (LocalClientUIGlobals->keyCatchers & 0x10) == 0 )
            CL_LocalClient_ClearCUIFlag(localClientNum, 64);
    }
}

void __cdecl Key_SetCatcher(int localClientNum, int catcher)
{
    clientUIActive_t *LocalClientUIGlobals; // [esp+0h] [ebp-4h]

    if ( localClientNum != -1 )
    {
        LocalClientUIGlobals = CL_GetLocalClientUIGlobals(localClientNum);
        if ( (LocalClientUIGlobals->keyCatchers & 1) != 0 )
            LocalClientUIGlobals->keyCatchers = catcher | 1;
        else
            LocalClientUIGlobals->keyCatchers = catcher;
        if ( (LocalClientUIGlobals->keyCatchers & 0x10) == 0 )
            CL_LocalClient_ClearCUIFlag(localClientNum, 64);
    }
}

void __cdecl Key_ContextIndex_SetCatcher(int contextIndex, int catcher)
{
    int clientNum; // [esp+0h] [ebp-4h]

    for ( clientNum = 0; clientNum < 1; ++clientNum )
    {
        if ( Com_LocalClient_GetUIContextIndex(clientNum) == contextIndex )
            Key_SetCatcher(clientNum, catcher);
    }
}

