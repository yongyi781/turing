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
        auto code = line;
        // auto v = it::split(line, '\t').to();
        // auto id = stoi(v[0]);
        // auto code = v[1];
        // int n1 = stoi(v[2]);
        // int n2 = stoi(v[3]);
        // int n3 = stoi(v[4]);
        // auto notes = v.size() < 7 ? "" : v[6];
        auto res = BouncerDecider{}.find(code, 2, 2e8, 20000, 20);
        // if (!res.found)
        //     fout2 << id << '\t' << code << '\t' << n1 << '\t' << n2 << '\t' << n3 << "\t\t" << notes << '\n';
        // else
        // {
        //     fout << id << '\t' << code << '\t' << res.start << '\t' << res.xPeriod << '\t' << n1 << '\t' << n2 <<
        //     '\t'
        //          << n3 << "\t\t" << notes << '\n';
        //     cout << id << '\t' << code << '\t' << res.start << '\t' << res.xPeriod << '\t' << n1 << '\t' << n2 <<
        //     '\t'
        //          << n3 << "\t\t" << notes << '\n';
        // }

        if (!res.found)
            fout2 << code << '\n';
        else
        {
            fout << code << '\t' << res.start << '\t' << res.xPeriod << '\n';
            cout << code << '\t' << res.start << '\t' << res.xPeriod << '\n';
        }
    });
}

int main() { printTiming(run); }
