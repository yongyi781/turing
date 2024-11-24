#include "pch.hpp"

using namespace std;
using namespace turing;

auto run(turing_rule rule, size_t numSteps, bool verbose)
{
    TuringMachine m{rule};
    array<array<size_t, maxSymbols>, maxStates> counts{};
    while (m.steps() < numSteps)
    {
        if (verbose)
            ++counts[m.state()][*m.tape()];
        if (!m.step().success)
            break;
    }
    if (verbose)
    {
        cout << "Transcript histogram:\n\n";
        table(range('A', (char)('A' + rule.numStates() - 1)), range(0, rule.numSymbols() - 1),
              fun2(s, j, counts[s - 'A'][j]))
            << '\n';
    }
    return pair{m.steps(), m.tape()};
}

int main(int argc, char *argv[])
{
    constexpr string_view help = R"(Simulates a Turing machine and outputs the final tape

Usage: ./run simulate <TM> <n>

Arguments:
  <TM>  The Turing machine
  <n>   Number of steps

Options:
  -h, --help     Show this help message
  -v, --verbose  Show more info
)";
    const span args(argv, argc);
    turing_rule rule;
    size_t numSteps = 0;
    bool verbose = false;
    int argPos = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-v") == 0 || strcmp(args[i], "--verbose") == 0)
            verbose = true;
        else if (argPos == 0)
        {
            ++argPos;
            rule = turing_rule(args[i]);
            if (rule.empty())
            {
                cerr << ansi::red << "Invalid TM: " << ansi::reset << args[i] << '\n' << help;
                return 0;
            }
        }
        else if (argPos == 1)
        {
            ++argPos;
            numSteps = parseNumber(args[i]);
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
    printTiming(run, rule, numSteps, verbose);
}
