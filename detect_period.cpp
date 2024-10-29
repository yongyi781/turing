// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "pch.hpp"

#include "detect_period.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

inline void print(const TuringMachine &m)
{
    cout << setw(10) << m.steps();
    cout << " | " << m.str(true, 20) << '\n';
}

auto solve(string code, Int periodBound)
{
    auto &&[a, b, c, d] = TranslatedCyclerDetector(true).findPeriodAndPreperiod(
        std::move(code), numeric_limits<size_t>::max(), periodBound);
    return tuple{a, b, c};
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    string code = "1RB0LC_1RD1LC_0LA1LB_1LC0RD";
    Int periodBound = 1000;
    if (argc > 1)
    {
        code = args[1];
        if (ranges::count(code, '_') == 0)
        {
            cout << "Usage: detect_period <code> [period bound]\n";
            return 0;
        }
    }
    if (argc > 2)
        periodBound = stoull(args[2]);
    printTiming(solve, code, periodBound);
}
