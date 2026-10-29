"""Leon's voice for the mod's runtime sounds (the engine's src/sound/snd_runtime.cpp): his pain grunts and death scream
from RE4's player sound bank (BIO4/snd/pl00.xwb, an XACT3 wave bank of 16-bit mono PCM), written as WAV files.

RE4 plays them with SndCall(1, n) (the player block): PlSetDamageSe picks pain SE 9, 10 or 11 (game/pl_sub.cpp),
Pl_R0_Die SE 0xD (game/pl_dmg.cpp); pl00.xsb's cues 9 / 10 / 11 / 13 play waves 39 / 38 / 37 / 33. And the gun raised to
aim: leon_aim.wav.

usage: python -I leon_sounds.py <pl00.xwb> <out dir>   (writes leon_pain_1..3.wav, leon_death.wav, the kick's; with
core.xwb next to pl00.xwb also leon_pickup.wav, with bio4evt.xwb the Merchant's lines: merchant_*.wav, and Leon's
"Bingo?": leon_bingo.wav, "Hasta luego!": leon_hasta_luego.wav and "Hey, it's that dog.": leon_that_dog.wav; with em10.xwb
and em1d.xwb Super Salvador's voice and chainsaw: salvador_*.wav; with em2c.xwb Verdugo's: verdugo_*.wav)"""
import array
import math
import mmap
import os
import struct
import sys

# (output file, wave index in pl00.xwb)
# and the kick's (em10.cpp plem10Kick: SE 0x11 + 0x10 when the wind-up ends, 0xF on a hit; cues 0x11 / 0x10 / 0xF play waves
# 28 / 29 / 41)
# and the gun raised to aim (every weapon's ready motion: SE 0x28 two 30 Hz frames in, wep/pl_handgun.cpp
# wep02_r3_ready10; cue 0x28 plays wave 46)
LEON_SOUNDS = [('leon_pain_1.wav', 39), ('leon_pain_2.wav', 38), ('leon_pain_3.wav', 37), ('leon_death.wav', 33),
               ('leon_kicka.wav', 28), ('leon_kickb.wav', 29), ('leon_kickhit.wav', 41), ('leon_aim.wav', 46)]
# The other playable characters' voices (the engine's CG_LeonVoice: "<character>_<sound>", Leon's where one has none),
# from their own banks next to pl00.xwb, at the same cues: pain 9 / 10 / 11, death 0xD, the kick's grunt 0x10 (its
# whoosh, cue 0x11, is the same wave in every bank: Leon's plays). Ada's bank is pl0b (no pl0c bank: the same voice);
# Luis, never the player, has a partner's cue table: his pain and death at those cues all the same (pl14/pl14.cpp
# SndCall 9 on a hit, 0xD as he dies), no kick - his shortest pain grunt for it. (bank, [(output file, wave)])
CHARACTER_SOUNDS = [
    ('pl0b.xwb', [('ada_pain_1.wav', 50), ('ada_pain_2.wav', 51), ('ada_pain_3.wav', 52), ('ada_death.wav', 56),
                  ('ada_kickb.wav', 59)]),
    ('pl06.xwb', [('hunk_pain_1.wav', 52), ('hunk_pain_2.wav', 53), ('hunk_pain_3.wav', 54), ('hunk_death.wav', 58),
                  ('hunk_kickb.wav', 62)]),
    ('pl14.xwb', [('luis_pain_1.wav', 4), ('luis_pain_2.wav', 5), ('luis_pain_3.wav', 6), ('luis_death.wav', 8),
                  ('luis_kickb.wav', 4)]),
]
# the levels some are brought to (loud: dB RMS): the aim's soft rustle is mastered ~24 dB under his voice, for RE4's quiet
# - lost under BO1's fight, it is brought up ~15 dB, still well under his voice; the heal's chime (-17.6 dB), played as
# his lines at Juggernog (the announcer's settings), went unheard ~6 dB under them: brought up to theirs
LEVELS = {'leon_aim.wav': -22.0, 'leon_heal.wav': -12.0}
# the speeds some play at (their rate raised: shorter and higher): Super Salvador's chainsaw rev a fifth faster, scarier
SPEEDS = {'salvador_sawrev.wav': 1.2}
# From RE4's core bank (BIO4/snd/core.xwb, block 0: SndCall(0, n), the cue index n), read from next to pl00.xwb when
# it is there: the pickup - taking ammo (game/mes.cpp: the message's "take" choice, attr 0x200: SE 0x11) or a weapon
# (attr 0x100: SE 0xF), both core.xsb cues playing wave 20; and an item used from the case, a heal (Sscrn/ss_item.cpp:
# SE 8, cue 8 playing wave 17), for a perk with no drink (maps/leon/_drink.gsc)
CORE_SOUNDS = [('leon_pickup.wav', 20), ('leon_heal.wav', 17)]
# From the Merchant's own bank (BIO4/snd/em10.xwb, the robed Ganado he is: em10/em10.cpp em10_R1_Trade's SndCall(8, n),
# em10.xsb cue n), for the Mystery Box's teddy bear: "Come back any time." as he closes his coat after the shop (cue
# 0x9A playing wave 50, 16-bit mono 16 kHz)
MERCHANT_SOUNDS = [('merchant_comeback.wav', 50)]
# Super Salvador in the dog rounds (maps/leon/_salvador.gsc), from the Ganados' banks next to pl00.xwb (bank, [(output
# file, wave)]): the island's (em1d.xwb) "¡Te voy a matar!" as he comes (wave 50, its soldiers' find shout: cue 0x22,
# Se_tbl[4] of voice set 1, the island chainsaw carrier's - em20/em20_set.cpp) and his chase laugh (wave 47, cue 0x27,
# the set's Se_tbl[7] played every 4 to 8 s on the move: em10.cpp em10BreathSe) and his shouts through the chainsaw
# walk (em10.cpp em10_R1_CSawWalkAtk: Se_tbl[12] as it starts, cue 0x55, wave 27; Se_tbl[15] now and then as it goes
# round, cue 0x58, wave 51) and its swish on each sweep (SndCall(8, 0x3D): cue 0x3D, wave 117); the village chainsaw
# man's (em10.xwb, voice set 3: em10.cpp Em10SetSeTbl) grunt as he flinches (cue 0x6F, wave 111) and death (cue 0x36,
# wave 116); and the chainsaw of the Mercenaries' Waterworld (room/st4/r404.xwb, room 0x404: mercenaries.cpp
# MercSysInitRoom's stage 3), its start as he comes (SndCall(6, 0x4C): cue 0x4C, wave 9) and its rev as he swings it
# (SndCall(6, 0x50): cue 0x50, wave 3). The waves picked by ear where RE4's code does not name the line.
SALVADOR_SOUNDS = [
    ('em1d.xwb', [('salvador_spawn.wav', 50), ('salvador_laugh.wav', 47), ('salvador_chase_1.wav', 27),
                  ('salvador_chase_2.wav', 51), ('salvador_sawswing.wav', 117)]),
    ('em10.xwb', [('salvador_pain.wav', 111), ('salvador_death.wav', 116)]),
    ('room/st4/r404.xwb', [('salvador_sawstart.wav', 9), ('salvador_sawrev.wav', 3)]),
]
# Verdugo in Five's special round (maps/leon/_verdugo.gsc), from his own bank (em2c.xwb, SndCall(8, n) in
# em2c/em2c.cpp = em2c.xsb cue n): his roar (Threat: cue 0x45, wave 8), his scream as he comes (Ambush: cue 0x40,
# wave 0), the slash's swish (the claw's and the tail's motion sequences: cue 0x26, wave 2), the tail's rattle through
# its sweep (cue 0x3A, wave 34), his growls as he winds up (AtkSign's sequence: cues 0x3D / 0x3E, waves 30 / 31), his
# hurt cries (em2cDmCk: cues 9 / 6, waves 5 / 13), a hit landing (em2cAtkCk: cue 0xD, wave 24) and his death scream
# (Die_Normal's sequence, its first: cue 0x44, wave 32)
VERDUGO_SOUNDS = [
    ('em2c.xwb', [('verdugo_roar.wav', 8), ('verdugo_scream.wav', 0), ('verdugo_swish.wav', 2), ('verdugo_tail.wav', 34),
                  ('verdugo_growl_1.wav', 30), ('verdugo_growl_2.wav', 31), ('verdugo_pain_1.wav', 5),
                  ('verdugo_pain_2.wav', 13), ('verdugo_hit.wav', 24), ('verdugo_death.wav', 32)]),
]
# The Merchant's shop lines, for the Mystery Box (maps/leon/_merchant.gsc), by the weapon's tier: RE4's shop
# (Sscrn/ss_shop.cpp shop_msg[]: shopStrPlay -> SndStrReq(1, n), the event stream block; on PC bio4evt.xsb cue n, named
# by the decimal n) plays streams 0xB4..0xC2 = cues 180..194 = bio4evt.xwb waves 5..19 (16-bit stereo 32 kHz); the
# lines are the shop's subtitles (SS/eng/ss_cmmn.dat). A tier's lines are one alias's variants (picked at random);
# written mono, as loud as Leon's voice (loud).
EVT_SOUNDS = [('merchant_amazing_1.wav', 14),  # 0xBD "Stranger, stranger, now that's a weapon!"
              ('merchant_strong_1.wav', 15),   # 0xBE "Not only will you need cash but you'll need guts to buy that weapon."
              ('merchant_strong_2.wav', 16),   # 0xBF "Stranger... What you need that for? Going huntin' an elephant?"
              ('merchant_good_1.wav', 18),     # 0xC1 "Ahh... A choice of an avid gun collector. It's a nice gun, stranger."
              ('merchant_good_2.wav', 17),     # 0xC0 "A wise choice, mate. Its ammo will penetrate just about anything."
              ('merchant_good_3.wav', 5),      # 0xB4 "Hehehe, thank you!"
              ('merchant_weak_1.wav', 8),      # 0xB7 "Is that all, stranger?"
              ('merchant_buyin.wav', 12),      # 0xBB "What're ya buyin'?"
              ('merchant_nocash.wav', 9)]      # 0xB8 "Not enough cash, stranger."
# Leon's "Bingo?" in the village (st1/r101.cpp r101_Event30, the bell's event r101s30: its one Str packet plays event
# stream 13 from frame 0 = bio4evt.xsb cue 13 = wave 138, the scene's whole pre-mixed soundtrack, 16-bit stereo 32 kHz;
# the line's subtitle - message 4 - shows at event frame 990, 33.03 s): the whole line cut from it, 32.95 s to
# 35.12 s ("Where's everyone going?" from 33.01 s, "Bingo?" 34.69 s to 35.06 s), the music bed taken out
# (centre_voice: his voice is centred, the bed wide; the bed's own spectrum from the pause between the two, 33.97 s
# to 34.65 s). (name, wave, start s, end s, bed start s, bed end s)
# Two more cut the same way: "Hasta luego!" to Mendez in the barn (st1/r11f.cpp: event r11fs02, its Str packet event
# stream 71 = cue 71 = wave 101; the subtitle at 10.00 s; the bed the fire and music just before it), and "Hey, it's
# that dog." in the village square as the dog comes to help with El Gigante (st1/r119.cpp r119_EventDogAppear: event
# r119s10, stream 26 = cue 26 = wave 153; the subtitle at 6.93 s; the bed the quiet before it).
EVT_CLIPS = [('leon_bingo.wav', 138, 32.95, 35.12, 33.97, 34.65),
             ('leon_hasta_luego.wav', 101, 9.86, 10.62, 9.50, 9.86),
             ('leon_that_dog.wav', 153, 6.98, 8.24, 6.72, 7.01)]


def waves(data):
    """The bank's entries: (channels, rate, bits, pcm bytes) per wave (WAVEBANKHEADER v46: 5 segments)."""
    sig, _version, _header_version = struct.unpack_from('<4sII', data, 0)
    assert sig == b'WBND', 'not an XACT wave bank'
    segments = [struct.unpack_from('<II', data, 12 + 8 * i) for i in range(5)]
    bank = segments[0][0]
    _flags, count = struct.unpack_from('<II', data, bank)
    meta_size, = struct.unpack_from('<I', data, bank + 72)
    out = []
    view = memoryview(data)  # (slices without copies: bio4evt.xwb is 655 MB)
    for i in range(count):
        _duration, fmt, offset, length, _lo, _ll = struct.unpack_from('<6I', data, segments[1][0] + i * meta_size)
        tag, channels, rate, bits = fmt & 3, (fmt >> 2) & 7, (fmt >> 5) & 0x3FFFF, fmt >> 31
        assert tag == 0, f'wave {i}: not PCM'
        start = segments[4][0] + offset
        out.append((channels, rate, 16 if bits else 8, view[start:start + length]))
    return out


def loud(pcm, target_db=-12.0, knee=0.7):
    """16-bit PCM brought to target_db RMS (the Merchant's lines sit ~9 dB under Leon's voice, mastered for RE4's
    quiet shop: in the game they were lost under the box's music), its peaks over the knee bent under full scale."""
    samples = array.array('h', bytes(pcm))
    rms = math.sqrt(sum(x * x for x in samples) / max(1, len(samples))) / 32768.0
    gain = 10.0 ** (target_db / 20.0) / rms if rms > 0 else 1.0
    out = array.array('h')
    for x in samples:
        y = x / 32768.0 * gain
        a = abs(y)
        if a > knee:
            a = knee + (1.0 - knee) * math.tanh((a - knee) / (1.0 - knee))
        out.append(int(math.copysign(a, y) * 32767.0))
    return out.tobytes()


def voiced(pcm, rate, target_db=-13.0, knee=0.85):
    """For a line cut from a scene's mix (EVT_CLIPS): brought to target_db RMS measured over its voiced 20 ms windows
    only, its peaks over a high knee bent under full scale. As loud measured it, over the whole clip, a line's pauses
    counted as quiet and its voice was pushed far over the knee ("Bingo?" sounded saturated); set by its peak alone,
    a short one ended up too quiet ("Hasta luego!")."""
    samples = array.array('h', bytes(pcm))
    win = max(1, rate // 50)
    energies = []
    for i in range(0, len(samples), win):
        chunk = samples[i:i + win]
        energies.append(sum(x * x for x in chunk) / max(1, len(chunk)))
    floor = max(energies, default=0) * 10.0 ** (-30.0 / 10.0)  # windows within 30 dB of the loudest
    loud_windows = [e for e in energies if e >= floor]
    rms = math.sqrt(sum(loud_windows) / max(1, len(loud_windows))) / 32768.0
    gain = 10.0 ** (target_db / 20.0) / rms if rms > 0 else 1.0
    out = array.array('h')
    for x in samples:
        y = x / 32768.0 * gain
        a = abs(y)
        if a > knee:
            a = knee + (1.0 - knee) * math.tanh((a - knee) / (1.0 - knee))
        out.append(int(math.copysign(a, y) * 32767.0))
    return out.tobytes()


def fft(x, inverse=False):
    """In-place iterative radix-2 FFT of a list of complex numbers (its length a power of two)."""
    n = len(x)
    j = 0
    for i in range(1, n):
        bit = n >> 1
        while j & bit:
            j ^= bit
            bit >>= 1
        j |= bit
        if i < j:
            x[i], x[j] = x[j], x[i]
    size = 2
    sign = 1.0 if inverse else -1.0
    while size <= n:
        w = complex(math.cos(2.0 * math.pi / size), sign * math.sin(2.0 * math.pi / size))
        half = size // 2
        for start in range(0, n, size):
            wk = 1.0 + 0.0j
            for k in range(start, start + half):
                t = wk * x[k + half]
                x[k + half] = x[k] - t
                x[k] = x[k] + t
                wk *= w
        size *= 2
    if inverse:
        for i in range(n):
            x[i] /= n
    return x


def centre_voice(left, right, bed_left, bed_right, n=1024, hop=256):
    """A centred voice out of a wide stereo mix (floats, -1..1): per short-time spectrum bin the two channels' mid,
    kept as far as they agree (a centred source is the same in both: 2 Re(L conj R) / (|L|^2 + |R|^2) near 1), less
    the bed's own level there (spectral subtraction, the bed measured from bed_left / bed_right, a stretch without
    the voice). Returns the mono voice, as long as the input."""
    window = [0.5 - 0.5 * math.cos(2.0 * math.pi * i / n) for i in range(n)]

    def frames(sig):
        padded = [0.0] * n + list(sig) + [0.0] * n
        for start in range(0, len(padded) - n + 1, hop):
            yield start, [complex(padded[start + i] * window[i], 0.0) for i in range(n)]

    # the bed: its mean magnitude per bin over the stretch without the voice
    bed = [0.0] * n
    count = 0
    for (_s, l), (_t, r) in zip(frames(bed_left), frames(bed_right)):
        fl, fr = fft(l), fft(r)
        for k in range(n):
            bed[k] += abs((fl[k] + fr[k]) * 0.5)
        count += 1
    bed = [b / max(1, count) for b in bed]
    length = len(left)
    out = [0.0] * (length + 2 * n)
    norm = [0.0] * (length + 2 * n)
    for (start, l), (_t, r) in zip(frames(left), frames(right)):
        fl, fr = fft(l), fft(r)
        mid = []
        for k in range(n):
            a, b = fl[k], fr[k]
            power = abs(a) ** 2 + abs(b) ** 2
            agree = 2.0 * (a * b.conjugate()).real / power if power > 1e-12 else 0.0
            agree = max(0.0, agree) ** 2
            m = (a + b) * 0.5
            mag = abs(m)
            kept = max(0.0, mag - 1.5 * bed[k]) / mag if mag > 1e-12 else 0.0
            mid.append(m * agree * kept)
        back = fft(mid, inverse=True)
        for i in range(n):
            out[start + i] += back[i].real * window[i]
            norm[start + i] += window[i] * window[i]
    return [out[n + i] / norm[n + i] if norm[n + i] > 1e-6 else 0.0 for i in range(length)]


def clip_voice(pcm, rate, start, end, bed_start, bed_end, fade=0.01):
    """The 16-bit stereo PCM's start..end seconds, its centred voice alone (centre_voice), faded in and out over fade
    seconds; mono 16-bit PCM."""
    samples = array.array('h', bytes(pcm))

    def stereo(a, b):
        i, j = int(a * rate), int(b * rate)
        return ([samples[2 * f] / 32768.0 for f in range(i, j)], [samples[2 * f + 1] / 32768.0 for f in range(i, j)])

    left, right = stereo(start, end)
    bed_left, bed_right = stereo(bed_start, bed_end)
    voice = centre_voice(left, right, bed_left, bed_right)
    ramp = max(1, int(fade * rate))
    for i in range(min(ramp, len(voice))):
        voice[i] *= i / ramp
        voice[-1 - i] *= i / ramp
    return array.array('h', [int(max(-1.0, min(1.0, v)) * 32767.0) for v in voice]).tobytes()


def wav(channels, rate, bits, pcm):
    align = channels * bits // 8
    return (b'RIFF' + struct.pack('<I', 36 + len(pcm)) + b'WAVE'
            + b'fmt ' + struct.pack('<IHHIIHH', 16, 1, channels, rate, rate * align, align, bits)
            + b'data' + struct.pack('<I', len(pcm)) + pcm)


if __name__ == '__main__':
    os.makedirs(sys.argv[2], exist_ok=True)
    sounds = [(sys.argv[1], name, index) for name, index in LEON_SOUNDS]
    for bank, bank_sounds in CHARACTER_SOUNDS:
        path = os.path.join(os.path.dirname(os.path.abspath(sys.argv[1])), bank)
        if os.path.exists(path):
            sounds += [(path, name, index) for name, index in bank_sounds]
        else:
            print(f"no {path}: {bank_sounds[0][0].split('_')[0]} has Leon's voice")
    core = os.path.join(os.path.dirname(os.path.abspath(sys.argv[1])), 'core.xwb')
    if os.path.exists(core):
        sounds += [(core, name, index) for name, index in CORE_SOUNDS]
    else:
        print(f'no {core}: no pickup or heal sound')
    merchant = os.path.join(os.path.dirname(os.path.abspath(sys.argv[1])), 'em10.xwb')
    if os.path.exists(merchant):
        sounds += [(merchant, name, index) for name, index in MERCHANT_SOUNDS]
    else:
        print(f"no {merchant}: no Merchant's goodbye")
    for bank, bank_sounds in SALVADOR_SOUNDS + VERDUGO_SOUNDS:
        path = os.path.join(os.path.dirname(os.path.abspath(sys.argv[1])), bank)
        if os.path.exists(path):
            sounds += [(path, name, index) for name, index in bank_sounds]
        else:
            print(f'no {path}: no {", ".join(n for n, _i in bank_sounds)}')
    evt =os.path.join(os.path.dirname(os.path.abspath(sys.argv[1])), 'bio4evt.xwb')
    if os.path.exists(evt):
        sounds += [(evt, name, index) for name, index in EVT_SOUNDS]
        sounds += [(evt, clip[0], clip) for clip in EVT_CLIPS]
    else:
        print(f'no {evt}: no Merchant')
    banks = {}
    for path, name, index in sounds:
        if path not in banks:
            with open(path, 'rb') as f:  # (mapped: read only what the waves need)
                banks[path] = waves(mmap.mmap(f.fileno(), 0, access=mmap.ACCESS_READ))
        clip = index if isinstance(index, tuple) else None
        if clip:
            index = clip[1]
        channels, rate, bits, pcm = banks[path][index]
        assert bits == 16, f'{name}: {bits}-bit'
        if clip:
            assert channels == 2, f'{name}: a clip needs the stereo mix'
            pcm = voiced(clip_voice(pcm, rate, *clip[2:]), rate)
            channels = 1
        elif channels == 2:
            # mono, as every other runtime sound (the Merchant's lines are stereo)
            frames = array.array('h', bytes(pcm))
            pcm = array.array('h', [(frames[i] + frames[i + 1]) // 2 for i in range(0, len(frames) - 1, 2)]).tobytes()
            channels = 1
        if name.startswith(('merchant_', 'salvador_', 'verdugo_')):
            pcm = loud(pcm)
        elif name in LEVELS:
            pcm = loud(pcm, LEVELS[name])
        rate = int(rate * SPEEDS.get(name, 1.0))
        open(os.path.join(sys.argv[2], name), 'wb').write(wav(channels, rate, bits, pcm))
        print(f'{name}: wave {index}, {channels} ch, {rate} Hz, {len(pcm) / (2 * channels * rate):.2f} s')
