// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "../pch.hpp"

#include "bouncer.hpp"

using namespace std;
using namespace turing;

void run(turing_rule rule, size_t numSteps, size_t maxSkip, bool verbose)
{
    auto res = BouncerDecider{verbose}.find(rule, numSteps, maxSkip);
    if (res.found)
        cout << "(startStep, skip) = " << tuple{res.startStep, res.skip} << '\n';
    else
        cout << "No bouncer found\n";
}

int main(int argc, char *argv[])
{
    constexpr string_view help =
        R"(Bouncer detection tool. Currently heuristic based on tape growth, and catches bells (including transient ones) along with bouncers.

Usage: ./run decide/bouncer <TM>

Arguments:
  <TM>  The Turing machine

Options:
  -h, --help       Show this help message
  -v, --verbose    Show verbose output
  -n, --num-steps  The number of steps to run for (default: 1000000)
  -m, --max-skip   The maximum skip number (default: 4)
)";
    span args(argv, argc);
    turing_rule rule;
    bool verbose = false;
    size_t numSteps = 1'000'000;
    size_t maxSkip = 4;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--verbose") == 0)
            verbose = true;
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = parseNumber(args[++i]);
        else if (strcmp(args[i], "-m") == 0 || strcmp(args[i], "--max-skip") == 0)
            maxSkip = parseNumber(args[++i]);
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
    printTiming(run, rule, numSteps, maxSkip, verbose);
}
