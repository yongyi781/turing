#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

bool compareSymbol(symbol_type pattern, symbol_type b)
{
    if (pattern == (uint8_t)-1)
        return true;
    return pattern == b;
}

/// Puts brackets around the number if it's ≥ 10.
inline std::string countToString(size_t c)
{
    if (c == 0)
        return {' '};
    if (c < 10)
        return std::to_string(c);
    return "[" + std::to_string(c) + "]";
}

/// Returns a string representation of this tape, by encoding runs of 1s. For example, 1_11_111 is 123, and 1__1
/// is 101.
std::string str1(const ranges::range auto &&data)
{
    string res;
    size_t c = 0;
    for (auto &&x : data)
        if (x == 0)
        {
            res += countToString(c);
            c = 0;
        }
        else
            ++c;
    res += countToString(c);
    return res;
}

std::string str(const Tape &tape, size_t width)
{
    auto headPrefix = getBgStyle(tape.state());
    auto headSuffix = ansi::str(ansi::reset);
    ostringstream ss;
    ss << setw(width / 2) << str1(tape.getSegment(tape.leftEdge(), tape.head() - 1).data) << headPrefix
       << (*tape == 0 ? ' ' : (char)(*tape + '0')) << headSuffix << setw(width / 2) << left
       << str1(tape.getSegment(tape.head() + 1, tape.rightEdge()).data);
    return std::move(ss).str();
}

inline vector<int> analyze(turing::TuringMachine m, state_type stateToAnalyze, symbol_type symbolToAnalyze,
                           size_t maxSteps = 10000, size_t printWidth = 60)
{
    if (m.state() == stateToAnalyze && compareSymbol(symbolToAnalyze, *m.tape()))
        cout << setw(7) << m.steps() << " | " << str(m.tape(), printWidth) << '\n';

    auto tape = m.tape();
    auto steps = m.steps();
    int64_t lh = tape.head();
    int64_t hh = tape.head();

    boost::unordered_flat_map<packed_transition, int> tMap;
    vector<int> ts;
    int counter = 0;
    bool first = m.state() != stateToAnalyze || !compareSymbol(symbolToAnalyze, *m.tape());

    for (size_t i = 0; i < maxSteps && !m.halted(); ++i)
    {
        m.step();
        if (!m.halted() && (m.state() != stateToAnalyze || !compareSymbol(symbolToAnalyze, *m.tape())))
        {
            lh = min(lh, m.tape().head());
            hh = max(hh, m.tape().head());
        }
        else
        {
            bool print = true;
            // If there is a symbol filter, update lh and hh
            if (symbolToAnalyze != (uint8_t)-1)
            {
                lh = min(lh, m.tape().head());
                hh = max(hh, m.tape().head());
            }
            // Print the result
            ostringstream ss;
            ss << setw(7) << m.steps() << " | ";
            ss << str(m.tape(), 2 * printWidth) << " | ";
            if (first)
                first = false;
            else
            {
                auto fromSegment = tape.getSegment(lh, hh);
                auto toSegment = m.tape().getSegment(lh, hh);
                packed_transition key{fromSegment, toSegment, m.steps() - steps};
                if (!tMap.contains(key))
                    tMap[key] = counter++;
                // else
                //     print = false;
                int mIndex = tMap[key];
                ts.push_back(mIndex + 1);
                ss << getFgStyle(mIndex) << setw(4) << "T" + to_string(mIndex + 1) << ansi::reset << " = " << key;
                // print = mIndex == 3;
            }
            if (print)
                cout << std::move(ss).str() << '\n';
            tape = m.tape();
            steps = m.steps();
            lh = hh = m.head();
        }
    }
    cout << "transitions = " << ts.size() << " | distinct transitions = " << tMap.size() << '\n';
    return ts;
}

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
