import os
import subprocess

class Color:
    GREEN = '\033[92m'
    RED   = '\033[91m'
    CYAN  = '\033[96m'
    BOLD  = '\033[1m'
    END   = '\033[0m'

COMPILER_PATH: str = "./bin/arcana"
VALID_TESTS_DIR: str = "tests/valid/"
OUTPUT_TESTS_DIR: str = "tests/build/"

failed_tests: list = []

def run_tests():
    print(f"{Color.BOLD}{Color.CYAN}Running tests...{Color.END}\n")

    passed: int = 0
    failed: int = 0

    for filename in os.listdir(VALID_TESTS_DIR):
        if (not filename.endswith(".arcn")):
            continue

        filepath = os.path.join(VALID_TESTS_DIR, filename)

        output_filename = filename.replace(".arcn", "")
        output_filepath = os.path.join(OUTPUT_TESTS_DIR, output_filename)

        command = [COMPILER_PATH, filepath, "-o", output_filepath]

        result = subprocess.run(command, capture_output=True)

        if (result.returncode == 0):
            print(f"{Color.GREEN}[OK]{Color.END} {filename}")
            passed += 1

        else:
            failed_tests.append([filename, result])
            failed += 1

    print()

    for f, r in failed_tests:
        print(f"{Color.RED}[BAD]{Color.END} {f}")
        print(r.stderr.decode('utf-8'))

        if (r.stdout):
            print(r.stdout.decode('utf-8'))


    print("\n" + "-" * 20)
    print(f"{Color.BOLD }{Color.CYAN}Summary:{Color.END}")
    print(f"{Color.GREEN}  PASSED: {passed}{Color.END}")
    print(f"{Color.RED  }  FAILED: {failed}{Color.END}")
    print("\n" + "-" * 20)

    if (failed):
        for f, _ in failed_tests:
            print(f"{Color.RED}[BAD]{Color.END} {f}")

if (__name__ == "__main__"):
    run_tests()
