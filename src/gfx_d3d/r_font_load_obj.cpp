#include "r_font_load_obj.h"
#include "r_material.h"
#include <universal/com_files.h>
#include <universal/com_memory.h>
#include <qcommon/common.h>

Font_s *__cdecl R_LoadFont(const char *fontName, int imageTrack)
{
    Material *v3; // eax
    char glowMaterialName[64]; // [esp+0h] [ebp-68h] BYREF
    Font_s *font; // [esp+44h] [ebp-24h]
    int totalMemSize; // [esp+48h] [ebp-20h]
    int file; // [esp+4Ch] [ebp-1Ch] BYREF
    int materialNameOffset; // [esp+50h] [ebp-18h] BYREF
    const char *materialName; // [esp+54h] [ebp-14h]
    int fileLen; // [esp+58h] [ebp-10h]
    int varMemSize; // [esp+5Ch] [ebp-Ch]
    int fontNameOffset; // [esp+60h] [ebp-8h] BYREF
    char *variableFontData; // [esp+64h] [ebp-4h]

    fileLen = FS_FOpenFileReadCurrentThread(fontName, &file);
    if ( fileLen >= 0 )
    {
        if ( fileLen >= 16 )
        {
            varMemSize = fileLen - 16;
            totalMemSize = fileLen - 16 + 24;
            font = (Font_s *)Hunk_Alloc(totalMemSize, "R_LoadFont", 23);
            variableFontData = (char *)&font[1];
            FS_Read((unsigned __int8 *)&fontNameOffset, 4u, file);
            FS_Read((unsigned __int8 *)&font->pixelHeight, 4u, file);
            FS_Read((unsigned __int8 *)&font->glyphCount, 4u, file);
            FS_Read((unsigned __int8 *)&materialNameOffset, 4u, file);
            FS_Read((unsigned __int8 *)variableFontData, varMemSize, file);
            FS_FCloseFile(file);
            font->glyphs = (Glyph *)variableFontData;
            font->fontName = &variableFontData[fontNameOffset - 16];
            materialName = &variableFontData[materialNameOffset - 16];
            font->material = Material_RegisterHandle((char *)materialName, imageTrack);
            Com_sprintf(glowMaterialName, 0x3Fu, "%s_glow", materialName);
            v3 = Material_RegisterHandle(glowMaterialName, imageTrack);
            font->glowMaterial = v3;
            return font;
        }
        else
        {
            FS_FCloseFile(file);
            Com_PrintError(8, "ERROR: Font file '%s' too small\n", fontName);
            return 0;
        }
    }
    else
    {
        if ( fileLen == -2 )
            Com_PrintError(8, "ERROR: Couldn't find font in iwd files or localized directories '%s'\n", fontName);
        else
            Com_PrintError(8, "ERROR: Couldn't find font '%s'\n", fontName);
        return 0;
    }
}

