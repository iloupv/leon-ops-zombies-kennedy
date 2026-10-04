#include "vpx.h"
#include <vpx/vpx_image.h>
#include <vpx/vp8cx.h>

#include <cstring>
#include <universal/q_shared.h>

struct //$0A669EED672D0614AB086EF67B48703D // sizeof=0x12C
{                                       // XREF: .data:_unnamed_type_vpx_globals_ vpx_globals/r
    bool init;                          // XREF: vpx_init(char const *,int,int)+4/r
                                        // vpx_init(char const *,int,int):loc_9B3917/w ...
    // padding byte
    // padding byte
    // padding byte
    FILE *outfile;                    // XREF: vpx_init(char const *,int,int)+75/w
    vpx_codec_ctx codec;                // XREF: vpx_init(char const *,int,int)+104/o
    vpx_codec_enc_cfg cfg;              // XREF: vpx_init(char const *,int,int)+8A/o
    int frame_cnt;                      // XREF: vpx_init(char const *,int,int)+26/w
    unsigned __int8 file_hdr[32];
    unsigned __int8 frame_hdr[12];
    vpx_image raw;                      // XREF: vpx_init(char const *,int,int)+4E/o
    int width;                          // XREF: vpx_init(char const *,int,int)+17/w
    int height;                         // XREF: vpx_init(char const *,int,int)+20/w
    int flags;                          // XREF: vpx_init(char const *,int,int)+30/w
} vpx_globals;

// aislop used to rework whole file

void vpx_init(const char *filename, int width, int height)
{
    if (vpx_globals.init)
        return;

    vpx_globals.width = width;
    vpx_globals.height = height;
    vpx_globals.frame_cnt = 0;

    // Allocate image (I420)
    if (!vpx_img_alloc(&vpx_globals.raw, VPX_IMG_FMT_I420, width, height, 1))
        return;

    vpx_globals.outfile = fopen(filename, "wb");
    if (!vpx_globals.outfile)
        return;

    if (vpx_codec_enc_config_default(&vpx_codec_vp8_cx_algo, &vpx_globals.cfg, 0))
        return;

    vpx_globals.cfg.g_w = width;
    vpx_globals.cfg.g_h = height;
    vpx_globals.cfg.g_timebase.num = 1;
    vpx_globals.cfg.g_timebase.den = 30; // 30 fps

    // Scale bitrate with resolution
    vpx_globals.cfg.rc_target_bitrate =
        vpx_globals.cfg.rc_target_bitrate *
        width * height /
        (vpx_globals.cfg.g_w * vpx_globals.cfg.g_h);

    if (vpx_codec_enc_init(
        &vpx_globals.codec,
        &vpx_codec_vp8_cx_algo,
        &vpx_globals.cfg,
        0))
    {
        return;
    }

    write_ivf_file_header(vpx_globals.outfile, &vpx_globals.cfg, 0);

    vpx_globals.init = true;
}

void __cdecl write_ivf_file_header(FILE *outfile, const vpx_codec_enc_cfg *cfg, unsigned int frame_cnt)
{
    char header[32]; // [esp+0h] [ebp-24h] BYREF

    if ( cfg->g_pass == VPX_RC_ONE_PASS || cfg->g_pass == VPX_RC_LAST_PASS )
    {
        memcpy(header, "DKIF", 4);
        mem_put_le16(&header[4], 0);
        mem_put_le16(&header[6], 32);
        mem_put_le32(&header[8], 0x30385056u);
        mem_put_le16(&header[12], cfg->g_w);
        mem_put_le16(&header[14], cfg->g_h);
        mem_put_le32(&header[16], cfg->g_timebase.den);
        mem_put_le32(&header[20], cfg->g_timebase.num);
        mem_put_le32(&header[24], frame_cnt);
        mem_put_le32(&header[28], 0);
        fwrite(header, 1u, 0x20u, outfile);
    }
}

void __cdecl mem_put_le16(char *mem, __int16 val)
{
    *(_WORD *)mem = val;
}

void __cdecl mem_put_le32(char *mem, unsigned int val)
{
    *(unsigned int *)mem = val;
}

void __cdecl vpx_encode_frame(
    unsigned __int8 *y,
    unsigned __int8 *u,
    unsigned __int8 *v,
    bool eof)
{
    if (!vpx_globals.init)
        return;

    // Copy image planes if this is a real frame
    if (!eof)
    {
        memcpy(vpx_globals.raw.planes[0],
            y,
            vpx_globals.width * vpx_globals.height);

        memcpy(vpx_globals.raw.planes[1],
            u,
            vpx_globals.width * vpx_globals.height / 4);

        memcpy(vpx_globals.raw.planes[2],
            v,
            vpx_globals.width * vpx_globals.height / 4);
    }

    // Encode frame (or flush if eof)
    if (vpx_codec_encode(
        &vpx_globals.codec,
        eof ? NULL : &vpx_globals.raw,
        vpx_globals.frame_cnt,
        1,                  // duration
        0,
        VPX_DL_REALTIME) != VPX_CODEC_OK)
    {
        return; // encode error
    }

    // Retrieve compressed packets
    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t *pkt;

    while ((pkt = vpx_codec_get_cx_data(
        &vpx_globals.codec,
        &iter)) != NULL)
    {
        if (pkt->kind == VPX_CODEC_CX_FRAME_PKT)
        {
            write_ivf_frame_header(
                vpx_globals.outfile,
                pkt);

            fwrite(pkt->data.frame.buf,
                1,
                pkt->data.frame.sz,
                vpx_globals.outfile);
        }
    }

    // Only increment frame counter for real frames
    if (!eof)
        vpx_globals.frame_cnt++;
}

void __cdecl write_ivf_frame_header(FILE *outfile, const vpx_codec_cx_pkt *pkt)
{
    char header[12]; // [esp+0h] [ebp-18h] BYREF
    __int64 pts; // [esp+10h] [ebp-8h]

    if ( pkt->kind == VPX_CODEC_CX_FRAME_PKT )
    {
        pts = pkt->data.frame.pts;
        mem_put_le32(header, pkt->data.frame.sz);
        mem_put_le32(&header[4], pts);
        mem_put_le32(&header[8], HIDWORD(pts));
        fwrite(header, 1u, 0xCu, outfile);
    }
}

void __cdecl vpx_shutdown()
{
    if (!vpx_globals.init)
        return;

    // Flush encoder
    vpx_encode_frame(NULL, NULL, NULL, 1);

    // Destroy codec
    if (!vpx_codec_destroy(&vpx_globals.codec))
    {
        // Rewrite IVF header with correct frame count
        fseek(vpx_globals.outfile, 0, SEEK_SET);

        write_ivf_file_header(
            vpx_globals.outfile,
            &vpx_globals.cfg,
            vpx_globals.frame_cnt);

        fclose(vpx_globals.outfile);

        // Free image properly (2010 style)
        vpx_img_free(&vpx_globals.raw);

        vpx_globals.init = 0;
    }
}

