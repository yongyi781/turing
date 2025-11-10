#pragma once

#include <ansi.hpp>
#include <boost/unordered/unordered_flat_map.hpp>
#include <euler/io.hpp>
#include <euler/types.hpp>
#include <iostream>
#include <string_view>

namespace dbg
{
template <typename T> void prettyPrint(const T &t) { std::cerr << t; }

template <typename T>
    requires euler::is_string<T>
void prettyPrint(const T &t)
{
    std::cerr << '"' << t << '"';
}

template <typename T> void prettyPrint(const boost::rational<T> &t)
{
    if (t.denominator() == 1)
        std::cerr << t.numerator();
    else
        std::cerr << t;
}

template <> void prettyPrint(const u8 &t) { std::cerr << (int)t; }

template <> void prettyPrint(const i8 &t) { std::cerr << (int)t; }

template <typename T> void prettyPrint(const T &t, bool comma)
{
    if (comma)
        std::cerr << ansi::dim << ", " << ansi::reset;
    prettyPrint(t);
}

/// Debug multiple items.
template <typename... Ts> decltype(auto) debug(std::string_view name, Ts &&...ts)
{
    std::cerr << ansi::brightCyan << name << ansi::reset << ansi::dim << " = " << ansi::reset;
    std::cerr << ansi::dim << '(' << ansi::reset;
    bool first = true;
    ((prettyPrint(ts, !first), first = false), ...);
    std::cerr << ansi::dim << ')' << ansi::reset << '\n';
    return std::tuple{std::forward<Ts>(ts)...};
}

/// Debug a single item.
template <typename T> decltype(auto) debug(std::string_view name, T &&t)
{
    std::cerr << ansi::brightCyan << name << ansi::reset << ansi::dim << " = " << ansi::reset;
    prettyPrint(t);
    std::cerr << '\n';
    return t;
}

/// Log a message.
inline decltype(auto) debug(std::string_view /*name*/, const char *&&t) // NOLINT
{
    std::cerr << ansi::bold << t << ansi::reset << '\n';
    return t;
}

/// Check that two functions agree on 1, 2, ..., N.
template <typename Fun1, typename Fun2> bool checkFns(Fun1 f, Fun2 g, int64_t N)
{
    size_t mismatches = 0;
    for (int64_t i = 1; i <= N; ++i)
    {
        auto const fi = f(i), gi = g(i);
        if (fi != gi)
        {
            if (++mismatches <= 5)
                std::cerr << ansi::red << "Mismatch at " << i << ": " << fi << ", " << gi << ansi::reset << '\n';
            else
                return false;
        }
    }
    if (mismatches == 0)
        std::cerr << ansi::green << "checkFns passed" << ansi::reset << '\n';
    return true;
}

/// Check that two ranges agree on 0, 1, ..., N.
template <std::ranges::random_access_range Range1, std::ranges::random_access_range Range2>
bool checkRanges(Range1 &&r1, Range2 &&r2)
{
    size_t const n = std::min(std::ranges::size(r1), std::ranges::size(r2));
    size_t mismatches = 0;
    for (size_t i = 0; i < n; ++i)
    {
        if (r1[i] != r2[i])
        {
            if (++mismatches <= 5)
                std::cerr << ansi::red << "Mismatch at " << i << ": " << r1[i] << ", " << r2[i] << ansi::reset << '\n';
            else
                return false;
        }
    }
    if (mismatches == 0)
        std::cerr << ansi::green << "checkRanges passed" << ansi::reset << '\n';
    return mismatches == 0;
}

/// Recursive function debugger. Add `auto &&self` to the beginning of the argument list of your function, and replace
/// recursive calls with `self`.
template <typename... Args, typename Fun> constexpr auto recursive(Fun f, size_t maxDepth = 10)
{
    return [f = std::move(f), depth = 0UZ,
            maxDepth]<typename Self>(this Self &&self, Args... args) -> std::invoke_result_t<Fun, Self, Args...> {
        if (depth > maxDepth)
            return f(std::forward<Self>(self), std::forward<Args>(args)...);
        for (size_t i = 0; i < depth; ++i)
            std::cout << " │";
        std::cout << " f" << std::tuple{args...} << ':' << "\n";
        ++depth;
        auto res = f(std::forward<Self>(self), std::forward<Args>(args)...);
        --depth;
        for (size_t i = 0; i < depth; ++i)
            std::cout << " │";
        std::cout << ansi::magenta << " f" << std::tuple{args...} << " = " << ansi::blue << res << ansi::reset << '\n';
        return res;
    };
}

/// Type for universal recursive memoizer.
template <typename Fun, typename... Args> struct memoize_recursive_t
{
    using key_type = std::tuple<Args...>;
    using value_type = std::invoke_result_t<Fun, std::reference_wrapper<memoize_recursive_t>, Args...>;

    Fun f;
    boost::unordered_flat_map<key_type, value_type> cache;
    size_t maxDepth = 0;
    size_t depth = 0;

    value_type operator()(Args... args)
    {
        key_type key{args...};
        if (depth <= maxDepth)
            for (size_t i = 0; i < depth; ++i)
                std::cout << " │";
        if (auto const it = cache.find(key); it != cache.end())
        {
            if (depth <= maxDepth)
                std::cout << ansi::yellow << " f" << key << " = " << it->second << ansi::reset << '\n';
            return it->second;
        }
        if (depth <= maxDepth)
            std::cout << " f" << key << ":\n";
        ++depth;
        auto res = f(std::ref(*this), std::move(args)...);
        --depth;
        if (depth <= maxDepth)
        {
            for (size_t i = 0; i < depth; ++i)
                std::cout << " │";
            std::cout << ansi::magenta << " f" << key << " = " << ansi::blue << res << ansi::reset << '\n';
        }
        cache.emplace(std::move(key), res);
        return res;
    }
};

/// Recursive function debugger with memoizer. Add `auto &&self` to the beginning of the argument list of your function,
/// and replace recursive calls with `self`.
template <typename... Args, typename Fun>
memoize_recursive_t<Fun, Args...> memoizeRecursive(Fun f, size_t maxDepth = 10)
{
    return {std::move(f), {}, maxDepth};
}

/// Convience class for printing timings of segments of code.
class timer
{
  public:
    timer(bool printOnDestroy = true, uint32_t labelWidth = 20,
          std::source_location const &loc = std::source_location::current())
        : printOnDestroy(printOnDestroy), labelWidth(labelWidth), line(loc.line()),
          t(std::chrono::high_resolution_clock::now())
    {
    }

    timer(const timer &) = default;
    timer(timer &&other) noexcept : line(other.line), t(other.t) { other.line = 0; }
    timer &operator=(const timer &) = default;
    timer &operator=(timer &&other) noexcept
    {
        if (this != &other)
        {
            line = other.line;
            t = other.t;
            other.line = 0;
        }
        return *this;
    }
    ~timer()
    {
        if (line > 0 && printOnDestroy)
        {
            auto const dt = std::chrono::high_resolution_clock::now() - t;
            std::string const header = "[:" + std::to_string(line) + " -> destroy] ";
            std::cerr << ansi::dim << std::setw(labelWidth) << header << ansi::reset << euler::to_string(dt) << '\n';
        }
    }

    /// Gets the elapsed time.
    [[nodiscard]] auto elapsed() const { return std::chrono::high_resolution_clock::now() - t; }

    /// Prints the elapsed time.
    void print(std::source_location const &loc = std::source_location::current()) const
    {
        auto const dt = std::chrono::high_resolution_clock::now() - t;
        std::string const header = "[:" + std::to_string(line) + " -> :" + std::to_string(loc.line()) + "] ";
        std::cerr << ansi::dim << std::setw(labelWidth) << header << ansi::reset << euler::to_string(dt) << '\n';
    }

    /// Prints the elapsed time with the given label.
    void print(std::string label) const
    {
        auto const dt = std::chrono::high_resolution_clock::now() - t;
        std::string const header = "[" + std::move(label) + "] ";
        std::cerr << ansi::dim << std::setw(labelWidth) << header << ansi::reset << euler::to_string(dt) << '\n';
    }

    /// Restarts the timer.
    void reset(std::source_location const &loc = std::source_location::current())
    {
        line = loc.line();
        t = std::chrono::high_resolution_clock::now();
    }

    /// Prints and resets.
    void operator()(std::source_location const &loc = std::source_location::current())
    {
        print(loc);
        reset(loc);
    }

    /// Prints with the given label and resets.
    void operator()(std::string label, std::source_location const &loc = std::source_location::current())
    {
        print(std::move(label));
        reset(loc);
    }

  private:
    bool printOnDestroy = false;
    uint32_t labelWidth = 20;
    uint32_t line = 0;
    std::chrono::high_resolution_clock::time_point t;
};
} // namespace dbg

#define dbg(...) dbg::debug(#__VA_ARGS__, __VA_ARGS__)
