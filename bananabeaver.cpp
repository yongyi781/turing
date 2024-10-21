#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using Int = int64_t;
using Z = ZMod<1'000'000'007LL>;

using namespace turing;

array<size_t, 4> hist{};
Int numBananas = 0;
Int pos = 0;

void print(const TuringMachine &m)
{
    cout << tuple{m.steps(), numBananas, pos};
    cout << " | " << hist[0];
    cout << " | " << m.str(true, 80) << "   ";
    // cout << m.str1(true);
    cout << '\n';
}

auto solve()
{
    auto m = turing::known::boydJohnson();
    numBananas = 0;
    pos = 0;
    vector<Int> bs;
    vector<Int> ps;
    while (!m.halted())
    {
        auto &&[b, dir, _] = m.rule()[m.state() - 'A', *m.tape()];
        numBananas += (int)b - (int)*m.tape();
        pos += dir == turing::direction::right ? 1 : -1;
        m.step();
        ++hist[m.state() - 'A'];
        if (hist[0] == 1000000)
            print(m);
        if (m.steps() >= 158491)
        {
            if (m.steps() == 158491 + 2 * 17620)
                break;
            if (m.state() == 'A')
            {
                bs.push_back(numBananas);
                ps.push_back(pos);
                // print(m);
            }
        }
    }

    constexpr Int base = 10;
    constexpr Int exponent = pow(10LL, 10);
    auto q1 = (Z(base).pow(exponent) - 37474);
    auto q2 = mod(powm(base, exponent, 4168) - 37474, 4168);
    auto q = Z(q1 - q2) / 4168;
    auto r = powm(base, exponent, 4168);
    // Int r = 45811 % 4168;
    // Int q = (45811 - 37474) / 4168;
    return q * 118 + ps[mod(r - 37474, 4168)];
    // return ps[4168] - ps[0];
}

auto solve2()
{
    constexpr Int N = 1e18;
    auto m = turing::known::boydJohnson();
    numBananas = 0;
    pos = 0;
    vector<Int> bs;
    vector<Int> ps;
    while (!m.halted())
    {
        auto &&[b, dir, _] = m.rule()[m.state() - 'A', *m.tape()];
        numBananas += (int)b - (int)*m.tape();
        pos += dir == turing::direction::right ? 1 : -1;
        m.step();
        ++hist[m.state() - 'A'];
        if (hist[0] == 1000000)
            print(m);
        if (m.steps() >= 158491)
        {
            if (m.state() == 'A')
            {
                bs.push_back(numBananas);
                ps.push_back(pos);
                // print(m);
            }
            if (m.steps() == 158491 + 17620)
                break;
        }
    }

    Int bIncr = bs.back() - bs.front();
    Int bMax = ranges::max(bs);
    Int pIncr = ps.back() - ps.front();

    // dbg(bIncr, bMax, pIncr, period);

    // Find number of periods n such that bMax + n * bIncr > N.
    // <=> n > (N - bMax) / bIncr
    // <=> n > floor((N - bMax) / bIncr)
    Int numPeriods = 1 + floorDiv(N - bMax, bIncr);
    // dbg(numPeriods);

    Int indexExceed = ranges::find_if(bs, fun(b, numPeriods * bIncr + b > N)) - bs.begin();

    return numPeriods * pIncr + ps[indexExceed - 1];
}

// A Project Euler submission.
// solve2 took 944.7 us.
int main()
{
    // ios::sync_with_stdio(false);
    printTiming(solve);
    printTiming(solve2);
}
