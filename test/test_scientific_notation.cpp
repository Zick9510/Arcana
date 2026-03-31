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

    Lexer lexer;

    // Test 1: Positive exponent (3e11)
    lexer.setSource("3e11");
    auto tokens1 = lexer.tokenize();
    test("Positive exponent: 3e11 is tokenized as a number",
         tokens1.size() >= 1 && tokens1[0].tipo == Tt::NUMERO && tokens1[0].valor == "3e11");

    // Test 2: Negative exponent (9e-3)
    lexer.setSource("9e-3");
    auto tokens2 = lexer.tokenize();
    test("Negative exponent: 9e-3 is tokenized as a number",
         tokens2.size() >= 1 && tokens2[0].tipo == Tt::NUMERO && tokens2[0].valor == "9e-3");

    // Test 3: Uppercase E (2E10)
    lexer.setSource("2E10");
    auto tokens3 = lexer.tokenize();
    test("Uppercase E: 2E10 is tokenized as a number",
         tokens3.size() >= 1 && tokens3[0].tipo == Tt::NUMERO && tokens3[0].valor == "2E10");

    // Test 4: Float with exponent (1.5e3)
    lexer.setSource("1.5e3");
    auto tokens4 = lexer.tokenize();
    test("Float with exponent: 1.5e3 is tokenized as a number",
         tokens4.size() >= 1 && tokens4[0].tipo == Tt::NUMERO && tokens4[0].valor == "1.5e3");

    std::cout << "\n=== Results ===\n";
    std::cout << "Passed: " << tests_passed << "\n";
    std::cout << "Failed: " << tests_failed << "\n";

    return tests_failed == 0 ? 0 : 1;
}