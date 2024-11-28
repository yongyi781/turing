#include "../pch.hpp"
#include "common.hpp"

using namespace std;
using namespace turing;

void lnf4x2Nop()
{
    const char *code = "1RB0RC_1LB0RD_1LC1LD_1RA1LD";
    const char *expectedCode = code;
    auto v = lexicalNormalForm(TuringMachine{code}.rule());
    assertEqual(v.str(), expectedCode);
    pass("lnf4x2Nop");
}

void lnf4x2Swap()
{
    const char *code = "1RB0RD_1LB0LC_1RA1LC_1LD1LC";
    const char *expectedCode = "1RB0RC_1LB0LD_1LC1LD_1RA1LD";
    auto v = lexicalNormalForm(TuringMachine{code}.rule());
    assertEqual(v.str(), expectedCode);
    pass("lnf4x2Swap");
}

void lnf4x2Swap2()
{
    const char *code = "1RB1LD_1RC0RA_0LA0RC_1LB0LB";
    const char *expectedCode = "1RB1LC_1RD0RA_1LB0LB_0LA0RD";
    auto v = lexicalNormalForm(TuringMachine{code}.rule());
    assertEqual(v.str(), expectedCode);
    pass("lnf4x2Swap2");
}

void lnf5x2Swap()
{
    const char *code = "1RB0LD_1LC1RC_1LA0RC_---0LE_0RB1LD";
    const char *expectedCode = "1RB0LC_1LD1RD_---0LE_1LA0RD_0RB1LC";
    auto v = lexicalNormalForm(TuringMachine{code}.rule());
    assertEqual(v.str(), expectedCode);
    pass("lnf5x2Swap");
}

void lnf5x2Rotate()
{
    const char *code = "1RB0LE_1LC1LA_1LD1LB_1RB---_0RE1RB";
    const char *expectedCode = "1RB0LC_1LD1LA_0RC1RB_1LE1LB_1RB---";
    auto v = lexicalNormalForm(TuringMachine{code}.rule());
    assertEqual(v.str(), expectedCode);
    pass("lnf5x2Rotate");
}

int main()
{
    lnf4x2Nop();
    lnf4x2Swap();
    lnf4x2Swap2();
    lnf5x2Swap();
    lnf5x2Rotate();
    pass("=== All lnf tests passed ===");
}
