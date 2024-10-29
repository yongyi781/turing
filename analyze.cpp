#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

string getFgStyle(int index)
{
    switch (index)
    {
    case 0:
        return ansi::str(ansi::red);
    case 1:
        return ansi::str(ansi::yellow);
    case 2:
        return ansi::str(ansi::blue);
    case 3:
        return ansi::str(ansi::green);
    case 4:
        return ansi::str(ansi::magenta);
    case 5:
        return ansi::str(ansi::brightCyan);
    default:
        return ansi::str(ansi::white);
    }
}

bool compareSymbol(symbol_type pattern, symbol_type b)
{
    if (pattern == (uint8_t)-1)
        return true;
    return pattern == b;
}

inline vector<int> analyze(turing::TuringMachine machine, state_type stateToAnalyze, symbol_type symbolToAnalyze,
                           size_t maxSteps = 10000, size_t printWidth = 60)
{
    if (machine.state() == stateToAnalyze && compareSymbol(symbolToAnalyze, *machine.tape()))
        cout << setw(7) << machine.steps() << " | " << machine.str(true, printWidth) << '\n';

    auto tape = machine.tape();
    auto steps = machine.steps();
    int64_t lh = tape.head();
    int64_t hh = tape.head();

    boost::unordered_flat_map<packed_transition, int> tMap;
    vector<int> ts;
    int counter = 0;
    bool first = machine.state() != stateToAnalyze || !compareSymbol(symbolToAnalyze, *machine.tape());

    for (size_t i = 0; i < maxSteps && !machine.halted(); ++i)
    {
        machine.step();
        if (!machine.halted() &&
            (machine.state() != stateToAnalyze || !compareSymbol(symbolToAnalyze, *machine.tape())))
        {
            lh = min(lh, machine.tape().head());
            hh = max(hh, machine.tape().head());
        }
        else
        {
            // If there is a symbol filter, update lh and hh
            if (symbolToAnalyze != (uint8_t)-1)
            {
                lh = min(lh, machine.tape().head());
                hh = max(hh, machine.tape().head());
            }
            // Print the result
            ostringstream ss;
            ss << setw(7) << machine.steps() << " | " << machine.str(true, printWidth) << " | ";
            ss << machine.str1(true, printWidth) << " | ";
            if (first)
                first = false;
            else
            {
                auto fromSegment = getTapeSegment(tape, stateToAnalyze, lh, hh);
                auto toSegment = getTapeSegment(machine.tape(), machine.state(), lh, hh);
                packed_transition key{fromSegment, toSegment, machine.steps() - steps};
                if (!tMap.contains(key))
                    tMap[key] = counter++;
                int mIndex = tMap[key];
                ts.push_back(mIndex + 1);
                auto x = machine.head() - tape.head();
                ss << getFgStyle(mIndex) << setw(4) << "T" + to_string(mIndex + 1) << ansi::reset << " = [" << key
                   << "] (" << (x < 0 ? '-' : '+') << abs(x) << ")";
            }
            cout << std::move(ss).str() << '\n';
            tape = machine.tape();
            steps = machine.steps();
            lh = hh = machine.head();
        }
    }
    cout << "transitions = " << ts.size() << " | distinct transitions = " << tMap.size() << '\n';
    return ts;
}

inline turing::TuringMachine universal23() { return {"1RB2LA1LA_2LA2RB0RA"}; }
inline turing::TuringMachine bb622() { return {"1RB0RF_1RC0LD_1LB1RC_---0LE_1RA1LE_---0RC"}; }

auto solve(string code, state_type state, state_type symbol, size_t steps)
{
    auto res = analyze(std::move(code), state, symbol, steps);
    return it::wrap(res).map fun(x, (char)(x >= 10 ? 'A' + x - 10 : '0' + x)).to<string>();
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    if (argc == 1 || (argc > 1 && ranges::count(string{args[1]}, '_') == 0))
    {
        cout << "Usage: analyze <code> [state[symbol]] [steps]\n";
        return 0;
    }
    string code = args[1];
    state_type state = 0;
    symbol_type symbol = -1;
    Int steps = 1000;
    if (argc > 2)
    {
        if (argc == 3 && strlen(args[2]) > 2)
            steps = stoull(args[2]);
        else
        {
            state = toupper(args[2][0]) - 'A';
            if (strlen(args[2]) > 1)
                symbol = toupper(args[2][1]) - '0';
        }
    }
    if (argc > 3)
        steps = stoull(args[3]);
    ios::sync_with_stdio(false);
    printTiming(solve, std::move(code), state, symbol, steps);
}
