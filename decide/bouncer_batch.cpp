#include "../pch.hpp"

#include "bouncer.hpp"

using namespace std;
using namespace turing;

auto run()
{
    ofstream fout("out/out.txt");
    ofstream fout2("out/out2.txt");
    fout << fixed << setprecision(10);
    cout << fixed << setprecision(10);
    it::lines("data/in.txt")([&](auto &&line) {
        // auto v = it::split(line, '\t').to();

        // auto id = stoi(v[0]);
        // auto code = v[1];
        // int n1 = stoi(v[2]);
        // int n2 = stoi(v[3]);
        const auto &code = line;
        auto res = BouncerDecider{}.find({code}, 2000, 4);
        if (!res.found)
            fout2 << code << '\n';
        else
        {
            fout << code << '\t' << res.startStep << '\t' << res.skip << '\n';
            cout << code << '\t' << res.startStep << '\t' << res.skip << '\n';
        }
        // if (res.period > 0)
        // {
        //     fout << id << '\t' << code << '\t' << n1 << '\t' << n2 << '\t' << res.period << '\t' << res.preperiod
        //          << '\t' << res.offset << '\n';
        //     cout << id << '\t' << code << '\t' << n1 << '\t' << n2 << '\t' << res.period << '\t' << res.preperiod
        //          << '\t' << res.offset << '\n';
        // }
        // else
        // {
        //     fout2 << id << '\t' << code << '\t' << n1 << '\t' << n2 << '\n';
        // }
    });
}

int main() { printTiming(run); }
