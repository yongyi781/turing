#include "../pch.hpp"
#include "../turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

constexpr int headerWidth = 30;

void bb5Champion(size_t nSteps = 47176870)
{
    auto m = known::bb5Champion();
    auto t1 = now();
    for (size_t i = 0; i < nSteps; ++i)
        if (!m.step().success)
            break;
    auto ns = duration_cast<std::chrono::nanoseconds>(now() - t1).count();
    cout << setw(headerWidth) << "BB5 champion: " << m.steps() << " steps, " << (double)ns / m.steps()
         << " ns per step\n";
}

void cycler483328(size_t nSteps = 100000000)
{
    TuringMachine m{"1RB1LC_0LA1RD_1LA0LC_0RB0RD"};
    auto t1 = now();
    for (size_t i = 0; i < nSteps; ++i)
        if (!m.step().success)
            break;
    auto ns = duration_cast<std::chrono::nanoseconds>(now() - t1).count();
    cout << setw(headerWidth) << "T-cycler p483328: " << m.steps() << " steps, " << (double)ns / m.steps()
         << " ns per step\n";
}

// Manual compilation is about twice as fast. `1RB1LC_0LA1RD_1LA0LC_0RB0RD`
void cycler483328Manual(size_t nSteps = 100000000)
{
    Tape tape;
    size_t steps = 0;
    auto t1 = now();
    while (true)
    {
        if (*tape == 0)
        {
            tape.step(1, direction::right);
            if (++steps == nSteps)
                break;
            while (*tape != 0)
            {
                tape.step(1, direction::right);
                if (++steps == nSteps)
                    break;
                while (*tape != 0)
                    tape.step(0, direction::right);
                if (++steps == nSteps)
                    break;
                tape.step(0, direction::right);
                if (++steps == nSteps)
                    break;
            }
            if (steps == nSteps)
                break;
            tape.step(0, direction::left);
            if (++steps == nSteps)
                break;
        }
        else
        {
            tape.step(1, direction::left);
            if (++steps == nSteps)
                break;
            while (*tape != 0)
                tape.step(0, direction::left);
            if (++steps == nSteps)
                break;
            tape.step(1, direction::left);
            if (++steps == nSteps)
                break;
        }
    }
    auto ns = duration_cast<std::chrono::nanoseconds>(now() - t1).count();
    cout << setw(headerWidth) << "T-cycler p483328 (manual): " << steps << " steps, " << (double)ns / steps
         << " ns per step\n";
}

void cycler32779478(size_t nSteps = 100000000)
{
    TuringMachine m{"1RB1LC_1RD1RB_0RD0RC_1LD1LA"};
    auto t1 = now();
    for (size_t i = 0; i < nSteps; ++i)
        if (!m.step().success)
            break;
    auto ns = duration_cast<std::chrono::nanoseconds>(now() - t1).count();
    cout << setw(headerWidth) << "T-cycler p1q32779478: " << m.steps() << " steps, " << (double)ns / m.steps()
         << " ns per step\n";
}

int main()
{
    cout << fixed << setprecision(2);
    bb5Champion();
    cycler483328();
    cycler483328Manual();
    cycler32779478();
}
