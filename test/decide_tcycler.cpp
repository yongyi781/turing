#include "../pch.hpp"

#include "../decide/tcycler.hpp"
#include "tests_common.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

void p17620()
{
    auto res = TranslatedCyclerDecider{}.find(known::boydJohnson(), 10'000'000);
    assertEqual(res.period, 17620);
    assertEqual(res.preperiod, 158491);
    assertEqual(res.offset, 118);
    pass("p17620");
}

void p1s32779478()
{
    auto res = TranslatedCyclerDecider{}.find(known::bbb4Champion(), 50'000'000);
    assertEqual(res.period, 1);
    assertEqual(res.preperiod, 32779478);
    assertEqual(res.offset, -1);
    pass("p1s32779478");
}

void p2575984()
{
    auto res = TranslatedCyclerDecider{}.find({"1RB0LA_0RC1LA_1RD0RD_1LB1RB"}, 70000000);
    assertEqual(res.period, 2575984);
    assertEqual(res.preperiod, 24378294);
    assertEqual(res.offset, 1440);
    pass("p2575984");
}

void p7129704()
{
    auto res = TranslatedCyclerDecider{}.find({"1RB0LC_1RD1LC_0LA1LB_1LC0RD"}, 500000000);
    assertEqual(res.period, 7129704);
    assertEqual(res.preperiod, 309086174);
    assertEqual(res.offset, 512);
    pass("p7129704");
}

void p33209131()
{
    auto res = TranslatedCyclerDecider{}.find({"1RB0RA3LB1RB_2LA0LB1RA2RB"}, 80000000, 30000000);
    assertEqual(res.period, 33209131);
    assertEqual(res.preperiod, 63141841);
    assertEqual(res.offset, -39579);
    pass("p33209131");
}

void cyclerP2()
{
    auto res = CyclerDecider{}.find({"1RB---_1RC1RC_1LC1LB"}, 300);
    assertEqual(res.period, 2);
    assertEqual(res.preperiod, 3);
    assertEqual(res.offset, 0);
    pass("cyclerP2");
}

void cyclerP120()
{
    auto res = CyclerDecider{}.find({"1RB0RB_1LC0RD_1LA1LB_0LC1RD"}, 2000);
    assertEqual(res.period, 120);
    assertEqual(res.preperiod, 6);
    assertEqual(res.offset, 0);
    pass("cyclerP120");
}

int main()
{
    setConsoleToUtf8();
    p17620();
    p1s32779478();
    printTiming(p2575984);
    printTiming(p7129704);
    printTiming(p33209131);
    cyclerP2();
    pass("=== All decide_tcycler tests passed ===");
}
