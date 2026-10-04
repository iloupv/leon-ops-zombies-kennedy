#include "r_image_load_common.h"
#include <universal/q_shared.h>
#include "r_init.h"
#include "r_dvars.h"
#include "rb_logfile.h"
#include <qcommon/common.h>

void __cdecl Image_TrackFullscreenTexture(
                GfxImage *image,
                int fullscreenWidth,
                int fullscreenHeight,
                int picmip,
                _D3DFORMAT format)
{
    unsigned int memory; // [esp+0h] [ebp-18h]
    unsigned int platformHeight; // [esp+4h] [ebp-14h]
    unsigned __int16 width; // [esp+8h] [ebp-10h] BYREF
    unsigned __int16 height; // [esp+Ch] [ebp-Ch] BYREF
    int platformWidth; // [esp+10h] [ebp-8h]
    int platform; // [esp+14h] [ebp-4h]

    for ( platform = 0; platform < 2; ++platform )
    {
        platformWidth = Image_GetPlatformScreenWidth(platform, fullscreenWidth);
        platformHeight = Image_GetPlatformScreenHeight(platform, fullscreenHeight);
        Image_GetMipmapResolution(platformWidth, platformHeight, picmip, &width, &height);
        memory = Image_GetCardMemoryAmount(3, format, width, height, 1u);
        if ( !useFastFile->current.enabled )
            Image_TrackTotalMemory(image, platform, memory);
        image->cardMemory.platform[platform] = memory;
    }
}

void __cdecl Image_GetMipmapResolution(
                unsigned int baseWidth,
                unsigned int baseHeight,
                int mipmap,
                unsigned __int16 *mipWidth,
                unsigned __int16 *mipHeight)
{
    unsigned int v5; // [esp+0h] [ebp-10h]
    unsigned int v6; // [esp+4h] [ebp-Ch]

    if ( (int)baseWidth <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    37,
                    0,
                    "%s\n\t(baseWidth) = %i",
                    "(baseWidth > 0)",
                    baseWidth) )
    {
        __debugbreak();
    }
    if ( (int)baseHeight <= 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    38,
                    0,
                    "%s\n\t(baseHeight) = %i",
                    "(baseHeight > 0)",
                    baseHeight) )
    {
        __debugbreak();
    }
    if ( mipmap < 0
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    39,
                    0,
                    "%s\n\t(mipmap) = %i",
                    "(mipmap >= 0)",
                    mipmap) )
    {
        __debugbreak();
    }
    if ( !mipWidth
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 40, 0, "%s", "mipWidth") )
    {
        __debugbreak();
    }
    if ( !mipHeight
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    41,
                    0,
                    "%s",
                    "mipHeight") )
    {
        __debugbreak();
    }
    if ( (int)(baseWidth >> mipmap) > 1 )
        v6 = baseWidth >> mipmap;
    else
        LOWORD(v6) = 1;
    *mipWidth = v6;
    if ( (int)(baseHeight >> mipmap) > 1 )
        v5 = baseHeight >> mipmap;
    else
        LOWORD(v5) = 1;
    *mipHeight = v5;
    if ( !*mipWidth
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    45,
                    1,
                    "%s\n\t(*mipWidth) = %i",
                    "(*mipWidth > 0)",
                    *mipWidth) )
    {
        __debugbreak();
    }
    if ( !*mipHeight
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    46,
                    1,
                    "%s\n\t(*mipHeight) = %i",
                    "(*mipHeight > 0)",
                    *mipHeight) )
    {
        __debugbreak();
    }
}

int __cdecl Image_GetPlatformScreenWidth(int platform, int screenWidth)
{
    if ( platform == 1 )
        return 640;
    if ( platform )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        56,
                        0,
                        "%s\n\t(platform) = %i",
                        "(platform == PICMIP_PLATFORM_USED)",
                        platform) )
            __debugbreak();
    }
    return screenWidth;
}

int __cdecl Image_GetPlatformScreenHeight(int platform, int screenHeight)
{
    if ( platform == 1 )
        return 480;
    if ( platform )
    {
        if ( !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        67,
                        0,
                        "%s\n\t(platform) = %i",
                        "(platform == PICMIP_PLATFORM_USED)",
                        platform) )
            __debugbreak();
    }
    return screenHeight;
}

void __cdecl Image_TrackTexture(GfxImage *image, char imageFlags, _D3DFORMAT format, int width, int height, int depth)
{
    unsigned int CardMemoryAmount; // eax
    unsigned int v7; // [esp+0h] [ebp-2Ch]
    unsigned int v8; // [esp+4h] [ebp-28h]
    unsigned int v9; // [esp+8h] [ebp-24h]
    int memory; // [esp+18h] [ebp-14h]
    int platform; // [esp+28h] [ebp-4h]

    for ( platform = 0; platform < 2; ++platform )
    {
        if ( (imageFlags & 1) != 0 )
        {
            CardMemoryAmount = Image_GetCardMemoryAmount(imageFlags, format, width, height, depth);
        }
        else
        {
            if ( width >> image->picmip.platform[platform] > 1 )
                v9 = width >> image->picmip.platform[platform];
            else
                v9 = 1;
            if ( height >> image->picmip.platform[platform] > 1 )
                v8 = height >> image->picmip.platform[platform];
            else
                v8 = 1;
            if ( depth >> image->picmip.platform[platform] > 1 )
                v7 = depth >> image->picmip.platform[platform];
            else
                v7 = 1;
            CardMemoryAmount = Image_GetCardMemoryAmount(imageFlags, format, v9, v8, v7);
        }
        memory = CardMemoryAmount;
        if ( image->cardMemory.platform[platform]
            && image->cardMemory.platform[platform] != CardMemoryAmount
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        133,
                        0,
                        "%s",
                        "!image->cardMemory.platform[platform] || (image->cardMemory.platform[platform] == memory)") )
        {
            __debugbreak();
        }
        if ( !useFastFile->current.enabled )
            Image_TrackTotalMemory(image, platform, memory);
        image->cardMemory.platform[platform] = memory;
    }
}

unsigned int __cdecl Image_GetCardMemoryAmountForMipLevel(
                _D3DFORMAT format,
                unsigned int mipWidth,
                unsigned int mipHeight,
                unsigned int mipDepth)
{
    unsigned int result; // eax
    const char *v5; // eax

    if ( format > D3DFMT_DXT1 )
    {
        if ( format == D3DFMT_DXT3 || format == D3DFMT_DXT5 )
        {
            return 16 * mipDepth * ((mipHeight + 3) >> 2) * ((mipWidth + 3) >> 2);
        }
        else
        {
LABEL_14:
            v5 = va("unhandled case: %d", format);
            if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 188, 1, v5) )
                __debugbreak();
            return 0;
        }
    }
    else if ( format == D3DFMT_DXT1 )
    {
        return 8 * mipDepth * ((mipHeight + 3) >> 2) * ((mipWidth + 3) >> 2);
    }
    else
    {
        switch ( format )
        {
            case D3DFMT_A8R8G8B8:
            case D3DFMT_X8R8G8B8:
            case D3DFMT_G16R16:
            case D3DFMT_D24S8:
            case D3DFMT_G16R16F:
            case D3DFMT_R32F:
                result = 4 * mipDepth * mipHeight * mipWidth;
                break;
            case D3DFMT_R5G6B5:
            case D3DFMT_A8L8:
            case D3DFMT_D16:
                result = 2 * mipDepth * mipHeight * mipWidth;
                break;
            case D3DFMT_A8:
            case D3DFMT_L8:
                result = mipDepth * mipHeight * mipWidth;
                break;
            case D3DFMT_A8B8G8R8:
                result = 4 * mipDepth * mipHeight * mipWidth;
                break;
            case D3DFMT_A16B16G16R16:
            case D3DFMT_A16B16G16R16F:
                result = 8 * mipDepth * mipHeight * mipWidth;
                break;
            default:
                goto LABEL_14;
        }
    }
    return result;
}

unsigned int __cdecl Image_GetCardMemoryAmount(
                char imageFlags,
                _D3DFORMAT format,
                unsigned int width,
                unsigned int height,
                unsigned int depth)
{
    unsigned int v6; // [esp+0h] [ebp-1Ch]
    unsigned int v7; // [esp+4h] [ebp-18h]
    unsigned int v8; // [esp+8h] [ebp-14h]
    unsigned int memory; // [esp+18h] [ebp-4h]

    memory = Image_GetCardMemoryAmountForMipLevel(format, width, height, depth);
    if ( (imageFlags & 2) == 0 )
    {
        while ( depth + height + width > 3 )
        {
            if ( width >> 1 > 1 )
                v8 = width >> 1;
            else
                v8 = 1;
            width = v8;
            if ( height >> 1 > 1 )
                v7 = height >> 1;
            else
                v7 = 1;
            height = v7;
            if ( depth >> 1 > 1 )
                v6 = depth >> 1;
            else
                v6 = 1;
            depth = v6;
            memory += Image_GetCardMemoryAmountForMipLevel(format, v8, v7, v6);
        }
    }
    if ( (imageFlags & 4) != 0 )
        memory *= 6;
    return memory;
}

void __cdecl Image_UploadData(
                GfxImage *image,
                _D3DFORMAT format,
                _D3DCUBEMAP_FACES face,
                unsigned int mipLevel,
                unsigned __int8 *src)
{

    if ( image->mapType != 5 || !mipLevel || gfxMetrics.canMipCubemaps )
    {
        if ( image->mapType == 4 )
            Image_Upload3D_CopyData_PC(image, format, mipLevel, src);
        else
            Image_Upload2D_CopyData_PC(image, format, face, mipLevel, src);
    }
}

void __cdecl Image_Upload2D_CopyData_PC(
                const GfxImage *image,
                _D3DFORMAT format,
                _D3DCUBEMAP_FACES face,
                unsigned int mipLevel,
                unsigned __int8 *src)
{
    const char *v5; // eax
    const char *v6; // eax
    const char *v7; // eax
    const char *v8; // eax
    unsigned int v9; // [esp+0h] [ebp-40h]
    unsigned int v10; // [esp+4h] [ebp-3Ch]
    int v12; // [esp+14h] [ebp-2Ch]
    int v14; // [esp+1Ch] [ebp-24h]
    int v16; // [esp+24h] [ebp-1Ch]
    int hr; // [esp+2Ch] [ebp-14h]
    _D3DLOCKED_RECT lockedRect; // [esp+30h] [ebp-10h] BYREF
    unsigned int width; // [esp+38h] [ebp-8h]
    unsigned int height; // [esp+3Ch] [ebp-4h]

    if ( image->width >> mipLevel > 1 )
        v10 = image->width >> mipLevel;
    else
        v10 = 1;
    width = v10;
    if ( image->height >> mipLevel > 1 )
        v9 = image->height >> mipLevel;
    else
        v9 = 1;
    height = v9;
    if ( image->mapType == 3 )
    {
        if ( !image->texture.basemap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        683,
                        0,
                        "%s",
                        "image->texture.map") )
        {
            __debugbreak();
        }
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("image->texture.map->LockRect( mipLevel, &lockedRect, 0, 0 )\n");
        hr = image->texture.map->LockRect(mipLevel, &lockedRect, 0, 0);
        //hr = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))image->texture.basemap->__vftable[1].Release)(
        //             (GfxTexture)image->texture.basemap,
        //             mipLevel,
        //             &lockedRect,
        //             0,
        //             0);
        if ( hr < 0 )
        {
            ++g_disableRendering;
            v5 = R_ErrorDescription(hr);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp (%i) image->texture.map->LockRect( mipLevel,"
                " &lockedRect, 0, 0 ) failed: %s\n",
                684,
                v5);
        }
        Image_Upload2D_CopyDataBlock_PC(width, height, src, format, lockedRect.Pitch, (unsigned __int8 *)lockedRect.pBits);
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("image->texture.map->UnlockRect( mipLevel )\n");
        v16 = image->texture.map->UnlockRect(mipLevel);
        //v16 = image->texture.basemap->__vftable[1].GetDevice(image->texture.basemap, (IDirect3DDevice9 **)mipLevel);
        if ( v16 < 0 )
        {
            ++g_disableRendering;
            v6 = R_ErrorDescription(v16);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp (%i) image->texture.map->UnlockRect( mipLeve"
                "l ) failed: %s\n",
                686,
                v6);
        }
    }
    else
    {
        if ( image->mapType != 5
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        690,
                        0,
                        "%s",
                        "image->mapType == MAPTYPE_CUBE") )
        {
            __debugbreak();
        }
        if ( (unsigned int)face >= (D3DCUBEMAP_FACE_POSITIVE_Z|D3DCUBEMAP_FACE_POSITIVE_Y)
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        692,
                        0,
                        "%s\n\t(face) = %i",
                        "(face == D3DCUBEMAP_FACE_POSITIVE_X || face == D3DCUBEMAP_FACE_NEGATIVE_X || face == D3DCUBEMAP_FACE_POSITIV"
                        "E_Y || face == D3DCUBEMAP_FACE_NEGATIVE_Y || face == D3DCUBEMAP_FACE_POSITIVE_Z || face == D3DCUBEMAP_FACE_NEGATIVE_Z)",
                        face) )
        {
            __debugbreak();
        }
        if ( !image->texture.basemap
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        693,
                        0,
                        "%s",
                        "image->texture.cubemap") )
        {
            __debugbreak();
        }
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("image->texture.cubemap->LockRect( face, mipLevel, &lockedRect, 0, 0 )\n");
        //v14 = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))image->texture.basemap->__vftable[1].Release)(
        //                (GfxTexture)image->texture.basemap,
        //                face,
        //                mipLevel,
        //                &lockedRect,
        //                0,
        //                0);
        v14 = image->texture.cubemap->LockRect(face, mipLevel, &lockedRect, 0, 0);
        if ( v14 < 0 )
        {
            ++g_disableRendering;
            v7 = R_ErrorDescription(v14);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp (%i) image->texture.cubemap->LockRect( face,"
                " mipLevel, &lockedRect, 0, 0 ) failed: %s\n",
                694,
                v7);
        }
        Image_Upload2D_CopyDataBlock_PC(width, height, src, format, lockedRect.Pitch, (unsigned __int8 *)lockedRect.pBits);
        if ( r_logFile && r_logFile->current.integer )
            RB_LogPrint("image->texture.cubemap->UnlockRect( face, mipLevel )\n");
        //v12 = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int))image->texture.basemap->__vftable[1].GetDevice)(
        //                (GfxTexture)image->texture.basemap,
        //                face,
        //                mipLevel);
        v12 = image->texture.cubemap->UnlockRect(face, mipLevel);
        if ( v12 < 0 )
        {
            ++g_disableRendering;
            v8 = R_ErrorDescription(v12);
            Com_Error(
                ERR_FATAL,
                "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp (%i) image->texture.cubemap->UnlockRect( fac"
                "e, mipLevel ) failed: %s\n",
                696,
                v8);
        }
    }
}

void __cdecl Image_Upload2D_CopyDataBlock_PC(
                int width,
                int height,
                unsigned __int8 *src,
                _D3DFORMAT format,
                int dstPitch,
                unsigned __int8 *dst)
{
    const char *v6; // eax
    const char *v7; // eax
    signed int srcStride; // [esp+4h] [ebp-Ch]
    int y; // [esp+8h] [ebp-8h]
    int dy; // [esp+Ch] [ebp-4h]

    BO1_RenderCount(BO1_RC_TEX_UPLOAD, 1); // p1: bo1_frameperf (measurement only)
    if ( !src
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 595, 0, "%s", "src") )
    {
        __debugbreak();
    }
    if ( !dst
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 596, 0, "%s", "dst") )
    {
        __debugbreak();
    }
    if ( format <= D3DFMT_DXT1 )
    {
        if ( format != D3DFMT_DXT1 )
        {
            switch ( format )
            {
                case D3DFMT_A8R8G8B8:
                case D3DFMT_X8R8G8B8:
                case D3DFMT_A8B8G8R8:
                case D3DFMT_G16R16:
                    srcStride = 4 * width;
                    dy = 1;
                    goto LABEL_23;
                case D3DFMT_R5G6B5:
                case D3DFMT_A8L8:
                    srcStride = 2 * width;
                    dy = 1;
                    goto LABEL_23;
                case D3DFMT_A8:
                case D3DFMT_L8:
                    srcStride = width;
                    dy = 1;
                    goto LABEL_23;
                case D3DFMT_A16B16G16R16:
                case D3DFMT_A16B16G16R16F:
                    srcStride = 4 * width;
                    dy = 1;
                    goto LABEL_23;
                case D3DFMT_R32F:
                    srcStride = 4 * width;
                    dy = 1;
                    goto LABEL_23;
                default:
                    goto LABEL_20;
            }
        }
        srcStride = 8 * ((width + 3) >> 2);
        dy = 4;
        goto LABEL_23;
    }
    if ( format == D3DFMT_DXT3 || format == D3DFMT_DXT5 )
    {
        srcStride = 16 * ((width + 3) >> 2);
        dy = 4;
LABEL_23:
        BO1_RenderCount(BO1_RC_TEX_UPLOAD_KB, (srcStride * ((height - 1) / dy + 1) + 1023) >> 10); // p1: bo1_frameperf
        if ( dstPitch < srcStride )
        {
            v7 = va("%i x %i: %i < %i", width, height, dstPitch, srcStride);
            if ( !Assert_MyHandler(
                            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                            647,
                            0,
                            "%s\n\t%s",
                            "dstPitch >= srcStride",
                            v7) )
                __debugbreak();
        }
        if ( dstPitch == srcStride )
        {
            memcpy(dst, src, srcStride * ((height - 1) / dy + 1));
        }
        else
        {
            for ( y = 0; y < height; y += dy )
            {
                memcpy(dst, src, srcStride);
                dst += dstPitch;
                src += srcStride;
            }
        }
        return;
    }
LABEL_20:
    v6 = va("unhandled case: %d", format);
    if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 643, 1, v6) )
        __debugbreak();
}

void __cdecl Image_Upload3D_CopyData_PC(
                const GfxImage *image,
                _D3DFORMAT format,
                unsigned int mipLevel,
                unsigned __int8 *src)
{
    const char *v4; // eax
    const char *v5; // eax
    int v6; // [esp+0h] [ebp-4Ch]
    int v7; // [esp+4h] [ebp-48h]
    int v8; // [esp+8h] [ebp-44h]
    int v10; // [esp+1Ch] [ebp-30h]
    int hr; // [esp+24h] [ebp-28h]
    int srcRowPitch; // [esp+2Ch] [ebp-20h]
    int sliceIndex; // [esp+30h] [ebp-1Ch]
    _D3DLOCKED_BOX lockedBox; // [esp+34h] [ebp-18h] BYREF
    int width; // [esp+40h] [ebp-Ch]
    int height; // [esp+44h] [ebp-8h]
    unsigned __int8 *dst; // [esp+48h] [ebp-4h]

    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 833, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( image->mapType != 4
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    834,
                    0,
                    "%s\n\t(image->mapType) = %i",
                    "(image->mapType == MAPTYPE_3D)",
                    image->mapType) )
    {
        __debugbreak();
    }
    if ( image->width >> mipLevel > 1 )
        v8 = image->width >> mipLevel;
    else
        v8 = 1;
    width = v8;
    if ( image->height >> mipLevel > 1 )
        v7 = image->height >> mipLevel;
    else
        v7 = 1;
    height = v7;
    if ( image->depth >> mipLevel > 1 )
        v6 = image->depth >> mipLevel;
    else
        v6 = 1;
    srcRowPitch = Image_SourceBytesPerSlice_PC(format, width, height);
    if ( !image->texture.basemap
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    841,
                    0,
                    "%s",
                    "image->texture.volmap") )
    {
        __debugbreak();
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("image->texture.volmap->LockBox( mipLevel, &lockedBox, 0, 0 )\n");
    //hr = ((int (__stdcall *)(unsigned int, unsigned int, unsigned int, unsigned int, unsigned int))image->texture.basemap->__vftable[1].Release)(
    //             (GfxTexture)image->texture.basemap,
    //             mipLevel,
    //             &lockedBox,
    //             0,
    //             0);
    hr = image->texture.volmap->LockBox(mipLevel, &lockedBox, 0, 0);
    if ( hr < 0 )
    {
        ++g_disableRendering;
        v4 = R_ErrorDescription(hr);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp (%i) image->texture.volmap->LockBox( mipLevel,"
            " &lockedBox, 0, 0 ) failed: %s\n",
            842,
            v4);
    }
    dst = (unsigned __int8 *)lockedBox.pBits;
    for ( sliceIndex = 0; sliceIndex < v6; ++sliceIndex )
    {
        Image_Upload2D_CopyDataBlock_PC(width, height, src, format, lockedBox.RowPitch, dst);
        src += srcRowPitch;
        dst += lockedBox.SlicePitch;
    }
    if ( r_logFile && r_logFile->current.integer )
        RB_LogPrint("image->texture.volmap->UnlockBox( mipLevel )\n");
    //v10 = image->texture.basemap->__vftable[1].GetDevice(image->texture.basemap, (IDirect3DDevice9 **)mipLevel);
    v10 = image->texture.volmap->UnlockBox(mipLevel);
    if ( v10 < 0 )
    {
        ++g_disableRendering;
        v5 = R_ErrorDescription(v10);
        Com_Error(
            ERR_FATAL,
            "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp (%i) image->texture.volmap->UnlockBox( mipLeve"
            "l ) failed: %s\n",
            852,
            v5);
    }
    image->texture.basemap->PreLoad();
}

int __cdecl Image_SourceBytesPerSlice_PC(_D3DFORMAT format, int width, int height)
{
    int result; // eax
    const char *v4; // eax

    if ( format > D3DFMT_D24S8 )
    {
        if ( format > D3DFMT_DXT1 )
        {
            if ( format == D3DFMT_DXT3 || format == D3DFMT_DXT5 )
                return 16 * ((height + 3) >> 2) * ((width + 3) >> 2);
        }
        else
        {
            switch ( format )
            {
                case D3DFMT_DXT1:
                    return 8 * ((height + 3) >> 2) * ((width + 3) >> 2);
                case D3DFMT_D16:
                    return 2 * height * width;
                case D3DFMT_R32F:
                    return 4 * height * width;
            }
        }
LABEL_18:
        v4 = va("unhandled case: %d", format);
        if ( !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 379, 1, v4) )
            __debugbreak();
        return 0;
    }
    if ( format == D3DFMT_D24S8 )
        return 4 * height * width;
    switch ( format )
    {
        case D3DFMT_A8R8G8B8:
            return 4 * height * width;
        case D3DFMT_X8R8G8B8:
            result = 3 * height * width;
            break;
        case D3DFMT_R5G6B5:
        case D3DFMT_A8L8:
            return 2 * height * width;
        case D3DFMT_A8:
        case D3DFMT_L8:
            result = height * width;
            break;
        default:
            goto LABEL_18;
    }
    return result;
}

unsigned int __cdecl Image_CountMipmaps(char imageFlags, unsigned int width, unsigned int height, unsigned int depth)
{
    unsigned int mipRes; // [esp+0h] [ebp-8h]
    unsigned int mipCount; // [esp+4h] [ebp-4h]

    if ( (imageFlags & 2) != 0 )
        return 1;
    mipCount = 1;
    for ( mipRes = 1; mipRes < width || mipRes < height || mipRes < depth; mipRes *= 2 )
        ++mipCount;
    return mipCount;
}

void __cdecl Image_GetPicmip(const GfxImage *image, Picmip *picmip)
{
    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 1472, 0, "%s", "image") )
    {
        __debugbreak();
    }
    if ( !picmip
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 1473, 0, "%s", "picmip") )
    {
        __debugbreak();
    }

    if (image->noPicmip)
    {
        //*picmip = 0;
        picmip->platform[0] = 0;
        picmip->platform[1] = 0;
    }
    else
    {
        Image_PicmipForSemantic(image->semantic, picmip);
    }
}

void __cdecl Image_Setup(GfxImage *image, int width, int height, int depth, int imageFlags, _D3DFORMAT imageFormat)
{
    bool mipmapCount; // [esp+0h] [ebp-8h]

    if ( !image
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp", 1502, 0, "%s", "image") )
    {
        __debugbreak();
    }
    image->width = width;
    image->height = height;
    image->depth = depth;
    mipmapCount = (imageFlags & 2) != 0;
    if ( r_loadForRenderer->current.enabled )
    {
        if ( (imageFlags & 4) != 0 )
        {
            Image_CreateCubeTexture_PC(image, image->width, mipmapCount, imageFormat);
        }
        else if ( (imageFlags & 8) != 0 )
        {
            Image_Create3DTexture_PC(image, image->width, image->height, image->depth, mipmapCount, imageFlags, imageFormat);
        }
        else
        {
            Image_Create2DTexture_PC(image, image->width, image->height, mipmapCount, imageFlags, imageFormat);
        }
        Image_TrackTexture(image, imageFlags, imageFormat, width, height, depth);
        if ( image->delayLoadPixels
            && !Assert_MyHandler(
                        "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                        1613,
                        0,
                        "%s",
                        "!image->delayLoadPixels") )
        {
            __debugbreak();
        }
    }
}

void __cdecl Image_SetupAndLoad(
                GfxImage *image,
                int width,
                int height,
                int depth,
                int imageFlags,
                _D3DFORMAT imageFormat)
{
    Image_Setup(image, width, height, depth, imageFlags, imageFormat);
}

unsigned int __cdecl Image_CubemapFace(unsigned int faceIndex)
{
    if ( faceIndex >= 6
        && !Assert_MyHandler(
                    "C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_image_load_common.cpp",
                    1725,
                    0,
                    "%s\n\t(faceIndex) = %i",
                    "(faceIndex < 6)",
                    faceIndex) )
    {
        __debugbreak();
    }
    return faceIndex;
}

