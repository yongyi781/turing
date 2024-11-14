#include "../pch.hpp"
#include "../turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

inline bool compareSymbol(symbol_type pattern, symbol_type b)
{
    if (pattern == (uint8_t)-1)
        return true;
    return pattern == b;
}

/// Puts brackets around the number if it's ≥ 10.
inline std::string countToString(size_t c)
{
    if (c < 10)
        return {(char)('0' + c)};
    return "[" + std::to_string(c) + "]";
}

// 01-RLE. For left side of head.
std::string str01(const ranges::range auto &&data)
{
    string res = "|";
    if (data.empty())
        return res;
    size_t c = 0;
    size_t i = 0;
    if (data[0] == 1)
    {
        c = 1;
        i = 1;
    }
    while (i < data.size())
    {
        while (i + 1 < data.size() && data[i] == 0 && data[i + 1] == 1)
        {
            ++c;
            i += 2;
        }
        res += countToString(c);
        c = 0;
        if (i >= data.size())
            break;
        if (data[i] == 0)
            res += '|';
        ++i;
    }
    return res;
}

// Encode consecutive 1s as integers. For right side of head
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
    if (c > 0)
        res += countToString(c);
    return res;
}

std::string str(const Tape &tape, size_t width)
{
    auto headPrefix = getBgStyle(tape.state());
    auto headSuffix = ansi::str(ansi::reset);
    ostringstream ss;
    ss << setw(width / 2) << str01(tape.getSegment(tape.leftEdge(), tape.head() - 1).data) << headPrefix
       << (*tape == 0 ? ' ' : (char)(*tape + '0')) << headSuffix << setw(width / 2) << left
       << str1(tape.getSegment(tape.head() + 1, tape.rightEdge()).data);
    return std::move(ss).str();
}

inline vector<int> analyze(turing::TuringMachine m, state_type stateToAnalyze, symbol_type symbolToAnalyze,
                           size_t maxSteps, size_t printWidth = 90)
{
    if (m.state() == stateToAnalyze && compareSymbol(symbolToAnalyze, *m.tape()))
        cout << setw(7) << m.steps() << " | " << m.prettyStr(printWidth) << '\n';

    auto tape = m.tape();
    auto steps = m.steps();
    int64_t lh = tape.head() - 1;
    int64_t hh = tape.head();

    boost::unordered_flat_map<macro_transition, int> tMap;
    vector<int> ts;
    int counter = 0;
    bool first = m.state() != stateToAnalyze || !compareSymbol(symbolToAnalyze, *m.tape());

    for (size_t i = 0; i < maxSteps && !m.halted(); ++i)
    {
        m.step();
        if (!m.halted() &&
            (m.state() != stateToAnalyze || !compareSymbol(symbolToAnalyze, *m.tape()) || m.tape()[m.head() - 1] != 0))
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
            --lh;
            // Print the result
            ostringstream ss;
            ss << setw(7) << m.steps() << " | ";
            // ss << setw(printWidth) << machine.tape().prettyStr() << " | ";
            ss << setw(printWidth) << str(m.tape(), printWidth) << " | ";
            if (first)
                first = false;
            else
            {
                auto fromSegment = tape.getSegment(lh, hh);
                auto toSegment = m.tape().getSegment(lh, hh);
                macro_transition key{fromSegment, toSegment, m.steps() - steps};
                if (!tMap.contains(key))
                    tMap[key] = counter++;
                int mIndex = tMap[key];
                ts.push_back(mIndex + 1);
                ss << getFgStyle(mIndex) << setw(4) << "T" + to_string(mIndex + 1) << ansi::reset << " = " << key;
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

auto run(string code, state_type state, state_type symbol, size_t steps)
{
    // return str01(it::wrap(string{"1010110101010110110110110110110110101101011011010101101010010"})
    //                  .map fun(x, (state_type)(x - '0'))
    //                  .to());
    auto res = analyze(std::move(code), state, symbol, steps);
    return it::wrap(res).map fun(x, (char)(x >= 10 ? 'A' + x - 10 : '0' + x)).to<string>();
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    string code = "1RB0RC_1LB1LD_0RA0LD_1LA1RC";
    state_type state = 0;
    symbol_type symbol = 0;
    Int steps = 15000;
    if (argc > 1)
    {
        if (argc == 2 && strlen(args[1]) > 2)
            steps = parseNumber(args[1]);
        else
        {
            state = toupper(args[1][0]) - 'A';
            if (strlen(args[1]) > 1)
                symbol = toupper(args[1][1]) - '0';
        }
    }
    if (argc > 2)
        steps = parseNumber(args[2]);
    ios::sync_with_stdio(false);
    printTiming(run, std::move(code), state, symbol, steps);
}
