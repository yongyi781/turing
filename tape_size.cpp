#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

double interpolateTapeSize(TuringMachine m, size_t steps)
{
    size_t stepsBefore = 0;
    size_t stepsAfter = 0;
    for (size_t i = 0; i < steps; ++i)
        if (m.step().tapeExpanded)
            stepsBefore = m.steps();
    size_t tapeSize = m.tape().size();
    while (true)
    {
        if (m.step().tapeExpanded)
        {
            stepsAfter = m.steps();
            break;
        }
    }
    return (double)tapeSize + (double)(steps - stepsBefore) / (stepsAfter - stepsBefore);
}

auto solve(size_t steps)
{
    ofstream fout("out/out.txt");
    fout << fixed << setprecision(10);
    cout << fixed << setprecision(10);
    it::lines("data/in.txt")([&](auto &&code) {
        double res = interpolateTapeSize({code}, steps);
        cout << code << " | " << res << '\n';
        fout << res << '\n' << flush;
    });
}

int main(int argc, char *argv[])
{
    constexpr string_view help = R"(Calculate tape size at a given step number for a list of Turing machines

Usage: ./run tape_size <n>

Arguments:
  <n>  Step number.

Options:
  -h, --help  Show this help message

Comments:
  The input is a list of Turing machines in data/in.txt. This file will write
  the list of tape sizes to standard out as well as to out/out.txt.
)";
    span args(argv, argc);
    size_t steps = 0;
    for (int i = 1; i < argc; ++i)
    {
        if (strcmp(args[i], "-h") == 0 || strcmp(args[i], "--help") == 0)
        {
            cout << help;
            return 0;
        }
        if (steps == 0)
            steps = stoull(args[i]);
    }
    if (steps == 0)
    {
        cout << help;
        return 0;
    }
    printTiming(solve, steps);
}
