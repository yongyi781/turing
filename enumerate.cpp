#include "pch.hpp"

#include "detect_period.hpp"

using namespace std;
using namespace turing;

template <typename Callback> bool enumTMs(int nStates, int nSymbols, size_t maxSteps, Callback f)
{
    auto nextIsHalt = [](const TuringMachine &m) { return m.peek().nextState == '\0'; };
    auto isFilled = [](const TuringMachine &m) { return ranges::all_of(m.rule(), fun(t, t.nextState != '\0')); };
    auto isPrimitive = [](const TuringMachine &m) {
        for (size_t i = 0; i < m.numStates(); ++i)
        {
            bool hasTransition = false;
            for (size_t j = 0; j < m.numColors(); ++j)
                if (m.rule()[i, j].nextState != '\0')
                    hasTransition = true;
            if (!hasTransition)
                return false;
        }
        return true;
    };
    turing_rule r(nStates, nSymbols);
    r[0, 0] = {1, direction::right, 'B'};
    TuringMachine root{r};
    root.step();
    return it::tree_preorder(
        tuple{root, (uint8_t)1, 'B'},
        [&](auto &&t, auto f) {
            auto &&[m, hSymbol, hState] = t;
            // Invariant: m's next state should be a halt state.
            if (nextIsHalt(m))
            {
                for (uint8_t symbol = 0; symbol <= min(nSymbols - 1, hSymbol + 1); ++symbol)
                    for (auto dir : {direction::left, direction::right})
                        for (char state = 'A'; state <= min('A' + nStates - 1, hState + 1); ++state)
                        {
                            auto r = m.rule();
                            r[m.state() - 'A', *m.tape()] = {symbol, dir, state};
                            TuringMachine m2{std::move(r), m.tape(), m.state(), m.steps()};
                            if (!isFilled(m2))
                                while (m2.steps() < maxSteps && !nextIsHalt(m2))
                                    m2.step();
                            if (!it::callbackResult(f, tuple{m2, max(hSymbol, symbol), max(hState, state)}))
                                return it::result_break;
                        }
            }
            return it::result_continue;
        },
        [&](auto &&t) {
            auto &&[m, hSymbol, hState] = t;
            return m.steps() < maxSteps && !isFilled(m);
        })([&](auto &&t) {
        auto &&[m, hSymbol, hState] = t;
        if (isFilled(m) || (m.steps() == maxSteps && isPrimitive(m)))
            if (!it::callbackResult(f, m))
                return it::result_break;
        return it::result_continue;
    });
}

auto solve(int nStates, int nSymbols, size_t maxSteps)
{
    size_t totalCyclers = 0;
    size_t totalTCyclers = 0;
    size_t totalOther = 0;
    size_t total = 0;
    string directory = "data/";
    directory += to_string(nStates) + "x" + to_string(nSymbols) + "/";
    ofstream foutc(directory + "cyclers.txt");
    ofstream fouto(directory + "others.txt");
    enumTMs(nStates, nSymbols, maxSteps, [&](auto &&m) {
        ++total;
        auto res = CyclerDetector{}.findPeriodAndPreperiod(m.rule(), 120, 240);
        if (res.period > 0)
        {
            ++totalCyclers;
            if (res.preperiod >= 100 || res.period >= 100)
                foutc << " cycler\t" << setw(8) << total << "\t" << m.rule_str() << "\t" << res.period << "\t"
                      << res.preperiod << '\n';
            return;
        }
        res = TranslatedCyclerDetector{}.findPeriodAndPreperiod(m.rule(), 10000, 20000);
        if (res.period > 0)
        {
            ++totalTCyclers;
            if (res.period >= 1000 || res.preperiod >= 1000)
                cout << "tcycler\t" << setw(8) << total << "\t" << m.rule_str() << "\t" << res.period << "\t"
                     << res.preperiod << "\t" << res.offset << '\n';
            return;
        }
        TuringMachine m2{m.rule()};
        size_t i = 0;
        for (; i < 100'000; ++i)
            m2.step();
        auto size1 = m2.tape().size();
        for (; i < 1'000'000; ++i)
            m2.step();
        auto size2 = m2.tape().size();
        fouto << setw(8) << total << "\t" << m.rule_str() << "\t" << size1 << "\t" << size2 << '\n';
        ++totalOther;
    });
    return tuple{totalCyclers, totalTCyclers, totalOther, total};
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    int nStates = 4;
    int nSymbols = 2;
    size_t maxSteps = 107;
    if (argc > 1)
        nStates = stoi(args[1]);
    if (argc > 2)
        nSymbols = stoi(args[2]);
    if (argc > 3)
        maxSteps = stoull(args[3]);
    nStates = std::min(nStates, 5);
    nSymbols = std::min(nSymbols, 4);
    // ios::sync_with_stdio(false);
    printTiming(solve, nStates, nSymbols, maxSteps);
}
