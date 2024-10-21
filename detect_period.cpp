// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

inline void print(const TuringMachine &m)
{
    cout << setw(10) << m.steps();
    cout << " | " << m.str(true, 20) << '\n';
}

tuple<int64_t, int64_t, int64_t, Tape, size_t, char> findPeriod(string code, Int periodBound)
{
    TuringMachine m{std::move(code)};
    Tape lastTapeWithoutPeriod = m.tape();
    size_t lastStepsWithoutPeriod = 0;
    char lastStateWithoutPeriod = 'A';
    Tape tape;
    size_t steps = 0;
    char state = 0;
    int expandDir = 0;
    while (true)
    {
        // Grab edge tape
        for (Int i = 0; i < periodBound; ++i)
        {
            auto &&[success, expanded] = m.step();
            if (!success)
                return {};
            if (expanded)
            {
                tape = m.tape();
                steps = m.steps();
                state = m.state();
                expandDir = m.head() < 0 ? -1 : 1;
                cout << m.steps() << " | " << m.str(true) << '\n';
                break;
            }
        }
        // If no edge tape, just continue
        if (state == 0)
            continue;
        // Now try to find a period
        int64_t lh = tape.head();
        int64_t hh = tape.head();
        for (Int p = 1; p <= periodBound; ++p)
        {
            auto &&[success, expanded] = m.step();
            if (!success)
                return {};
            lh = std::min(lh, m.tape().head());
            hh = std::max(hh, m.tape().head());
            if (expanded && m.state() == state)
            {
                int expandDir2 = m.head() < 0 ? -1 : 1;
                if (expandDir != expandDir2)
                    continue;

                if ((m.head() < 0 && spansEqual(tape, m.tape(), 0, hh - tape.head())) ||
                    (m.head() > 0 && spansEqual(tape, m.tape(), lh - tape.head(), 0)))
                {
                    cout << ansi::green << ansi::bold << "[found] " << ansi::reset << m.steps() << " | " << m.str(true)
                         << '\n';
                    return {p,
                            m.steps() - p,
                            m.head() - tape.head(),
                            lastTapeWithoutPeriod,
                            lastStepsWithoutPeriod,
                            lastStateWithoutPeriod};
                }
            }
        }
        lastTapeWithoutPeriod = tape;
        lastStepsWithoutPeriod = steps;
        lastStateWithoutPeriod = state;
    }
}

/// Returns whether the machine is periodic with the given period, without a preperiod.
inline bool isPeriodic(TuringMachine m, size_t period)
{
    auto startTape = m.tape();
    auto startState = m.state();
    int64_t startHead = startTape.head();
    int64_t ld = 0;
    int64_t hd = 0;
    for (size_t p = 1; p <= period; ++p)
    {
        m.step();
        if (m.halted())
            return false;
        ld = std::min(ld, m.tape().head() - startHead);
        hd = std::max(hd, m.tape().head() - startHead);
    }
    if (m.state() == startState)
    {
        auto l = ld;
        auto h = hd;
        if (m.tape().head() < startHead)
            l = std::min(-startTape.offset() - startHead, -m.tape().offset() - m.tape().head());
        else if (m.tape().head() > startHead)
            h = std::max(startTape.data().size() - startHead - startTape.offset(),
                         m.tape().data().size() - m.tape().head() - m.tape().offset());
        if (spansEqual(startTape, m.tape(), l, h))
            return true;
    }
    return false;
}

/// Finds the preperiod. Requires exact period to be found.
inline int64_t findPreperiod(TuringMachine m, size_t period, int64_t low, int64_t high)
{
    if (low < 0)
        low = 0;
    // Binary search
    for (int64_t i = m.steps(); i < low; ++i)
        m.step();
    while (high - low > 1)
    {
        auto mid = low + (high - low) / 2;
        auto copy = m;
        for (int64_t i = low; i < mid; ++i)
            copy.step();
        if (isPeriodic(copy, period))
        {
            cout << "Preperiod " << mid << " ✅\n";
            high = mid;
        }
        else
        {
            cout << "Preperiod " << mid << " ❌\n";
            low = mid;
            m = copy;
        }
    }
    return high;
}

auto solve(string code, Int periodBound)
{
    auto &&[p, preperiodBound, offset, last, steps, state] = findPeriod(code, periodBound);
    cout << "Preliminary: " << tuple{p, preperiodBound, offset} << '\n';
    TuringMachine m{std::move(code)};
    m.tape(last);
    m.steps(steps);
    m.state(state);
    cout << "Performing binary search with low = " << steps << " and high = " << preperiodBound << '\n';
    auto preperiod = findPreperiod(m, p, steps, preperiodBound);
    if (preperiod == 2)
        cout << "Note: preperiod may be 0\n";
    return tuple{p, preperiod, offset};
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    string code = "1RB0LC_1RD1LC_0LA1LB_1LC0RD";
    Int periodBound = 250'000'000;
    if (argc > 1)
    {
        code = args[1];
        if (ranges::count(code, '_') == 0)
        {
            cout << "Usage: detect_period <code>\n";
            return 0;
        }
    }
    if (argc > 2)
        periodBound = stoull(args[2]);
    printTiming(solve, code, periodBound);
}
