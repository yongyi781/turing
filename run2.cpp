#include "pch.hpp"

#include "turing.hpp"

using namespace std;
using Int = int64_t;

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
            cout << "I'm sorry, I didn't catch that. Type it again?\n";
            continue;
        }
        auto arr = it::split(std::move(s), ' ').to();
        char matchState = '\0';
        size_t maxSteps = 32779500;
        if (arr.size() >= 2)
            matchState = arr[1][0];
        if (arr.size() >= 3)
            maxSteps = stoull(arr[2]);
        TuringMachine m{arr[0]};
        // auto m = turing::known::boydJohnson();
        print(m);
        int64_t prev = 1;
        size_t million = 0;
        while (!m.halted() && m.steps() <= maxSteps)
        {
            m.step();
            ++hist[m.state() - 'A'];
            if (m.steps() == 1000000)
                million = m.tape().size();
            if (*m.tape() == 0 && (matchState == '\0' || m.state() == matchState))
            {
                cout << fixed << setprecision(15) << (double)m.steps() / prev << " | ";
                // cout << setw(8) << m.steps() - prev << " | ";
                print(m);
                prev = m.steps();
            }
        }
        cout << "Tape size at 1000000 steps: " << million << '\n';
    }
}

int main()
{
    // ios::sync_with_stdio(false);
    printTiming(solve);
}
