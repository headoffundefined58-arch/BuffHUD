"""Stage the built plugin, rebuild the distribution archive, then try to deploy.

The assets (icons, TSV tables, MCM config, translations, esp, pex) live in the
MO2 mod folder; this script only swaps the compiled binaries and writes a clean
.7z. The archive is always built from a staging copy, never from the live
folder: while Skyrim is running the installed DLL is locked and cannot be
overwritten, and packing around that would ship the previous binary.

    python deploy.py            # stage + pack + deploy
    python deploy.py --pack     # stage + pack only
"""

import os
import shutil
import sys

ROOT = r"D:\dev\BuffHUD"
BUILD = os.path.join(ROOT, "build", "Release")
STAGE = os.path.join(ROOT, "build", "staging")
MOD = r"E:\MO2（2.5.2）\AE-mods\【自制】BuffHUD"
DIST = os.path.join(ROOT, "dist")
VERSION = "1.2.0"

# MO2's own bookkeeping - must never ship in a release archive.
EXCLUDE = {"meta.ini"}


def stage() -> int:
    dll = os.path.join(BUILD, "BuffHUD.dll")
    pdb = os.path.join(BUILD, "BuffHUD.pdb")
    if not os.path.isfile(dll):
        print(f"缺少产物: {dll}\n先跑 python build.py")
        return 1

    if os.path.isdir(STAGE):
        shutil.rmtree(STAGE)
    shutil.copytree(MOD, STAGE, ignore=shutil.ignore_patterns(*EXCLUDE))

    plugins = os.path.join(STAGE, "SKSE", "Plugins")
    shutil.copy2(dll, os.path.join(plugins, "BuffHUD.dll"))
    if os.path.isfile(pdb):
        shutil.copy2(pdb, os.path.join(plugins, "BuffHUD.pdb"))
    print(f"已暂存到 {STAGE}")
    return 0


def pack() -> int:
    try:
        import py7zr
    except ImportError:
        print("没装 py7zr，跳过打包（pip install py7zr）")
        return 0

    os.makedirs(DIST, exist_ok=True)
    out = os.path.join(DIST, f"BuffHUD-{VERSION}.7z")
    if os.path.isfile(out):
        os.remove(out)

    count = 0
    with py7zr.SevenZipFile(out, "w", filters=[{"id": py7zr.FILTER_LZMA2, "preset": 7}]) as z:
        for dirpath, _dirs, files in os.walk(STAGE):
            for name in files:
                full = os.path.join(dirpath, name)
                z.write(full, os.path.relpath(full, STAGE))
                count += 1

    print(f"已打包 {count} 个文件 -> {out} ({os.path.getsize(out) / 1048576:.1f} MB)")
    return 0


def deploy() -> int:
    """Best effort: the game holds an exclusive handle while it runs."""
    plugins = os.path.join(MOD, "SKSE", "Plugins")
    for name in ("BuffHUD.dll", "BuffHUD.pdb"):
        src = os.path.join(BUILD, name)
        if not os.path.isfile(src):
            continue
        try:
            shutil.copy2(src, os.path.join(plugins, name))
            print(f"已部署 {name}")
        except PermissionError:
            print(f"无法覆盖 {name}：文件被占用（Skyrim 或 MO2 正在运行），"
                  f"关掉游戏后重跑 python deploy.py")
            return 1
    return 0


def main() -> int:
    rc = stage()
    if rc != 0:
        return rc
    rc = pack()
    if rc != 0:
        return rc
    if "--pack" in sys.argv:
        return 0
    return deploy()


if __name__ == "__main__":
    sys.exit(main())
