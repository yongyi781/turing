#include "pch.hpp"
#include "turing.hpp"

using namespace std;
using namespace turing;
using Int = int64_t;

struct S
{
    int a, b;
};

auto solve()
{
    S s1{2, 5};
    S s2{5, 5};
}

// sanitize
int main() { printTiming(solve); }
