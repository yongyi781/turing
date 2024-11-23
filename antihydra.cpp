#include "pch.hpp"

#undef fun
#include <boost/multiprecision/gmp.hpp>

using namespace std;

auto run()
{
    mpz_int a = 8;
    int64_t b = 0;
    for (size_t i = 0; b != -1; ++i)
    {
        b += (int64_t)(2 - (a & 1) * 3);
        a += a >> 1;
        if (i % 100000 == 0)
            cout << i << " | log2(a) = " << msb(a) << " | b = " << b << '\n';
    }
}

int main()
{
    // ios::sync_with_stdio(false);
    printTiming(run);
}
