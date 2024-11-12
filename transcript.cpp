// Utility to analyze a Turing machine based on tape growth.

#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

void run(turing_rule rule, size_t numSteps, state_type breakState, symbol_type breakSymbol)
{
    TuringMachine m{rule};
    cout << (char)('A' + m.state()) << "_ ";
    while (m.steps() < numSteps)
    {
        auto res = m.step();
        if (!res.success)
            break;
        if (breakState == -1 ||
            (m.state() == breakState && (breakSymbol == (symbol_type)-1 || *m.tape() == breakSymbol)))
            cout << '\n';
        cout << (char)('A' + m.state()) << (res.tapeExpanded ? '_' : (char)('0' + *m.tape())) << ' ';
    }
    cout << '\n';
}

int main(int argc, char *argv[])
{
    constexpr string_view help = R"(Turing machine transcript tool

Usage: ./run transcript <TM> [n]

Arguments:
  <TM>  The Turing machine.
  [n]   Number of steps (default: 1000)

Options:
  -h, --help       Show this help message
  -b, --break <x>  Break on state or state/symbol
)";
    span args(argv, argc);
    turing_rule rule;
    size_t numSteps = 1000;
    int argPos = 0;
    state_type breakState = -1;
    symbol_type breakSymbol = -1;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-b") == 0 || strcmp(args[i], "--break") == 0)
        {
            breakState = toupper(args[++i][0]) - 'A';
            if (strlen(args[i]) > 1)
                breakSymbol = toupper(args[i][1]) - '0';
        }
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
    printTiming(run, rule, numSteps, breakState, breakSymbol);
}
