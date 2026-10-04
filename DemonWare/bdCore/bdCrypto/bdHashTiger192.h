#pragma once
#include <cstddef>
#include <cstdint>
#include <libs/libtomcrypt-1.17/src/headers/tomcrypt.h>

// fuck whoever made demonware. Quote me on that

struct bdHashTiger192
{
    static constexpr std::size_t HASH_SIZE = 24; // 192 bits

    static bool hash(
        const void *data,
        std::size_t length,
        uint8_t out[HASH_SIZE])
    {
        if (!data || !out)
            return false;

        hash_state md;

        if (tiger_init(&md) != 0)
            return false;

        if (tiger_process(&md,
            static_cast<const unsigned char *>(data),
            static_cast<unsigned long>(length)) != 0)
            return false;

        if (tiger_done(&md, out) != 0)
            return false;

        return true;
    }
};