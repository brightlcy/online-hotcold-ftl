"""Local Clang formatting, analyzer and source coverage with exclusive evidence files."""
import argparse
import datetime
import hashlib
import json
import os
from pathlib import Path
import plistlib
import shutil
import subprocess

parser = argparse.ArgumentParser()
parser.add_argument("--label", required=True)
parser.add_argument("--clang-format", default="clang-format")
args = parser.parse_args()
if Path(args.label).name != args.label:
    parser.error("label must be a filename component")
root = Path(__file__).resolve().parents[1]
os.chdir(root)
log = root / "results/validation" / (args.label + ".log")
report = log.with_suffix(".json")
if log.exists() or report.exists():
    parser.error("evidence already exists; choose a new label")
checks = []
data = {"started_at_utc": datetime.datetime.now(datetime.timezone.utc).isoformat(),
        "source_commit": subprocess.check_output(["git", "rev-parse", "HEAD"], text=True).strip(),
        "working_tree": subprocess.check_output(["git", "status", "--porcelain"], text=True),
        "status": "RUNNING", "checks": checks, "clang_tidy": "NOT RUN (unavailable)"}
files = sorted(p for d in ["include", "src", "tests/cpp", "tests/differential"] for p in Path(d).rglob("*")
               if p.suffix in {".hpp", ".cpp"})
core = sorted(str(p) for p in Path("src").rglob("*.cpp"))
data["source_sha256"] = {str(p): hashlib.sha256(p.read_bytes()).hexdigest()
                          for p in files + [Path("CMakeLists.txt")]}

def run(command, env=None):
    result = subprocess.run(command, text=True, stdout=subprocess.PIPE,
                            stderr=subprocess.STDOUT, env=env)
    with log.open("a") as stream:
        stream.write("$ " + " ".join(command) + "\n" + result.stdout
                     + "\nEXIT " + str(result.returncode) + "\n")
    checks.append({"command": command, "exit_code": result.returncode})
    result.check_returncode()
    return result.stdout

try:
    log.touch(exist_ok=False)
    run([args.clang_format, "--dry-run", "--Werror"] + [str(p) for p in files])
    for i, source in enumerate(core):
        output = root / "build-coverage" / ("analyzer-" + str(i) + ".plist")
        output.parent.mkdir(exist_ok=True)
        run(["clang++", "--analyze", "-std=c++20", "-Iinclude", source, "-o", str(output)])
        issues = plistlib.loads(output.read_bytes()).get("diagnostics", [])
        if issues:
            raise RuntimeError("static analyzer reported diagnostics: " + source)
    data["static_analyzer_diagnostics"] = 0
    run(["cmake", "-S", ".", "-B", "build-coverage", "-DCMAKE_BUILD_TYPE=Debug", "-DATLAS_COVERAGE=ON"])
    run(["cmake", "--build", "build-coverage", "--parallel", "2"])
    env = os.environ.copy()
    env["LLVM_PROFILE_FILE"] = str(root / "build-coverage" / (args.label + ".profraw"))
    run(["./build-coverage/atlas_tests"], env=env)
    def llvm(tool):
        return ["xcrun", tool] if shutil.which("xcrun") else [tool]
    profile = "build-coverage/" + args.label
    run(llvm("llvm-profdata") + ["merge", "-sparse", profile + ".profraw", "-o", profile + ".profdata"])
    options = ["build-coverage/atlas_tests", "-instr-profile=" + profile + ".profdata"]
    run(llvm("llvm-cov") + ["report"] + options + core)
    raw = subprocess.check_output(llvm("llvm-cov") + ["export"] + options + core, text=True)
    exported = json.loads(raw)["data"][0]
    data["coverage"] = exported["totals"]
    data["files"] = [{"filename": f["filename"], "summary": f["summary"]} for f in exported["files"]]
    data["coverage_scope"] = "core cpp sources only, excludes tests and vendored framework"
    data["status"] = "PASS"
except Exception as error:
    data["status"] = "FAIL"
    data["error"] = str(error)
    raise
finally:
    data["finished_at_utc"] = datetime.datetime.now(datetime.timezone.utc).isoformat()
    with report.open("x") as stream:
        json.dump(data, stream, indent=2)
        stream.write("\n")
print(json.dumps({"status": data["status"], "coverage": data.get("coverage")}, indent=2))
