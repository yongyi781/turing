#pragma once

#include "../turing.hpp"

namespace turing
{
struct bouncer_result
{
    /// Polynomial degree.
    bool found = false;
    size_t degree = 0;
    size_t start = 0;
    /// Number of tape growth events (on a given side) per repeat.
    size_t xPeriod = 0;
    /// The side the bouncer was detected on.
    direction side = direction::left;
};

class BouncerDecider
{
  public:
    constexpr explicit BouncerDecider(bool verbose = false) : _verbose(verbose) {}

    /// Precondition: degree ≥ 1.
    [[nodiscard]] bouncer_result find(TuringMachine m, size_t degree, size_t maxSteps, size_t maxPeriod,
                                      size_t confidenceLevel = 5) const
    {
        std::vector<size_t> ls{0};
        std::vector<size_t> rs{0};
        while (!m.halted() && m.steps() < maxSteps)
        {
            auto res = m.step();
            if (!res.tapeExpanded)
                continue;
            // Tape grew
            if (m.head() < 0)
            {
                if (_verbose)
                {
                    std::cout << ansi::fg(204, 220, 255) << "L | ";
                    std::cout << std::setw(13) << formatDelta(ls.back(), m.steps()) << " | ";
                    print(m) << ansi::reset;
                }
                ls.push_back(m.steps());
                for (size_t p = 1; p <= maxPeriod; ++p)
                {
                    auto res = checkPoly(ls, degree, p, confidenceLevel);
                    if (res.found)
                    {
                        if (_verbose)
                            std::cout << "L: " << ls << ", xPeriod = " << p << '\n';
                        res.side = direction::left;
                        return res;
                    }
                }
            }
            else
            {
                if (_verbose)
                {
                    std::cout << ansi::fg(255, 204, 204) << " R| ";
                    std::cout << std::setw(13) << formatDelta(rs.back(), m.steps()) << " | ";
                    print(m) << ansi::reset;
                }
                rs.push_back(m.steps());
                for (size_t p = 1; p <= maxPeriod; ++p)
                {
                    auto res = checkPoly(rs, degree, p, confidenceLevel);
                    if (res.found)
                    {
                        if (_verbose)
                            std::cout << "R: " << rs << ", xPeriod = " << p << '\n';
                        res.side = direction::right;
                        return res;
                    }
                }
            }
        }
        return {};
    }

  private:
    bool _verbose;

    [[nodiscard]] bouncer_result checkPoly(const std::vector<size_t> &v, size_t degree, size_t p,
                                           size_t confidenceLevel) const
    {
        size_t n = degree + confidenceLevel; // Number of elements to check
        size_t N = 1 + (n - 1) * p;          // Size of range to check
        if (v.size() < N)
            return {};
        std::vector w(n, 0LL);
        auto start = v.end() - N;
        for (size_t i = 0; i < n; ++i)
            w[i] = *(start + i * p);
        for (size_t d = 1; d <= degree; ++d)
        {
            for (size_t i = 0; i < w.size() - 1; ++i)
                w[i] = w[i + 1] - w[i];
            w.pop_back();
            // Check if w is constant
            if (std::ranges::equal(std::ranges::subrange(w.begin(), w.end() - 1),
                                   std::ranges::subrange(w.begin() + 1, w.end())))
            {
                if (_verbose)
                    std::cout << "v = " << v << ", w = " << w << '\n';
                return {true, d, *start, p};
            }
        }
        return {};
    }

    static std::ostream &print(const TuringMachine &m)
    {
        std::cout << std::setw(6) << m.tape().size() << " | " << std::setw(10) << m.steps();
        std::cout << " | " << m.prettyStr(40) << '\n';
        return std::cout;
    }

    static std::string formatDelta(size_t a, size_t b)
    {
        std::ostringstream ss;
        double r = (double)b / a;
        if (a == 0 || r < 1.1)
            ss << "+" << b - a;
        else
            ss << std::fixed << std::setprecision(10) << r;
        return std::move(ss).str();
    }
};
} // namespace turing
