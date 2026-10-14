"""Leon's voice for the mod's runtime sounds (the engine's src/sound/snd_runtime.cpp): his pain grunts and death scream
from RE4's player sound bank (BIO4/snd/pl00.xwb, an XACT3 wave bank of 16-bit mono PCM), written as WAV files.

RE4 plays them with SndCall(1, n) (the player block): PlSetDamageSe picks pain SE 9, 10 or 11 (game/pl_sub.cpp),
Pl_R0_Die SE 0xD (game/pl_dmg.cpp); pl00.xsb's cues 9 / 10 / 11 / 13 play waves 39 / 38 / 37 / 33.

usage: python -I leon_sounds.py <pl00.xwb> <out dir>   (writes leon_pain_1..3.wav, leon_death.wav, the kick's; with
core.xwb next to pl00.xwb also leon_pickup.wav)"""
import os
import struct
import sys

# (output file, wave index in pl00.xwb)
# and the kick's (em10.cpp plem10Kick: SE 0x11 + 0x10 when the wind-up ends, 0xF on a hit; cues 0x11 / 0x10 / 0xF play waves
# 28 / 29 / 41)
LEON_SOUNDS = [('leon_pain_1.wav', 39), ('leon_pain_2.wav', 38), ('leon_pain_3.wav', 37), ('leon_death.wav', 33),
               ('leon_kicka.wav', 28), ('leon_kickb.wav', 29), ('leon_kickhit.wav', 41)]
# From RE4's core bank (BIO4/snd/core.xwb, block 0: SndCall(0, n), the cue index n), read from next to pl00.xwb when
# it is there: the pickup - taking ammo (game/mes.cpp: the message's "take" choice, attr 0x200: SE 0x11) or a weapon
# (attr 0x100: SE 0xF), both core.xsb cues playing wave 20
CORE_SOUNDS = [('leon_pickup.wav', 20)]


def waves(data):
    """The bank's entries: (channels, rate, bits, pcm bytes) per wave (WAVEBANKHEADER v46: 5 segments)."""
    sig, _version, _header_version = struct.unpack_from('<4sII', data, 0)
    assert sig == b'WBND', 'not an XACT wave bank'
    segments = [struct.unpack_from('<II', data, 12 + 8 * i) for i in range(5)]
    bank = segments[0][0]
    _flags, count = struct.unpack_from('<II', data, bank)
    meta_size, = struct.unpack_from('<I', data, bank + 72)
    out = []
    for i in range(count):
        _duration, fmt, offset, length, _lo, _ll = struct.unpack_from('<6I', data, segments[1][0] + i * meta_size)
        tag, channels, rate, bits = fmt & 3, (fmt >> 2) & 7, (fmt >> 5) & 0x3FFFF, fmt >> 31
        assert tag == 0, f'wave {i}: not PCM'
        start = segments[4][0] + offset
        out.append((channels, rate, 16 if bits else 8, data[start:start + length]))
    return out


def wav(channels, rate, bits, pcm):
    align = channels * bits // 8
    return (b'RIFF' + struct.pack('<I', 36 + len(pcm)) + b'WAVE'
            + b'fmt ' + struct.pack('<IHHIIHH', 16, 1, channels, rate, rate * align, align, bits)
            + b'data' + struct.pack('<I', len(pcm)) + pcm)


if __name__ == '__main__':
    os.makedirs(sys.argv[2], exist_ok=True)
    sounds = [(sys.argv[1], name, index) for name, index in LEON_SOUNDS]
    core = os.path.join(os.path.dirname(os.path.abspath(sys.argv[1])), 'core.xwb')
    if os.path.exists(core):
        sounds += [(core, name, index) for name, index in CORE_SOUNDS]
    else:
        print(f'no {core}: no pickup sound')
    banks = {}
    for path, name, index in sounds:
        if path not in banks:
            banks[path] = waves(open(path, 'rb').read())
        channels, rate, bits, pcm = banks[path][index]
        assert bits == 16, f'{name}: {bits}-bit'
        open(os.path.join(sys.argv[2], name), 'wb').write(wav(channels, rate, bits, pcm))
        print(f'{name}: wave {index}, {channels} ch, {rate} Hz, {len(pcm) / (2 * channels * rate):.2f} s')
