#pragma once

#include <algorithm>
#include <bitset>
#include <cassert>
#include <cmath>
#include <complex>
#include <deque>
#include <exception>
#include <fstream>
#include <functional>
#include <iomanip>
#include <ios>

#include <array>
#include <atomic>
#include <chrono>
#include <iosfwd>
#include <iostream>
#include <istream>
#include <iterator>
#include <limits>
#include <list>
#include <locale>
#include <map>
#include <memory>
#include <new>
#include <numeric>
#include <ostream>
#include <queue>
#include <set>
#include <sstream>
#include <stack>
#include <stdexcept>
#include <streambuf>
#include <string>
#include <typeinfo>
#include <utility>
// #include <valarray>
#include <vector>

#include <codecvt>
#include <condition_variable>
#include <forward_list>
#include <future>
#include <initializer_list>
#include <mutex>
#include <random>
#include <ratio>
#include <regex>
#include <scoped_allocator>
#include <system_error>
#include <thread>
#include <tuple>
#include <type_traits>
#include <typeindex>
#include <unordered_map>
#include <unordered_set>

// C++17
#include <execution>
#include <optional>
#include <string_view>
// C++20
#include <bit>
#include <compare>
#include <concepts>
#include <format>
#include <numbers>
#include <print>
#include <ranges>
#include <span>

#include <boost/unordered/unordered_flat_map.hpp>
#include <boost/unordered/unordered_flat_set.hpp>

#include <ansi.hpp>
#include <euler.hpp>
#include <oklch.hpp>

#include "dbg.hpp"

// /** assert for debugging my own code */
// #undef assert
// #define assert(e) (void)((!!(e)) || (_assert(#e, __FILE__, __LINE__), 0))

/// Shortcut to write one-line lambdas
#define fun(var, ...) ([&](auto &&var) -> decltype(auto) { return __VA_ARGS__; })

/// Shortcut to write one-line lambdas
#define fun2(var1, var2, ...) ([&](auto &&var1, auto &&var2) -> decltype(auto) { return __VA_ARGS__; })

/// Like the dbg! macro in Rust
// #define dbg(...) cout << '(' << #__VA_ARGS__ << ") = " << tuple{__VA_ARGS__} << "\n";
