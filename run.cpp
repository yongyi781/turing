#include "../pch.hpp"

#include "turing.hpp"

using namespace std;
using Int = int64_t;
using Z = ZMod<1'000'000'007LL>;

using namespace turing;

array<size_t, 3> hist{};

void print(const TuringMachine &m)
{
    cout << setw(10) << m.steps();
    // cout << " | " << setw(20) << hist;
    cout << " | " << m.str(true, 50) << "   ";
    // cout << m.str1(true);
    cout << '\n';
}

auto solve()
{
    while (true)
    {
        string s;
        cout << "Enter code: ";
        getline(cin, s);
        if (ranges::count(s, '_') < 2)
        {
            cout << ansi::red << "I'm sorry, I didn't catch that. Type it again?\n" << ansi::reset;
            continue;
        }
        auto arr = it::split(std::move(s), ' ').to();
        int growDir = 0;
        char matchState = '\0';
        size_t maxSteps = 250'000'000;
        if (arr.size() >= 2)
            growDir = arr[1] == "R" ? 1 : -1;
        if (arr.size() >= 3)
            matchState = arr[2][0];
        if (arr.size() >= 4)
            maxSteps = stoull(arr[3]);
        TuringMachine m{arr[0]};
        // auto m = turing::known::boydJohnson();
        print(m);
        int64_t prev = 1;
        size_t prevSize = m.tape().size();
        size_t million = 0;
        while (!m.halted() && m.steps() <= maxSteps)
        {
            m.step();
            ++hist[m.state() - 'A'];
            if (m.steps() == 1000000)
                million = m.tape().size();
            if (m.tape().size() != prevSize && (growDir != 1 || m.head() > 0) && (growDir != -1 || m.head() < 0) &&
                (matchState == '\0' || m.state() == matchState))
            {
                cout << fixed << setprecision(15) << (double)m.steps() / prev << " | ";
                // cout << setw(8) << m.steps() - prev << " | ";
                print(m);
                prev = m.steps();
            }
            prevSize = m.tape().size();
        }
        cout << "Tape size at 1000000 steps: " << million << '\n';
    }
}

int main()
{
    // ios::sync_with_stdio(false);
    printTiming(solve);
}
