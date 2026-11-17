"""Compile mcm/scripts/BuffHUD.psc into Scripts/BuffHUD.pex.

Nothing about this step is clever; it exists because the Papyrus compiler has no
concept of "header only" imports and this machine has no Creation Kit, so the
import path has to be assembled from three sources:

  1. _ref/psc            MCM Helper's public/private sources (MCM_ConfigBase and
                         the SKI_* chain) — fetched once from GitHub, because the
                         installed copy ships compiled .pex inside a BSA only.
  2. Data/Scripts/Source  vanilla headers that shipped with the game (62 core
                         types: Form, Quest, Actor, ...).
  3. papyrus-tools/headers the remaining decompiled vanilla headers.

(3) was produced by a decompiler and has two defects that break compilation of
anything that pulls in Race/ActorBase/ObjectReference: `VoiceType` and
`WordOfPower` are missing from the type set, and Debug.Trace lost its
`aiSeverity = 0` default. Both are patched into local copies here rather than
touching the shared header folder, so other projects keep compiling as before.
None of this leaks into the output: the compiler records only the script's own
name and its direct parent ("MCM_ConfigBase") in the .pex.
"""

import os
import re
import shutil
import subprocess
import sys
import urllib.request

ROOT = r"D:\dev\BuffHUD"
ASSET_ROOT = r"E:\Skyrim Special Edition\BuffHUD"
WORK = os.path.join(ROOT, "_ref", "psc")
OUT_DIR = os.path.join(ASSET_ROOT, "build", "Scripts")
SCRIPT = "BuffHUD.psc"

COMPILER_CANDIDATES = [
    r"D:\dev\VeyronThruster\_ref\papyrus-compiler\bin\Original Compiler\PapyrusCompiler.exe",
    r"E:\MO2（2.5.2）\papyrus-tools\pc\papyrus-compiler\Original Compiler\PapyrusCompiler.exe",
]
FLAGS_CANDIDATES = [
    r"D:\dev\VeyronThruster\_ref\papyrus-compiler\bin\Original Compiler\TESV_Papyrus_Flags.flg",
    r"E:\MO2（2.5.2）\papyrus-tools\pc\papyrus-compiler\Original Compiler\TESV_Papyrus_Flags.flg",
]
IMPORT_DIRS = [
    WORK,
    r"E:\Skyrim Special Edition\Data\Scripts\Source",
    r"E:\MO2（2.5.2）\papyrus-tools\headers",
]
DECOMPILED_HEADERS = r"E:\MO2（2.5.2）\papyrus-tools\headers"

MCM_HELPER_SCRIPTS = [
    ("MCM_ConfigBase.psc", "scripts/private/MCM_ConfigBase.psc"),
    ("SKI_ConfigBase.psc", "scripts/private/SKI_ConfigBase.psc"),
    ("SKI_ConfigManager.psc", "scripts/private/SKI_ConfigManager.psc"),
    ("SKI_QuestBase.psc", "scripts/private/SKI_QuestBase.psc"),
    ("SKI_PlayerLoadGameAlias.psc", "scripts/private/SKI_PlayerLoadGameAlias.psc"),
    ("SKI_ConfigMenu.psc", "scripts/public/SKI_ConfigMenu.psc"),
    ("MCM.psc", "scripts/public/MCM.psc"),
]
CDN = "https://cdn.jsdelivr.net/gh/Exit-9B/MCM-Helper@master/"


def first_existing(paths, what):
    for p in paths:
        if os.path.isfile(p):
            return p
    raise SystemExit(f"找不到{what}，试过:\n  " + "\n  ".join(paths))


def ensure_references():
    os.makedirs(WORK, exist_ok=True)
    for name, rel in MCM_HELPER_SCRIPTS:
        dst = os.path.join(WORK, name)
        if os.path.isfile(dst) and os.path.getsize(dst) > 0:
            continue
        url = CDN + rel
        print(f"下载 {name} <- {url}")
        try:
            with urllib.request.urlopen(url, timeout=30) as r:
                data = r.read()
        except Exception as exc:  # noqa: BLE001 - offline is a normal outcome
            raise SystemExit(f"下载 {name} 失败（{exc}）；装好 MCM Helper 源码后重试") from exc
        with open(dst, "wb") as f:
            f.write(data)


def ensure_stubs():
    """Patch the two defects in the decompiled vanilla header set."""
    debug = os.path.join(WORK, "Debug.psc")
    if not os.path.isfile(debug):
        src = open(os.path.join(DECOMPILED_HEADERS, "debug.psc"), encoding="utf-8",
                   errors="replace").read()
        src = re.sub(r"(Function (?:Trace|TraceStack|TraceAndBox)\(String as\w+, Int aiSeverity)(\))",
                     r"\1 = 0\2", src)
        src = re.sub(r"(Bool Function TraceUser\(String asUserLog, String as\w+, Int aiSeverity)(\))",
                     r"\1 = 0\2", src)
        open(debug, "w", encoding="utf-8").write(src)

    for name in ("VoiceType", "WordOfPower"):
        path = os.path.join(WORK, name + ".psc")
        if os.path.isfile(path):
            continue
        open(path, "w", encoding="utf-8").write(
            f"Scriptname {name} extends Form\n"
            "{编译占位头：本机的反编译头文件集缺这个类型；输出不影响 .pex 内容}\n")


def main() -> int:
    compiler = first_existing(COMPILER_CANDIDATES, "PapyrusCompiler.exe")
    flags = first_existing(FLAGS_CANDIDATES, "TESV_Papyrus_Flags.flg")

    ensure_references()
    ensure_stubs()

    shutil.copy2(flags, os.path.join(WORK, os.path.basename(flags)))
    shutil.copy2(os.path.join(ASSET_ROOT, "mcm", "scripts", SCRIPT),
                 os.path.join(WORK, SCRIPT))

    os.makedirs(OUT_DIR, exist_ok=True)
    cmd = [
        compiler, SCRIPT,
        f'-f={os.path.basename(flags)}',
        "-i=" + ";".join(IMPORT_DIRS),
        f"-o={OUT_DIR}",
    ]
    print(" ".join(cmd))
    rc = subprocess.run(cmd, cwd=WORK).returncode
    pex = os.path.join(OUT_DIR, "BuffHUD.pex")
    if rc != 0 or not os.path.isfile(pex):
        print("编译失败；上面每一行 <文件>(行,列) 都指向具体的头文件缺陷")
        return 1
    print(f"已生成 {pex}（{os.path.getsize(pex)} 字节）")
    return 0


if __name__ == "__main__":
    sys.exit(main())
