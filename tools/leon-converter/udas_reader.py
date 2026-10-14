"""RE4 PC (UHD) .udas container (the GameCube .drs layout, little-endian; see udas_probe.py): body entries."""
import struct

TABLE = 0x400


def entries(d):
    """(index, tag, bytes) for every entry of the type-0 body record."""
    body_ofs = body_size = None
    for r in range(0x20, TABLE, 0x20):
        rtype, size, _, ofs = struct.unpack_from('<4I', d, r)
        if rtype == 0xFFFFFFFF:
            break
        if rtype == 0:
            body_ofs, body_size = ofs, size
    body = d[body_ofs:body_ofs + body_size]
    count, rel_ofs = struct.unpack_from('<2I', body, 0)
    offs = struct.unpack_from(f'<{count}I', body, 16)
    tags = [body[16 + 4 * count + 4 * i:16 + 4 * count + 4 * i + 4] for i in range(count)]
    end = rel_ofs or len(body)
    for i in range(count):
        tag = tags[i].rstrip(b'\0').decode('latin1')
        nxt = next((o for o in offs[i + 1:] if o > offs[i]), end)
        yield i, tag, body[offs[i]:nxt]
