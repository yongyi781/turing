// Utility to analyze a Turing machine based on tape growth.

#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

void run(turing_rule rule, size_t numSteps, bool noBlanks, state_type breakState, symbol_type breakSymbol)
{
    TuringMachine m{rule};
    cout << (char)('A' + m.state()) << (noBlanks ? "0 " : "_ ");
    while (m.steps() < numSteps)
    {
        auto res = m.step();
        if (!res.success)
            break;
        if (m.state() == breakState && (breakSymbol == (symbol_type)-1 || *m.tape() == breakSymbol))
            cout << '\n';
        cout << (char)('A' + m.state()) << (res.tapeExpanded && !noBlanks ? '_' : (char)('0' + *m.tape())) << ' ';
    }
    cout << '\n';
}

void runRLE(turing_rule rule, size_t numSteps, bool /*unused*/, state_type breakState, symbol_type breakSymbol)
{
    TuringMachine m{rule};
    string token = "A0";
    size_t c = 1;
    while (m.steps() < numSteps)
    {
        auto res = m.step();
        if (!res.success)
            break;
        const string token2{(char)('A' + m.state()), (char)('0' + *m.tape())};
        if (token2 == token)
            ++c;
        else
        {
            cout << token;
            if (c > 1)
                cout << "^" << c;
            cout << ' ';
            token = token2;
            c = 1;
        }
        if (m.state() == breakState && (breakSymbol == (symbol_type)-1 || *m.tape() == breakSymbol))
            cout << '\n';
    }
    if (c > 0)
    {
        cout << token;
        if (c > 1)
            cout << "^" << c;
        cout << ' ';
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
  -n, --no-blanks  Don't show new records as blanks
  -r, --rle        Output run length encoding
  -b, --break <x>  Break on state or state/symbol
)";
    const span args(argv, argc);
    turing_rule rule;
    size_t numSteps = 1000;
    bool noBlanks = false;
    bool rle = false;
    state_type breakState = -1;
    symbol_type breakSymbol = -1;
    int argPos = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--no-blanks") == 0)
            noBlanks = true;
        else if (strcmp(args[i], "-r") == 0 || strcmp(args[i], "--rle") == 0)
            rle = true;
        else if (strcmp(args[i], "-b") == 0 || strcmp(args[i], "--break") == 0)
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
    printTiming(rle ? runRLE : run, rule, numSteps, noBlanks, breakState, breakSymbol);
}
