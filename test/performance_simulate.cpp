#include "../pch.hpp"
#include "../turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

constexpr Int nSteps = 100'000'000;

auto solve()
{
    TuringMachine m{"1RB1LC_0LA1RD_1LA0LC_0RB0RD"};
    for (Int i = 0; i < nSteps; ++i)
        m.step();
    return m.tape().size();
}

// Manual compilation is about twice as fast.
auto solve2()
{
    Tape tape;
    size_t i = 0;
    while (true)
    {
        if (*tape == 0)
        {
            tape.step(1, direction::right);
            if (++i == nSteps)
                break;
            while (*tape != 0)
            {
                tape.step(1, direction::right);
                if (++i == nSteps)
                    break;
                while (*tape != 0)
                    tape.step(0, direction::right);
                if (++i == nSteps)
                    break;
                tape.step(0, direction::right);
                if (++i == nSteps)
                    break;
            }
            if (i == nSteps)
                break;
            tape.step(0, direction::left);
            if (++i == nSteps)
                break;
        }
        else
        {
            tape.step(1, direction::left);
            if (++i == nSteps)
                break;
            while (*tape != 0)
                tape.step(0, direction::left);
            if (++i == nSteps)
                break;
            tape.step(1, direction::left);
            if (++i == nSteps)
                break;
        }
    }
    return tape.size();
}
int main()
{
    printTiming(solve);
    printTiming(solve2);
}
