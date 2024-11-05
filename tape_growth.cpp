// Utility to analyze a Turing machine based on tape growth.

#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

constexpr int diffLowerBound = 2;

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

auto solve(string_view code, int growDir, state_type matchState, size_t maxSteps)
{
    TuringMachine m{string{code}};
    size_t million = 0;
    vector<size_t> lSteps{0};
    vector<size_t> rSteps{0};
    cout << fixed << setprecision(10);
    cout << "  | " << setw(13) << "initial tape" << " | ";
    print(m);
    while (!m.halted() && m.steps() < maxSteps)
    {
        auto res = m.step();
        if (m.steps() == 1000000)
            million = m.tape().size();
        if (res.tapeExpanded && (matchState == -1 || m.state() == matchState))
        {
            // Tape grew
            if (growDir != 1 && m.head() < 0 && m.steps() - lSteps.back() >= diffLowerBound)
            {
                cout << ansi::fg(204, 220, 255) << "L | ";
                cout << setw(13) << formatDelta(lSteps.back(), m.steps()) << " | ";
                print(m) << ansi::reset;
                lSteps.push_back(m.steps());
            }
            if (growDir != -1 && m.head() > 0 && m.steps() - rSteps.back() >= diffLowerBound)
            {
                cout << ansi::fg(255, 204, 204) << " R| ";
                cout << setw(13) << formatDelta(rSteps.back(), m.steps()) << " | ";
                print(m) << ansi::reset;
                rSteps.push_back(m.steps());
            }
        }
    }
    if (million > 0)
        cout << "Tape size at 1000000 steps: " << million << '\n';
    cout << "Tape size at " << m.steps() << " steps: " << m.tape().size() << '\n';
    ofstream fout("data/tape_growth.txt", ios::app);
    fout << "\n# " << code << " | steps = " << m.steps() << '\n';
    if (lSteps.size() > 1)
    {
        fout << "left = ";
        println(lSteps, SIZE_MAX, fout);
    }
    if (rSteps.size() > 1)
    {
        fout << "right = ";
        println(rSteps, SIZE_MAX, fout);
    }
    fout << "list_plot_loglog(left, color=\"blue\", plotjoined=True) + list_plot_loglog(right, color=\"red\", "
            "plotjoined=True)\n";
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    string code = "1RB1LC_0LA1RD_1LA0LC_0RB0RD";
    int growDir = 0;
    state_type matchState = -1;
    size_t maxSteps = 100'000'000;
    if (argc > 1)
    {
        code = args[1];
        if (ranges::count(code, '_') == 0)
        {
            cout << "Usage: tape_growth <code> [growDir] [matchState] [maxSteps]\n";
            return 0;
        }
    }
    if (argc > 2)
    {
        if (strcmp(args[2], "R") == 0)
            growDir = 1;
        else if (strcmp(args[2], "L") == 0)
            growDir = -1;
    }
    if (argc > 3)
    {
        if (argc == 4 && strlen(args[3]) > 1)
            maxSteps = stoull(args[3]);
        else
            matchState = toupper(args[3][0]) - 'A';
    }
    if (argc > 4)
        maxSteps = stoull(args[4]);
    printTiming(solve, code, growDir, matchState, maxSteps);
}
