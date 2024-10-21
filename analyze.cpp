#include "pch.hpp"
#include "turing.hpp"

using namespace std;

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

array<size_t, 4> hist{1};

void print(const turing::TuringMachine &m)
{
    cout << std::setw(5) << m.steps();
    cout << " | " << hist;
    cout << " | " << m.str(true, 80) << " | ";
    // cout << m.str1(true);
    cout << '\n';
}

auto run(turing::TuringMachine m, int pauseMs = 0, char stateFilter = '\0')
{
    print(m);
    while (!m.halted())
    {
        m.step();
        ++hist[m.state() - 'A'];
        if (stateFilter == '\0' || m.state() == stateFilter)
        {
            print(m);
            if (pauseMs > 0)
                this_thread::sleep_for(chrono::milliseconds(pauseMs));
        }
    }
    return m.steps();
}

ostream &printSegment(ostream &o, const vector<uint8_t> &v, int headPos, char state)
{
    if (headPos == -1)
        o << turing::getBgStyle(state) << ' ' << ansi::reset;
    for (size_t i = 0; i < v.size(); ++i)
        if (headPos == (int)i)
            o << turing::getBgStyle(state) << (char)('0' + v[i]) << ansi::reset;
        else
            o << (char)('0' + v[i]);
    if (headPos == (int)v.size())
        o << turing::getBgStyle(state) << ' ' << ansi::reset;
    return o;
}

vector<int> analyze(turing::TuringMachine machine, char stateToAnalyze, size_t steps = 10000,
                    size_t printWidth = turing::Tape::defaultPrintWidth)
{
    using T = tuple<vector<uint8_t>, int, vector<uint8_t>, int>;
    size_t width2 = 60;

    if (machine.state() == stateToAnalyze)
        cout << setw(7) << machine.steps() << " | " << machine.str(true, printWidth) << '\n';

    vector tapeData = machine.tape().data();
    int headPos = machine.tape().head();
    int offset = 0;
    int ld = 0;
    int hd = 0;

    boost::unordered_flat_map<T, int> tMap;
    vector<int> ts;
    int counter = 0;
    bool first = !(machine.state() == stateToAnalyze);

    for (size_t i = 0; i < steps; ++i)
    {
        auto &&[b, dir, s2] = machine.rule()[machine.state() - 'A', *machine.tape()];
        offset += dir == turing::direction::right ? 1 : -1;
        machine.step();
        if (machine.state() != 'Z' && machine.state() != stateToAnalyze)
        {
            ld = min(ld, offset);
            hd = max(hd, offset);
        }
        else
        {
            // Print the result
            ostringstream ss;
            tapeData.resize(machine.tape().data().size());
            if (headPos + ld < 0)
            {
                tapeData.insert(tapeData.begin(), -headPos - ld, 0);
                headPos = -ld;
            }
            auto fromRange = ranges::subrange(tapeData.begin() + headPos + ld, tapeData.begin() + headPos + hd + 1) |
                             ranges::to<vector>();
            int fromIndex = -ld;
            tapeData = machine.tape().data();
            headPos = machine.tape().head();
            auto toRange = ranges::subrange(tapeData.begin() + headPos - offset + ld,
                                            tapeData.begin() + headPos - offset + hd + 1) |
                           ranges::to<vector>();
            int toIndex = offset - ld;
            ss << setw(7) << machine.steps() << " | " << machine.str(true, printWidth) << " | ";
            ss << machine.str1(true, width2) << " | ";
            if (first)
            {
                cout << std::move(ss).str() << '\n';
                first = false;
            }
            else
            {
                tuple key{fromRange, fromIndex, toRange, toIndex};
                if (!tMap.contains(key))
                    tMap[key] = counter++;
                int mIndex = tMap[key];
                ts.push_back(mIndex + 1);
                ss << getFgStyle(mIndex) << setw(4) << "T" + to_string(mIndex + 1) << ansi::reset << " = [";
                printSegment(ss, fromRange, fromIndex, stateToAnalyze) << " → ";
                if (machine.state() == 'Z')
                    ss << 'Z';
                else
                    printSegment(ss, toRange, toIndex, stateToAnalyze);
                int x = toIndex - fromIndex;
                ss << "] (" << (x < 0 ? '-' : '+') << abs(x) << ")";
                // if (mIndex == 3)
                cout << std::move(ss).str() << '\n';
            }
            // if (machine.tape().head() == machine.tape().data().end() - 1)
            offset = ld = hd = 0;
            if (machine.state() == 'Z')
                break;
        }
    }
    return ts;
}

inline turing::TuringMachine universal23() { return {"1RB2LA1LA_2LA2RB0RA"}; }
inline turing::TuringMachine bb622() { return {"1RB0RF_1RC0LD_1LB1RC_---0LE_1RA1LE_---0RC"}; }

string formatRule(const turing::TuringMachine::rule_type &rule)
{
    string s;
    for (size_t i = 0; i < rule.rows(); ++i)
    {
        if (i != 0)
            s += "_";
        for (size_t j = 0; j < rule.columns(); ++j)
        {
            auto &&[symbol, dir, state] = rule[i, j];
            s += (char)('0' + symbol);
            s += dir == turing::direction::left ? 'L' : 'R';
            s += state;
        }
    }
    return s;
}

bool enumTuringRules(int numStates, int numSymbols, auto f)
{
    using R = turing::TuringMachine::rule_type;
    using V = R::value_type;
    vector<V> trs = views::cartesian_product(range((uint8_t)0, (uint8_t)(numSymbols - 1)),
                                             array{turing::direction::left, turing::direction::right},
                                             range('A', (char)('A' + numStates - 1))) |
                    ranges::to<vector>();
    return it::power(trs, numSymbols * numStates - 1)([&](auto &&t) {
        R rule(numStates, numSymbols);
        rule[0, 0] = {1, turing::direction::right, 'B'};
        for (size_t i = 0; i < t.size(); ++i)
            rule[(i + 1) / numSymbols, (i + 1) % numSymbols] = t[i];
        if (!it::callbackResult(f, rule))
            return it::result_break;
        return it::result_continue;
    });
}

bool allStatesReachable(turing::TuringMachine::rule_type rule)
{
    int nStates = rule.rows();
    int nSymbols = rule.columns();
    vector visited(nStates, false);
    vector<char> s{'A'};
    while (!s.empty())
    {
        auto state = s.back();
        s.pop_back();
        for (int symbol = 0; symbol < nSymbols; ++symbol)
        {
            auto &&[_, dir, next] = rule[state - 'A', symbol];
            if (next >= 'A' && next < 'A' + nStates && next != state && !visited[next - 'A'])
            {
                visited[next - 'A'] = true;
                s.push_back(next);
            }
        }
    }
    return all_of(visited.begin(), visited.end(), identity{});
}

auto solve()
{
    // string code;
    // code = "1RB1LF_1LB1LC_1RD0LE_---0RB_0RC0LA_1RC0RF";
    // cout << "Enter code: ";
    // cin >> code;
    // cout << "Turing machine = " << code << '\n';
    auto res = analyze(turing::known::boydJohnson(), 'A', 1000, 40);
    // auto res = findTranslatedCyclerPeriod({"1RB1LA_0LA0RA_1LA0RA"}, 1000, 50000);
    return res;
    // return it::wrap(res).map fun(x, (char)('0' + x)).to<string>();
    // run({code}, 0, 'A');
}

int main()
{
    // ios::sync_with_stdio(false);
    printTiming(solve);
}
