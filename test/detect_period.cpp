#include "../pch.hpp"

#include "../detect_period.hpp"
#include "tests_common.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

void testTranslatedCycler()
{
    auto &&[p, pp, offset, _] =
        TranslatedCyclerDetector{}.findPeriodAndPreperiod(known::boydJohnson(), 1000000, 10'000'000);
    assertEqual(p, 17620);
    assertEqual(pp, 158491);
    assertEqual(offset, 118);
    pass("testTranslatedCycler");
}

void testTranslatedCycler2()
{
    auto &&[p, pp, offset, _] =
        TranslatedCyclerDetector{}.findPeriodAndPreperiod(known::bbb4Champion(), 10000, 50'000'000);
    assertEqual(p, 1);
    assertEqual(pp, 32779478);
    assertEqual(offset, -1);
    pass("testTranslatedCycler2");
}

void testTranslatedCycler3()
{
    auto &&[p, pp, offset, _] =
        TranslatedCyclerDetector{}.findPeriodAndPreperiod({"1RB0LA_0RC1LA_1RD0RD_1LB1RB"}, 3000000, 30000000);
    assertEqual(p, 2575984);
    assertEqual(pp, 24378294);
    assertEqual(offset, 1440);
    pass("testTranslatedCycler3");
}

void testTranslatedCycler4()
{
    auto &&[p, pp, offset, _] =
        TranslatedCyclerDetector{}.findPeriodAndPreperiod({"1RB0LC_1RD1LC_0LA1LB_1LC0RD"}, 10000000, 400000000);
    assertEqual(p, 7129704);
    assertEqual(pp, 309086174);
    assertEqual(offset, 512);
    pass("testTranslatedCycler4");
}

void testCycler()
{
    auto &&[p, pp, offset, _] = CyclerDetector{}.findPeriodAndPreperiod({"1RB---_1RC1RC_1LC1LB"}, 20, 300);
    assertEqual(p, 2);
    assertEqual(pp, 3);
    assertEqual(offset, 0);
    pass("testCycler");
}

void testCycler2()
{
    auto &&[p, pp, offset, _] = CyclerDetector{}.findPeriodAndPreperiod({"1RB0RB_1LC0RD_1LA1LB_0LC1RD"}, 500, 2000);
    assertEqual(p, 120);
    assertEqual(pp, 6);
    assertEqual(offset, 0);
    pass("testCycler2");
}

int main()
{
    setConsoleToUtf8();
    testTranslatedCycler();
    testTranslatedCycler2();
    testTranslatedCycler3();
    testTranslatedCycler4();
    testCycler();
    testCycler2();
    pass("=== All detect_period tests passed ===");
}
