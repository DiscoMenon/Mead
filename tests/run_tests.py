
import argparse
import shutil
import subprocess
import sys
from pathlib import Path


EXPECTED_OUTPUTS = {
    "arithmetic.mead": (
        "Arithmetic:\n"
        "30\n"
        "200\n"
        "-10\n"
        "5\n"
        "1\n"
    ),
    "control_flow.mead": (
        "0\n"
        "1\n"
        "2\n"
        "loop complete\n"
    ),
}

INVALID_TESTS = {
    "undefined_variable.mead": "undefined variable",
    "invalid_assignment.mead": "assignment",
}


def run(command, cwd=None):
    result = subprocess.run(
        command,
        cwd=cwd,
        capture_output=True,
        text=True,
        encoding="utf-8",
        errors="replace",
    )
    if result.returncode != 0:
        raise RuntimeError(
            f"Command failed ({result.returncode}):\n"
            f"{subprocess.list2cmdline(command)}\n"
            f"{result.stdout}\n{result.stderr}"
        )
    return result


def main():
    parser = argparse.ArgumentParser()
    parser.add_argument("--compiler", required=True)
    parser.add_argument("--source-dir", required=True)
    parser.add_argument("--work-dir", required=True)
    args = parser.parse_args()

    compiler = Path(args.compiler).resolve()
    source_dir = Path(args.source_dir).resolve()
    work_dir = Path(args.work_dir).resolve()
    work_dir.mkdir(parents=True, exist_ok=True)

    ml64 = shutil.which("ml64")
    linker = shutil.which("link")

    if not ml64 or not linker:
        print(
            "ERROR: ml64.exe or link.exe not found. "
            "Run tests from a VS 2022 x64 Native Tools prompt.",
            file=sys.stderr,
        )
        return 1

    passed = 0
    total = len(EXPECTED_OUTPUTS) + len(INVALID_TESTS)

    for filename, expected in EXPECTED_OUTPUTS.items():
        source = source_dir / filename
        assembly = work_dir / source.with_suffix(".asm").name
        obj = work_dir / source.with_suffix(".obj").name
        exe = work_dir / source.with_suffix(".exe").name

        try:
            run([str(compiler), str(source), "-o", str(assembly)])
            run([ml64, "/c", "/Fo", str(obj), str(assembly)])
            run([
                linker,
                "/SUBSYSTEM:CONSOLE",
                "/ENTRY:main",
                str(obj),
                "kernel32.lib",
                f"/OUT:{exe}",
            ])

            result = run([str(exe)])
            if result.stdout != expected:
                raise RuntimeError(
                    f"Output mismatch for {filename}\n"
                    f"Expected: {expected!r}\n"
                    f"Actual:   {result.stdout!r}"
                )

            print(f"PASS: {filename}")
            passed += 1

        except Exception as error:
            print(f"FAIL: {filename}\n{error}", file=sys.stderr)

    for filename, expected_error in INVALID_TESTS.items():
        source = source_dir / filename
        assembly = work_dir / source.with_suffix(".asm").name

        try:
            result = subprocess.run(
                [str(compiler), str(source), "-o", str(assembly)],
                capture_output=True,
                text=True,
                encoding="utf-8",
                errors="replace",
            )

            if result.returncode == 0:
                raise RuntimeError("Compiler accepted invalid source.")

            diagnostic = result.stdout + result.stderr
            if expected_error not in diagnostic.lower():
                raise RuntimeError(
                    f"Expected diagnostic containing "
                    f"{expected_error!r}, got:\n{diagnostic}"
                )

            print(f"PASS: rejects {filename}")
            passed += 1

        except Exception as error:
            print(f"FAIL: {filename}\n{error}", file=sys.stderr)

    print(f"\nResult: {passed}/{total} tests passed.")
    return 0 if passed == total else 1


if __name__ == "__main__":
    raise SystemExit(main())
