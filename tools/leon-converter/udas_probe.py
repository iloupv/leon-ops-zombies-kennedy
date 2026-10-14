"""Probe a RE4 PC (UHD) .udas: list the body entries and check every FCV motion against the decomp's
parser (tools/motion/fcv.py) with little-endian fields, by parse -> serialise round trip.

usage: python -I udas_probe.py <re4 decomp root> <file.udas>..."""
import collections
import struct
import sys

sys.path.insert(0, sys.argv[1] + '/tools/motion')
import fcv  # noqa: E402

TABLE = 0x400


def entries(d):
    """The body entries: (index, tag, bytes). The body is the type-0 record of the container."""
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


for path in sys.argv[2:]:
    d = open(path, 'rb').read()
    kinds = collections.Counter()
    ok = bad = 0
    first_bad = None
    for i, tag, data in entries(d):
        kinds[tag or '(empty)'] += 1
        if tag != 'FCV' or not data:
            continue
        try:
            m = fcv.parse(data, '<')
            again = fcv.serialise(m, '<')
            if data[:len(again)] == again:
                ok += 1
            else:
                bad += 1
                first_bad = first_bad or (i, 'round trip differs')
        except Exception as e:  # report and continue
            bad += 1
            first_bad = first_bad or (i, repr(e)[:120])
    print(f'{path}: {dict(kinds)}')
    print(f'  FCV motions: {ok} round-trip byte-identical, {bad} not' + (f' (first: entry {first_bad[0]}: {first_bad[1]})' if first_bad else ''))
