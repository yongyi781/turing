// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "../pch.hpp"

#include "tcycler.hpp"

using namespace std;
using namespace turing;

void run(turing_rule rule, size_t numSteps, size_t initialPeriodBound, bool fast, bool verbose)
{
    find_period_result res;
    if (fast)
        res = TranslatedCyclerDecider{verbose}.findPeriodOnly(rule, numSteps, initialPeriodBound);
    else
        res = TranslatedCyclerDecider(verbose).find(rule, numSteps, initialPeriodBound);
    if (res.period == 0)
        cout << "No period found\n";
    else
        cout << "(period, preperiod, offset) = " << tuple{res.period, res.preperiod, res.offset} << '\n';
}

int main(int argc, char *argv[])
{
    constexpr string_view help =
        R"(Translated cycler decider. Output is in the form (period, preperiod, offset).

Usage: ./run decide/tcycler <TM>

Arguments:
  <TM>  The Turing machine

Options:
  -h, --help           Show this help message
  -f, --fast           Don't calculate exact preperiod
  -n, --num-steps <n>  The number of steps to run for (default: unbounded)
  -p, --period <n>     The initial period bound (default: 10000)
  -v, --verbose        Show verbose output
)";
    span args(argv, argc);
    turing_rule rule;
    bool fast = false;
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
        if (strcmp(args[i], "-f") == 0 || strcmp(args[i], "--fast") == 0)
            fast = true;
        else if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--verbose") == 0)
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
    printTiming(run, rule, numSteps, initialPeriodBound, fast, verbose);
}
