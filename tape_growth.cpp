// Utility to analyze a Turing machine based on tape growth.

#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

ostream &print(const TuringMachine &m)
{
    cout << setw(6) << m.tape().size() << " | " << setw(10) << m.steps();
    cout << " | " << m.prettyStr(40) << '\n';
    return cout;
}

string formatDelta(size_t a, size_t b)
{
    ostringstream ss;
    double r = (double)b / a;
    if (a == 0 || r < 1.1)
        ss << "+" << b - a;
    else
        ss << fixed << setprecision(10) << r;
    return std::move(ss).str();
}

auto solve(turing_rule rule, int growDir, state_type state, size_t numSteps, size_t diffLowerBound)
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
        if (growDir != 1 && m.head() < 0)
        {
            if (m.steps() - lSteps.back() >= diffLowerBound)
            {
                cout << ansi::fg(204, 220, 255) << "L | ";
                cout << setw(13) << formatDelta(lSteps.back(), m.steps()) << " | ";
                print(m) << ansi::reset;
            }
            lSteps.push_back(m.steps());
        }
        if (growDir != -1 && m.head() > 0)
        {
            if (m.steps() - rSteps.back() >= diffLowerBound)
            {
                cout << ansi::fg(255, 204, 204) << " R| ";
                cout << setw(13) << formatDelta(rSteps.back(), m.steps()) << " | ";
                print(m) << ansi::reset;
            }
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

Usage: ./run tape_growth <code>

Arguments:
  <code>   The Turing machine.

Options:
  -h, --help                  Show this help message
  -d, --dir <L|R>             Measure left or right (default: both)
  -s, --state <A|B|...>       Match state (default: all states)
  -n, --num-steps <number>    Maximum number of steps (default: 1000000)
  -l, --lower-bound <number>  Lower bound to report tape growth (default: 2)
)";
    span args(argv, argc);
    turing_rule rule;
    int growDir = 0;
    state_type matchState = -1;
    size_t numSteps = 1'000'000;
    size_t diffLowerBound = 2;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (strcmp(args[i], "-d") == 0 || strcmp(args[i], "--dir") == 0)
        {
            if (strcmp(args[++i], "L") == 0)
                growDir = -1;
            else if (strcmp(args[i], "R") == 0)
                growDir = 1;
        }
        else if (strcmp(args[i], "-s") == 0 || strcmp(args[i], "--state") == 0)
            matchState = toupper(args[++i][0]) - 'A';
        else if (strcmp(args[i], "-n") == 0 || strcmp(args[i], "--num-steps") == 0)
            numSteps = stoull(args[++i]);
        else if (strcmp(args[i], "-l") == 0 || strcmp(args[i], "--lower-bound") == 0)
            diffLowerBound = stoull(args[++i]);
        else if (rule.numStates() == 0)
        {
            rule = turing_rule(args[i]);
            if (rule.numStates() == 0)
            {
                cerr << ansi::red << "Invalid code: " << ansi::reset << args[i] << '\n' << help;
                return 0;
            }
        }
        else
        {
            cerr << ansi::red << "Unexpected argument: " << ansi::reset << args[i] << '\n' << help;
            return 0;
        }
    }
    if (rule.numStates() == 0)
    {
        cout << help;
        return 0;
    }
    printTiming(solve, rule, growDir, matchState, numSteps, diffLowerBound);
}
