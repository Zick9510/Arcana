import os
import subprocess

COMPILER_PATH: str = "./bin/arcana"
VALID_TESTS_DIR: str = "tests/valid/"
OUTPUT_TESTS_DIR: str = "tests/build/"

def run_valid_tests():
    print("Running valid tests...")

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
            print(f"[OK] {filename}")
            passed += 1

        else:
            print(f"[BAD] {filename}")
            print(result.stderr.decode('utf-8'))
            failed += 1

            if (result.stdout):
                print(result.stdout.decode('utf-8'))


    print("-" * 20)
    print(f"GOOD: {passed}")
    print(f"BAD : {failed}")

if (__name__ == "__main__"):
    run_valid_tests()
