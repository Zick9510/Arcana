import os
import subprocess
import time

class Color:
    GREEN  = '\033[92m'
    YELLOW = '\033[33m'
    RED    = '\033[91m'
    CYAN   = '\033[96m'
    MAG    = '\033[95m'
    BOLD   = '\033[1m'
    END    = '\033[0m'

COMPILER_PATH: str = "./bin/arcana"
VALID_TESTS_DIR: str = "tests/valid/"
OUTPUT_TESTS_DIR: str = "tests/build/"

failed_tests: list = []
passed      : int  = 0
failed      : int  = 0
timeout_fail: int  = 0

def run_single_test(filepath, filename):
    global passed, failed, timeout_fail

    os.makedirs(OUTPUT_TESTS_DIR, exist_ok=True)

    output_filename = filename.replace(".arcn", "")
    output_filepath = os.path.join(OUTPUT_TESTS_DIR, output_filename)

    command = [COMPILER_PATH, filepath, "-o", output_filepath]

    try:
        comp_result = subprocess.run(command, capture_output=True, timeout=0.500)

    except subprocess.TimeoutExpired:
        print(f"{Color.YELLOW}[TIME] {filename} (Exceeded 500ms)")
        failed_tests.append((filename, "Timeout", b"", "Process killed after 500ms"))
        timeout_fail += 1
        return

    if (comp_result.returncode != 0):
        failed_tests.append((filename, "Compilation Error", comp_result.stdout, comp_result.stderr))
        print(f"  {Color.RED}[FAIL]{Color.END} {filename} (Compilation Error)")
        failed += 1
        return

    exec_cmd = ["lli", output_filepath + ".ll"]
    exec_result = subprocess.run(exec_cmd, capture_output=True)

    if (exec_result.returncode >= 0):
        print(f"  {Color.GREEN}[OK]{Color.END}  {filename}")
        passed += 1

    else:
        failed_tests.append((filename, "Execution Error", exec_result.stdout, exec_result.stderr))
        print(f"  {Color.RED}[FAIL]{Color.END} {filename} (Exec Error)")
        failed += 1

def run_tests():
    print(f"{Color.BOLD}{Color.MAG}--- ARCANA TEST SUITE ---{Color.END}\n")
    start_time = time.perf_counter()

    items = sorted(os.listdir(VALID_TESTS_DIR))
    subdirs = [d for d in items if os.path.isdir(os.path.join(VALID_TESTS_DIR, d))]
    root_files = [f for f in items if f.endswith(".arcn") and os.path.isfile(os.path.join(VALID_TESTS_DIR, f))]

    for subdir in subdirs:
        subdir_path = os.path.join(VALID_TESTS_DIR, subdir)
        files_subdir = [f for f in sorted(os.listdir(subdir_path)) if f.endswith(".arcn")]

        if (not files_subdir):
            continue

        print(f"{Color.BOLD}{Color.CYAN}[ FOLDER: {subdir} ]{Color.END}")
        for filename in files_subdir:
            filepath = os.path.join(subdir_path, filename)
            run_single_test(filepath, filename)
        print()

    if (root_files):
        print(f"{Color.BOLD}{Color.CYAN}[ MISC ]{Color.END}")
        for filename in root_files:
            filepath = os.path.join(VALID_TESTS_DIR, filename)
            run_single_test(filepath, filename)
        print()

    end_time = time.perf_counter() - start_time

    if (failed_tests):
        print(f"\n{Color.BOLD}{Color.RED}--- FAILED TEST DETAILS ---{Color.END}")
        for f, err_type, out, err in failed_tests:
            print(f"\n{Color.RED}>> {f} ({err_type}){Color.END}")
            if (err): print(err.decode('utf-8') if isinstance(err, bytes) else err)
            if (out): print(out.decode('utf-8') if isinstance(out, bytes) else out)


    print("\n" + "=" * 30)
    print(f"{Color.BOLD}{Color.CYAN}Summary:{Color.END}")
    print(f"{Color.GREEN}  PASSED: {passed}{Color.END}")
    print(f"{Color.RED}  FAILED: {failed}{Color.END}")
    print(f"{Color.YELLOW}  TIMEOUT: {timeout_fail}{Color.END}")
    print(f"{Color.BOLD}{Color.CYAN}  TOTAL TIME: {end_time:.4f}s{Color.END}")
    print("=" * 30)

if (__name__ == '__main__'):
    run_tests()

"""
Benchmark Environment

    CPU: AMD FX-6300 (6 Cores @ 3.5GHz)

    RAM: 16 GB DDR3 (800 MT/s)

    OS: Linux (Kernel 6.x-lts)


Reproducible Benchmarks:

    deploy   build: 0.9 - 1.0 seconds

    shy      build: 1.0 - 1.3 seconds

    segfault build: 1.4 - 1.6 seconds

"""
