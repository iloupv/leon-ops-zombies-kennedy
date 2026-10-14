/* lfsunpack: unpack a Resident Evil 4 (PC UHD) .lfs file into its raw contents.
 *
 * LFS layout (re4-research/re4lfs.cpp): header { magic 'RDLX'/'XLDR', magic2, size_decompressed,
 * size_compressed, num_chunks }, then num_chunks entries { u16 size_compressed, u16 size_decompressed
 * (0 = 0x10000), u32 offset (from the chunk table, bit 0 = compressed) }. A compressed chunk is an
 * XMemCompress (LZX) stream: blocks of { [0xFF, u16be frame_size], u16be block_size, data }, frame size
 * 0x8000 by default. The blocks are concatenated and decoded with libmspack's lzxd (window 2^17, the
 * XMemCompress default), as Xenia does.
 *
 * Usage: lfsunpack <in.lfs> <out> */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>
#include "mspack.h"
#include "system.h"
#include "lzx.h"

struct memfile { const uint8_t *in; size_t in_len, in_pos; uint8_t *out; size_t out_len, out_pos; };

static int mf_read(struct mspack_file *f, void *buf, int n)
{
    struct memfile *m = (struct memfile *)f;
    size_t left = m->in_len - m->in_pos;
    if ((size_t)n > left) n = (int)left;
    memcpy(buf, m->in + m->in_pos, n);
    m->in_pos += n;
    return n;
}
static int mf_write(struct mspack_file *f, void *buf, int n)
{
    struct memfile *m = (struct memfile *)f;
    if (m->out_pos + n > m->out_len) return -1;
    memcpy(m->out + m->out_pos, buf, n);
    m->out_pos += n;
    return n;
}
static void *mf_alloc(struct mspack_system *s, size_t n) { (void)s; return malloc(n); }
static void mf_free(void *p) { free(p); }
static void mf_copy(void *src, void *dst, size_t n) { memmove(dst, src, n); }
static void mf_message(struct mspack_file *f, const char *fmt, ...) { (void)f; (void)fmt; }

static struct mspack_system g_sys = { NULL, NULL, mf_read, mf_write, NULL, NULL, mf_message, mf_alloc, mf_free, mf_copy, NULL };

/* Decode one XMemCompress chunk into out (out_len bytes). Returns 0 on success. */
static int xmem_decompress(const uint8_t *src, size_t src_len, uint8_t *out, size_t out_len)
{
    uint8_t *lzx = malloc(src_len);
    size_t lzx_len = 0, pos = 0;
    while (pos + 2 <= src_len) {
        size_t frame = 0x8000;
        if (src[pos] == 0xFF) {
            if (pos + 5 > src_len) break;
            frame = ((size_t)src[pos + 1] << 8) | src[pos + 2];
            pos += 3;
        }
        size_t block = ((size_t)src[pos] << 8) | src[pos + 1];
        pos += 2;
        if (block == 0 || frame == 0 || pos + block > src_len) break;
        memcpy(lzx + lzx_len, src + pos, block);
        lzx_len += block;
        pos += block;
    }
    struct memfile m = { lzx, lzx_len, 0, out, out_len, 0 };
    struct lzxd_stream *s = lzxd_init(&g_sys, (struct mspack_file *)&m, (struct mspack_file *)&m, 17, 0, 0x8000, (off_t)out_len, 0);
    int err = s ? lzxd_decompress(s, (off_t)out_len) : MSPACK_ERR_NOMEMORY;
    if (s) lzxd_free(s);
    free(lzx);
    return err != MSPACK_ERR_OK || m.out_pos != out_len;
}

static uint32_t rd32(const uint8_t *p, int be) { return be ? ((uint32_t)p[0] << 24 | p[1] << 16 | p[2] << 8 | p[3]) : ((uint32_t)p[3] << 24 | p[2] << 16 | p[1] << 8 | p[0]); }
static uint16_t rd16(const uint8_t *p, int be) { return be ? (uint16_t)(p[0] << 8 | p[1]) : (uint16_t)(p[1] << 8 | p[0]); }

int main(int argc, char **argv)
{
    if (argc != 3) { fprintf(stderr, "usage: lfsunpack <in.lfs> <out>\n"); return 2; }
    FILE *f = fopen(argv[1], "rb");
    if (!f) { perror(argv[1]); return 1; }
    fseek(f, 0, SEEK_END); long len = ftell(f); fseek(f, 0, SEEK_SET);
    uint8_t *d = malloc(len);
    if (fread(d, 1, len, f) != (size_t)len) { fprintf(stderr, "read failed\n"); return 1; }
    fclose(f);

    int be = !memcmp(d, "XLDR", 4);
    if (!be && memcmp(d, "RDLX", 4)) { fprintf(stderr, "not an LFS file\n"); return 1; }
    uint32_t total = rd32(d + 8, be), n = rd32(d + 16, be);
    const uint8_t *table = d + 20;
    uint8_t *out = malloc(total);
    size_t out_pos = 0;
    for (uint32_t i = 0; i < n; ++i) {
        const uint8_t *e = table + 8 * i;
        size_t csize = rd16(e, be), usize = rd16(e + 2, be);
        uint32_t off = rd32(e + 4, be);
        if (!csize) csize = 0x10000;
        if (!usize) usize = 0x10000;
        const uint8_t *src = table + (off & ~1u);
        if (out_pos + usize > total || src + csize > d + len) { fprintf(stderr, "chunk %u out of range\n", i); return 1; }
        if (off & 1) {
            if (xmem_decompress(src, csize, out + out_pos, usize)) { fprintf(stderr, "chunk %u failed to decompress\n", i); return 1; }
        } else {
            memcpy(out + out_pos, src, usize);
        }
        out_pos += usize;
    }
    FILE *o = fopen(argv[2], "wb");
    if (!o) { perror(argv[2]); return 1; }
    fwrite(out, 1, out_pos, o);
    fclose(o);
    printf("%s: %u chunks, %zu bytes\n", argv[1], n, out_pos);
    return 0;
}
