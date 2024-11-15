// Utility to analyze a Turing machine based on tape growth.

#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

ostream &print(const TuringMachine &m)
{
    cout << setw(6) << m.tape().size() << " | " << setw(10) << m.steps();
    cout << " | " << m.prettyStr(80) << '\n';
    return cout;
}

string formatDelta(size_t a, size_t b)
{
    ostringstream ss;
    const double r = (double)b / a;
    if (a == 0 || r < 1.1)
        ss << "+" << b - a;
    else
        ss << fixed << setprecision(10) << r;
    return std::move(ss).str();
}

auto run(turing_rule rule, int growDir, state_type state, size_t numSteps, bool allDeltas)
{
    TuringMachine m{rule};
    vector<size_t> lSteps{0};
    vector<size_t> rSteps{0};
    cout << fixed << setprecision(10);
    cout << "  | " << setw(13) << "initial tape" << " | ";
    print(m);
    while (!m.halted() && m.steps() < numSteps)
    {
        auto res = m.step();
        if (!res.tapeExpanded || (state != -1 && m.state() != state))
            continue;
        // Tape grew
        if (growDir != 1 && m.head() < 0 &&
            (allDeltas || lSteps.size() <= 1 || m.steps() - lSteps.back() > lSteps.back() - lSteps[lSteps.size() - 2]))
        {
            cout << ansi::fg(204, 220, 255) << "L | ";
            cout << setw(13) << formatDelta(lSteps.back(), m.steps()) << " | ";
            print(m) << ansi::reset;
            lSteps.push_back(m.steps());
        }
        if (growDir != -1 && m.head() > 0 &&
            (allDeltas || rSteps.size() <= 1 || m.steps() - rSteps.back() > rSteps.back() - rSteps[rSteps.size() - 2]))
        {
            cout << ansi::fg(255, 204, 204) << " R| ";
            cout << setw(13) << formatDelta(rSteps.back(), m.steps()) << " | ";
            print(m) << ansi::reset;
            rSteps.push_back(m.steps());
        }
    }
    if (growDir != 1)
    {
        cout << "Left growth sequence = ";
        println(lSteps, 15);
    }
    if (growDir != -1)
    {
        cout << "Right growth sequence = ";
        println(rSteps, 15);
    }
    if (growDir == 0 && state == -1)
    {
        ofstream fout("out/tape_growth.log", ios::app);
        fout << rule.str() << '\t' << m.steps() << '\t' << lSteps << '\t' << rSteps << '\n';
    }
    return pair{lSteps.size(), rSteps.size()};
}

int main(int argc, char *argv[])
{
    constexpr string_view help = R"(Tape growth tool

Usage: ./run tape_growth <TM>

Arguments:
  <TM>  The Turing machine.

Options:
  -h, --help                  Show this help message
  -a, --all                   Show all deltas, not just increasing deltas.
  -s, --side <L|R>            Measure left or right side (default: both)
  -s, --state <A|B|...>       Match state (default: all states)
  -n, --num-steps <number>    Maximum number of steps (default: 1000000)
)";
    const span args(argv, argc);
    turing_rule rule;
    int growDir = 0;
    state_type matchState = -1;
    size_t numSteps = 1'000'000;
    bool allDeltas = false;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-a") == 0 || strcmp(args[i], "--all") == 0)
            allDeltas = true;
        else if (strcmp(args[i], "-s") == 0 || strcmp(args[i], "--side") == 0)
        {
            if (strcmp(args[++i], "L") == 0)
                growDir = -1;
            else if (strcmp(args[i], "R") == 0)
                growDir = 1;
        }
        else if (strcmp(args[i], "-s") == 0 || strcmp(args[i], "--state") == 0)
            matchState = toupper(args[++i][0]) - 'A';
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = parseNumber(args[++i]);
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
    printTiming(run, rule, growDir, matchState, numSteps, allDeltas);
}
