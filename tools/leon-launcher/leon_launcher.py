"""Leon Ops: Zombies Kennedy - the launcher for playing without Melty (LeonLauncher.exe, next to BO1Zombies.exe in a
release). It does what Melty's install recipe (melty.json) does:

  1. finds Call of Duty: Black Ops (Steam app 42700) and Resident Evil 4 (2014, app 254700) in the Steam libraries
     (steamapps/libraryfolders.vdf, each app's appmanifest), or asks for their folders once; remembered in
     leon_launcher.json
  2. the first time, builds Leon from Resident Evil 4 (leon_setup\\leon_setup.exe, until mods\\leon\\rtmodels\\
     leon_ready_v8.txt is there: the converter's SETUP_VERSION)
  3. starts BO1Zombies.exe with BO1Z_GAME_DIR = the Black Ops folder and the mod's command line

Neither game's folder is changed."""
import json
import os
import re
import subprocess
import sys

APPS = {
    'bo1': ('42700', 'Call of Duty: Black Ops', lambda d: os.path.isdir(os.path.join(d, 'zone'))
            and os.path.isfile(os.path.join(d, 'localization.txt'))),
    're4': ('254700', 'Resident Evil 4 (2014, Ultimate HD Edition)',
            lambda d: os.path.isfile(os.path.join(d, 'BIO4', 'Em', 'pl00.udas.lfs'))),
}
GAME_ARGS = ['+set', 'bo1_zombies', '1', '+set', 'net_ip', '127.0.0.1', '+set', 'fs_game', 'mods/leon',
             '+set', 'bo1_rtmodels', 'leon', '+exec', 'leon.cfg']

here = os.path.dirname(os.path.abspath(sys.executable if getattr(sys, 'frozen', False) else __file__))
if not getattr(sys, 'frozen', False):
    here = os.path.normpath(os.path.join(here, '..', '..', 'build', 'melty', 'pkg'))  # a dev run: the staged package


def message(text, title='Leon Ops: Zombies Kennedy', error=True):
    import ctypes
    ctypes.windll.user32.MessageBoxW(0, text, title, 0x10 if error else 0x40)


def steam_root():
    try:
        import winreg
        with winreg.OpenKey(winreg.HKEY_CURRENT_USER, r'Software\Valve\Steam') as key:
            return os.path.normpath(winreg.QueryValueEx(key, 'SteamPath')[0])
    except OSError:
        return r'C:\Program Files (x86)\Steam'


def steam_libraries():
    root = steam_root()
    libraries = [root]
    try:
        vdf = open(os.path.join(root, 'steamapps', 'libraryfolders.vdf'), encoding='utf-8', errors='replace').read()
        libraries += [p.replace('\\\\', '\\') for p in re.findall(r'"path"\s+"([^"]+)"', vdf)]
    except OSError:
        pass
    return list(dict.fromkeys(os.path.normpath(p) for p in libraries))


def find_app(app_id):
    for library in steam_libraries():
        manifest = os.path.join(library, 'steamapps', f'appmanifest_{app_id}.acf')
        try:
            acf = open(manifest, encoding='utf-8', errors='replace').read()
        except OSError:
            continue
        found = re.search(r'"installdir"\s+"([^"]+)"', acf)
        if found:
            folder = os.path.join(library, 'steamapps', 'common', found.group(1))
            if os.path.isdir(folder):
                return folder
    return None


def ask_folder(name):
    import tkinter
    from tkinter import filedialog
    root = tkinter.Tk()
    root.withdraw()
    folder = filedialog.askdirectory(title=f'Where is {name} installed?')
    root.destroy()
    return os.path.normpath(folder) if folder else None


def game_folders():
    saved_path = os.path.join(here, 'leon_launcher.json')
    try:
        saved = json.load(open(saved_path, encoding='utf-8'))
    except (OSError, ValueError):
        saved = {}
    folders = {}
    for key, (app_id, name, valid) in APPS.items():
        folder = saved.get(key)
        if not folder or not valid(folder):
            folder = find_app(app_id)
        if not folder or not valid(folder):
            message(f"{name} wasn't found in your Steam libraries. Choose the folder it is installed in.",
                    error=False)
            folder = ask_folder(name)
        if not folder or not valid(folder):
            message(f'{name} is needed to play, and that folder does not have it:\n{folder or "(none chosen)"}')
            sys.exit(1)
        folders[key] = folder
    json.dump(folders, open(saved_path, 'w', encoding='utf-8'), indent=2)
    return folders


def main():
    exe = os.path.join(here, 'BO1Zombies.exe')
    if not os.path.isfile(exe):
        message(f'BO1Zombies.exe is not next to the launcher ({here}): unzip the whole release first.')
        sys.exit(1)
    folders = game_folders()
    if '--find' in sys.argv[1:]:  # only find the games (leon_launcher.json), start nothing
        return
    if not os.path.isfile(os.path.join(here, 'mods', 'leon', 'rtmodels', 'leon_ready_v8.txt')):  # leon_setup's SETUP_VERSION
        setup = os.path.join(here, 'leon_setup', 'leon_setup.exe')
        result = subprocess.run([setup, '--re4', folders['re4'], '--bo1', folders['bo1'], '--out', here])
        if result.returncode:
            message('Building Leon from Resident Evil 4 failed. What happened is in:\n'
                    + os.path.join(here, 'leon_setup.log'))
            sys.exit(1)
    env = dict(os.environ, BO1Z_GAME_DIR=folders['bo1'])
    subprocess.Popen([exe] + GAME_ARGS + sys.argv[1:], cwd=here, env=env)


if __name__ == '__main__':
    main()
