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
        TuringMachine m{code};
        double res = interpolateTapeSize(m, steps);
        cout << code << " | " << res << '\n';
        fout << res << '\n' << flush;
    });
}

int main(int argc, char *argv[])
{
    span args(argv, argc);
    size_t steps = 1'000'000;
    if (argc > 1)
        steps = stoull(args[1]);
    cout << "Running each machine for " << steps << " steps\n";
    printTiming(solve, steps);
}
