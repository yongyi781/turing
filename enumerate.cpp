#include "pch.hpp"

#include "detect_period.hpp"

using namespace std;
using namespace turing;

constexpr size_t defaultMaxSteps(int nStates, int nSymbols)
{
    if (nSymbols == 2)
    {
        // Busy beaver numbers
        if (nStates == 2)
            return 6;
        if (nStates == 3)
            return 21;
        if (nStates == 4)
            return 107;
    }
    return 2000;
}

constexpr pair<size_t, size_t> getCyclerBounds(int nStates, int nSymbols)
{
    if (nSymbols == 2)
    {
        if (nStates == 2)
            return pair{4, 8};
        if (nStates == 3)
            return pair{18, 36};
        if (nStates == 4)
            return pair{120, 240};
    }
    return pair{500, 1500};
}

constexpr pair<size_t, size_t> getTranslatedCyclerBounds(int nStates, int nSymbols)
{
    if (nSymbols == 2)
    {
        if (nStates == 2)
            return pair{9, 18};
        if (nStates == 3)
            return pair{92, 184};
        if (nStates == 4)
            return pair{1200, 2400};
    }
    return pair{10000, 30000};
}

auto growthProfile(TuringMachine m, size_t maxSteps)
{
    maxSteps += m.steps();
    vector<size_t> l{0};
    vector<size_t> r{0};
    while (!m.halted() && m.steps() < maxSteps)
    {
        if (m.step().tapeExpanded)
        {
            if (m.head() < 0)
                l.push_back(m.steps());
            else
                r.push_back(m.steps());
        }
    }
    return pair{l, r};
}

bool secondDiffsConstant(std::ranges::range auto &&r)
{
    if (r.size() <= 3)
        return false;
    for (auto it = r.begin() + 3; it < r.end(); ++it)
        if (*it - 3 * *(it - 1) + 3 * *(it - 2) - *(it - 3) != 0)
            return false;
    return true;
}

template <typename Callback> bool enumTMs(int nStates, int nSymbols, size_t maxSteps, Callback f)
{
    auto nextIsHalt = [](const TuringMachine &m) { return m.peek().nextState == -1; };
    auto isFilled = [](const TuringMachine &m) { return ranges::all_of(m.rule(), fun(t, t.nextState != -1)); };
    turing_rule r(nStates, nSymbols);
    r[0, 0] = {1, direction::right, 1};
    TuringMachine root{r};
    root.step();
    return it::tree_preorder(
        tuple{root, (symbol_type)1, (state_type)1},
        [&](auto &&t, auto f) {
            auto &&[m, hSymbol, hState] = t;
            // Invariant: m's next state should be a halt state.
            if (nextIsHalt(m))
            {
                for (symbol_type symbol = 0; symbol <= min(nSymbols - 1, hSymbol + 1); ++symbol)
                    for (auto dir : {direction::left, direction::right})
                        for (state_type state = 0; state <= min(nStates - 1, hState + 1); ++state)
                        {
                            auto r = m.rule();
                            r[m.state(), *m.tape()] = {symbol, dir, state};
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
        if (!nextIsHalt(m) &&
            (isFilled(m) || (m.steps() == maxSteps && hSymbol == nSymbols - 1 && hState == nStates - 1)))
            if (!it::callbackResult(f, m))
                return it::result_break;
        return it::result_continue;
    });
}

string lnfRuleStr(const TuringMachine &m) { return to_string(lexicalNormalForm(m.rule())); }

auto solve(int nStates, int nSymbols, size_t maxSteps, size_t simulationSteps)
{
    auto &&[pc, msc] = getCyclerBounds(nStates, nSymbols);
    auto &&[pt, mst] = getTranslatedCyclerBounds(nStates, nSymbols);
    size_t totalCyclers = 0;
    size_t totalTCyclers = 0;
    size_t totalCounters = 0;
    size_t totalBouncers = 0;
    size_t totalOther = 0;
    size_t total = 0;
    string directory = "out/";
    directory += to_string(nStates) + "x" + to_string(nSymbols) + "/";
    // ofstream foutc(directory + "cyclers.txt");
    ofstream foutc(directory + "counters.txt");
    ofstream foutt(directory + "tcyclers.txt");
    ofstream foutb(directory + "bouncers.txt");
    ofstream foutu(directory + "unclassified.txt");
    foutu << fixed << setprecision(6);
    enumTMs(nStates, nSymbols, maxSteps, [&](auto m) {
        ++total;
        if (total % 10'000 == 0)
            cout << "So far: " << total << " total | " << totalCyclers << " cyclers | " << totalTCyclers
                 << " tcyclers | " << totalBouncers << " bouncers | " << totalOther << " other\n";
        // Get the short translated cyclers out of the way first.
        // m.reset();
        auto res2 = TranslatedCyclerDetector{}.findPeriod(m, 1000, 500);
        if (res2.period > 0)
        {
            ++totalTCyclers;
            // if (res2.period >= 1000 || res2.preperiod >= 1000)
            // foutt << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << res2.period << '\t' << res2.preperiod <<
            // '\t'
            //       << res2.offset << '\n';
            return;
        }
        auto res = CyclerDetector{}.findPeriod(m, msc, pc);
        if (res.period > 0)
        {
            ++totalCyclers;
            // if (res.preperiod >= 100 || res.period >= 100)
            // foutc << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << res.period << '\t' <<
            // res.preperiod << '\n';
            return;
        }
        m.reset();
        res2 = TranslatedCyclerDetector{}.findPeriodAndPreperiod(m, mst, pt);
        if (res2.period > 0)
        {
            ++totalTCyclers;
            // if (res2.period >= 1000 || res2.preperiod >= 1000)
            foutt << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << res2.period << '\t' << res2.preperiod << '\t'
                  << res2.offset << '\n';
            return;
        }
        // Test bouncer
        // for (size_t i = 0; i < 10000; ++i)
        //     m.step();
        // auto &&[l, r] = bounceProfile(m, 10000);
        // if ((secondDiffsConstant(l)) || (secondDiffsConstant(r)))
        // {
        //     ++totalBouncers;
        //     foutb << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << res.period << '\t' <<
        //     res.preperiod << '\n'; return;
        // }
        if (simulationSteps > 0)
        {
            TuringMachine m2{m.rule()};
            size_t i = 0;
            for (; i < simulationSteps / 10; ++i)
                m2.step();
            auto size1 = m2.tape().size();
            for (; i < simulationSteps; ++i)
                m2.step();
            auto size2 = m2.tape().size();
            auto r = (double)size2 / size1;
            if (r < 1.6)
            {
                ++totalCounters;
                foutc << setw(9) << r << '\t' << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << size1 << '\t'
                      << size2 << '\n';
            }
            else if (abs(r - sqrt(10)) < 0.01)
            {
                ++totalBouncers;
                foutb << setw(9) << r << '\t' << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << size1 << '\t'
                      << size2 << '\n';
            }
            else
            {
                ++totalOther;
                foutu << setw(9) << r << '\t' << setw(8) << total << '\t' << lnfRuleStr(m) << '\t' << size1 << '\t'
                      << size2 << '\n';
            }
        }
        else
        {
            ++totalOther;
            foutu << setw(8) << total << '\t' << lnfRuleStr(m) << '\n';
        }
    });
    cout << "Total: " << total << " total | " << totalCyclers << " cyclers | " << totalTCyclers << " tcyclers | "
         << totalCounters << " counters | " << totalBouncers << " bouncers | " << totalOther << " other\n";
    return tuple{total, totalCyclers, totalTCyclers, totalBouncers, totalOther};
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    int nStates = 3;
    if (argc > 1)
        nStates = stoi(args[1]);
    int nSymbols = 2;
    if (argc > 2)
        nSymbols = stoi(args[2]);
    size_t maxSteps = defaultMaxSteps(nStates, nSymbols);
    if (argc > 3)
        maxSteps = stoull(args[3]);
    size_t simulationSteps = 10000;
    if (argc > 4)
        simulationSteps = stoull(args[4]);
    nStates = std::min(nStates, 5);
    nSymbols = std::min(nSymbols, 4);
    cout << "(# states, # symbols, max steps) = " << tuple{nStates, nSymbols, maxSteps} << '\n';
    printTiming(solve, nStates, nSymbols, maxSteps, simulationSteps);
}
