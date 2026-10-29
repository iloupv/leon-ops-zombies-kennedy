"""The Leon mod's setup step (the Melty install runs it once before the game first starts): builds Leon from the
player's own Resident Evil 4 (2014 UHD, Steam app 254700) into the mod, and readies the DirectX 9 runtime from their
own Black Ops. Nothing of either game is shipped; both installs are only read.

  leon_setup.exe --re4 <Resident Evil 4 folder> --bo1 <Black Ops folder> --out <the mod's install folder>

  1. unpacks Leon's archives (BIO4/Em/pl00, em10, wep00..wep19 .udas.lfs) and his HD textures
     (BIO4/ImagePackHD/01000000.pack.yz2.lfs) into a work folder (lfsunpack.exe)
  2. converts him (leon_rlm.py --armed: model, textures, RE4's motions baked with the game's own motion code)
     into <out>/mods/leon/rtmodels/leon.rlm, and Ada, HUNK and Luis (pl0c, pl06, pl14: --character) into ada.rlm,
     hunk.rlm and luis.rlm (any that fails is left out; Leon still plays)
  3. converts the enemies (ENEMIES, leon_rlm.py --enemy: Super Salvador from em20 and its texture pack 00000020) into
     salvador.rlm (one that fails is left out: the dog rounds go without him)
  4. writes his voice (leon_sounds.py: BIO4/snd/pl00.xwb, core.xwb, ...) into <out>/mods/leon/sound/rt
  5. extracts d3dx9_43.dll and xinput1_3.dll from Black Ops' Redist/DirectX next to the game's exe
  6. writes <out>/mods/leon/rtmodels/leon_ready_v<SETUP_VERSION>.txt (what Melty and the launcher wait for: a new
     SETUP_VERSION - Leon converted differently - has them build him again); a later run finding it does nothing
     (--force redoes it). From a version whose characters this one keeps (CHARACTERS_KEPT_FROM) only steps 3 and 4
     run again.

Its log is <out>/leon_setup.log."""
import argparse
import os
import runpy
import shutil
import struct
import subprocess
import sys
import time

# 2: the guns 20 mm further out in his hand, the shotgun's close with its whole pump; 3: Leon's "Bingo?"; 4: the gun's
# raise to aim (leon_aim); 5: Ada, HUNK and Luis (CHARACTERS), "Hasta luego!" and "Hey, it's that dog."; 8: the perk's
# heal sound (leon_heal) and the Merchant's "Come back any time." (merchant_comeback); 9: Super Salvador (ENEMIES) and
# his sounds (salvador_*)
SETUP_VERSION = '9'
# the versions whose characters are this one's: from them only the enemies and the sounds are built (seconds)
CHARACTERS_KEPT_FROM = ('7', '8')
# the other playable characters: (name, their archive, their texture pack) - leon_rlm.py --character
CHARACTERS = [('ada', 'pl0c', '0100000c'), ('hunk', 'pl06', '01000006'), ('luis', 'pl14', '01000014')]
# the enemies: (name, their archive, their texture pack) - leon_rlm.py --enemy
ENEMIES = [('salvador', 'em20', '00000020')]
ARCHIVES = (['pl00', 'em10'] + [f'wep{n:02d}' for n in (0, 1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 19)]
            + [archive for _name, archive, _pack in CHARACTERS])
REQUIRED = {'pl00', 'wep01'}
DIRECTX = [('Jun2010_d3dx9_43_x86.cab', 'd3dx9_43.dll'), ('APR2007_xinput_x86.cab', 'xinput1_3.dll')]
NO_WINDOW = 0x08000000  # CREATE_NO_WINDOW

here = sys._MEIPASS if getattr(sys, 'frozen', False) else os.path.dirname(os.path.abspath(__file__))
sys.path.insert(0, here)
log_file = None


def log(text):
    line = time.strftime('%H:%M:%S ') + text
    print(line, flush=True)
    if log_file:
        log_file.write(line + '\n')
        log_file.flush()


def bio4_folder(re4):
    for folder in (os.path.join(re4, 'BIO4'), re4):
        if os.path.isfile(os.path.join(folder, 'Em', 'pl00.udas.lfs')):
            return folder
    raise SystemExit(f'Resident Evil 4 not found in {re4}: it needs BIO4\\Em\\pl00.udas.lfs (the 2014 UHD edition)')


def check_multiplayer(bo1):
    r"""Black Ops Multiplayer's files (zone\Common\code_pre_gfx_mp.ff, ...): the mod's zombies run on them, and Steam
    installs them with the game's Multiplayer, an app of its own beside the campaign's. A player without them saw the
    game stop at its start on a zone it could not find."""
    zone = os.path.join(bo1, 'zone')
    if os.path.isfile(os.path.join(zone, 'Common', 'code_pre_gfx_mp.ff')):
        return
    if os.path.isdir(zone) and any(os.path.isfile(os.path.join(zone, d, 'code_pre_gfx_mp.ff')) for d in os.listdir(zone)):
        return
    raise SystemExit(f'Black Ops Multiplayer is not installed (no zone\\Common\\code_pre_gfx_mp.ff in {bo1}): install '
                     f'"Call of Duty: Black Ops - Multiplayer" from your Steam library - it comes with Black Ops, and '
                     f'the mod runs on its files')


def unpack(src, dst):
    subprocess.run([os.path.join(here, 'lfsunpack.exe'), src, dst], check=True, creationflags=NO_WINDOW,
                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)


def is_texture_pack(path):
    """An RE4 texture pack (uhd_pack: a pack id, an image count, their offsets), not some other file by its name."""
    try:
        with open(path, 'rb') as f:
            head = f.read(8)
        if len(head) < 8:
            return False
        _pack_id, count = struct.unpack('<II', head)
        return 0 < count < 100000
    except OSError:
        return False


def pack_candidates(bio4, pack='01000000'):
    """Where Leon's texture pack (his TPLs' pack 0x01000000) may be, best first: ImagePackHD's, then ImagePack's, packed
    (.lfs) or plain, under any letter case - and as a texture mod or repack left it: the original renamed out of the way
    (.bak, .old, ...: mods that replace a pack set the .lfs aside so the game reads theirs), or moved deeper into BIO4."""
    found = []
    for folder in ('ImagePackHD', 'ImagePack'):
        d = os.path.join(bio4, folder)
        if not os.path.isdir(d):
            continue
        names = sorted(os.listdir(d), key=lambda n: (not n.lower().endswith('.pack.yz2.lfs'),
                                                     not n.lower().endswith('.pack.yz2'), n.lower()))
        found += [os.path.join(d, n) for n in names if n.lower().startswith(pack + '.pack')]
    for root, dirs, files in os.walk(bio4):
        if root.count(os.sep) - bio4.count(os.sep) >= 3:
            dirs[:] = []
        found += [os.path.join(root, n) for n in sorted(files) if n.lower().startswith(pack + '.pack')
                  and os.path.join(root, n) not in found]
    return found


def describe_folders(bio4):
    """What the texture folders hold (the log, when Leon's pack is not found: the next report shows the layout)."""
    for folder in ('ImagePackHD', 'ImagePack'):
        d = os.path.join(bio4, folder)
        if not os.path.isdir(d):
            log(f'  no {folder} folder')
            continue
        names = sorted(os.listdir(d))
        log(f'  {folder}: {len(names)} entries, e.g. {", ".join(names[:12])}')
    log(f'  BIO4: {", ".join(sorted(os.listdir(bio4))[:30])}')


def run_tool(module, argv):
    """One of the converter's scripts, as if run from the command line (they read sys.argv)."""
    saved = sys.argv
    sys.argv = [module + '.py'] + argv
    try:
        runpy.run_module(module, run_name='__main__')
    finally:
        sys.argv = saved


def main():
    global log_file
    ap = argparse.ArgumentParser(description='Builds Leon from your Resident Evil 4 into the Leon mod.')
    ap.add_argument('--re4', required=True, help='the Resident Evil 4 (2014) folder')
    ap.add_argument('--bo1', help='the Call of Duty: Black Ops folder (DirectX 9 runtime)')
    ap.add_argument('--out', required=True, help="the mod's install folder (BO1Zombies.exe's)")
    ap.add_argument('--force', action='store_true', help='redo it even if it is done')
    args = ap.parse_args()

    out = os.path.abspath(args.out)
    os.makedirs(out, exist_ok=True)
    log_file = open(os.path.join(out, 'leon_setup.log'), 'a', encoding='utf-8')
    rtmodels = os.path.join(out, 'mods', 'leon', 'rtmodels')
    ready = os.path.join(rtmodels, f'leon_ready_v{SETUP_VERSION}.txt')
    if args.bo1: # (at every start: it may have been uninstalled since)
        check_multiplayer(args.bo1)
    if not args.force and os.path.isfile(ready):
        log('Leon is ready already')
        return
    started = time.time()
    if not args.force and any(os.path.isfile(os.path.join(rtmodels, f'leon_ready_v{v}.txt')) for v in CHARACTERS_KEPT_FROM):
        log(f'setup {SETUP_VERSION}: the enemies and the sounds only, RE4 {args.re4}, out {out}')
        bio4 = bio4_folder(args.re4)
        work = os.path.join(out, 'leon_setup_work')
        shutil.rmtree(work, ignore_errors=True)
        os.makedirs(work)
        try:
            build_enemies(bio4, work, rtmodels)
            write_sounds(bio4, out)
        finally:
            shutil.rmtree(work, ignore_errors=True)
        write_ready(ready, started)
        return
    log(f'setup {SETUP_VERSION}: RE4 {args.re4}, Black Ops {args.bo1}, out {out}')

    # the DirectX 9 runtime the game loads (Black Ops installs it from these cabs; a fresh Windows lacks it)
    if args.bo1:
        for cab, dll in DIRECTX:
            if os.path.isfile(os.path.join(out, dll)):
                continue
            cab_path = os.path.join(args.bo1, 'Redist', 'DirectX', cab)
            if os.path.isfile(cab_path):
                subprocess.run(['expand.exe', cab_path, f'-F:{dll}', out], check=True, creationflags=NO_WINDOW,
                               stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
                log(f'{dll} from {cab}')
            else:
                log(f'no {cab_path}: {dll} must come from the system (DirectX End-User Runtime)')

    bio4 = bio4_folder(args.re4)
    work = os.path.join(out, 'leon_setup_work')
    shutil.rmtree(work, ignore_errors=True)
    os.makedirs(work)
    try:
        for name in ARCHIVES:
            if not fetch([os.path.join(bio4, 'Em', f'{name}.udas')], os.path.join(work, f'{name}.udas')):
                if name in REQUIRED:
                    raise SystemExit(f'missing {os.path.join(bio4, "Em", name + ".udas.lfs")}')
                log(f'no {name}.udas: its motions are left out')
        pack_dest = os.path.join(work, '01000000.pack.yz2')
        pack = fetch_pack(bio4, '01000000', pack_dest)
        # and the shared pack, where a TPL may point too (the RE4 HD Project's Leon has his face and eyes there)
        shared_dest = os.path.join(work, '07000000.pack')
        shared = fetch_pack(bio4, '07000000', shared_dest)
        packs_arg = pack_dest + (';' + shared_dest if shared else '')
        if shared:
            log(f'shared textures: {shared}')
        if pack:
            log(f'unpacked (textures: {pack})')
        else:
            log(f'no Leon texture pack ({os.path.join(bio4, "ImagePackHD", "01000000.pack.yz2.lfs")}, nor its plain '
                f'.pack.yz2, nor ImagePack\'s): Leon is built without his textures (grey)')
            describe_folders(bio4)
            with open(pack_dest, 'wb') as f:
                f.write(struct.pack('<II', 0x01000000, 0))

        os.makedirs(rtmodels, exist_ok=True)
        rlm = os.path.join(rtmodels, 'leon.rlm')
        run_tool('leon_rlm', [os.path.join(work, 'pl00.udas'), packs_arg, rlm + '.tmp', '--armed'])
        os.replace(rlm + '.tmp', rlm)
        log(f'leon.rlm: {os.path.getsize(rlm)} bytes')

        # the others, each on its own: one that fails is left out of the game's choice (bo1_rtmodels_loaded), Leon
        # still plays
        for name, archive, pack_name in CHARACTERS:
            udas = os.path.join(work, f'{archive}.udas')
            if not os.path.isfile(udas):
                continue
            try:
                own_dest = os.path.join(work, f'{pack_name}.pack')
                if not fetch_pack(bio4, pack_name, own_dest):
                    log(f'no {pack_name} texture pack: {name} is built grey')
                    with open(own_dest, 'wb') as f:
                        f.write(struct.pack('<II', int(pack_name, 16), 0))
                char_rlm = os.path.join(rtmodels, f'{name}.rlm')
                run_tool('leon_rlm', [udas, own_dest + (';' + shared_dest if shared else ''), char_rlm + '.tmp',
                                      '--armed', '--character', name])
                os.replace(char_rlm + '.tmp', char_rlm)
                log(f'{name}.rlm: {os.path.getsize(char_rlm)} bytes')
            except Exception as e:  # noqa: BLE001 - Leon still plays without this one
                log(f'{name}: not built ({e!r})')

        build_enemies(bio4, work, rtmodels)
        write_sounds(bio4, out)
    finally:
        shutil.rmtree(work, ignore_errors=True)
    write_ready(ready, started)


# RE4 reads a file either packed (.lfs) or plain, and texture mods / repacks leave some one way or the other (the game's
# own ImagePackHD mixes both): either form is taken, the HD textures falling back to ImagePack's
def fetch(candidates, dest):
    for src in candidates:
        if os.path.isfile(src + '.lfs'):
            unpack(src + '.lfs', dest)
            return src + '.lfs'
        if os.path.isfile(src):
            shutil.copyfile(src, dest)
            return src
    return None


# A model's textures: the first candidate that is a real pack once read (a packed one unpacked first); with none, it is
# built grey and the player still plays (installs with the RE4 HD Project failed the whole setup here)
def fetch_pack(bio4, name, dest):
    for src in pack_candidates(bio4, name):
        try:
            if '.lfs' in os.path.basename(src).lower():
                unpack(src, dest)
            else:
                shutil.copyfile(src, dest)
        except (OSError, subprocess.CalledProcessError) as e:
            log(f'  {src}: not read ({e})')
            continue
        if is_texture_pack(dest):
            return src
        log(f'  {src}: not a texture pack')
    return None


# the enemies, each on its own: one that fails is left out (bo1_rtmodels_loaded), the game plays without it
def build_enemies(bio4, work, rtmodels):
    os.makedirs(rtmodels, exist_ok=True)
    for name, archive, pack_name in ENEMIES:
        try:
            udas = os.path.join(work, f'{archive}.udas')
            if not fetch([os.path.join(bio4, 'Em', f'{archive}.udas')], udas):
                log(f'no {archive}.udas: no {name}')
                continue
            pack = os.path.join(work, f'{pack_name}.pack')
            if not fetch_pack(bio4, pack_name, pack):
                log(f'no {pack_name} texture pack: {name} is built grey')
                with open(pack, 'wb') as f:
                    f.write(struct.pack('<II', int(pack_name, 16), 0))
            out_rlm = os.path.join(rtmodels, f'{name}.rlm')
            run_tool('leon_rlm', [udas, pack, out_rlm + '.tmp', '--enemy', name])
            os.replace(out_rlm + '.tmp', out_rlm)
            log(f'{name}.rlm: {os.path.getsize(out_rlm)} bytes')
        except Exception as e:  # noqa: BLE001 - the game plays without this one
            log(f'{name}: not built ({e!r})')


def write_sounds(bio4, out):
    run_tool('leon_sounds', [os.path.join(bio4, 'snd', 'pl00.xwb'), os.path.join(out, 'mods', 'leon', 'sound', 'rt')])
    log('sounds written')


def write_ready(ready, started):
    with open(ready, 'w') as f:
        f.write(SETUP_VERSION + '\n')
    log(f'Leon is ready ({time.time() - started:.0f} s)')


if __name__ == '__main__':
    try:
        main()
    except SystemExit as e:
        if e.code not in (None, 0):
            log(f'FAILED: {e.code}')
        raise
    except Exception as e:  # noqa: BLE001 - the log is all a player has
        import traceback
        log('FAILED: ' + ''.join(traceback.format_exception(e)))
        sys.exit(1)
