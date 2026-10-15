"""The Leon mod's setup step (the Melty install runs it once before the game first starts): builds Leon from the
player's own Resident Evil 4 (2014 UHD, Steam app 254700) into the mod, and readies the DirectX 9 runtime from their
own Black Ops. Nothing of either game is shipped; both installs are only read.

  leon_setup.exe --re4 <Resident Evil 4 folder> --bo1 <Black Ops folder> --out <the mod's install folder>

  1. unpacks Leon's archives (BIO4/Em/pl00, em10, wep00..wep19 .udas.lfs) and his HD textures
     (BIO4/ImagePackHD/01000000.pack.yz2.lfs) into a work folder (lfsunpack.exe)
  2. converts him (leon_rlm.py --armed: model, textures, RE4's motions baked with the game's own motion code)
     into <out>/mods/leon/rtmodels/leon.rlm
  3. writes his voice (leon_sounds.py: BIO4/snd/pl00.xwb, core.xwb) into <out>/mods/leon/sound/rt
  4. extracts d3dx9_43.dll and xinput1_3.dll from Black Ops' Redist/DirectX next to the game's exe
  5. writes <out>/mods/leon/rtmodels/leon_ready_v<SETUP_VERSION>.txt (what Melty and the launcher wait for: a new
     SETUP_VERSION - Leon converted differently - has them build him again); a later run finding it does nothing
     (--force redoes it)

Its log is <out>/leon_setup.log."""
import argparse
import os
import runpy
import shutil
import subprocess
import sys
import time

# 2: the guns 20 mm further out in his hand, the shotgun's close with its whole pump; 3: Leon's "Bingo?"
SETUP_VERSION = '3'
ARCHIVES = ['pl00', 'em10'] + [f'wep{n:02d}' for n in (0, 1, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 19)]
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


def unpack(src, dst):
    subprocess.run([os.path.join(here, 'lfsunpack.exe'), src, dst], check=True, creationflags=NO_WINDOW,
                   stdout=subprocess.PIPE, stderr=subprocess.STDOUT)


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
    if not args.force and os.path.isfile(ready):
        log('Leon is ready already')
        return
    log(f'setup {SETUP_VERSION}: RE4 {args.re4}, Black Ops {args.bo1}, out {out}')
    started = time.time()

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
        # RE4 reads a file either packed (.lfs) or plain, and texture mods / repacks leave some one way or the other
        # (the game's own ImagePackHD mixes both): either form is taken, the HD textures falling back to ImagePack's
        def fetch(candidates, dest):
            for src in candidates:
                if os.path.isfile(src + '.lfs'):
                    unpack(src + '.lfs', dest)
                    return src + '.lfs'
                if os.path.isfile(src):
                    shutil.copyfile(src, dest)
                    return src
            return None

        for name in ARCHIVES:
            if not fetch([os.path.join(bio4, 'Em', f'{name}.udas')], os.path.join(work, f'{name}.udas')):
                if name in REQUIRED:
                    raise SystemExit(f'missing {os.path.join(bio4, "Em", name + ".udas.lfs")}')
                log(f'no {name}.udas: its motions are left out')
        pack = fetch([os.path.join(bio4, 'ImagePackHD', '01000000.pack.yz2'),
                      os.path.join(bio4, 'ImagePack', '01000000.pack.yz2')], os.path.join(work, '01000000.pack.yz2'))
        if not pack:
            raise SystemExit(f'missing {os.path.join(bio4, "ImagePackHD", "01000000.pack.yz2.lfs")} (nor its plain '
                             f'.pack.yz2, nor ImagePack\'s)')
        log(f'unpacked (textures: {pack})')

        os.makedirs(rtmodels, exist_ok=True)
        rlm = os.path.join(rtmodels, 'leon.rlm')
        run_tool('leon_rlm', [os.path.join(work, 'pl00.udas'), os.path.join(work, '01000000.pack.yz2'),
                              rlm + '.tmp', '--armed'])
        os.replace(rlm + '.tmp', rlm)
        log(f'leon.rlm: {os.path.getsize(rlm)} bytes')

        run_tool('leon_sounds', [os.path.join(bio4, 'snd', 'pl00.xwb'), os.path.join(out, 'mods', 'leon', 'sound', 'rt')])
        log('sounds written')
    finally:
        shutil.rmtree(work, ignore_errors=True)

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
