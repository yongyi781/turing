#include "../pch.hpp"

#include "../decide/bouncer.hpp"
#include "common.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

void bo65()
{
    auto res = BouncerDecider{}.find({"1RB0RC_1RC1LC_1LD1RA_0LB0LA"}, 2, 100000, 100);
    assertEqual(res.found, true);
    assertEqual(res.degree, 2);
    assertEqual(res.start, 65);
    assertEqual(res.xPeriod, 36);
    pass("bo65");
}

void bo145729()
{
    auto res = BouncerDecider{}.find({"1RB1LC_0RD0LC_1LB0LA_1LD1RA"}, 2, 1e7, 500, 6);
    assertEqual(res.found, true);
    assertEqual(res.degree, 2);
    assertEqual(res.start, 145729);
    assertEqual(res.xPeriod, 474);
    pass("bo145729");
}

void bo83158409()
{
    auto res = BouncerDecider{}.find({"1RB1LA_1LC0RC_0RA1LD_1RC0LD"}, 2, 1e8, 80, 3);
    assertEqual(res.found, true);
    assertEqual(res.degree, 2);
    assertEqual(res.start, 83158409);
    assertEqual(res.xPeriod, 66);
    pass("bo83158409");
}

void cu145()
{
    auto res = BouncerDecider{}.find({"1RB0LB_1RC1LB_0LD0RD_1LA1RD"}, 4, 1e8, 4, 6);
    assertEqual(res.found, true);
    assertEqual(res.degree, 3);
    assertEqual(res.start, 145);
    assertEqual(res.xPeriod, 2);
    pass("cu145");
}

int main()
{
    setConsoleToUtf8();
    bo65();
    printTiming(bo145729);
    printTiming(bo83158409);
    printTiming(cu145);
    pass("=== All decide_bouncer tests passed ===");
}
