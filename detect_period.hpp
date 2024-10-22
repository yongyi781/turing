#pragma once

#include "turing.hpp"
#include <euler.hpp>

namespace turing
{
/// Checks period of before and after.
inline bool checkForPeriod(const TuringMachine &before, const TuringMachine &after, int64_t start, int64_t stop)
{
    if (before.state() != after.state())
        return false;
    auto l = after.head() < before.head() ? after.tape().leftEdge() : start;
    auto h = after.head() > before.head() ? after.tape().rightEdge() : stop;
    return spansEqual(before.tape(), after.tape(), l - before.head(), h - before.head());
}

/// Returns whether the machine (at its current state) is purely periodic with the given period.
inline bool isPeriodic(TuringMachine m, size_t period)
{
    auto start = m;
    int64_t lh = start.head();
    int64_t hh = start.head();
    for (size_t p = 0; p < period; ++p)
    {
        if (!m.step().success)
            return false;
        lh = std::min(lh, m.head());
        hh = std::max(hh, m.head());
    }
    return checkForPeriod(start, m, lh, hh);
}

struct find_period_result
{
    size_t period = 0;
    size_t preperiod = 0;
    int64_t offset = 0;
    /// Last known machine that didn't yield a period.
    TuringMachine lastMachine;
};

class CyclerDetector
{
  public:
    constexpr CyclerDetector(bool verbose = false) : _verbose(verbose) {}

    [[nodiscard]]
    find_period_result findPeriod(TuringMachine m, size_t periodBound, size_t maxSteps) const
    {
        TuringMachine prev2 = m;
        while (m.steps() <= maxSteps)
        {
            if (_verbose)
                std::cout << m.steps() << " | " << m.str(true) << '\n';
            TuringMachine prev = m;
            int64_t lh = prev.head();
            int64_t hh = prev.head();
            for (size_t i = 1; i <= periodBound; ++i)
            {
                m.step();
                lh = std::min(lh, m.head());
                hh = std::max(hh, m.head());
                if (m.head() == prev.head() && checkForPeriod(prev, m, lh, hh))
                    return {i, m.steps() - i, m.head() - prev.head(), std::move(prev2)};
            }
            prev2 = prev;
        }
        return {};
    }

    /// Finds the preperiod. Requires exact period to be known.
    [[nodiscard]] size_t findPreperiod(TuringMachine m, size_t period, size_t low, size_t high) const
    {
        assert(low <= high);
        if (low < 0)
            low = 0;
        // Binary search
        for (size_t i = m.steps(); i < low; ++i)
            m.step();
        while (high - low > 1)
        {
            auto mid = low + (high - low) / 2;
            auto copy = m;
            for (size_t i = low; i < mid; ++i)
                copy.step();
            if (isPeriodic(copy, period))
            {
                if (_verbose)
                    std::cout << "Preperiod " << mid << " ✅\n";
                high = mid;
            }
            else
            {
                if (_verbose)
                    std::cout << "Preperiod " << mid << " ❌\n";
                low = mid;
                m = copy;
            }
        }
        return high;
    }

    /// The main period detection function. Returns (period, preperiod, offset).
    [[nodiscard]] find_period_result findPeriodAndPreperiod(TuringMachine machine, int64_t periodBound,
                                                            size_t maxSteps = std::numeric_limits<size_t>::max()) const
    {
        auto rule = machine.rule();
        auto res = findPeriod(machine, periodBound, maxSteps);
        if (res.period == 0)
        {
            // No period found.
            return {0UZ, 0UZ, 0LL, std::move(machine)};
        }
        if (_verbose)
            std::cout << "Preliminary: " << std::tuple{res.period, res.preperiod, res.offset} << '\n';
        auto &m = res.lastMachine;
        if (_verbose)
            std::cout << "Performing binary search with low = " << m.steps() << " and high = " << res.preperiod << '\n';
        auto preperiod = findPreperiod(m, res.period, m.steps(), res.preperiod);
        if (_verbose && (preperiod == 1 || preperiod == 2))
            std::cout << "Note: preperiod may be 0\n";
        return {res.period, preperiod, res.offset, std::move(machine)};
    }

  private:
    bool _verbose = false;
};

/// Detects translated cyclers. This will not catch cyclers.
class TranslatedCyclerDetector
{
  public:
    constexpr TranslatedCyclerDetector(bool verbose = false) : _verbose(verbose) {}

    /// Finds a period for the given Turing machine rule code with the given period bound. The returned preperiod is
    /// only an upper bound.
    [[nodiscard]]
    find_period_result findPeriod(TuringMachine m, size_t periodBound, size_t maxSteps) const
    {
        TuringMachine prev2 = m;
        while (m.steps() <= maxSteps)
        {
            TuringMachine prev;
            int expandDir = 0;
            // Grab edge tape
            for (size_t i = 0; i < periodBound; ++i)
            {
                auto &&[success, expanded] = m.step();
                if (!success)
                    return {};
                if (expanded)
                {
                    prev = m;
                    expandDir = m.head() < 0 ? -1 : 1;
                    if (_verbose)
                        std::cout << m.steps() << " | " << m.str(true) << '\n';
                    break;
                }
            }
            // If no edge tape, just continue
            if (expandDir == 0)
                continue;
            // Now try to find a period
            int64_t lh = prev.head();
            int64_t hh = prev.head();
            for (size_t p = 1; p <= periodBound; ++p)
            {
                auto &&[success, expanded] = m.step();
                if (!success)
                    return {};
                lh = std::min(lh, m.head());
                hh = std::max(hh, m.head());
                if (expanded && m.state() == prev.state())
                {
                    int expandDir2 = m.head() < 0 ? -1 : 1;
                    if (expandDir != expandDir2)
                        continue;

                    auto l = m.head() < 0 ? 0 : lh - prev.head();
                    auto h = m.head() < 0 ? hh - prev.head() : 0;
                    if (spansEqual(prev.tape(), m.tape(), l, h))
                    {
                        if (_verbose)
                            std::cout << ansi::green << ansi::bold << "[found] " << ansi::reset << m.steps() << " | "
                                      << m.str(true) << '\n';
                        return {p, m.steps() - p, m.head() - prev.head(), std::move(prev2)};
                    }
                }
            }
            prev2 = prev;
        }
        return {};
    }

    /// Finds the preperiod. Requires exact period to be known.
    [[nodiscard]] size_t findPreperiod(TuringMachine m, size_t period, size_t low, size_t high) const
    {
        assert(low <= high);
        if (low < 0)
            low = 0;
        // Binary search
        for (size_t i = m.steps(); i < low; ++i)
            m.step();
        while (high - low > 1)
        {
            auto mid = low + (high - low) / 2;
            auto copy = m;
            for (size_t i = low; i < mid; ++i)
                copy.step();
            if (isPeriodic(copy, period))
            {
                if (_verbose)
                    std::cout << "Preperiod " << mid << " ✅\n";
                high = mid;
            }
            else
            {
                if (_verbose)
                    std::cout << "Preperiod " << mid << " ❌\n";
                low = mid;
                m = copy;
            }
        }
        return high;
    }

    /// The main period detection function. Returns (period, preperiod, offset).
    [[nodiscard]] find_period_result findPeriodAndPreperiod(TuringMachine machine, int64_t periodBound,
                                                            size_t maxSteps = std::numeric_limits<size_t>::max()) const
    {
        auto rule = machine.rule();
        auto res = findPeriod(machine, periodBound, maxSteps);
        if (res.period == 0)
        {
            // No period found.
            return {0UZ, 0UZ, 0LL, std::move(machine)};
        }
        if (_verbose)
            std::cout << "Preliminary: " << std::tuple{res.period, res.preperiod, res.offset} << '\n';
        auto &m = res.lastMachine;
        if (_verbose)
            std::cout << "Performing binary search with low = " << m.steps() << " and high = " << res.preperiod << '\n';
        auto preperiod = findPreperiod(m, res.period, m.steps(), res.preperiod);
        if (_verbose && (preperiod == 1 || preperiod == 2))
            std::cout << "Note: preperiod may be 0\n";
        return {res.period, preperiod, res.offset, std::move(machine)};
    }

  private:
    bool _verbose = false;
};
} // namespace turing
