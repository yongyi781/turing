// Utility to detect a translated cycler efficiently. This won't detect in-place cyclers but those are easily detected
// by spotting that the tape size becomes eventually constant.

#include "../pch.hpp"

#include "bouncer.hpp"

using namespace std;
using namespace turing;

void run(turing_rule rule, size_t degree, size_t numSteps, size_t maxPeriod, size_t confidenceLevel, bool verbose)
{
    auto res = BouncerDecider{verbose}.find(rule, degree, numSteps, maxPeriod, confidenceLevel);
    if (res.found)
        cout << "(degree, start, xPeriod, side) = "
             << tuple{res.degree, res.start, res.xPeriod, res.side == direction::left ? 'L' : 'R'} << '\n';
    else
        cout << "No bouncer found\n";
}

int main(int argc, char *argv[])
{
    constexpr string_view help =
        R"(Heuristic bouncer decider

Usage: ./run decide/bouncer <TM>

Arguments:
  <TM>  The Turing machine

Options:
  -h, --help              Show this help message
  -v, --verbose           Show verbose output
  -d, --degree            The maximum degree to check for (default: 3)
  -n, --num-steps         The number of steps to run for (default: 1e8)
  -p, --period            The maximum x-period to check for (default: 100)
  -c, --confidence-level  The number of extra tape growth terms to check (default: 5)

Comments:
  Currently heuristic based on tape growth, and catches bells (including transient ones)
  along with bouncers. Can detect cubic, quartic, etc. bells as well.
)";
    span args(argv, argc);
    turing_rule rule;
    bool verbose = false;
    size_t numSteps = 100'000'000;
    size_t degree = 3;
    size_t maxPeriod = 100;
    size_t confidenceLevel = 5;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--verbose") == 0)
            verbose = true;
        else if (strcmp(args[i], "-d") == 0 || strcmp(args[i], "--degree") == 0)
            degree = parseNumber(args[++i]);
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = parseNumber(args[++i]);
        else if (strcmp(args[i], "-p") == 0 || strcmp(args[i], "--period") == 0)
            maxPeriod = parseNumber(args[++i]);
        else if (strcmp(args[i], "-c") == 0 || strcmp(args[i], "--confidence-level") == 0)
            confidenceLevel = parseNumber(args[++i]);
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
    ios::sync_with_stdio(false);
    printTiming(run, rule, degree, numSteps, maxPeriod, confidenceLevel, verbose);
}
