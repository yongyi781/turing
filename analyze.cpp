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

    boost::unordered_flat_map<macro_transition, int> tMap;
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
            const bool print = true;
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
                const macro_transition key{.from = fromSegment, .to = toSegment, .steps = m.steps() - steps};
                if (!tMap.contains(key))
                    tMap[key] = counter++;
                // else
                //     print = false;
                const int mIndex = tMap[key];
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

auto run(turing_rule rule, state_type stateFilter, state_type symbolFilter, size_t steps, size_t width)
{
    auto res = analyze(rule, stateFilter, symbolFilter, steps, width);
    return it::wrap(res).map fun(x, (char)(x >= 10 ? 'A' + x - 10 : '0' + x)).to<string>();
}

int main(int argc, char *argv[])
{
    constexpr string_view help = R"(Turing machine macro transition analyzer

Usage: ./run analyze <TM>

Arguments:
  <TM>  The Turing machine.

Options:
  -h, --help                Show this help message
  -f, --filter <filter>     The state and/or symbol to filter on, e.g. A, A0, B, B1 (default: A)
  -n, --num-steps <number>  Maximum number of steps (default: 1000)
  -w, --width <number>      Print width of tape output (default: 60)
)";
    const span args(argv, argc);
    turing_rule rule;
    state_type stateFilter = 0;
    symbol_type symbolFilter = -1;
    size_t numSteps = 1000;
    size_t width = 40;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-f") == 0 || strcmp(args[i], "--filter") == 0)
        {
            stateFilter = toupper(args[++i][0]) - 'A';
            if (strlen(args[i]) > 1)
                symbolFilter = toupper(args[i][1]) - '0';
        }
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = parseNumber(args[++i]);
        else if (strcmp(args[i], "-w") == 0 || strcmp(args[i], "--width") == 0)
            width = parseNumber(args[++i]);
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
    printTiming(run, rule, stateFilter, symbolFilter, numSteps, width);
}
