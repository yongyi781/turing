#include "../pch.hpp"

#include "../test/tests_common.hpp"
#include "../turing.hpp"

using namespace std;
using namespace turing;

string transcript(turing_rule rule, size_t numSteps)
{
    TuringMachine m{rule};
    string s;
    s.reserve(numSteps * 3);
    while (m.steps() < numSteps)
    {
        if (m.steps() > 0)
            s += ' ';
        s += (char)('A' + m.state());
        s += (char)('0' + *m.tape());
        if (!m.step().success)
            break;
    }
    return s;
}

string repeat(string_view s, int n)
{
    string res;
    res.reserve(n * s.size());
    for (int i = 0; i < n; i++)
        res += s;
    return res;
}

string R(size_t n, size_t m)
{
    if (n <= 0)
        return "A0 B0" + repeat(" C1", m) + " C0";
    static boost::unordered_flat_map<pair<size_t, size_t>, string> cache;
    if (auto it = cache.find({n, m}); it != cache.end())
        return it->second;
    return cache[{n, m}] = R(n - 1, m) + repeat(" D0", pow(2LL, n - 1) + m + 1) + " D1" +
                           repeat(" A1", pow(2LL, n) - 1) + " " + R(n - 1, 0) + " D1 " + R(n - 1, m + pow(2LL, n - 1));
}

auto solve()
{
    turing_rule const rule{"1RB0LA_0RC1RB_0LD0RD_1RC1LA"};
    string const t = transcript(rule, 1e7);
    for (size_t n = 0;; ++n)
    {
        auto r = R(n, 0);
        if (r.size() > t.size())
            break;
        size_t const size = (r.size() + 1) / 3;
        assertEqual(r, t.substr(0, r.size()));
        pass(to_string(n) + " " + to_string(size));
    }
}

int main() { printTiming(solve); }
