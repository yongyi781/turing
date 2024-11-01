#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;

auto solve()
{
    ofstream fout("out/out.txt");
    fout << fixed << setprecision(10);
    cout << fixed << setprecision(10);
    boost::unordered_flat_set<pair<int, int>> seen;
    it::lines("data/in.txt")([&](auto &&line) {
        auto v = it::split(line, '\t').to();

        auto id = stoi(v[1]);
        auto code = v[2];
        int n1 = stoi(v[3]);
        int n2 = stoi(v[4]);
        // auto notes = v.size() < 6 ? "" : v[5];
        if (seen.insert({n1, n2}).second)
        {
            TuringMachine m{code};
            fout << id << '\t' << code << '\t' << n1 << '\t' << n2 << '\n';
        }
    });
}

int main() { printTiming(solve); }
