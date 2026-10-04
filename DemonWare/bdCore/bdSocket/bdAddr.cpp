#include "bdAddr.h"


int bdInAddr::toString(char *const dst, unsigned int size) const
{
    return 0;
#if 0
    if (!dst || size == 0)
        return 0;

    in_addr addr;
    addr.s_addr = inUn.m_iaddr;

    const char *src = inet_ntoa(addr);
    if (!src)
        return 0;

    bdStrlcpy(dst, (char *)src, size);

    // Decompiled code calls strlen and returns its result
    return static_cast<int>(std::strlen(dst));
#endif
}


unsigned int bdAddr::toString(char *const dst, unsigned int capacity)
{
    return 0;
#if 0
    //unsigned int written = bdInetAddr::toString(&m_address, dst, capacity);
    unsigned int written = m_address.toString(dst, capacity);

    unsigned int remaining = (written <= capacity) ? (capacity - written) : 0;

    return written +
        bdSnprintf(&dst[written], remaining, ":%u", m_port);
#endif
}

void bdCommonAddr::calculateHash()
{
#if 0
    bdAddr effective;

    if (m_localAddrs.m_size > 0)
        effective = m_localAddrs.m_data[0];
    else
        effective = bdAddr();

    //if (bdInetAddr::isValid(&m_publicAddr.m_address))
    if (m_publicAddr.m_address.isValid())
        effective = m_publicAddr;

    unsigned char buffer[6];
    unsigned int offset = 0;

    //if (!bdAddr::serialize(&effective, buffer, 6u, 0, &offset))
    if (!effective.serialize(buffer, 6u, 0, &offset))
    {
        m_hash = 0;
        return;
    }

    bdHashTiger192 hash;
    unsigned char digest[24];
    unsigned int digestSize = 24;

    if (!hash.hash(buffer, 6u, digest, &digestSize))
    {
        m_hash = 0;
        return;
    }

    unsigned char hashBytes[4];
    offset = 0;

    if (bdBytePacker::removeBuffer(digest, 24u, 0, &offset, hashBytes, 4u))
        m_hash = *reinterpret_cast<unsigned int *>(hashBytes);
    else
        m_hash = 0;
#endif
}