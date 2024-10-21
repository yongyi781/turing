#include "../pch.hpp"
#include "../turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

void pass(string_view message)
{
    cout << ansi::brightGreen << ansi::bold << "[PASS] " << ansi::reset << message << '\n';
}
void fail(string_view message) { cout << ansi::brightRed << ansi::bold << "[FAIL] " << ansi::reset << message << '\n'; }

string to_string(const vector<uint8_t> &v)
{
    string s;
    for (auto x : v)
        s += (char)('0' + x);
    return s;
}

template <typename T, typename U> void assertEqual(const T &actual, const U &expected)
{
    using Tp = common_type_t<T, U>;
    if (Tp(actual) != Tp(expected))
    {
        fail("Expected " + to_string(expected) + ", got " + to_string(actual));
        assert(false);
    }
}

size_t countOnes(const Tape &t) { return ranges::count(t.data(), 1); }

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
    assertEqual(segment.data, vector<uint8_t>{1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1});
    assertEqual(segment.head, 5);
    assertEqual(segment.state, 'C');
    segment = getTapeSegment(m.tape(), m.state(), m.head() + 5, m.head() + 10);
    assertEqual(segment.data, vector<uint8_t>{1, 1, 0, 1, 1, 0});
    assertEqual(segment.head, -5);
    assertEqual(segment.state, 'C');
    segment = getTapeSegment(m.tape(), m.state(), m.head() - 550, m.head() - 500);
    assertEqual(segment.data, vector(51, (uint8_t)0));
    assertEqual(segment.head, 550);
    assertEqual(segment.state, 'C');
    segment = getTapeSegment(m.tape(), m.state(), m.head() + 500, m.head() + 510);
    assertEqual(segment.data, vector(11, (uint8_t)0));
    assertEqual(segment.head, -500);
    assertEqual(segment.state, 'C');
    pass("testTapeSegment");
}

int main()
{
    testBB5();
    testSimulation();
    testTapeSegment();
}
