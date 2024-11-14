#include "pch.hpp"

#include "decide/bouncer.hpp"
#include "decide/tcycler.hpp"

using namespace std;
using namespace turing;

constexpr size_t defaultMaxSteps(int nStates, int nSymbols)
{
    // Busy beaver numbers
    if (nSymbols == 2)
    {
        if (nStates == 2)
            return 6;
        if (nStates == 3)
            return 21;
        if (nStates == 4)
            return 107;
    }
    if (nSymbols == 3)
    {
        if (nStates == 2)
            return 38;
    }
    return 2000;
}

constexpr pair<size_t, size_t> getCyclerBounds(int nStates, int nSymbols)
{
    if (nSymbols == 2)
    {
        if (nStates == 2)
            return pair{2, 8};
        if (nStates == 3)
            return pair{18, 40};
        if (nStates == 4)
            return pair{120, 360};
    }
    return pair{240, 10000};
}

constexpr pair<size_t, size_t> getTCBounds(int nStates, int nSymbols)
{
    if (nSymbols == 2)
    {
        if (nStates == 2)
            return pair{6, 18};
        if (nStates == 3)
            return pair{92, 200};
        if (nStates == 4)
            return pair{1000000, 2500000};
    }
    return pair{10000, 25000};
}

constexpr size_t interestingTCCutoff(int nStates, int nSymbols)
{
    if (nSymbols == 2)
    {
        if (nStates == 2)
            return 0;
        if (nStates == 3)
            return 23;
        if (nStates == 4)
            return 1000;
        if (nStates == 5)
            return 5000;
    }
    if (nSymbols == 3)
    {
        if (nStates == 2)
            return 0;
        if (nStates == 3)
            return 1000;
    }
    return 5000;
}

template <typename Callback> bool enumTMs(int nStates, int nSymbols, size_t maxSteps, Callback f)
{
    auto nextIsHalt = [](const TuringMachine &m) { return m.peek().toState == -1; };
    turing_rule r(nStates, nSymbols);
    r[0, 0] = {1, direction::right, 1};
    TuringMachine root{r};
    root.step();
    return it::tree_preorder(
        tuple{root, (symbol_type)1, (state_type)1},
        [&](auto &&t, auto rec) {
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
                            TuringMachine m2{std::move(r), m.tape(), m.steps()};
                            if (!m2.rule().filled())
                                while (m2.steps() < maxSteps && !nextIsHalt(m2))
                                    m2.step();
                            if (!it::callbackResult(rec,
                                                    tuple{std::move(m2), max(hSymbol, symbol), max(hState, state)}))
                                return it::result_break;
                        }
            }
            return it::result_continue;
        },
        [&](auto &&t) {
            auto &&[m, hSymbol, hState] = t;
            return m.steps() < maxSteps && !m.rule().filled();
        })([&](auto &&t) {
        auto &&[m, hSymbol, hState] = t;
        if (!nextIsHalt(m) &&
            (m.rule().filled() || (m.steps() == maxSteps && hSymbol == nSymbols - 1 && hState == nStates - 1)))
            if (!it::callbackResult(f, std::forward<decltype(m)>(m)))
                return it::result_break;
        return it::result_continue;
    });
}

struct enumerate_info
{
    ofstream fout;
    size_t count = 0;
};

size_t total = 0;
vector<string> names{"cyclers",       "tcyclers", "bouncers", "cubic bells", "quartic bells",
                     "quintic bells", "bells",    "counters", "unclassified"};
boost::unordered_flat_map<string, enumerate_info> enumData;
// vector stats(1000, 0UZ);

inline bool cycler(TuringMachine &m, size_t maxSteps, size_t startPeriodBound, size_t printCutoff)
{
    auto res = CyclerDecider{}.find(m, maxSteps, startPeriodBound);
    if (res.period > 0)
    {
        ++enumData["cyclers"].count;
        if (res.preperiod >= printCutoff || res.period >= printCutoff)
            enumData["cyclers"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                     << res.period << '\t' << res.preperiod << '\n';
        return true;
    }
    return false;
}

inline bool cyclerFast(TuringMachine &m, size_t maxSteps, size_t startPeriodBound)
{
    auto res = CyclerDecider{}.findPeriodOnly(m, maxSteps, startPeriodBound);
    if (res.period > 0)
    {
        ++enumData["cyclers"].count;
        return true;
    }
    return false;
}

inline bool tc(TuringMachine &m, size_t maxSteps, size_t startPeriodBound, size_t printCutoff)
{
    auto res = TranslatedCyclerDecider{}.find(m, maxSteps, startPeriodBound);
    if (res.period > 0)
    {
        ++enumData["tcyclers"].count;
        if (res.period >= printCutoff || res.preperiod >= printCutoff)
            enumData["tcyclers"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                      << res.period << '\t' << res.preperiod << '\t' << res.offset << '\n';
        return true;
    }
    return false;
}

inline bool tcFast(TuringMachine &m, size_t maxSteps, size_t startPeriodBound)
{
    auto res = TranslatedCyclerDecider{}.findPeriodOnly(m, maxSteps, startPeriodBound);
    if (res.period > 0)
    {
        ++enumData["tcyclers"].count;
        return true;
    }
    return false;
}

inline bool bouncer(TuringMachine &m, size_t degree, size_t maxSteps, size_t maxPeriod, size_t confidenceLevel,
                    auto &&printFilter)
{
    auto res = BouncerDecider{}.find(m, degree, maxSteps, maxPeriod, confidenceLevel);
    if (res.found)
    {
        if (res.degree == 1)
        {
            ++enumData["tcyclers"].count;
            enumData["tcyclers"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t' << "?"
                                      << '\t' << res.start << '\t' << res.xPeriod << '\n';
        }
        else if (res.degree == 2)
        {
            // Check for bell or fake bouncer
            auto res2 = BouncerDecider{}.find(m, 2, 4 * maxSteps, res.xPeriod, 3 * confidenceLevel);
            if (res2.start != res.start || res2.xPeriod != res.xPeriod)
            {
                ++enumData["bells"].count;
                enumData["bells"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\n';
            }
            else
            {
                ++enumData["bouncers"].count;
                if (printFilter(res))
                    enumData["bouncers"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                              << res.start << '\t' << res.xPeriod << '\n';
            }
        }
        else if (res.degree == 3)
        {
            ++enumData["cubic bells"].count;
            if (printFilter(res))
                enumData["cubic bells"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                             << res.start << '\t' << res.xPeriod << '\n';
        }
        else if (res.degree == 4)
        {
            ++enumData["quartic bells"].count;
            enumData["quartic bells"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                           << res.degree << '\t' << res.start << '\t' << res.xPeriod << '\n';
        }
        else
        {
            ++enumData["quintic bells"].count;
            enumData["quintic bells"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                           << res.degree << '\t' << res.start << '\t' << res.xPeriod << '\n';
        }
        return true;
    }
    return false;
}

inline bool counter(TuringMachine &m, size_t simulationSteps)
{
    if (simulationSteps > 0)
    {
        const size_t tapeSizeBound = 25 * log10(simulationSteps);
        for (size_t i = 0; i < simulationSteps; ++i)
        {
            m.step();
            if (m.tape().size() > tapeSizeBound)
                break;
        }
        if (m.tape().size() <= tapeSizeBound)
        {
            ++enumData["counters"].count;
            enumData["counters"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\t'
                                      << m.tape().size() << '\n';
            return true;
        }
    }
    return false;
}

void run(int nStates, int nSymbols, size_t maxSteps, size_t simulationSteps)
{
    auto &&[cyclerPBound, cyclerSBound] = getCyclerBounds(nStates, nSymbols);
    auto &&[tcPBound, tcSBound] = getTCBounds(nStates, nSymbols);
    size_t tcCutoff = interestingTCCutoff(nStates, nSymbols);
    string directory = "out/";
    directory += to_string(nStates) + "x" + to_string(nSymbols) + "/";
    for (auto &&name : names)
        enumData[name].fout.open(directory + name + ".txt");
    enumData["unclassified"].fout << fixed << setprecision(6);

    auto printCounts = [&]() {
        cout << total << " total";
        for (auto &&name : names)
            if (enumData[name].count > 0)
                cout << " | " << enumData[name].count << ' ' << name;
        cout << '\n';
    };

    enumTMs(nStates, nSymbols, maxSteps, [&](auto m) {
        m.reset();
        ++total;
        if (total % 10'000 == 0)
        {
            cout << ansi::dim << "  (so far) ";
            printCounts();
            cout << ansi::reset;
        }
        if (tcFast(m, 32, 16))
            return;
        if (cyclerFast(m, cyclerSBound, cyclerPBound))
            return;
        if (tc(m, 2048, 1024, tcCutoff))
            return;
        if (bouncer(m, 4, 25000, 3000, 6, [&](auto res) {
                if (res.degree == 2)
                    return nStates <= 3 || res.xPeriod >= 10 || res.start >= 1000;
                if (res.degree == 3)
                    return nStates <= 4 || res.xPeriod >= 10 || res.start >= 1000;
                return true;
            }))
            return;
        if (counter(m, simulationSteps))
            return;
        if (tc(m, tcSBound, tcPBound, tcCutoff))
            return;

        ++enumData["unclassified"].count;
        enumData["unclassified"].fout << setw(8) << total << '\t' << lexicalNormalForm(m.rule()).str() << '\n';
    });
    cout << "Final count: ";
    printCounts();
}

int main(int argc, char *argv[])
{
    constexpr string_view help =
        R"(Turing machine enumeration and analysis tool

Usage: ./run enumerate [n] [k]

Arguments:
  [n]  The number of states (default: 3)
  [k]  The number of symbols (default: 2)

Options:
  -h, --help       Show this help message
  -m, --max-steps  The maximum number of steps to determine halting (default:
                   BB(n, k) when it is known)
  -s, --sim-steps  The number of steps to simulate enumerated machines for, for
                   purposes of classification (default: 1000000)

Comments:
  This tool outputs to a file in the directory out/{n}x{k}. Please create this
  directory if it does not exist before running this tool, otherwise no file
  will be written.
)";
    const span args(argv, argc);
    int nStates = 3;
    int nSymbols = 2;
    size_t maxSteps = std::numeric_limits<size_t>::max();
    size_t simSteps = 100000;
    int argPos = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-m") == 0 || strcmp(args[i], "--max-steps") == 0)
            maxSteps = parseNumber(args[++i]);
        else if (strcmp(args[i], "-s") == 0 || strcmp(args[i], "--sim-steps") == 0)
            simSteps = parseNumber(args[++i]);
        else if (argPos == 0)
        {
            ++argPos;
            nStates = stoi(args[i]);
        }
        else if (argPos == 1)
        {
            ++argPos;
            nSymbols = stoi(args[i]);
        }
        else
        {
            cerr << ansi::red << "Unexpected argument: " << ansi::reset << args[i] << '\n' << help;
            return 0;
        }
    }
    nStates = std::min(nStates, 6);
    nSymbols = std::min(nSymbols, 4);
    if (maxSteps == std::numeric_limits<size_t>::max())
        maxSteps = defaultMaxSteps(nStates, nSymbols);
    cout << "(# states, # symbols, max steps) = " << tuple{nStates, nSymbols, maxSteps} << '\n';
    printTiming(run, nStates, nSymbols, maxSteps, simSteps);
}
