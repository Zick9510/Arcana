// test_scientific_notation.cpp
// Tests for scientific notation support in the Lexer

#include <iostream>
#include <string>
#include "../src/Common.hpp"
#include "../src/Lexer.hpp"

int tests_passed = 0;
int tests_failed = 0;

void test(const std::string& description, bool condition) {
    if (condition) {
        std::cout << "[PASS] " << description << "\n";
        tests_passed++;
    } else {
        std::cout << "[FAIL] " << description << "\n";
        tests_failed++;
    }
}

int main() {
    std::cout << "=== Scientific Notation Tests ===\n\n";

    // Test 1: Positive exponent (3e11)
    {
        Lexer lexer("3e11");
        auto tokens = lexer.tokenize();
        test("Positive exponent: 3e11 is tokenized as a number",
             tokens.size() >= 1 && tokens[0].tipo == Tt::NUMERO && tokens[0].valor == "3e11");
    }

    // Test 2: Negative exponent (9e-3)
    {
        Lexer lexer("9e-3");
        auto tokens = lexer.tokenize();
        test("Negative exponent: 9e-3 is tokenized as a number",
             tokens.size() >= 1 && tokens[0].tipo == Tt::NUMERO && tokens[0].valor == "9e-3");
    }

    // Test 3: Uppercase E (2E10)
    {
        Lexer lexer("2E10");
        auto tokens = lexer.tokenize();
        test("Uppercase E: 2E10 is tokenized as a number",
             tokens.size() >= 1 && tokens[0].tipo == Tt::NUMERO && tokens[0].valor == "2E10");
    }

    // Test 4: Float with exponent (1.5e3)
    {
        Lexer lexer("1.5e3");
        auto tokens = lexer.tokenize();
        test("Float with exponent: 1.5e3 is tokenized as a number",
             tokens.size() >= 1 && tokens[0].tipo == Tt::NUMERO && tokens[0].valor == "1.5e3");
    }

    std::cout << "\n=== Results ===\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";

    return tests_failed == 0 ? 0 : 1;
}