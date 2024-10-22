#include "../pch.hpp"

#include "tests_common.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

void testParseFormat()
{
    string code = "1RB0LA_0RB1LA";
    TuringMachine m{code};
    assertEqual(m.rule_str(), code);
    pass("testParseFormat");
}

void testBB5()
{
    auto m = known::bb5Champion();
    for (int i = 0; i < 47'176'869; ++i)
        m.step();
    assertEqual(m.halted(), false);
    m.step();
    assertEqual(m.halted(), true);
    assertEqual(m.head(), -12242);
    assertEqual(m.tape().size(), 12289);
    assertEqual(countOnes(m.tape()), 4098);
    pass("testBB5");
}

void testSimulation()
{
    auto m = known::boydJohnson();
    for (int i = 0; i < 10'000'000; ++i)
        m.step();
    assertEqual(m.tape().size(), 66726);
    pass("testSimulation");
}

void testTapeSegment()
{
    auto m = known::boydJohnson();
    for (int i = 0; i < 10'000; ++i)
        m.step();
    auto segment = getTapeSegment(m.tape(), m.state(), m.head() - 5, m.head() + 5);
    assertEqual(segment.data, vector<symbol_type>{1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1});
    assertEqual(segment.head, 5);
    assertEqual(segment.state, 2);
    segment = getTapeSegment(m.tape(), m.state(), m.head() + 5, m.head() + 10);
    assertEqual(segment.data, vector<symbol_type>{1, 1, 0, 1, 1, 0});
    assertEqual(segment.head, -5);
    assertEqual(segment.state, 2);
    segment = getTapeSegment(m.tape(), m.state(), m.head() - 550, m.head() - 500);
    assertEqual(segment.data, vector(51, (symbol_type)0));
    assertEqual(segment.head, 550);
    assertEqual(segment.state, 2);
    segment = getTapeSegment(m.tape(), m.state(), m.head() + 500, m.head() + 510);
    assertEqual(segment.data, vector(11, (symbol_type)0));
    assertEqual(segment.head, -500);
    assertEqual(segment.state, 2);
    pass("testTapeSegment");
}

int main()
{
    testParseFormat();
    testSimulation();
    testBB5();
    testTapeSegment();
    pass("=== All basic tests passed ===");
}
