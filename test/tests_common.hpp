#pragma once

#include <iostream>

#include "../turing.hpp"

inline void pass(std::string_view message)
{
    std::cout << ansi::brightGreen << ansi::bold << "[PASS] " << ansi::reset << message << '\n';
}
inline void fail(std::string_view message)
{
    std::cout << ansi::brightRed << ansi::bold << "[FAIL] " << ansi::reset << message << '\n';
}

inline std::string to_string(const std::vector<uint8_t> &v)
{
    std::string s;
    for (auto x : v)
        s += (char)('0' + x);
    return s;
}

template <typename T, typename U> void assertEqual(const T &actual, const U &expected)
{
    using Tp = std::common_type_t<T, U>;
    if (Tp(actual) != Tp(expected))
    {
        fail("Expected " + to_string(expected) + ", got " + to_string(actual));
        assert(false);
    }
}

inline size_t countOnes(const turing::Tape &t) { return std::ranges::count(t.data(), 1); }
