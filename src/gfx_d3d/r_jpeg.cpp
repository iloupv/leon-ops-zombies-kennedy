#include "r_jpeg.h"
#include <universal/com_memory.h>
#include <qcommon/common.h>
#include <universal/com_files.h>

int hackSize;
int s_hunk_mark;
jpeg_decompress_struct s_read_cinfo;
jpeg_error_mgr s_read_jerr;

void *__cdecl Z_MallocJpeg(unsigned int size)
{
    return Z_Malloc(size, "Z_MallocJpeg", 11);
}

void __cdecl Z_FreeJpeg(void *ptr)
{
    Z_Free((char*)ptr, 11);
}

void __cdecl R_SaveJpg(
                char *filename,
                int quality,
                unsigned int image_width,
                unsigned int image_height,
                unsigned __int8 *image_buffer,
                unsigned __int8 *output_buffer)
{
    unsigned __int8 *out; // [esp+0h] [ebp-214h]
    unsigned __int8 *row_pointer[1]; // [esp+8h] [ebp-20Ch] BYREF
    jpeg_compress_struct cinfo; // [esp+Ch] [ebp-208h] BYREF
    jpeg_error_mgr jerr; // [esp+17Ch] [ebp-98h] BYREF

    cinfo.err = jpeg_std_error(&jerr, Jpeg_Error, Jpeg_Print);
    jpeg_set_jpeg_alloc(Z_MallocJpeg, Z_FreeJpeg);
    jpeg_CreateCompress(&cinfo, 62, 0x170u);
    if ( output_buffer )
    {
        out = output_buffer;
        jpegDest((jpeg_common_struct *)&cinfo, output_buffer, 3 * image_height * image_width);
    }
    else
    {
        out = (unsigned __int8 *)Hunk_AllocateTempMemory(3 * image_height * image_width, "SaveJPG");
        jpegDest((jpeg_common_struct *)&cinfo, out, 3 * image_height * image_width);
    }
    cinfo.image_width = image_width;
    cinfo.image_height = image_height;
    cinfo.input_components = 3;
    cinfo.in_color_space = JCS_RGB;
    jpeg_set_defaults(&cinfo);
    jpeg_set_quality(&cinfo, quality, 1u);
    jpeg_start_compress(&cinfo, 1u);
    while ( cinfo.next_scanline < cinfo.image_height )
    {
        row_pointer[0] = &image_buffer[3 * image_width * cinfo.next_scanline];
        jpeg_write_scanlines(&cinfo, row_pointer, 1u);
    }
    jpeg_finish_compress(&cinfo);
    FS_WriteFile(filename, (char *)out, hackSize);
    if ( !output_buffer )
        Hunk_FreeTempMemory((char *)out);
    //jpeg_destroy_decompress(&cinfo); // bug?
    jpeg_destroy_compress(&cinfo);
    jpeg_set_jpeg_alloc(0, 0);
}

void __cdecl jpegDest(jpeg_common_struct *cinfo, unsigned __int8 *outfile, int size)
{
    my_destination_mgr *dest; // [esp+0h] [ebp-4h]

    if ( !cinfo[1].err )
        cinfo[1].err = (jpeg_error_mgr *)cinfo->mem->alloc_small(cinfo, 0, 28);
    dest = (my_destination_mgr *)cinfo[1].err;
    dest->pub.init_destination = init_destination;
    dest->pub.empty_output_buffer = (jpeg_boolean (__cdecl *)(jpeg_compress_struct *))Flame_GetLocalClientSourceRange;
    dest->pub.term_destination = term_destination;
    dest->outfile = (FILE*)outfile;
    dest->size = size;
}

void __cdecl init_destination(jpeg_compress_struct *cinfo)
{
    my_destination_mgr *dest; // [esp+0h] [ebp-4h]

    dest = (my_destination_mgr *)cinfo->dest;
    dest->pub.next_output_byte = (JOCTET*)dest->outfile;
    dest->pub.free_in_buffer = dest->size;
}

void __cdecl term_destination(jpeg_compress_struct *cinfo)
{
    hackSize = cinfo->dest[1].free_in_buffer - cinfo->dest->free_in_buffer;
}

void __cdecl Jpeg_Print(char *message)
{
    Com_Printf(8, "%s\n", message);
}

jpeg_decompress_struct *__cdecl R_ReadJpgSetup(
                const unsigned __int8 *data,
                unsigned int dataLen,
                int *width,
                int *height,
                _D3DFORMAT *imageFormat)
{
    iassert(s_hunk_mark == 0);

    s_read_cinfo.err = jpeg_std_error(&s_read_jerr, Jpeg_Error, Jpeg_Print);
    jpeg_set_jpeg_alloc(Jpeg_HunkAlloc, BLOPS_NULLSUB);
    s_hunk_mark = Hunk_SetMark();
    jpeg_CreateDecompress(&s_read_cinfo, 62, 0x1B8u);
    jpeg_memory_src(&s_read_cinfo, data, dataLen);
    jpeg_read_header(&s_read_cinfo, 1u);
    jpeg_start_decompress(&s_read_cinfo);
    if ( s_read_cinfo.output_components == 3 )
    {
        *width = s_read_cinfo.output_width;
        *height = s_read_cinfo.output_height;
        *imageFormat = D3DFMT_A8R8G8B8;
        return &s_read_cinfo;
    }
    else
    {
        Com_PrintWarning(8, "WARNING: jpeg image is not RGB\n");
        jpeg_destroy_decompress(&s_read_cinfo);
        jpeg_set_jpeg_alloc(0, 0);
        Hunk_ClearToMark(s_hunk_mark);
        s_hunk_mark = 0;
        return 0;
    }
}

void*__cdecl Jpeg_HunkAlloc(unsigned int size)
{
    return Hunk_Alloc(size, "ReadJpg", 11);
}

void Jpeg_Error()
{
    Com_Error(ERR_FATAL, "jpeg internal error");
}

char __cdecl R_ReadJpg(jpeg_decompress_struct *context, unsigned __int8 *pixels, int pitch, int height)
{
    unsigned __int8 *scanline; // [esp+8h] [ebp-Ch] BYREF
    jpeg_decompress_struct *cinfo; // [esp+Ch] [ebp-8h]
    unsigned int i; // [esp+10h] [ebp-4h]

    cinfo = context;
    if ( !s_hunk_mark
        && !Assert_MyHandler("C:\\projects_pc\\cod\\codsrc\\src\\gfx_d3d\\r_jpeg.cpp", 515, 0, "%s", "s_hunk_mark != 0") )
    {
        __debugbreak();
    }
    scanline = (unsigned __int8 *)Hunk_AllocateTempMemory(cinfo->output_components * cinfo->output_width, "R_ReadJpg");
    while ( cinfo->output_scanline < cinfo->output_height && cinfo->output_scanline != height )
    {
        jpeg_read_scanlines(cinfo, &scanline, 1u);
        for ( i = 0; i < cinfo->output_width; ++i )
        {
            pixels[4 * i] = scanline[cinfo->output_components * i];
            pixels[4 * i + 1] = scanline[cinfo->output_components * i + 1];
            pixels[4 * i + 2] = scanline[cinfo->output_components * i + 2];
            pixels[4 * i + 3] = -1;
        }
        pixels += pitch;
    }
    jpeg_finish_decompress(cinfo);
    jpeg_destroy_decompress(cinfo);
    jpeg_set_jpeg_alloc(0, 0);
    Hunk_FreeTempMemory((char *)scanline);
    Hunk_ClearToMark(s_hunk_mark);
    s_hunk_mark = 0;
    return 1;
}

