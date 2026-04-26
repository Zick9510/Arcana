import os
import subprocess
import time

class Color:
    GREEN  = '\033[92m'
    YELLOW = '\033[33m'
    RED    = '\033[91m'
    CYAN   = '\033[96m'
    BOLD   = '\033[1m'
    END    = '\033[0m'

COMPILER_PATH: str = "./bin/arcana"
VALID_TESTS_DIR: str = "tests/valid/"
OUTPUT_TESTS_DIR: str = "tests/build/"

failed_tests: list = []

def run_tests():
    print(f"{Color.BOLD}{Color.CYAN}Running tests...{Color.END}\n")

    passed: int = 0
    failed: int = 0
    timeout_fail: int = 0

    start_time = time.perf_counter()

    for filename in os.listdir(VALID_TESTS_DIR):
        if (not filename.endswith(".arcn")):
            continue

        filepath = os.path.join(VALID_TESTS_DIR, filename)

        output_filename = filename.replace(".arcn", "")
        output_filepath = os.path.join(OUTPUT_TESTS_DIR, output_filename)

        command = [COMPILER_PATH, filepath, "-o", output_filepath]

        try:
            comp_result = subprocess.run(command, capture_output=True, timeout=0.500)

        except subprocess.TimeoutExpired:
            print(f"{Color.RED}[TIME] {filename} (Exceeded 500ms)")
            failed_tests.append((filename, "Timeout", b"", "Process killed after 500ms"))
            timeout_fail += 1
            continue

        if (comp_result.returncode != 0):
            failed_tests.append((filename, "Compilation Error", comp_result.stdout, comp_result.stderr))
            failed += 1
            continue

        exec_cmd = ["lli", output_filepath + ".ll"]
        exec_result = subprocess.run(exec_cmd, capture_output=True)

        if (exec_result.returncode >= 0):
            print(f"{Color.GREEN}[OK]{Color.END} {filename}")
            passed += 1

        else:
            failed_tests.append((filename, "Execution Error", exec_result.stdout, exec_result.stderr))
            failed += 1

    end_time = time.perf_counter()
    total_duration = end_time - start_time

    print()

    for f, err_type, out, err in failed_tests:
        print(f"{Color.RED}[BAD - {err_type}]{Color.END} {f}")

        if (err):
            print(err.decode('utf-8') if (isinstance(err, bytes)) else err)

        if (out):
            print(out.decode('utf-8') if (isinstance(out, bytes)) else out)




    print("\n" + "-" * 20)
    print(f"{Color.BOLD  }{Color.CYAN}Summary:{Color.END}")
    print(f"{Color.GREEN }  PASSED : {passed}{Color.END}")
    print(f"{Color.RED   }  FAILED : {failed}{Color.END}")
    print(f"{Color.YELLOW}  TIMEOUT: {timeout_fail}{Color.END}")

    print(f"{Color.BOLD}{Color.CYAN}  TOTAL TIME: {total_duration:.4f} seconds{Color.END}")

    print("\n" + "-" * 20)

    if (failed or timeout_fail):
        print()
        for f, err_type, _, _ in failed_tests:
            print(f"{Color.RED}[{err_type}]{Color.END} {f}")

if (__name__ == "__main__"):
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
