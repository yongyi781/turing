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

inline vector<int> analyze(turing::TuringMachine machine, char stateToAnalyze, size_t steps = 10000,
                           size_t printWidth = 60)
{
    if (machine.state() == stateToAnalyze)
        cout << setw(7) << machine.steps() << " | " << machine.str(true, printWidth) << '\n';

    auto tape = machine.tape();
    int64_t lh = tape.head();
    int64_t hh = tape.head();

    boost::unordered_flat_map<packed_transition, int> tMap;
    vector<int> ts;
    int counter = 0;
    bool first = !(machine.state() == stateToAnalyze);

    for (size_t i = 0; i < steps && !machine.halted(); ++i)
    {
        machine.step();
        if (!machine.halted() && machine.state() != stateToAnalyze)
        {
            lh = min(lh, machine.tape().head());
            hh = max(hh, machine.tape().head());
        }
        else
        {
            // Print the result
            ostringstream ss;
            ss << setw(7) << machine.steps() << " | " << machine.str(true, printWidth) << " | ";
            // ss << machine.str1(true, width2) << " | ";
            if (first)
                first = false;
            else
            {
                auto fromSegment = getTapeSegment(tape, stateToAnalyze, lh, hh);
                auto toSegment = getTapeSegment(machine.tape(), machine.state(), lh, hh);
                packed_transition key{fromSegment, toSegment};
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
            lh = hh = machine.head();
        }
    }
    cout << "transitions = " << ts.size() << " | distinct transitions = " << tMap.size() << '\n';
    return ts;
}

inline turing::TuringMachine universal23() { return {"1RB2LA1LA_2LA2RB0RA"}; }
inline turing::TuringMachine bb622() { return {"1RB0RF_1RC0LD_1LB1RC_---0LE_1RA1LE_---0RC"}; }

auto solve(string code, char state, size_t steps)
{
    auto res = analyze(std::move(code), state, steps);
    return it::wrap(res).map fun(x, (char)(x >= 10 ? 'A' + x - 10 : '0' + x)).to<string>();
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    string code = "1RB0LC_1RC1RB_1LA0LD_0LC0RB";
    char state = 'A';
    Int steps = 1000;
    if (argc > 1)
    {
        code = args[1];
        if (ranges::count(code, '_') == 0)
        {
            cout << "Usage: analyze <code> [steps]\n";
            return 0;
        }
    }
    if (argc > 2)
    {
        if (argc == 3 && strlen(args[2]) > 1)
            steps = stoull(args[2]);
        else
            state = toupper(args[2][0]);
    }
    if (argc > 3)
        steps = stoull(args[3]);
    ios::sync_with_stdio(false);
    printTiming(solve, std::move(code), state, steps);
}
