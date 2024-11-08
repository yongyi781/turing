// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "../pch.hpp"

#include "tcycler.hpp"

using namespace std;
using namespace turing;

void run(turing_rule rule, size_t numSteps, size_t initialPeriodBound, bool verbose)
{
    auto &&res = CyclerDecider(verbose).find(rule, numSteps, initialPeriodBound);
    if (res.period == 0)
        cout << "No period found\n";
    else
        cout << "(period, preperiod) = " << tuple{res.period, res.preperiod} << '\n';
}

int main(int argc, char *argv[])
{
    constexpr string_view help =
        R"(Cycler decider. Output is in the form (period, preperiod).

Usage: ./run decide/cycler <TM>

Arguments:
  <TM>  The Turing machine

Options:
  -h, --help           Show this help message
  -v, --verbose        Show verbose output
  -p, --period <n>     The initial period bound (default: 10000)
  -n, --num-steps <n>  The number of steps to run for (default: unbounded)
)";
    span args(argv, argc);
    turing_rule rule;
    bool verbose = false;
    size_t numSteps = std::numeric_limits<size_t>::max();
    size_t initialPeriodBound = 10000;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--verbose") == 0)
            verbose = true;
        else if (strcmp(args[i], "-p") == 0 || strcmp(args[i], "--period") == 0)
            initialPeriodBound = parseNumber(args[++i]);
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = parseNumber(args[++i]);
        else if (rule.empty())
        {
            rule = turing_rule(args[i]);
            if (rule.empty())
            {
                cerr << ansi::red << "Invalid TM: " << ansi::reset << args[i] << '\n' << help;
                return 0;
            }
        }
        else
        {
            cerr << ansi::red << "Unexpected argument: " << ansi::reset << args[i] << '\n' << help;
            return 0;
        }
    }
    if (rule.empty())
    {
        cout << help;
        return 0;
    }
    printTiming(run, rule, numSteps, initialPeriodBound, verbose);
}
