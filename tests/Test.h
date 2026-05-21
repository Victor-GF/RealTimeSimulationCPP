#pragma once
#include <iostream>
#include <cmath>
#include <stdexcept>
#include <functional>
#include <stdexcept>

// ── Minimal test framework ────────────────────────────────────────────────────

namespace Test {


static int passed = 0;
static int failed = 0;


void Expect(bool condition, const char* message) {
    if (!condition) throw std::runtime_error(message);
}


void ExpectNear(float actual, float expected, float epsilon, const char* message) {
    if (std::abs(actual - expected) > epsilon)
        throw std::runtime_error(std::string(message) +
            " (got " + std::to_string(actual) + ", expected " + std::to_string(expected) + ")");
}


void Run(const char* name, std::function<void()> fn) {
    try {
        fn();
        std::cout << "[PASS] " << name << "\n";
        passed++;
    } catch (const std::exception& e) {
        std::cout << "[FAIL] " << name << " — " << e.what() << "\n";
        failed++;
    }
}

void Summary() {
    std::cout << "\n" << passed << "/" << (passed + failed) << " tests passed\n";
}

} // namespace Test