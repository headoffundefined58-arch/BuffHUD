"""Configure / build BuffHUD inside a proper MSVC environment.

CMake cannot locate cl.exe on its own on this machine, so the environment
produced by vcvars64.bat is captured and handed to every child process.

    python build.py            # configure + build Release
    python build.py --configure
    python build.py --fresh    # clear the build dir first

Long CommonLibSSE-NG compiles are expected on the first run (~5-10 min).
"""
import os
import subprocess
import sys

VCVARS = (r"C:\Program Files\Microsoft Visual Studio\2022\Community"
          r"\VC\Auxiliary\Build\vcvars64.bat")
CMAKE = (r"C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE"
         r"\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe")

SOURCE = r"D:\dev\BuffHUD"
BUILD = os.path.join(SOURCE, "build")

# MSBuild hands its environment to cl.exe through a case-insensitive dictionary
# and aborts with MSB6001 ("已添加项 ... HTTP_PROXY") when both HTTP_PROXY and
# http_proxy are present, which is exactly what the Git Bash session exports.
PROXY_KEYS = {"http_proxy", "https_proxy", "all_proxy", "no_proxy"}


def msvc_env() -> dict:
    if not os.path.isfile(VCVARS):
        raise RuntimeError(f"vcvars64.bat not found at {VCVARS}")
    helper = os.path.join(SOURCE, "_msvc_env.bat")
    with open(helper, "w", encoding="utf-8") as fh:
        fh.write("@echo off\n")
        fh.write(f'call "{VCVARS}"\n')
        fh.write("set\n")
    proc = subprocess.run(
        ["cmd", "/c", helper],
        capture_output=True, text=True, encoding="utf-8", errors="replace",
    )
    # reg.exe gets blocked by the sandbox policy inside vcvars; that is noisy
    # but harmless as long as the environment itself came out complete.
    if "PATH=" not in proc.stdout:
        raise RuntimeError(f"vcvars64.bat failed: {proc.stderr[:500]}")

    env = {}
    seen = set()
    for line in proc.stdout.splitlines():
        if "=" not in line:
            continue
        key, value = line.split("=", 1)
        low = key.lower()
        if low in PROXY_KEYS or low in seen:
            continue
        seen.add(low)
        env[key] = value
    for key, value in os.environ.items():
        low = key.lower()
        if low in PROXY_KEYS or low in seen:
            continue
        seen.add(low)
        env[key] = value
    return env


def run(cmd: list, env: dict) -> int:
    print(f"\n$ {' '.join(cmd)}\n", flush=True)
    proc = subprocess.run(
        cmd, env=env, capture_output=True, text=True,
        encoding="utf-8", errors="replace",
    )
    tail = 8000
    if proc.stdout:
        print(proc.stdout[-tail:], flush=True)
    if proc.stderr:
        print(proc.stderr[-tail:], flush=True)
    print(f"[exit {proc.returncode}]", flush=True)
    return proc.returncode


def main() -> int:
    configure_only = "--configure" in sys.argv
    env = msvc_env()

    if os.path.isdir(BUILD) and "--fresh" in sys.argv:
        import shutil
        shutil.rmtree(BUILD, ignore_errors=True)

    rc = run([CMAKE, "-B", BUILD, "-S", SOURCE,
              "-G", "Visual Studio 17 2022", "-A", "x64"], env)
    if rc != 0:
        return rc
    if configure_only:
        return 0

    return run([CMAKE, "--build", BUILD, "--config", "Release", "--parallel"], env)


if __name__ == "__main__":
    sys.exit(main())
