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
    /// The number of steps taken to decide this bouncer.
    size_t steps = 0;
};

struct record
{
    size_t t = 0;
    direction side = direction::left;
    state_type state = 0;
};

class BouncerDecider
{
  public:
    constexpr explicit BouncerDecider(bool verbose = false) : _verbose(verbose) {}

    /// Precondition: degree ≥ 1.
    [[nodiscard]] bouncer_result find(TuringMachine m, size_t degree, size_t maxSteps, size_t maxPeriod,
                                      size_t confidenceLevel = 5) const
    {
        std::vector<record> records{{}};
        while (!m.halted() && m.steps() < maxSteps)
        {
            auto res = m.step();
            if (!res.tapeExpanded)
                continue;
            const record rec{
                .t = m.steps(), .side = m.head() < 0 ? direction::left : direction::right, .state = m.state()};
            if (_verbose)
            {
                if (rec.side == direction::left)
                    std::cout << ansi::fg(204, 220, 255) << "L | ";
                else
                    std::cout << ansi::fg(255, 204, 204) << " R| ";
                std::cout << std::setw(13) << formatDelta(records.back().t, m.steps()) << " | ";
                print(m) << ansi::reset;
            }
            records.push_back(rec);
            const auto hp = std::min(maxPeriod, (records.size() - 1) / (degree + confidenceLevel - 1));
            for (size_t p = 1; p <= hp; ++p)
            {
                auto res = checkPoly(records, degree, p, confidenceLevel);
                if (res.found)
                    return res;
            }
        }
        return {};
    }

  private:
    bool _verbose;

    [[nodiscard]] bouncer_result checkPoly(const std::vector<record> &v, size_t degree, size_t p,
                                           size_t confidenceLevel) const
    {
        const size_t n = degree + confidenceLevel; // Number of elements to check
        const size_t N = 1 + (n - 1) * p;          // Size of range to check
        if (v.size() < N)
            return {};
        auto start = v.end() - N;
        if (!sameRecordType(*start, v.back()))
            return {};
        std::vector w(n, (int64_t)start->t);
        for (size_t i = 1; i < n; ++i)
        {
            const auto rec = start + i * p;
            if (!sameRecordType(*rec, *start))
                return {};
            w[i] = rec->t;
        }
        // Calculate iterated finite differences
        for (size_t d = 1; d <= degree; ++d)
        {
            for (size_t i = 0; i < w.size() - 1; ++i)
                w[i] = w[i + 1] - w[i];
            w.pop_back();
            // Check if w is constant and that the coefficient is positive
            if (w.front() > 0 && std::ranges::equal(std::ranges::subrange(w.begin(), w.end() - 1),
                                                    std::ranges::subrange(w.begin() + 1, w.end())))
            {
                if (_verbose)
                    std::cout << "w = " << w << '\n';
                return {.found = true,
                        .degree = d,
                        .start = start->t,
                        .xPeriod = p,
                        .side = start->side,
                        .steps = v.back().t};
            }
        }
        return {};
    }

    static bool sameRecordType(const record &a, const record &b)
    {
        return (a.t == 0 || b.t == 0 || a.side == b.side) && a.state == b.state;
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
        const double r = (double)b / a;
        if (a == 0 || r < 1.1)
            ss << "+" << b - a;
        else
            ss << std::fixed << std::setprecision(10) << r;
        return std::move(ss).str();
    }
};
} // namespace turing
