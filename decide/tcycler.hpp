#pragma once

#include "../turing.hpp"

namespace turing
{
constexpr double periodGrowthRatio = 1.1;

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

/// Finds the preperiod. Requires exact period to be known.
[[nodiscard]] inline size_t findPreperiod(TuringMachine m, size_t period, size_t low, size_t high, bool verbose = false)
{
    assert(low <= high);
    // Binary search
    if (m.steps() > low)
        m.reset();
    for (size_t i = m.steps(); i < low; ++i)
        m.step();
    if (isPeriodic(m, period))
        return low;
    while (high - low > 1)
    {
        auto mid = low + (high - low) / 2;
        auto copy = m;
        for (size_t i = low; i < mid; ++i)
            copy.step();
        if (isPeriodic(copy, period))
        {
            if (verbose)
                std::cout << "  preperiod " << mid << " ✅\n";
            high = mid;
        }
        else
        {
            if (verbose)
                std::cout << "  preperiod " << mid << " ❌\n";
            low = mid;
            m = copy;
        }
    }
    return high;
}

struct cycler_result
{
    size_t period = 0;
    size_t preperiod = 0;
    int64_t offset = 0;
    /// Last known machine that didn't yield a period.
    TuringMachine lastMachine;
};

class CyclerDecider
{
  public:
    constexpr CyclerDecider(bool verbose = false) : _verbose(verbose) {}

    [[nodiscard]] constexpr bool verbose() const { return _verbose; }

    [[nodiscard]]
    cycler_result findPeriodOnly(TuringMachine machine, size_t maxSteps, size_t startPeriodBound = 100) const
    {
        size_t periodBound = startPeriodBound;
        TuringMachine prev2 = machine;
        maxSteps += machine.steps();
        while (machine.steps() <= maxSteps)
        {
            if (_verbose)
                std::cout << machine.steps() << " | " << machine.prettyStr() << '\n';
            const TuringMachine prev = machine;
            int64_t lh = prev.head();
            int64_t hh = prev.head();
            for (size_t i = 1; i <= periodBound; ++i)
            {
                machine.step();
                lh = std::min(lh, machine.head());
                hh = std::max(hh, machine.head());
                if (machine.head() == prev.head() && checkForPeriod(prev, machine, lh, hh))
                {
                    TuringMachine lastMachine =
                        i <= startPeriodBound ? std::move(prev2) : TuringMachine{machine.rule()};
                    return {.period = i,
                            .preperiod = machine.steps() - i,
                            .offset = machine.head() - prev.head(),
                            .lastMachine = std::move(lastMachine)};
                }
            }
            periodBound = std::max(periodBound + 1, (size_t)(periodBound * periodGrowthRatio));
            prev2 = prev;
        }
        return {};
    }

    /// The main period detection function. Returns (period, preperiod, offset).
    template <typename Self>
    [[nodiscard]] cycler_result find(this const Self &self, TuringMachine machine, size_t maxSteps,
                                     size_t startPeriodBound = 100)
    {
        auto res = self.findPeriodOnly(machine, maxSteps, startPeriodBound);
        if (res.period == 0)
        {
            // No period found.
            return {.period = 0UZ, .preperiod = 0UZ, .offset = 0LL, .lastMachine = std::move(machine)};
        }
        if (self._verbose)
            std::cout << "period = " << res.period << ", offset = " << res.offset << '\n';
        auto &m = res.lastMachine;
        if (self._verbose)
            std::cout << "Performing binary search with low = " << m.steps() << " and high = " << res.preperiod << '\n';
        auto preperiod = findPreperiod(m, res.period, m.steps(), res.preperiod, self._verbose);
        // If preperiod was the minimum, restart binary search from 0.
        if (preperiod == m.steps())
            preperiod = findPreperiod(m, res.period, 0, preperiod, self._verbose);
        return {res.period, preperiod, res.offset, std::move(machine)};
    }

  private:
    bool _verbose = false;
};

/// Detects translated cyclers. This will not catch cyclers.
class TranslatedCyclerDecider : public CyclerDecider
{
  public:
    constexpr TranslatedCyclerDecider(bool verbose = false) : CyclerDecider(verbose) {}

    /// Finds a period for the given Turing machine rule code with the given period bound. The returned preperiod is
    /// only an upper bound.
    [[nodiscard]]
    cycler_result findPeriodOnly(TuringMachine machine, size_t maxSteps, size_t startPeriodBound = 1000) const
    {
        size_t periodBound = startPeriodBound;
        size_t prevPeriodBound = 0;
        TuringMachine prev2 = machine;
        maxSteps += machine.steps();
        while (machine.steps() <= maxSteps)
        {
            TuringMachine prev;
            int expandDir = 0;
            // Grab edge tape
            for (size_t i = 0; i < periodBound; ++i)
            {
                auto res = machine.step();
                if (!res.success)
                    return {};
                if (res.tapeExpanded)
                {
                    prev = machine;
                    expandDir = machine.head() < 0 ? -1 : 1;
                    if (verbose())
                        std::cout << "period bound = " << periodBound << " | " << machine.steps() << " | "
                                  << machine.prettyStr() << '\n';
                    break;
                }
            }
            // If no edge tape, just continue
            if (expandDir == 0)
                continue;
            // Now try to find a period
            int64_t lh = prev.head();
            int64_t hh = prev.head();
            for (size_t i = 1; i <= periodBound; ++i)
            {
                auto res = machine.step();
                if (!res.success)
                    return {};
                lh = std::min(lh, machine.head());
                hh = std::max(hh, machine.head());
                if (res.tapeExpanded && machine.state() == prev.state())
                {
                    const int expandDir2 = machine.head() < 0 ? -1 : 1;
                    if (expandDir != expandDir2)
                        continue;

                    auto l = machine.head() < 0 ? 0 : lh - prev.head();
                    auto h = machine.head() < 0 ? hh - prev.head() : 0;
                    if (spansEqual(prev.tape(), machine.tape(), l, h))
                    {
                        if (verbose())
                            std::cout << ansi::green << ansi::bold << "[found] " << ansi::reset << machine.steps()
                                      << " | " << machine.prettyStr() << '\n';
                        // i = period.
                        TuringMachine lastMachine =
                            prevPeriodBound >= i ? std::move(prev2) : TuringMachine{machine.rule()};
                        return {.period = i,
                                .preperiod = machine.steps() - i,
                                .offset = machine.head() - prev.head(),
                                .lastMachine = std::move(lastMachine)};
                    }
                }
            }
            prevPeriodBound = periodBound;
            periodBound = std::max(periodBound + 1, (size_t)(periodBound * periodGrowthRatio));
            prev2 = prev;
        }
        return {};
    }
};
} // namespace turing
