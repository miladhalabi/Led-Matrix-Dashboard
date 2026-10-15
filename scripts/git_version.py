# PlatformIO build script: puts the output of "git describe" into the firmware as FIRMWARE_VERSION,
# e.g. "v1.2.0", "v1.2.0-3-gabc1234" (3 commits after the tag) or "abc1234-dirty" (uncommitted changes).
import subprocess

Import("projenv")


def git(*args):
    try:
        return subprocess.check_output(["git", *args], text=True, stderr=subprocess.DEVNULL).strip()
    except Exception:
        return ""


version = git("describe", "--tags", "--always", "--dirty") or "unknown"
projenv.Append(CPPDEFINES=[("FIRMWARE_VERSION", projenv.StringifyMacro(version))])
