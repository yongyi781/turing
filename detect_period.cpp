// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "pch.hpp"

#include "detect_period.hpp"

using namespace std;
using namespace turing;

auto solve(turing_rule rule, size_t numSteps, size_t initialPeriodBound)
{
    auto &&res = TranslatedCyclerDetector(true).findPeriodAndPreperiod(rule, numSteps, initialPeriodBound);
    return tuple{res.period, res.preperiod, res.offset};
}

int main(int argc, char *argv[])
{
    constexpr string_view help =
        R"(Translated cycler period detection tool. Output is in the form (period, preperiod, offset).

Usage: ./run detect_period <code>

Arguments:
  <code>   The Turing machine

Options:
  -h, --help       Show this help message
  -p, --period     The initial period bound (default: 10000)
  -n, --num-steps  The number of steps to run for (default: unbounded)
)";
    span args(argv, argc);
    turing_rule rule;
    size_t numSteps = std::numeric_limits<size_t>::max();
    size_t initialPeriodBound = 10000;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-p") == 0 || strcmp(args[i], "--period") == 0)
            initialPeriodBound = stoull(args[++i]);
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = stoull(args[++i]);
        else if (rule.numStates() == 0)
        {
            rule = turing_rule(args[i]);
            if (rule.numStates() == 0)
            {
                cerr << ansi::red << "Invalid code: " << ansi::reset << args[i] << '\n' << help;
                return 0;
            }
        }
        else
        {
            cerr << ansi::red << "Unexpected argument: " << ansi::reset << args[i] << '\n' << help;
            return 0;
        }
    }
    if (rule.numStates() == 0)
    {
        cout << help;
        return 0;
    }
    printTiming(solve, rule, numSteps, initialPeriodBound);
}
